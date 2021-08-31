
#ifndef SL_VERTEX_CACHE_HPP
#define SL_VERTEX_CACHE_HPP

#include "lightsky/math/mat4.h"

#include "softlight/SL_Config.hpp"
#include "softlight/SL_Shader.hpp" // SL_VertexParam
#include "softlight/SL_ShaderUtil.hpp" // SL_VERTEX_CACHE_SIZE



/**
 * @brief Pre-Transform Vertex Cache
 *
 * This class helps to cache vertices immediately output from a shader.
 */
class SL_PTVCache
{
  private:
    enum : size_t
    {
        PTV_CACHE_SIZE = SL_VERTEX_CACHE_SIZE,
        PTV_CACHE_MISS = ~(size_t)0,
    };

    static_assert(ls::math::is_pow2<size_t>(PTV_CACHE_SIZE), "Vertex cache size must be a power of 2.");

    size_t mIndices[PTV_CACHE_SIZE];

    SL_VertexParam* mParam;

    ls::math::vec4_t<float> (*mShader)(SL_VertexParam&);

    SL_TransformedVert mVertices[PTV_CACHE_SIZE];

  public:
    SL_PTVCache(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept;

    SL_PTVCache(const SL_PTVCache&) noexcept;

    SL_PTVCache(SL_PTVCache&&) noexcept;

    SL_PTVCache& operator=(const SL_PTVCache&) noexcept;

    SL_PTVCache& operator=(SL_PTVCache&&) noexcept;

    SL_TransformedVert* query_and_update(size_t key, const ls::math::mat4& scissorMat) noexcept;

    void query_and_update(size_t key, const ls::math::mat4& scissorMat, SL_TransformedVert& out) noexcept;

    void reset(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept;
};



/*-------------------------------------
 * Constructor
-------------------------------------*/
inline LS_INLINE SL_PTVCache::SL_PTVCache(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept
{
    reset(pShader, inParam);
}



/*-------------------------------------
 * Query the cache or insert a new element
-------------------------------------*/
inline LS_INLINE SL_TransformedVert* SL_PTVCache::query_and_update(size_t key, const ls::math::mat4& scissorMat) noexcept
{
    const size_t i = key & (PTV_CACHE_SIZE-1);

    if (LS_UNLIKELY(mIndices[i] != key))
    {
        mIndices[i] = key;
        mParam->vertId = key;
        mParam->pVaryings = mVertices[i].varyings;
        mVertices[i].vert = scissorMat * mShader(*mParam);
    }

    return mVertices+i;
}



/*-------------------------------------
 * Query the cache or insert a new element
-------------------------------------*/
inline LS_INLINE void SL_PTVCache::query_and_update(size_t key, const ls::math::mat4& scissorMat, SL_TransformedVert& out) noexcept
{
    static_assert(SL_SHADER_MAX_VARYING_VECTORS == 4, "Please update the vertex varying count.");

    const SL_TransformedVert* pIn = query_and_update(key, scissorMat);

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
