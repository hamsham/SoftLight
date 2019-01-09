
#include <utility> // std::move()

#include "soft_render/SR_IndexBuffer.hpp"



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer::~SR_IndexBuffer() noexcept
{
    terminate();
}



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer::SR_IndexBuffer() noexcept :
    mType{SR_DataType::VERTEX_DATA_INT},
    mBytesPerId{sr_bytes_per_vertex(SR_DataType::VERTEX_DATA_INT, SR_Dimension::VERTEX_DIMENSION_1)},
    mCount{0},
    mBuffer{nullptr}
{}



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer::SR_IndexBuffer(const SR_IndexBuffer& v) noexcept :
    mType{v.mType},
    mBytesPerId{v.mBytesPerId},
    mCount{v.mCount},
    mBuffer{nullptr}
{
    if (v.mBuffer != nullptr)
    {
        const uint32_t numBytes = v.mBytesPerId * v.mCount;
        mBuffer = ls::utils::Pointer<unsigned char[], ls::utils::AlignedDeleter>{(unsigned char*)ls::utils::aligned_malloc(numBytes)};
        ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), numBytes);
    }
}



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer::SR_IndexBuffer(SR_IndexBuffer&& v) noexcept :
    mType{v.mType},
    mBytesPerId{v.mBytesPerId},
    mCount{v.mCount},
    mBuffer{std::move(v.mBuffer)}
{
    v.mType = SR_DataType::VERTEX_DATA_INT;
    v.mCount = 0;
    v.mBytesPerId = sr_bytes_per_vertex(SR_DataType::VERTEX_DATA_INT, SR_Dimension::VERTEX_DIMENSION_1);
}



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer& SR_IndexBuffer::operator=(const SR_IndexBuffer& v) noexcept
{
    if (this != &v)
    {
        mType = v.mType;
        mBytesPerId = v.mBytesPerId;
        mCount = v.mCount;

        if (v.mBuffer != nullptr)
        {
            const uint32_t numBytes = v.mBytesPerId * v.mCount;
            mBuffer = ls::utils::Pointer<unsigned char[], ls::utils::AlignedDeleter>{(unsigned char*)ls::utils::aligned_malloc(numBytes)};
            ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), numBytes);
        }
    }
    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
SR_IndexBuffer& SR_IndexBuffer::operator=(SR_IndexBuffer&& v) noexcept
{
    if (this != &v)
    {
        mType = v.mType;
        v.mType = SR_DataType::VERTEX_DATA_INT;

        mBytesPerId = v.mBytesPerId;
        v.mBytesPerId = sr_bytes_per_vertex(SR_DataType::VERTEX_DATA_INT, SR_Dimension::VERTEX_DIMENSION_1);

        mCount = v.mCount;
        v.mCount = 0;

        mBuffer = std::move(v.mBuffer);
    }

    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
int SR_IndexBuffer::init(uint32_t numElements, SR_DataType type, const void* pData)
{
    assert(type == SR_DataType::VERTEX_DATA_BYTE ||
           type == SR_DataType::VERTEX_DATA_SHORT ||
           type == SR_DataType::VERTEX_DATA_INT);

    if (!numElements)
    {
        return -1;
    }

    const uint32_t bytesPerType = sr_bytes_per_vertex(type, SR_Dimension::VERTEX_DIMENSION_1);
    const uint32_t totalBytes = numElements * bytesPerType;

    mType = type;
    mBytesPerId = bytesPerType;
    mCount = numElements;
    mBuffer = ls::utils::Pointer<unsigned char[], ls::utils::AlignedDeleter>{(unsigned char*)ls::utils::aligned_malloc(totalBytes)};

    if (pData != nullptr)
    {
        assign(pData, 0, numElements);
    }

    return 0;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_IndexBuffer::terminate()
{
    mType = SR_DataType::VERTEX_DATA_INT;
    mBytesPerId = sr_bytes_per_vertex(mType, SR_Dimension::VERTEX_DIMENSION_1);
    mCount = 0;
    mBuffer.reset();
}
