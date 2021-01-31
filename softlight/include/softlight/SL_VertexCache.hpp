
#ifndef SL_VERTEX_CACHE_HPP
#define SL_VERTEX_CACHE_HPP

#include "softlight/SL_Config.hpp"
#include "softlight/SL_Shader.hpp" // SL_VertexParam
#include "softlight/SL_ShaderUtil.hpp" // SL_VERTEX_CACHE_SIZE



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

    SL_VertexParam& mParam;

    ls::math::vec4_t<float> (*mShader)(SL_VertexParam&);

    SL_TransformedVert mVertices[PTV_CACHE_SIZE];

  public:
    SL_PTVCache(const SL_PTVCache&)            = delete;
    SL_PTVCache(SL_PTVCache&&)                 = delete;
    SL_PTVCache& operator=(const SL_PTVCache&) = delete;
    SL_PTVCache& operator=(SL_PTVCache&&)      = delete;

    SL_PTVCache(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept;

    SL_TransformedVert* query_and_update(size_t key) noexcept;

    void query_and_update(size_t key, SL_TransformedVert& out) noexcept;
};



SL_PTVCache::SL_PTVCache(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept :
    //mIndices{},
    mParam{inParam},
    mShader{pShader}
    //mVertices{}
{
    for (size_t& index : mIndices)
    {
        index = PTV_CACHE_MISS;
    }
}



inline LS_INLINE SL_TransformedVert* SL_PTVCache::query_and_update(size_t key) noexcept
{
    const size_t i = key & (PTV_CACHE_SIZE-1);

    if (LS_LIKELY(mIndices[i] != key))
    {
        mIndices[i] = key;
        mParam.vertId = key;
        mParam.pVaryings = mVertices[i].varyings;
        mVertices[i].vert = mShader(mParam);
    }

    return mVertices+i;
}



inline LS_INLINE void SL_PTVCache::query_and_update(size_t key, SL_TransformedVert& out) noexcept
{
    const SL_TransformedVert* pIn = query_and_update(key);

    #if defined(LS_X86_SSE)
        _mm_store_ps(reinterpret_cast<float*>(&out)+0, _mm_load_ps(reinterpret_cast<const float*>(pIn)+0));
        _mm_store_ps(reinterpret_cast<float*>(&out)+4, _mm_load_ps(reinterpret_cast<const float*>(pIn)+4));
        _mm_store_ps(reinterpret_cast<float*>(&out)+8, _mm_load_ps(reinterpret_cast<const float*>(pIn)+8));
        _mm_store_ps(reinterpret_cast<float*>(&out)+12, _mm_load_ps(reinterpret_cast<const float*>(pIn)+12));
        _mm_store_ps(reinterpret_cast<float*>(&out)+16, _mm_load_ps(reinterpret_cast<const float*>(pIn)+16));

    #elif defined(LS_ARCH_ARM)
        vst1q_f32(reinterpret_cast<float*>(&out)+0,  vld1q_f32(reinterpret_cast<const float*>(pIn)+0));
        vst1q_f32(reinterpret_cast<float*>(&out)+4,  vld1q_f32(reinterpret_cast<const float*>(pIn)+4));
        vst1q_f32(reinterpret_cast<float*>(&out)+8,  vld1q_f32(reinterpret_cast<const float*>(pIn)+8));
        vst1q_f32(reinterpret_cast<float*>(&out)+12, vld1q_f32(reinterpret_cast<const float*>(pIn)+12));
        vst1q_f32(reinterpret_cast<float*>(&out)+16, vld1q_f32(reinterpret_cast<const float*>(pIn)+16));

    #else
        out = *pIn;

    #endif
}



#endif /* SL_VERTEX_CACHE_HPP */
