
#include <utility> // std::move()

#include "soft_render/SR_VertexBuffer.hpp"



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer::~SR_VertexBuffer() noexcept
{
    terminate();
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer() noexcept :
    mNumBytes{},
    mBuffer{}
{}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer(const SR_VertexBuffer& v) noexcept :
    mNumBytes{v.mNumBytes},
    mBuffer{(v.mBuffer == nullptr) ? nullptr : new unsigned char[v.mNumBytes]}
{
    if (v.mBuffer != nullptr)
    {
        ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), v.mNumBytes);
    }
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer(SR_VertexBuffer&& v) noexcept :
    mNumBytes{v.mNumBytes},
    mBuffer{std::move(v.mBuffer)}
{
    v.mNumBytes = 0;
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer& SR_VertexBuffer::operator=(const SR_VertexBuffer& v) noexcept
{
    if (this != &v)
    {
        mNumBytes = v.mNumBytes;

        if (v.mBuffer != nullptr)
        {
            mBuffer.reset(new unsigned char[v.mNumBytes]);
            ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), v.mNumBytes);
        }
    }
    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
SR_VertexBuffer& SR_VertexBuffer::operator=(SR_VertexBuffer&& v) noexcept
{
    if (this != &v)
    {
        mNumBytes = v.mNumBytes;
        v.mNumBytes = 0;

        mBuffer = std::move(v.mBuffer);
    }

    return *this;
}



/*--------------------------------------
 *
--------------------------------------*/
int SR_VertexBuffer::init(uint32_t numBytes, const void* pData)
{
    if (!numBytes)
    {
        return -1;
    }

    mNumBytes = numBytes;
    mBuffer.reset(new unsigned char[numBytes]);

    if (pData != nullptr)
    {
        assign(pData, 0, numBytes);
    }

    return 0;
}



/*--------------------------------------
 *
--------------------------------------*/
void SR_VertexBuffer::terminate()
{
    mNumBytes = 0;
    mBuffer.reset();
}
