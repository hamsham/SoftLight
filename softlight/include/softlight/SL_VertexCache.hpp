
#ifndef SL_VERTEX_CACHE_HPP
#define SL_VERTEX_CACHE_HPP

#include "lightsky/utils/IndexedCache.hpp"
#include "lightsky/utils/LRUCache.hpp"
#include "lightsky/utils/LRU8WayCache.hpp"

#include "lightsky/math/mat4.h"

#include "softlight/SL_Config.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_VERTEX_CACHE_SIZE



static_assert(ls::math::is_pow2<size_t>(SL_VERTEX_CACHE_SIZE), "Vertex cache size must be a power of 2.");



/**
 * @brief Pre-Transform Vertex Cache
 *
 * This class helps to cache vertices immediately output from a shader.
 */
typedef ls::utils::IndexedCache<SL_TransformedVert, SL_VERTEX_CACHE_SIZE> SL_PTVCache;
//typedef ls::utils::LRUCache<SL_TransformedVert, SL_VERTEX_CACHE_SIZE> SL_PTVCache;
//typedef ls::utils::LRU8WayCache<SL_TransformedVert> SL_PTVCache;



template <typename UpdateFunc>
inline LS_INLINE void sl_cache_query_or_update(SL_PTVCache& cache, size_t key, SL_TransformedVert& out, const UpdateFunc& func) noexcept
{
    static_assert(SL_SHADER_MAX_VARYING_VECTORS == 4, "Please update the vertex varying count.");
    const SL_TransformedVert& cacheVal = cache.query_or_update(key, func);
    const SL_TransformedVert* pIn = &cacheVal;

    #if defined(LS_X86_SSE)
        out.vert.simd        = _mm_load_ps(reinterpret_cast<const float*>(pIn)+0);
        out.varyings[0].simd = _mm_load_ps(reinterpret_cast<const float*>(pIn)+4);
        out.varyings[1].simd = _mm_load_ps(reinterpret_cast<const float*>(pIn)+8);
        out.varyings[2].simd = _mm_load_ps(reinterpret_cast<const float*>(pIn)+12);
        out.varyings[3].simd = _mm_load_ps(reinterpret_cast<const float*>(pIn)+16);

    #elif defined(LS_ARM_NEON)
        out.vert.simd        = vld1q_f32(reinterpret_cast<const float*>(pIn)+0);
        out.varyings[0].simd = vld1q_f32(reinterpret_cast<const float*>(pIn)+4);
        out.varyings[1].simd = vld1q_f32(reinterpret_cast<const float*>(pIn)+8);
        out.varyings[2].simd = vld1q_f32(reinterpret_cast<const float*>(pIn)+12);
        out.varyings[3].simd = vld1q_f32(reinterpret_cast<const float*>(pIn)+16);

    #else
        out.vert        = pIn->vert;
        out.varyings[0] = pIn->varyings[0];
        out.varyings[1] = pIn->varyings[1];
        out.varyings[2] = pIn->varyings[2];
        out.varyings[3] = pIn->varyings[3];

    #endif
}



#endif /* SL_VERTEX_CACHE_HPP */
