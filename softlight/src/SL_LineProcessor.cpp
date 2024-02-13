
#include "lightsky/math/mat4.h"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_LineProcessor.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_PostVertexTransform.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"
#include "softlight/SL_ViewportState.hpp"

namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_LineProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_LineProcessor::push_bin(
    size_t primIndex,
    const SL_TransformedVert& a,
    const SL_TransformedVert& b) noexcept
{
    const uint_fast32_t numVaryings = (uint_fast32_t)mShader->pipelineState.num_varyings();

    const math::vec4& p0 = a.vert;
    const math::vec4& p1 = b.vert;

    // Check if the output bin is full
    uint_fast64_t binId;

    // Attempt to grab a bin index. Flush the bins if they've filled up.
    while ((binId = active_num_bins_used().count.fetch_add(1, std::memory_order_acq_rel)) >= SL_SHADER_MAX_BINNED_PRIMS)
    {
        flush_rasterizer<SL_LineRasterizer>();
    }

    // place a triangle into the next available bin
    SL_FragmentBin* const pFragBins = active_frag_bins();
    SL_FragmentBin& bin = pFragBins[binId];
    bin.mScreenCoords[0] = p0;
    bin.mScreenCoords[1] = p1;

    for (unsigned i = 0; i < numVaryings; ++i)
    {
        bin.mVaryings[i+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[i];
        bin.mVaryings[i+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[i];
    }

    bin.primIndex = primIndex;
    active_bin_index(binId).count = binId;
}



/*-------------------------------------
 * Ensure only visible lines get rendered. Lines should have already been
 * tested for visibility within clip-space. Now we need to clip the remaining
 * lines and generate new ones
-------------------------------------*/
void SL_LineProcessor::clip_and_process_lines(
    size_t primIndex,
    const ls::math::vec4& viewportDims,
    const SL_TransformedVert& a,
    const SL_TransformedVert& b
) noexcept
{
    const float dx = b.vert[0] - a.vert[0];
    const float dy = b.vert[1] - a.vert[1];
    const float xMin = viewportDims[0];
    const float yMin = viewportDims[1];
    const float xMax = viewportDims[2];
    const float yMax = viewportDims[3];

    if (math::abs(dx) < LS_EPSILON && math::abs(dy) < LS_EPSILON)
    {
        if (a.vert[0] >= xMin && a.vert[0] <= xMax && a.vert[1] >= yMin && a.vert[1] <= yMax)
        {
            push_bin(primIndex, a, b);
            return;
        }
    }

    const unsigned numVarys = (unsigned)mShader->pipelineState.num_varyings();

    const auto _interpolate_varyings = [&numVarys](const math::vec4* inVarys0, const math::vec4* inVarys1, math::vec4* outVarys, float amt) noexcept->void
    {
        for (unsigned i = numVarys; i--;)
        {
            *outVarys++ = math::mix(*inVarys0++, *inVarys1++, amt);
        }
    };

    bool (*const _clip_segment)(float, float, float&, float&) = [](float num, float denom, float& tE, float& tL) noexcept->bool
    {
        if (math::abs(denom) < LS_EPSILON)
            return num < 0.f;

        const float t = num / denom;

        if (denom > 0.f)
        {
            if (t > tL)
            {
                return false;
            }

            if (t > tE)
            {
                tE = t;
            }
        }
        else
        {
            if (t < tE)
            {
                return false;
            }

            if (t < tL)
            {
                tL = t;
            }
        }

        return true;
    };

    SL_TransformedVert p0 = a;
    SL_TransformedVert p1 = b;

    float& x1 = p0.vert[0];
    float& y1 = p0.vert[1];
    float& x2 = p1.vert[0];
    float& y2 = p1.vert[1];

    const float dist = math::inversesqrt(math::length_squared(math::vec2_cast(p0.vert)-math::vec2_cast(p1.vert)));
    float tE = 0.f;
    float tL = 1.f;

    if (_clip_segment(xMin-x1,  dx, tE, tL) &&
        _clip_segment(x1-xMax, -dx, tE, tL) &&
        _clip_segment(yMin-y1,  dy, tE, tL) &&
        _clip_segment(y1-yMax, -dy, tE, tL))
    {
        if (tL < 1.f)
        {
            x2 = x1 + tL * dx;
            y2 = y1 + tL * dy;
        }

        if (tE > 0.f)
        {
            x1 += tE * dx;
            y1 += tE * dy;
        }

        const float len0 = math::length(p0.vert - a.vert);
        const float len1 = math::length(p1.vert - b.vert);

        const float interp0 = len0 * dist;
        const float interp1 = len1 * dist;

        p0.vert[2] = math::mix(a.vert[2], b.vert[2], interp0);
        p0.vert[3] = math::mix(a.vert[3], b.vert[3], interp0);
        p1.vert[2] = math::mix(b.vert[2], a.vert[2], interp1);
        p1.vert[3] = math::mix(b.vert[3], a.vert[3], interp1);

        _interpolate_varyings(a.varyings, b.varyings, p0.varyings, interp0);
        _interpolate_varyings(b.varyings, a.varyings, p1.varyings, interp1);

        push_bin(primIndex, p0, p1);
    }
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
void SL_LineProcessor::process_verts(
    const SL_Mesh& m,
    size_t instanceId,
    const ls::math::mat4_t<float>& scissorMat,
    const ls::math::vec4_t<float>& viewportDims) noexcept
{
    if (active_frag_processors().count.load(std::memory_order_consume))
    {
        flush_rasterizer<SL_LineRasterizer>();
    }

    SL_TransformedVert     pVert0;
    SL_TransformedVert     pVert1;
    const auto             vertShader   = mShader->pVertShader;
    const SL_VertexArray&  vao          = mContext->vao(m.vaoId);
    const SL_IndexBuffer*  pIbo         = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;
    const bool             usingIndices = m.mode == RENDER_MODE_INDEXED_LINES;

    SL_VertexParam params;
    params.pUniforms  = mShader->pUniforms;
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

        SL_PTVCache ptvCache{};
        const auto&& vertTransform = [&](size_t key, SL_TransformedVert& tv)->void {
            params.vertId = key;
            params.pVaryings = tv.varyings;
            tv.vert = scissorMat * vertShader(params);
        };

    #else
        const size_t begin = m.elementBegin + mThreadId * 2u;
        const size_t end   = m.elementEnd;
        const size_t step  = mNumThreads * 2u;
    #endif

    for (size_t i = begin; i < end; i += step)
    {
        const size_t index0 = i;
        const size_t index1 = i + 1;

        #if SL_VERTEX_CACHING_ENABLED
            const size_t vertId0 = usingIndices ? pIbo->index(index0) : index0;
            const size_t vertId1 = usingIndices ? pIbo->index(index1) : index1;

            sl_cache_query_or_update(ptvCache, vertId0, pVert0, vertTransform);
            sl_cache_query_or_update(ptvCache, vertId1, pVert1, vertTransform);

        #else
            params.vertId    = usingIndices ? pIbo->index(index0) : index0;
            params.pVaryings = pVert0.varyings;
            pVert0.vert = scissorMat * vertShader(params);

            params.vertId = usingIndices ? pIbo->index(index1) : index1;
            params.pVaryings = pVert1.varyings;
            pVert1.vert = scissorMat * vertShader(params);
        #endif

        // Clip-space culling
        if (pVert0.vert[3] < 0.f || pVert1.vert[3] < 0.f)
        {
            continue;
        }

        const SL_ClipStatus visStatus = sl_ndc_clip_status(pVert0.vert, pVert1.vert);
        if (visStatus == SL_CLIP_STATUS_NOT_VISIBLE)
        {
            continue;
        }

        sl_perspective_divide(pVert0.vert, pVert1.vert);
        sl_ndc_to_screen_coords(pVert0.vert, pVert1.vert, viewportDims);

        if (visStatus == SL_CLIP_STATUS_FULLY_VISIBLE)
        {
            push_bin(i, pVert0, pVert1);
        }
        else if (visStatus == SL_CLIP_STATUS_PARTIALLY_VISIBLE)
        {
            clip_and_process_lines(i*instanceId+i, viewportDims, pVert0, pVert1);
        }
    }
}



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_LineProcessor::execute() noexcept
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

    this->cleanup<SL_LineRasterizer>();
}
