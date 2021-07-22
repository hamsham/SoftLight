
#include <utility> // std::move()

#include "softlight/SL_VertexArray.hpp"



/*--------------------------------------
 * Destructor
--------------------------------------*/
SL_VertexArray::~SL_VertexArray() noexcept
{
    terminate();
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_VertexArray::SL_VertexArray() noexcept :
    mVboId{SL_INVALID_BUFFER_ID},
    mIboId{SL_INVALID_BUFFER_ID},
    mBindings{nullptr},
    mNumBindings{0}
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_VertexArray::SL_VertexArray(const SL_VertexArray& v) noexcept :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mBindings{v.mBindings ? ls::utils::make_unique_aligned_array<SL_VertexArray::BindInfo>(v.num_bindings()) : nullptr},
    mNumBindings{v.num_bindings()}
{
    if (mBindings)
    {
        for (std::size_t i = 0; i < v.num_bindings(); ++i)
        {
            mBindings[i] = v.mBindings[i];
        }
    }
    else
    {
        mVboId = SL_INVALID_BUFFER_ID;
        mIboId = SL_INVALID_BUFFER_ID;
        mNumBindings = 0;
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_VertexArray::SL_VertexArray(SL_VertexArray&& v) noexcept :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mBindings{std::move(v.mBindings)},
    mNumBindings{std::move(v.mNumBindings)}
{
    v.mVboId = SL_INVALID_BUFFER_ID;
    v.mIboId = SL_INVALID_BUFFER_ID;
    v.mNumBindings = 0;
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SL_VertexArray& SL_VertexArray::operator=(const SL_VertexArray& v) noexcept
{
    if (this != &v)
    {
        mVboId = v.mVboId;
        mIboId = v.mIboId;

        mBindings = ls::utils::make_unique_aligned_array<SL_VertexArray::BindInfo>(v.num_bindings());
        if (mBindings)
        {
            for (std::size_t i = 0; i < v.num_bindings(); ++i)
            {
                mBindings[i] = v.mBindings[i];
            }

            mNumBindings = v.mNumBindings;
        }
        else
        {
            mVboId = SL_INVALID_BUFFER_ID;
            mIboId = SL_INVALID_BUFFER_ID;
            mNumBindings = 0;
        }
    }

    return *this;
}



/*--------------------------------------
 * Move Operator
--------------------------------------*/
SL_VertexArray& SL_VertexArray::operator=(SL_VertexArray&& v) noexcept
{
    if (this != &v)
    {
        mVboId = v.mVboId;
        v.mVboId = SL_INVALID_BUFFER_ID;

        mIboId = v.mIboId;
        v.mIboId = SL_INVALID_BUFFER_ID;

        mBindings = std::move(v.mBindings);
        mNumBindings = std::move(v.mNumBindings);
    }

    return *this;
}



/*--------------------------------------
 * Set the number of VBO bindings to monitor
--------------------------------------*/
int SL_VertexArray::set_num_bindings(std::size_t numBindings) noexcept
{
    if (numBindings == num_bindings())
    {
        return 0;
    }

    int ret = 0;

    // increased the number of bindings
    if (numBindings > mNumBindings)
    {
        ret = (int)(numBindings - mNumBindings);
    }
    else if (numBindings < mNumBindings)
    {
        // decreased the number of bindings
        ret = (int)(mNumBindings - numBindings);
        ret = -ret;
    }

    mBindings = ls::utils::make_unique_aligned_array<SL_VertexArray::BindInfo>(numBindings);
    mNumBindings = mBindings ? numBindings : 0;

    return ret;
}



/*--------------------------------------
 * Set the metadata of a VBO binding
--------------------------------------*/
void SL_VertexArray::set_binding(
    std::size_t bindId,
    ptrdiff_t offset,
    ptrdiff_t stride,
    SL_Dimension numDimens,
    SL_DataType vertType) noexcept
{
    SL_VertexArray::BindInfo& binding = mBindings[bindId];
    binding.dimens  = numDimens;
    binding.type   = vertType;
    binding.offset = offset;
    binding.stride = stride;
}



/*--------------------------------------
 * Remove a VBO binding
--------------------------------------*/
void SL_VertexArray::remove_binding(std::size_t bindId) noexcept
{
    if (!mNumBindings)
    {
        return;
    }
    if (bindId == mNumBindings-1)
    {
        mNumBindings -= 1;
    }
    else if (bindId == 0 && mNumBindings == 1)
    {
        mBindings.reset();
        mNumBindings = 0;
    }
    else
    {
        for (std::size_t i = bindId; i < mNumBindings-1; ++i)
        {
            mBindings[i] = mBindings[i+1];
        }

        mNumBindings -= 1;
    }
}



/*--------------------------------------
 * Clear all data assigned to *this.
--------------------------------------*/
void SL_VertexArray::terminate() noexcept
{
    mVboId = SL_INVALID_BUFFER_ID;
    mIboId = SL_INVALID_BUFFER_ID;
    mBindings.reset();
    mNumBindings = 0;
}
