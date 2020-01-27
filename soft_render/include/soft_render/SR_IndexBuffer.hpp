
#ifndef SR_INDEXBUFFER_HPP
#define SR_INDEXBUFFER_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Geometry.hpp" // SR_DataType



class SR_IndexBuffer
{
  private:
    SR_DataType mType;

    uint32_t mBytesPerId;

    uint32_t mCount;

    ls::utils::Pointer<unsigned char[], ls::utils::AlignedDeleter> mBuffer;

  public:
    ~SR_IndexBuffer() noexcept;

    SR_IndexBuffer() noexcept;

    SR_IndexBuffer(const SR_IndexBuffer& v) noexcept;

    SR_IndexBuffer(SR_IndexBuffer&& v) noexcept;

    SR_IndexBuffer& operator=(const SR_IndexBuffer& v) noexcept;

    SR_IndexBuffer& operator=(SR_IndexBuffer&& v) noexcept;

    int init(uint32_t numElements, SR_DataType type, const void* pData = nullptr) noexcept;

    void terminate() noexcept;

    SR_DataType type() const noexcept;

    std::size_t count() const noexcept;

    std::size_t num_bytes() const noexcept;

    unsigned bytes_per_element() const noexcept;

    void* element(const ptrdiff_t index) noexcept;

    const void* element(const ptrdiff_t index) const noexcept;

    void* data() noexcept;

    const void* data() const noexcept;

    void assign(const void* pInputData, ptrdiff_t offset, std::size_t count) noexcept;

    bool valid() const noexcept;
};



/*--------------------------------------
 * Determine the type of data contained within the IBO
--------------------------------------*/
inline SR_DataType SR_IndexBuffer::type() const noexcept
{
    return mType;
}



/*--------------------------------------
 * Count the number of elements in the IBO
--------------------------------------*/
inline std::size_t SR_IndexBuffer::count() const noexcept
{
    return mCount;
}



/*--------------------------------------
 * Retrieve the number of bytes used.
--------------------------------------*/
inline std::size_t SR_IndexBuffer::num_bytes() const noexcept
{
    return mCount * mBytesPerId;
}



/*--------------------------------------
 * Get the byte size of each element
--------------------------------------*/
inline unsigned SR_IndexBuffer::bytes_per_element() const noexcept
{
    return mBytesPerId;
}



/*--------------------------------------
 * Retrieve a single element
--------------------------------------*/
inline void* SR_IndexBuffer::element(const ptrdiff_t index) noexcept
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 * Retrieve a single element (const)
--------------------------------------*/
inline const void* SR_IndexBuffer::element(const ptrdiff_t index) const noexcept
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 * Retrieve the raw data in *this.
--------------------------------------*/
inline void* SR_IndexBuffer::data() noexcept
{
    return mBuffer.get();
}



/*--------------------------------------
 * Retrieve the raw data in *this (const)
--------------------------------------*/
inline const void* SR_IndexBuffer::data() const noexcept
{
    return mBuffer.get();
}



/*--------------------------------------
 * Assign a set of predefined values
--------------------------------------*/
inline void SR_IndexBuffer::assign(const void* pInputData, ptrdiff_t offset, std::size_t count) noexcept
{
    ls::utils::fast_memcpy(&mBuffer[0]+offset, pInputData, count*mBytesPerId);
}



/*--------------------------------------
 * Check if *this has been initialized.
--------------------------------------*/
inline bool SR_IndexBuffer::valid() const noexcept
{
    return mBuffer != nullptr;
}



#endif /* SR_INDEXBUFFER_HPP */
