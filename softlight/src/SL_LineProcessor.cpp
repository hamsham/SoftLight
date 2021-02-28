
#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_LineProcessor.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;



namespace
{



/*--------------------------------------
 * Convert world coordinates to screen coordinates.
--------------------------------------*/
inline LS_INLINE void sl_world_to_screen_coords2(ls::math::vec4& v0, ls::math::vec4& v1, const float widthScale, const float heightScale) noexcept
{
    const float wInv0 = ls::math::rcp(v0.v[3]);
    const float wInv1 = ls::math::rcp(v1.v[3]);

    ls::math::vec4&& temp0 = v0 * wInv0;
    ls::math::vec4&& temp1 = v1 * wInv1;

    temp0[0] = ls::math::fmadd(temp0[0], widthScale, widthScale);
    temp0[1] = ls::math::fmadd(temp0[1], heightScale, heightScale);
    v0 = math::vec4{temp0[0], temp0[1], temp0[2], wInv0};

    temp1[0] = ls::math::fmadd(temp1[0], widthScale, widthScale);
    temp1[1] = ls::math::fmadd(temp1[1], heightScale, heightScale);
    v0 = math::vec4{temp1[0], temp1[1], temp1[2], wInv1};
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_LineProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_LineProcessor::push_bin(size_t primIndex, float fboW, float fboH, const SL_TransformedVert& a, const SL_TransformedVert& b) const noexcept
{
    SL_BinCounterAtomic<uint32_t>* const pLocks = mBinsUsed;
    SL_FragmentBin* const pFragBins = mFragBins;
    const uint_fast32_t numVaryings = mShader->get_num_varyings();

    const math::vec4& p0 = a.vert;
    const math::vec4& p1 = b.vert;

    // establish a bounding box to detect overlap with a thread's tiles
    const float bboxMinX = math::min(p0[0], p1[0]);
    const float bboxMinY = math::min(p0[1], p1[1]);
    const float bboxMaxX = math::max(p0[0], p1[0]);
    const float bboxMaxY = math::max(p0[1], p1[1]);

    int isPrimHidden = (bboxMaxX < 0.f || bboxMaxY < 0.f || fboW < bboxMinX || fboH < bboxMinY);
    isPrimHidden = isPrimHidden || (bboxMaxX-bboxMinX < 1.f) || (bboxMaxY-bboxMinY < 1.f);
    if (LS_UNLIKELY(isPrimHidden))
    {
        return;
    }

    // Check if the output bin is full
    uint_fast64_t binId;

    // Attempt to grab a bin index. Flush the bins if they've filled up.
    while ((binId = pLocks->count.fetch_add(1, std::memory_order_acq_rel)) >= SL_SHADER_MAX_BINNED_PRIMS)
    {
        flush_rasterizer<SL_LineRasterizer>();
    }

    // place a triangle into the next available bin
    SL_FragmentBin& bin = pFragBins[binId];
    bin.mScreenCoords[0] = p0;
    bin.mScreenCoords[1] = p1;

    for (unsigned i = 0; i < numVaryings; ++i)
    {
        bin.mVaryings[i+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[i];
        bin.mVaryings[i+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[i];
    }

    bin.primIndex = primIndex;
    mBinIds[binId].count = binId;
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
void SL_LineProcessor::process_verts(const SL_Mesh& m, size_t instanceId) noexcept
{
    if (mFragProcessors->count.load(std::memory_order_consume))
    {
        flush_rasterizer<SL_LineRasterizer>();
    }

    SL_TransformedVert      pVert0;
    SL_TransformedVert      pVert1;
    const SL_VertexShader   vertShader   = mShader->mVertShader;
    const auto              shader       = vertShader.shader;
    const SL_VertexArray&   vao          = mContext->vao(m.vaoId);
    const float             fboW         = (float)mFbo->width();
    const float             fboH         = (float)mFbo->height();
    const float             widthScale   = fboW * 0.5f;
    const float             heightScale  = fboH * 0.5f;
    const SL_IndexBuffer*   pIbo         = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;
    const bool              usingIndices = m.mode == RENDER_MODE_INDEXED_LINES;

    SL_VertexParam params;
    params.pUniforms  = mShader->mUniforms;
    params.instanceId = instanceId;
    params.pVao       = &vao;
    params.pVbo       = &mContext->vbo(vao.get_vertex_buffer());

    #if SL_VERTEX_CACHING_ENABLED
        size_t begin;
        size_t end;
        constexpr size_t step = 2;

        sl_calc_indexed_parition<2>(m.elementEnd-m.elementBegin, mNumThreads, mThreadId, begin, end);
        begin += m.elementBegin;
        end += m.elementBegin;

        SL_PTVCache ptvCache{shader, params};

    #else
        const size_t begin = m.elementBegin + mThreadId * 2u;
        const size_t end   = m.elementEnd;
        const size_t step  = mNumThreads * 2u;
    #endif

    for (size_t i = begin; i < end; i += step)
    {
        const size_t index0  = i;
        const size_t index1  = i + 1;

        #if SL_VERTEX_CACHING_ENABLED
            const size_t vertId0 = usingIndices ? get_next_vertex(pIbo, index0) : index0;
            const size_t vertId1 = usingIndices ? get_next_vertex(pIbo, index1) : index1;
            ptvCache.query_and_update(vertId0, pVert0);
            ptvCache.query_and_update(vertId1, pVert1);
        #else
            params.vertId    = usingIndices ? pIbo->index(index0) : index0;
            params.pVaryings = pVert0.varyings;
            pVert0.vert      = shader(params);

            params.vertId    = usingIndices ? pIbo->index(index1) : index1;
            params.pVaryings = pVert1.varyings;
            pVert1.vert      = shader(params);
        #endif

        if (pVert0.vert[3] >= 0.f && pVert1.vert[3] >= 0.f)
        {
            sl_world_to_screen_coords2(pVert0.vert, pVert1.vert, widthScale, heightScale);

            push_bin(i, fboW, fboH, pVert0, pVert1);
        }
    }
}



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_LineProcessor::execute() noexcept
{
    if (mNumInstances == 1)
    {
        for (size_t i = 0; i < mNumMeshes; ++i)
        {
            process_verts(mMeshes[i], 0);
        }
    }
    else
    {
        for (size_t i = 0; i < mNumInstances; ++i)
        {
            process_verts(mMeshes[0], i);
        }
    }

    this->cleanup<SL_LineRasterizer>();
}
