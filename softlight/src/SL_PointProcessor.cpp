
#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_PointProcessor.hpp"
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
inline LS_INLINE void sl_world_to_screen_coords(ls::math::vec4& v, const float widthScale, const float heightScale) noexcept
{
    const float wInv = ls::math::rcp(v.v[3]);
    ls::math::vec4&& temp = v * wInv;

    temp[0] = ls::math::fmadd(temp[0], widthScale, widthScale);
    temp[1] = ls::math::fmadd(temp[1], heightScale, heightScale);

    v = math::vec4{temp[0], temp[1], temp[2], wInv};
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_PointProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_PointProcessor::push_bin(size_t primIndex, float fboW, float fboH, const SL_TransformedVert& a) const noexcept
{
    SL_BinCounterAtomic<uint32_t>* const pLocks = mBinsUsed;
    SL_FragmentBin* const pFragBins = mFragBins;
    const uint_fast32_t numVaryings = mShader->get_num_varyings();

    const math::vec4& p0 = a.vert;

    const float bboxMinX = p0[0];
    const float bboxMaxX = p0[0];
    const float bboxMinY = p0[1];
    const float bboxMaxY = p0[1];

    if (LS_UNLIKELY(bboxMaxX < 0.f || bboxMaxY < 0.f || fboW < bboxMinX || fboH < bboxMinY))
    {
        return;
    }

    // Check if the output bin is full
    uint_fast64_t binId;

    // Attempt to grab a bin index. Flush the bins if they've filled up.
    while ((binId = pLocks->count.fetch_add(1, std::memory_order_acq_rel)) >= SL_SHADER_MAX_BINNED_PRIMS)
    {
        flush_rasterizer();
    }

    // place a triangle into the next available bin
    SL_FragmentBin& bin = pFragBins[binId];
    bin.mScreenCoords[0] = p0;

    for (unsigned i = 0; i < numVaryings; ++i)
    {
        bin.mVaryings[i] = a.varyings[i];
    }

    bin.primIndex = primIndex;
    mBinIds[binId].count = binId;
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
void SL_PointProcessor::process_verts(const SL_Mesh& m, size_t instanceId) noexcept
{
    if (mFragProcessors->count.load(std::memory_order_consume))
    {
        flush_rasterizer();
    }

    SL_TransformedVert      pVert0;
    const SL_VertexShader   vertShader   = mShader->mVertShader;
    const auto              shader       = vertShader.shader;
    const SL_VertexArray&   vao          = mContext->vao(m.vaoId);
    const float             fboW         = (float)mFbo->width();
    const float             fboH         = (float)mFbo->height();
    const float             widthScale   = fboW * 0.5f;
    const float             heightScale  = fboH * 0.5f;
    const SL_IndexBuffer*   pIbo         = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;
    const bool              usingIndices = m.mode == RENDER_MODE_INDEXED_POINTS;

    SL_VertexParam params;
    params.pUniforms  = mShader->mUniforms;
    params.instanceId = instanceId;
    params.pVao       = &vao;
    params.pVbo       = &mContext->vbo(vao.get_vertex_buffer());

    #if SL_VERTEX_CACHING_ENABLED
        size_t begin;
        size_t end;
        constexpr size_t step = 1;

        sl_calc_indexed_parition<1>(m.elementEnd-m.elementBegin, mNumThreads, mThreadId, begin, end);
        begin += m.elementBegin;
        end += m.elementBegin;

        SL_PTVCache ptvCache{shader, params};

    #else
        const size_t begin = m.elementBegin + mThreadId;
        const size_t end   = m.elementEnd;
        const size_t step  = mNumThreads;
    #endif

    for (size_t i = begin; i < end; i += step)
    {
        #if SL_VERTEX_CACHING_ENABLED
            const size_t vertId = usingIndices ? pIbo->index(i) : i;
            ptvCache.query_and_update(vertId, pVert0);

        #else
            params.vertId    = usingIndices ? pIbo->index(i) : i;
            params.pVaryings = pVert0.varyings;
            pVert0.vert      = shader(params);
        #endif

        if (pVert0.vert[3] > 0.f)
        {
            sl_world_to_screen_coords(pVert0.vert, widthScale, heightScale);
            push_bin(i, fboW, fboH, pVert0);
        }
    }
}



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_PointProcessor::execute() noexcept
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

    this->cleanup();
}
