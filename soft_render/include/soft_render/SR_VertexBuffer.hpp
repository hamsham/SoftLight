
#ifndef SR_VERTEXBUFFER_HPP
#define SR_VERTEXBUFFER_HPP

#include <cstddef> // ptrdiff_t
#include <memory>

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Geometry.hpp"



class SR_VertexBuffer
{
  private:
      size_t mNumBytes;

    ls::utils::Pointer<unsigned char[], ls::utils::AlignedDeleter> mBuffer;

  public:
    ~SR_VertexBuffer() noexcept;

    SR_VertexBuffer() noexcept;

    SR_VertexBuffer(const SR_VertexBuffer& v) noexcept;

    SR_VertexBuffer(SR_VertexBuffer&& v) noexcept;

    SR_VertexBuffer& operator=(const SR_VertexBuffer& v) noexcept;

    SR_VertexBuffer& operator=(SR_VertexBuffer&& v) noexcept;

    int init(size_t numBytes, const void* pData = nullptr);

    void terminate();

    std::size_t num_bytes() const;

    template <typename data_type = unsigned char>
    data_type* element(const ptrdiff_t offset);

    template <typename data_type = unsigned char>
    const data_type* element(const ptrdiff_t offset) const;

    void* data();

    const void* data() const;

    void assign(const void* pInputData, ptrdiff_t offset, std::size_t numBytes);

    bool valid() const;
};



/*--------------------------------------
 *
--------------------------------------*/
inline std::size_t SR_VertexBuffer::num_bytes() const
{
    return mNumBytes;
}



/*--------------------------------------
 *
--------------------------------------*/
template <typename data_type>
inline data_type* SR_VertexBuffer::element(const ptrdiff_t offset)
{
    return reinterpret_cast<data_type*>(mBuffer.get() + offset);
}



/*--------------------------------------
 *
--------------------------------------*/
template <typename data_type>
inline const data_type* SR_VertexBuffer::element(const ptrdiff_t offset) const
{
    return reinterpret_cast<const data_type*>(mBuffer.get() + offset);
}



/*--------------------------------------
 *
--------------------------------------*/
inline void* SR_VertexBuffer::data()
{
    return mBuffer.get();
}



/*--------------------------------------
 *
--------------------------------------*/
inline const void* SR_VertexBuffer::data() const
{
    return mBuffer.get();
}



/*--------------------------------------
 *
--------------------------------------*/
inline void SR_VertexBuffer::assign(const void* pInputData, ptrdiff_t offset, std::size_t numBytes)
{
    ls::utils::fast_memcpy(mBuffer.get()+offset, pInputData, numBytes);
}



/*--------------------------------------
 *
--------------------------------------*/
inline bool SR_VertexBuffer::valid() const
{
    return mBuffer != nullptr;
}



#endif /* SR_VERTEXBUFFER_HPP */
