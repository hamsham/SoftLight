
#include "lightsky/math/mat4.h"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_LineProcessor.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"
#include "softlight/SL_ViewportState.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;



namespace
{



/*--------------------------------------
 * Convert world coordinates to screen coordinates
--------------------------------------*/
inline LS_INLINE void sl_perspective_divide2(math::vec4& LS_RESTRICT_PTR v0, math::vec4& LS_RESTRICT_PTR v1) noexcept
{
    #if defined(LS_X86_SSE4_1)
        const __m128 wInv0 = _mm_rcp_ps(_mm_shuffle_ps(v0.simd, v0.simd, 0xFF));
        const __m128 wInv1 = _mm_rcp_ps(_mm_shuffle_ps(v1.simd, v1.simd, 0xFF));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);

        v0.simd = _mm_blend_ps(wInv0, vMul0, 0x07);
        v1.simd = _mm_blend_ps(wInv1, vMul1, 0x07);

    #elif defined(LS_X86_SSSE3)
        const __m128 wInv0 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v0.simd), 0xFF)));
        const __m128 wInv1 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v1.simd), 0xFF)));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);

        const __m128i wi0  = _mm_shuffle_epi8(_mm_castps_si128(wInv0), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
        const __m128i wi1  = _mm_shuffle_epi8(_mm_castps_si128(wInv1), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

        const __m128i vi0  = _mm_shuffle_epi8(_mm_castps_si128(vMul0), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
        const __m128i vi1  = _mm_shuffle_epi8(_mm_castps_si128(vMul1), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

        v0.simd = _mm_or_ps(_mm_castsi128_ps(vi0), _mm_castsi128_ps(wi0));
        v1.simd = _mm_or_ps(_mm_castsi128_ps(vi1), _mm_castsi128_ps(wi1));

    #elif defined(LS_ARCH_AARCH64)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vdivq_f32(vdupq_n_f32(1.f), w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, wInv0);
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vdivq_f32(vdupq_n_f32(1.f), w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, wInv1);
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

    #elif defined(LS_ARM_NEON)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vrecpeq_f32(w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, vmulq_f32(vrecpsq_f32(w0, wInv0), wInv0));
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vrecpeq_f32(w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, vmulq_f32(vrecpsq_f32(w1, wInv1), wInv1));
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

    #else
        const math::vec4&& wInv0 = math::rcp(v0[3]);
        const math::vec4&& wInv1 = math::rcp(v1[3]);

        const math::vec4&& vMul0 = v0 * wInv0;
        const math::vec4&& vMul1 = v1 * wInv1;

        v0 = {vMul0[0], vMul0[1], vMul0[2], wInv0[0]};
        v1 = {vMul1[0], vMul1[1], vMul1[2], wInv1[0]};

    #endif
}



/*--------------------------------------
 * Convert world coordinates to screen coordinates
--------------------------------------*/
inline LS_INLINE void sl_world_to_screen_coords_divided2(
    math::vec4& LS_RESTRICT_PTR p0,
    math::vec4& LS_RESTRICT_PTR p1,
    const math::vec4& viewportDims
) noexcept
{
    #if defined(LS_X86_FMA)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_add_ps(one, p0.simd);
        v0 = _mm_floor_ps(_mm_fmadd_ps(v0, wh, dims));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

        __m128 v1 = _mm_add_ps(one, p1.simd);
        v1 = _mm_floor_ps(_mm_fmadd_ps(v1, wh, dims));
        p1.simd = _mm_shuffle_ps(_mm_max_ps(v1, _mm_setzero_ps()), p1.simd, 0xE4);

    #elif defined(LS_X86_SSE)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_mul_ps(_mm_add_ps(one, p0.simd), wh);
        v0 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v0, dims)));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

        __m128 v1 = _mm_mul_ps(_mm_add_ps(one, p1.simd), wh);
        v1 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v1, dims)));
        p1.simd = _mm_shuffle_ps(_mm_max_ps(v1, _mm_setzero_ps()), p1.simd, 0xE4);

    #elif defined(LS_ARCH_AARCH64)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vrndm_f32(vmla_f32(offset, wh, v0));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

        float32x2_t v1 = vadd_f32(vld1_f32(&p1), one);
        v1 = vrndm_f32(vmla_f32(offset, wh, v1));
        vst1_f32(&p1, vmax_f32(v1, vdup_n_f32(0.f)));

    #elif defined(LS_ARM_NEON)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v0)));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

        float32x2_t v1 = vadd_f32(vld1_f32(&p1), one);
        v1 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v1)));
        vst1_f32(&p1, vmax_f32(v1, vdup_n_f32(0.f)));

    #else
        // NDC->screen space transform is clamped to the framebuffer bounds.
        // For example:
        // x = math::clamp(x, 0.f, fboW);
        // w = min(w, fboW-x);
        const float w = viewportDims[2] * 0.5f;
        const float h = viewportDims[3] * 0.5f;

        p0[0] = math::max(math::floor(math::fmadd(p0[0]+1.f, w, viewportDims[0])), 0.f);
        p0[1] = math::max(math::floor(math::fmadd(p0[1]+1.f, h, viewportDims[1])), 0.f);

        p1[0] = math::max(math::floor(math::fmadd(p1[0]+1.f, w, viewportDims[0])), 0.f);
        p1[1] = math::max(math::floor(math::fmadd(p1[1]+1.f, h, viewportDims[1])), 0.f);

    #endif
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_LineProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_LineProcessor::push_bin(size_t primIndex, const math::vec4& viewportDims, const SL_TransformedVert& a, const SL_TransformedVert& b) const noexcept
{
    SL_BinCounterAtomic<uint32_t>* const pLocks = mBinsUsed;
    SL_FragmentBin* const pFragBins = mFragBins;
    const uint_fast32_t numVaryings = (uint_fast32_t)mShader->pipelineState.num_varyings();

    const math::vec4& p0 = a.vert;
    const math::vec4& p1 = b.vert;

    // establish a bounding box to detect overlap with a thread's tiles
    const float bboxMinX = math::min(p0[0], p1[0]);
    const float bboxMinY = math::min(p0[1], p1[1]);
    const float bboxMaxX = math::max(p0[0], p1[0]);
    const float bboxMaxY = math::max(p0[1], p1[1]);

    const float fboW = viewportDims[0]+viewportDims[2];
    const float fboH = viewportDims[1]+viewportDims[3];

    int isPrimHidden = (bboxMaxX < viewportDims[0] || bboxMaxY < viewportDims[1] || fboW < bboxMinX || fboH < bboxMinY);
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
void SL_LineProcessor::process_verts(
    const SL_Mesh& m,
    size_t instanceId,
    const ls::math::mat4_t<float>& scissorMat,
    const ls::math::vec4_t<float>& viewportDims) noexcept
{
    if (mFragProcessors->count.load(std::memory_order_consume))
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

        if (pVert0.vert[3] >= 0.f && pVert1.vert[3] >= 0.f)
        {
            sl_perspective_divide2(pVert0.vert, pVert1.vert);
            sl_world_to_screen_coords_divided2(pVert0.vert, pVert1.vert, viewportDims);

            push_bin(i, viewportDims, pVert0, pVert1);
        }
    }
}



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_LineProcessor::execute() noexcept
{
    mAmDone = 0;

    const math::vec4&&      fboDims      = (math::vec4)math::vec4_t<int>{0, 0, mFbo->width(), mFbo->height()};
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

    mAmDone = 1;

    this->cleanup<SL_LineRasterizer>();
}
