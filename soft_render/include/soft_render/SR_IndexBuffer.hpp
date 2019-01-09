
#ifndef SR_INDEXBUFFER_HPP
#define SR_INDEXBUFFER_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Geometry.hpp"



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

    int init(uint32_t numElements, SR_DataType type, const void* pData = nullptr);

    void terminate();

    SR_DataType type() const;

    std::size_t count() const;

    std::size_t num_bytes() const;

    unsigned bytes_per_element() const;

    void* element(const ptrdiff_t index);

    const void* element(const ptrdiff_t index) const;

    void* data();

    const void* data() const;

    void assign(const void* pInputData, ptrdiff_t offset, std::size_t count);

    bool valid() const;
};



/*--------------------------------------
 *
--------------------------------------*/
inline SR_DataType SR_IndexBuffer::type() const
{
    return mType;
}



/*--------------------------------------
 *
--------------------------------------*/
inline std::size_t SR_IndexBuffer::count() const
{
    return mCount;
}



/*--------------------------------------
 *
--------------------------------------*/
inline std::size_t SR_IndexBuffer::num_bytes() const
{
    return mCount * mBytesPerId;
}



/*--------------------------------------
 *
--------------------------------------*/
inline unsigned SR_IndexBuffer::bytes_per_element() const
{
    return mBytesPerId;
}



/*--------------------------------------
 *
--------------------------------------*/
inline void* SR_IndexBuffer::element(const ptrdiff_t index)
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const void* SR_IndexBuffer::element(const ptrdiff_t index) const
{
    const ptrdiff_t offset = index * mBytesPerId;
    return mBuffer.get() + offset;
}



/*--------------------------------------
 *
--------------------------------------*/
inline void* SR_IndexBuffer::data()
{
    return mBuffer.get();
}



/*--------------------------------------
 *
--------------------------------------*/
inline const void* SR_IndexBuffer::data() const
{
    return mBuffer.get();
}



/*--------------------------------------
 *
--------------------------------------*/
inline void SR_IndexBuffer::assign(const void* pInputData, ptrdiff_t offset, std::size_t count)
{
    ls::utils::fast_memcpy(&mBuffer[0]+offset, pInputData, count*mBytesPerId);
}



/*--------------------------------------
 *
--------------------------------------*/
inline bool SR_IndexBuffer::valid() const
{
    return mBuffer != nullptr;
}



#endif /* SR_INDEXBUFFER_HPP */
