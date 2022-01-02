
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
    mVboId{SL_VertexArray::MAX_BINDINGS},
    mIboId{SL_VertexArray::MAX_BINDINGS},
    mNumBindings{0}
{
    for (SL_VertexArray::BindInfo& info : mBindings)
    {
        info.dimens = SL_Dimension::VERTEX_DIMENSION_1;
        info.type = SL_DataType::VERTEX_DATA_INVALID;
        info.offset = 0;
        info.stride = 0;
    }
}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_VertexArray::SL_VertexArray(const SL_VertexArray& v) noexcept :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mNumBindings{v.num_bindings()}
{
    for (std::size_t i = 0; i < v.num_bindings(); ++i)
    {
        mBindings[i].dimens = v.mBindings[i].dimens;
        mBindings[i].type = v.mBindings[i].type;
        mBindings[i].offset = v.mBindings[i].offset;
        mBindings[i].stride = v.mBindings[i].stride;
    }

    for (std::size_t i = v.num_bindings(); i < SL_VertexArray::MAX_BINDINGS; ++i)
    {
        mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_1;
        mBindings[i].type = SL_DataType::VERTEX_DATA_INVALID;
        mBindings[i].offset = 0;
        mBindings[i].stride = 0;
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_VertexArray::SL_VertexArray(SL_VertexArray&& v) noexcept :
    mVboId{v.mVboId},
    mIboId{v.mIboId},
    mNumBindings{v.num_bindings()}
{
    for (std::size_t i = 0; i < v.num_bindings(); ++i)
    {
        mBindings[i].dimens = v.mBindings[i].dimens;
        mBindings[i].type = v.mBindings[i].type;
        mBindings[i].offset = v.mBindings[i].offset;
        mBindings[i].stride = v.mBindings[i].stride;
    }

    for (std::size_t i = v.num_bindings(); i < SL_VertexArray::MAX_BINDINGS; ++i)
    {
        mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_1;
        mBindings[i].type = SL_DataType::VERTEX_DATA_INVALID;
        mBindings[i].offset = 0;
        mBindings[i].stride = 0;
    }

    v.terminate();
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
        mNumBindings = v.num_bindings();

        for (std::size_t i = 0; i < v.num_bindings(); ++i)
        {
            mBindings[i].dimens = v.mBindings[i].dimens;
            mBindings[i].type = v.mBindings[i].type;
            mBindings[i].offset = v.mBindings[i].offset;
            mBindings[i].stride = v.mBindings[i].stride;
        }

        for (std::size_t i = v.num_bindings(); i < SL_VertexArray::MAX_BINDINGS; ++i)
        {
            mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_1;
            mBindings[i].type = SL_DataType::VERTEX_DATA_INVALID;
            mBindings[i].offset = 0;
            mBindings[i].stride = 0;
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
        mIboId = v.mIboId;
        mNumBindings = v.num_bindings();

        for (std::size_t i = 0; i < v.num_bindings(); ++i)
        {
            mBindings[i].dimens = v.mBindings[i].dimens;
            mBindings[i].type = v.mBindings[i].type;
            mBindings[i].offset = v.mBindings[i].offset;
            mBindings[i].stride = v.mBindings[i].stride;
        }

        for (std::size_t i = v.num_bindings(); i < SL_VertexArray::MAX_BINDINGS; ++i)
        {
            mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_1;
            mBindings[i].type = SL_DataType::VERTEX_DATA_INVALID;
            mBindings[i].offset = 0;
            mBindings[i].stride = 0;
        }
    }

    return *this;
}



/*--------------------------------------
 * Set the number of VBO bindings to monitor
--------------------------------------*/
int SL_VertexArray::set_num_bindings(std::size_t numBindings) noexcept
{
    if (numBindings >= SL_VertexArray::MAX_BINDINGS)
    {
        return -1;
    }

    if (numBindings == num_bindings())
    {
        return (int)numBindings;
    }

    if (numBindings > mNumBindings) // increased the number of valid bindings
    {
        for (std::size_t i = mNumBindings; i < numBindings; ++i)
        {
            mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_4;
            mBindings[i].type = SL_DataType::VERTEX_DATA_FLOAT;
            mBindings[i].offset = 0;
            mBindings[i].stride = 0;
        }
    }
    else if (numBindings < mNumBindings) // decrease the number of bindings
    {
        for (std::size_t i = numBindings; i < mNumBindings; ++i)
        {
            mBindings[i].dimens = SL_Dimension::VERTEX_DIMENSION_1;
            mBindings[i].type = SL_DataType::VERTEX_DATA_INVALID;
            mBindings[i].offset = 0;
            mBindings[i].stride = 0;
        }
    }

    mNumBindings = numBindings;

    return (int)numBindings;
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
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);
    // LS_DEBUG_ASSERT(bindId < mNumBindings);

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
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);

    // Avoid redundancy
    // LS_DEBUG_ASSERT(mBindings[bindId].type != SL_DataType::VERTEX_DATA_INVALID);

    if (mNumBindings > 0)
    {
        // re-pack the bindings so no invalid elements exist between valid
        // elements
        for (std::size_t i = bindId; i < (mNumBindings-1); ++i)
        {
            SL_VertexArray::BindInfo& currBinding = mBindings[i];
            SL_VertexArray::BindInfo& nextBinding = mBindings[i+1];
            currBinding.dimens = nextBinding.dimens;
            currBinding.type = nextBinding.type;
            currBinding.offset = nextBinding.offset;
            currBinding.stride = nextBinding.stride;
        }

        SL_VertexArray::BindInfo& binding = mBindings[SL_VertexArray::MAX_BINDINGS-1];
        binding.dimens = SL_Dimension::VERTEX_DIMENSION_1;
        binding.type = SL_DataType::VERTEX_DATA_INVALID;
        binding.offset = 0;
        binding.stride = 0;

        mNumBindings -= 1;
    }
}



/*--------------------------------------
 * Clear all data assigned to *this.
--------------------------------------*/
void SL_VertexArray::terminate() noexcept
{
    if (mNumBindings > 0)
    {
        mVboId = SL_VertexArray::MAX_BINDINGS;
        mIboId = SL_VertexArray::MAX_BINDINGS;
        mNumBindings = 0;

        for (SL_VertexArray::BindInfo& info: mBindings)
        {
            info.dimens = SL_Dimension::VERTEX_DIMENSION_1;
            info.type = SL_DataType::VERTEX_DATA_INVALID;
            info.offset = 0;
            info.stride = 0;
        }
    }
}
