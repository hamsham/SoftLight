
#ifndef SL_INDEXBUFFER_HPP
#define SL_INDEXBUFFER_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"

#include "softlight/SL_Geometry.hpp" // SL_DataType



/*-----------------------------------------------------------------------------
 * @brief Index Buffer Class
 *
 * This class manages element array buffer values for unsigned char, unsigned
 * short, and unsigned long indices. These indices are used during rendering
 * to identify vertices in vertex buffer objects.
-----------------------------------------------------------------------------*/
class SL_IndexBuffer
{
  private:
    SL_DataType mType;

    uint32_t mBytesPerId;

    uint32_t mCount;

    ls::utils::UniqueAlignedArray<unsigned char> mBuffer;

  public:
    ~SL_IndexBuffer() noexcept;

    SL_IndexBuffer() noexcept;

    SL_IndexBuffer(const SL_IndexBuffer& v) noexcept;

    SL_IndexBuffer(SL_IndexBuffer&& v) noexcept;

    SL_IndexBuffer& operator=(const SL_IndexBuffer& v) noexcept;

    SL_IndexBuffer& operator=(SL_IndexBuffer&& v) noexcept;

    int init(uint32_t numElements, SL_DataType type, const void* pData = nullptr) noexcept;

    void terminate() noexcept;

    SL_DataType type() const noexcept;

    std::size_t count() const noexcept;

    std::size_t num_bytes() const noexcept;

    unsigned bytes_per_element() const noexcept;

    void* element(const ptrdiff_t index) noexcept;

    const void* element(const ptrdiff_t index) const noexcept;

    size_t index(size_t index) const noexcept;

    void* data() noexcept;

    const void* data() const noexcept;

    void assign(const void* pInputData, ptrdiff_t offset, std::size_t count) noexcept;

    bool valid() const noexcept;
};



/*--------------------------------------
 * Determine the type of data contained within the IBO
--------------------------------------*/
inline SL_DataType SL_IndexBuffer::type() const noexcept
{
    return mType;
}



/*--------------------------------------
 * Count the number of elements in the IBO
--------------------------------------*/
inline std::size_t SL_IndexBuffer::count() const noexcept
{
    return mCount;
}



/*--------------------------------------
 * Retrieve the number of bytes used.
--------------------------------------*/
inline std::size_t SL_IndexBuffer::num_bytes() const noexcept
{
    return mCount * mBytesPerId;
}



/*--------------------------------------
 * Get the byte size of each element
--------------------------------------*/
inline unsigned SL_IndexBuffer::bytes_per_element() const noexcept
{
    return mBytesPerId;
}



/*--------------------------------------
 * Retrieve a single element
--------------------------------------*/
inline void* SL_IndexBuffer::element(const ptrdiff_t index) noexcept
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 * Retrieve a single element (const)
--------------------------------------*/
inline const void* SL_IndexBuffer::element(const ptrdiff_t index) const noexcept
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 * Retrieve a single element
--------------------------------------*/
inline size_t SL_IndexBuffer::index(const size_t index) const noexcept
{
    switch (mType)
    {
        case VERTEX_DATA_BYTE:  return *reinterpret_cast<const unsigned char*>(this->element(index));
        case VERTEX_DATA_SHORT: return *reinterpret_cast<const unsigned short*>(this->element(index));
        case VERTEX_DATA_INT:   return *reinterpret_cast<const unsigned int*>(this->element(index));
        default:
            LS_UNREACHABLE();
    }

    return (size_t)~0ull;
}



/*--------------------------------------
 * Retrieve the raw data in *this.
--------------------------------------*/
inline void* SL_IndexBuffer::data() noexcept
{
    return mBuffer.get();
}



/*--------------------------------------
 * Retrieve the raw data in *this (const)
--------------------------------------*/
inline const void* SL_IndexBuffer::data() const noexcept
{
    return mBuffer.get();
}



/*--------------------------------------
 * Assign a set of predefined values
--------------------------------------*/
inline void SL_IndexBuffer::assign(const void* pInputData, ptrdiff_t offset, std::size_t count) noexcept
{
    ls::utils::fast_memcpy(&mBuffer[0]+offset, pInputData, count*mBytesPerId);
}



/*--------------------------------------
 * Check if *this has been initialized.
--------------------------------------*/
inline bool SL_IndexBuffer::valid() const noexcept
{
    return mBuffer != nullptr;
}



#endif /* SL_INDEXBUFFER_HPP */
