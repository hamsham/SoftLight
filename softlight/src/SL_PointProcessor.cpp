
#include "lightsky/math/mat4.h"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_PointProcessor.hpp"
#include "softlight/SL_PointRasterizer.hpp"
#include "softlight/SL_PostVertexTransform.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"
#include "softlight/SL_ViewportState.hpp"

namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_PointProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_PointProcessor::push_bin(size_t primIndex, const SL_TransformedVert& a) noexcept
{
    const uint_fast32_t numVaryings = (uint_fast32_t)mShader->pipelineState.num_varyings();

    // Check if the output bin is full
    uint_fast64_t binId;

    // Attempt to grab a bin index. Flush the bins if they've filled up.
    while (true)
    {
        binId = active_num_bins_used().count.fetch_add(1, std::memory_order_acq_rel);
        if (LS_UNLIKELY(binId < SL_SHADER_MAX_BINNED_PRIMS))
        {
            break;
        }

        flush_rasterizer<SL_PointRasterizer>();
    }

    // place a triangle into the next available bin
    SL_FragmentBin* const pFragBins = active_frag_bins();
    SL_FragmentBin& bin = pFragBins[binId];
    bin.mScreenCoords[0] = a.vert;

    for (unsigned i = 0; i < numVaryings; ++i)
    {
        bin.mVaryings[i] = a.varyings[i];
    }

    bin.primIndex = primIndex;
    active_bin_index(binId).count = binId;
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
void SL_PointProcessor::process_verts(
    const SL_Mesh& m,
    size_t instanceId,
    const ls::math::mat4_t<float>& scissorMat,
    const ls::math::vec4_t<float>& viewportDims) noexcept
{
    if (active_frag_processors().count.load(std::memory_order_consume))
    {
        flush_rasterizer<SL_PointRasterizer>();
    }

    SL_TransformedVert     pVert0;
    const auto             vertShader   = mShader->pVertShader;
    const SL_VertexArray&  vao          = mContext->vao(m.vaoId);
    const SL_IndexBuffer*  pIbo         = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;
    const bool             usingIndices = m.mode == RENDER_MODE_INDEXED_POINTS;

    SL_VertexParam params;
    params.pUniforms  = mShader->pUniforms;
    params.instanceId = instanceId;
    params.pVao       = &vao;
    params.pVbo       = &mContext->vbo(vao.get_vertex_buffer());

    size_t begin;
    size_t end;
    sl_calc_indexed_parition<1, true>(m.elementEnd-m.elementBegin, mNumThreads, mThreadId, begin, end);

    begin += m.elementBegin;
    end += m.elementBegin;

    SL_PTVCache ptvCache{};
    const auto&& vertTransform = [&](size_t key, SL_TransformedVert& tv)->void {
        params.vertId = key;
        params.pVaryings = tv.varyings;
        tv.vert = scissorMat * vertShader(params);
    };

    for (size_t i = begin; i < end; ++i)
    {
        const size_t vertId = usingIndices ? pIbo->index(i) : i;
        sl_cache_query_or_update(ptvCache, vertId, pVert0, vertTransform);

        const SL_ClipStatus visStatus = sl_ndc_clip_status(pVert0.vert);
        if (visStatus != SL_CLIP_STATUS_NOT_VISIBLE)
        {
            sl_perspective_divide(pVert0.vert);
            sl_ndc_to_screen_coords(pVert0.vert, viewportDims);

            push_bin(i, pVert0);
        }
    }
}



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_PointProcessor::execute() noexcept
{
    const math::vec4&&      fboDims      = (math::vec4)math::vec4_t<int>{0, 0, mFragFuncs->pDepthAttachment->width, mFragFuncs->pDepthAttachment->height};
    const SL_ViewportState& viewState    = mContext->viewport_state();
    const math::mat4&&      scissorMat   = viewState.scissor_matrix(fboDims[2], fboDims[3]);
    const math::vec4&&      viewportDims = viewState.viewport_rect(fboDims[2], fboDims[3]);

    if (mNumInstances == 1)
    {
        for (size_t i = 0; i < mNumMeshes; ++i)
        {
            process_verts(mMeshes[i], 0, scissorMat, viewportDims);
        }
    }
    else
    {
        for (size_t i = 0; i < mNumInstances; ++i)
        {
            process_verts(mMeshes[0], i, scissorMat, viewportDims);
        }
    }

    this->cleanup<SL_PointRasterizer>();
}
