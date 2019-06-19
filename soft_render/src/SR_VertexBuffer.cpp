
#include <utility> // std::move()

#include "soft_render/SR_VertexBuffer.hpp"



/*--------------------------------------
 * Destructor
--------------------------------------*/
SR_VertexBuffer::~SR_VertexBuffer() noexcept
{
    terminate();
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer() noexcept :
    mNumBytes{},
    mBuffer{}
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer(const SR_VertexBuffer& v) noexcept :
    mNumBytes{v.mNumBytes},
    mBuffer{(v.mBuffer == nullptr) ? nullptr : (unsigned char*)ls::utils::aligned_malloc(v.mNumBytes)}
{
    if (v.mBuffer != nullptr)
    {
        ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), v.mNumBytes);
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SR_VertexBuffer::SR_VertexBuffer(SR_VertexBuffer&& v) noexcept :
    mNumBytes{v.mNumBytes},
    mBuffer{std::move(v.mBuffer)}
{
    v.mNumBytes = 0;
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SR_VertexBuffer& SR_VertexBuffer::operator=(const SR_VertexBuffer& v) noexcept
{
    if (this != &v)
    {
        mNumBytes = v.mNumBytes;

        if (v.mBuffer != nullptr)
        {
            mBuffer.reset((unsigned char*)ls::utils::aligned_malloc(v.mNumBytes));
            ls::utils::fast_memcpy(mBuffer.get(), v.mBuffer.get(), v.mNumBytes);
        }
    }
    return *this;
}



/*--------------------------------------
 * Move Operator
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
 * Initialize the data in *this
--------------------------------------*/
int SR_VertexBuffer::init(size_t numBytes, const void* pData) noexcept
{
    if (!numBytes)
    {
        return -1;
    }

    mNumBytes = numBytes;
    mBuffer.reset((unsigned char*)ls::utils::aligned_malloc(numBytes));

    if (pData != nullptr)
    {
        assign(pData, 0, numBytes);
    }

    return 0;
}



/*--------------------------------------
 * Delete all data used by *this.
--------------------------------------*/
void SR_VertexBuffer::terminate() noexcept
{
    mNumBytes = 0;
    mBuffer.reset();
}
