
#include <utility> // std::move()

#include "lightsky/utils/Assertions.h"

#include "softlight/SL_IndexBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helpers
-----------------------------------------------------------------------------*/
namespace
{


enum
{
    _SL_IBO_PADDING_BYTES = sizeof(unsigned) * 4
};



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_IndexBuffer Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Destructor
-------------------------------------*/
SL_IndexBuffer::~SL_IndexBuffer() noexcept
{
    terminate();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_IndexBuffer::SL_IndexBuffer() noexcept :
    mType{SL_DataType::VERTEX_DATA_INT},
    mBytesPerId{sl_bytes_per_vertex(SL_DataType::VERTEX_DATA_INT, SL_Dimension::VERTEX_DIMENSION_1)},
    mCount{0},
    mBuffer{nullptr}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_IndexBuffer::SL_IndexBuffer(const SL_IndexBuffer& v) noexcept :
    mType{v.mType},
    mBytesPerId{v.mBytesPerId},
    mCount{v.mCount},
    mBuffer{nullptr}
{
    if (v.mBuffer != nullptr)
    {
        const uint32_t numBytes = v.mBytesPerId * v.mCount;
        mBuffer = ls::utils::make_unique_aligned_array<unsigned char>(numBytes + (_SL_IBO_PADDING_BYTES - (numBytes % _SL_IBO_PADDING_BYTES)));
        ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), numBytes);
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_IndexBuffer::SL_IndexBuffer(SL_IndexBuffer&& v) noexcept :
    mType{v.mType},
    mBytesPerId{v.mBytesPerId},
    mCount{v.mCount},
    mBuffer{std::move(v.mBuffer)}
{
    v.mType = SL_DataType::VERTEX_DATA_INT;
    v.mCount = 0;
    v.mBytesPerId = sl_bytes_per_vertex(SL_DataType::VERTEX_DATA_INT, SL_Dimension::VERTEX_DIMENSION_1);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_IndexBuffer& SL_IndexBuffer::operator=(const SL_IndexBuffer& v) noexcept
{
    if (this != &v)
    {
        mType = v.mType;
        mBytesPerId = v.mBytesPerId;
        mCount = v.mCount;

        if (v.mBuffer != nullptr)
        {
            const uint32_t numBytes = v.mBytesPerId * v.mCount;
            mBuffer = ls::utils::make_unique_aligned_array<unsigned char>(numBytes + (_SL_IBO_PADDING_BYTES - (numBytes % _SL_IBO_PADDING_BYTES)));
            ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), numBytes);
        }
    }
    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_IndexBuffer& SL_IndexBuffer::operator=(SL_IndexBuffer&& v) noexcept
{
    if (this != &v)
    {
        mType = v.mType;
        v.mType = SL_DataType::VERTEX_DATA_INT;

        mBytesPerId = v.mBytesPerId;
        v.mBytesPerId = sl_bytes_per_vertex(SL_DataType::VERTEX_DATA_INT, SL_Dimension::VERTEX_DIMENSION_1);

        mCount = v.mCount;
        v.mCount = 0;

        mBuffer = std::move(v.mBuffer);
    }

    return *this;
}



/*-------------------------------------
 * Initialize the data in *this to empty values
-------------------------------------*/
int SL_IndexBuffer::init(uint32_t numElements, SL_DataType type, const void* pData) noexcept
{
    LS_ASSERT(type == SL_DataType::VERTEX_DATA_BYTE
    || type == SL_DataType::VERTEX_DATA_SHORT
    || type == SL_DataType::VERTEX_DATA_INT);

    if (!numElements)
    {
        return -1;
    }

    const uint32_t bytesPerType = sl_bytes_per_vertex(type, SL_Dimension::VERTEX_DIMENSION_1);
    const uint32_t numBytes = numElements * bytesPerType;

    mType = type;
    mBytesPerId = bytesPerType;
    mCount = numElements;
    mBuffer = ls::utils::make_unique_aligned_array<unsigned char>(numBytes + (_SL_IBO_PADDING_BYTES - (numBytes % _SL_IBO_PADDING_BYTES)));

    if (pData != nullptr)
    {
        assign(pData, 0, numElements);
    }

    return 0;
}



/*-------------------------------------
 * Delete all data in *this.
-------------------------------------*/
void SL_IndexBuffer::terminate() noexcept
{
    mType = SL_DataType::VERTEX_DATA_INT;
    mBytesPerId = sl_bytes_per_vertex(mType, SL_Dimension::VERTEX_DIMENSION_1);
    mCount = 0;
    mBuffer.reset();
}
