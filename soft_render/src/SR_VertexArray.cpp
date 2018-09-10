
#include <limits> // std::numeric_limits
#include <utility> // std::move()

#include "soft_render/SR_VertexArray.hpp"


constexpr uint32_t SR_INVALID_BUFFER_ID = std::numeric_limits<uint32_t>::max();



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray::~SR_VertexArray()
{
    terminate();
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray::SR_VertexArray() :
    mVboId{SR_INVALID_BUFFER_ID},
    mIboId{SR_INVALID_BUFFER_ID},
    mDimens{},
    mTypes{},
    mOffsets{},
    mStrides{}
{}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray::SR_VertexArray(const SR_VertexArray& v) :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mDimens{v.mDimens},
    mTypes{v.mTypes},
    mOffsets{v.mOffsets},
    mStrides{v.mStrides}
{}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray::SR_VertexArray(SR_VertexArray&& v) :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mDimens{std::move(v.mDimens)},
    mTypes{std::move(v.mTypes)},
    mOffsets{std::move(v.mOffsets)},
    mStrides{std::move(v.mStrides)}
{
    v.mVboId = SR_INVALID_BUFFER_ID;
    v.mIboId = SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray& SR_VertexArray::operator=(const SR_VertexArray& v)
{
    if (this != &v)
    {
        mVboId = v.mVboId;
        mIboId = v.mIboId;

        mDimens = v.mDimens;
        mTypes = v.mTypes;
        mOffsets = v.mOffsets;
        mStrides = v.mStrides;
    }

    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexArray& SR_VertexArray::operator=(SR_VertexArray&& v)
{
    if (this != &v)
    {
        mVboId = v.mVboId;
        v.mVboId = SR_INVALID_BUFFER_ID;

        mIboId = v.mIboId;
        v.mIboId = SR_INVALID_BUFFER_ID;

        mDimens = std::move(v.mDimens);
        mTypes = std::move(v.mTypes);
        mOffsets = std::move(v.mOffsets);
        mStrides = std::move(v.mStrides);
    }

    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
int SR_VertexArray::set_num_bindings(std::size_t numBindings)
{
    if (numBindings == num_bindings())
    {
        return 0;
    }

    int ret = 0;

    // increased the number of bindings
    if (numBindings > mDimens.size())
    {
        ret = (int)(numBindings - mDimens.size());
    }
    else if (numBindings < mDimens.size())
    {
        // decreased the number of bindings
        ret = (int)(mDimens.size() - numBindings);
        ret = -ret;
    }

    mDimens.resize(numBindings);
    mTypes.resize(numBindings);
    mOffsets.resize(numBindings);
    mStrides.resize(numBindings);

    return ret;
}



/*--------------------------------------
 *
--------------------------------------*/
std::size_t SR_VertexArray::num_bindings() const
{
    return mDimens.size();
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::set_binding(
    std::size_t bindId,
    ptrdiff_t offset,
    ptrdiff_t stride,
    SR_Dimension numDimens,
    SR_DataType vertType)
{
    mDimens[bindId]  = numDimens;
    mTypes[bindId]   = vertType;
    mOffsets[bindId] = offset;
    mStrides[bindId] = stride;
}



/*--------------------------------------
 *
--------------------------------------*/
ptrdiff_t SR_VertexArray::offset(std::size_t bindId) const
{
    return mOffsets[bindId];
}



/*--------------------------------------
 *
--------------------------------------*/
ptrdiff_t SR_VertexArray::offset(std::size_t bindId, std::size_t vertId) const
{
    return mOffsets[bindId] + (mStrides[bindId] * vertId);
}



/*--------------------------------------
 *
--------------------------------------*/
ptrdiff_t SR_VertexArray::stride(std::size_t bindId) const
{
    return mStrides[bindId];
}



/*--------------------------------------
 *
--------------------------------------*/
SR_DataType SR_VertexArray::type(std::size_t bindId) const
{
    return mTypes[bindId];
}



/*--------------------------------------
 *
--------------------------------------*/
SR_Dimension SR_VertexArray::dimensions(std::size_t bindId) const
{
    return mDimens[bindId];
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::remove_binding(std::size_t bindId)
{
    mDimens.erase(mDimens.begin() + bindId);
    mTypes.erase(mTypes.begin() + bindId);
    mOffsets.erase(mOffsets.begin() + bindId);
    mStrides.erase(mStrides.begin() + bindId);
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::set_vertex_buffer(uint32_t vboId)
{
    mVboId = vboId;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::remove_vertex_buffer()
{
    mVboId = SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 *
--------------------------------------*/
bool SR_VertexArray::has_vertex_buffer() const
{
    return mVboId != SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::set_index_buffer(uint32_t iboId)
{
    mIboId = iboId;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::remove_index_buffer()
{
    mIboId = SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 *
--------------------------------------*/
bool SR_VertexArray::has_index_buffer() const
{
    return mIboId != SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexArray::terminate()
{
    mVboId = SR_INVALID_BUFFER_ID;
    mIboId = SR_INVALID_BUFFER_ID;
    mDimens.clear();
    mTypes.clear();
    mOffsets.clear();
    mStrides.clear();
}
