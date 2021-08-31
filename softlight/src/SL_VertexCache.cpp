
#include "softlight/SL_VertexCache.hpp"



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_PTVCache::SL_PTVCache(const SL_PTVCache& ptv) noexcept :
    mParam{ptv.mParam},
    mShader{ptv.mShader}
{
    for (size_t i = 0; i < PTV_CACHE_SIZE; ++i)
    {
        mIndices[i] = ptv.mIndices[i];
        mVertices[i] = ptv.mVertices[i];
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_PTVCache::SL_PTVCache(SL_PTVCache&& ptv) noexcept :
    mParam{ptv.mParam},
    mShader{ptv.mShader}
{
    for (size_t i = 0; i < PTV_CACHE_SIZE; ++i)
    {
        mIndices[i] = ptv.mIndices[i];
        mVertices[i] = ptv.mVertices[i];
    }
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_PTVCache& SL_PTVCache::operator=(const SL_PTVCache& ptv) noexcept
{
    mParam = ptv.mParam;
    mShader = ptv.mShader;

    for (size_t i = 0; i < PTV_CACHE_SIZE; ++i)
    {
        mIndices[i] = ptv.mIndices[i];
        mVertices[i] = ptv.mVertices[i];
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_PTVCache& SL_PTVCache::operator=(SL_PTVCache&& ptv) noexcept
{
    mParam = ptv.mParam;
    mShader = ptv.mShader;

    for (size_t i = 0; i < PTV_CACHE_SIZE; ++i)
    {
        mIndices[i] = ptv.mIndices[i];
        mVertices[i] = ptv.mVertices[i];
    }

    return *this;
}



/*-------------------------------------
 * Reset and Re-initialize
-------------------------------------*/
void SL_PTVCache::reset(ls::math::vec4_t<float> (*pShader)(SL_VertexParam&), SL_VertexParam& inParam) noexcept
{
    mParam = &inParam;
    mShader = pShader;

    for (size_t& index : mIndices)
    {
        index = PTV_CACHE_MISS;
    }
}
