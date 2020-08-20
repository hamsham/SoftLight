
#include <utility> // std::move()

#include "softlight/SL_VertexBuffer.hpp"



/*--------------------------------------
 * Destructor
--------------------------------------*/
SL_VertexBuffer::~SL_VertexBuffer() noexcept
{
    terminate();
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_VertexBuffer::SL_VertexBuffer() noexcept :
    mNumBytes{},
    mBuffer{}
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_VertexBuffer::SL_VertexBuffer(const SL_VertexBuffer& v) noexcept :
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
SL_VertexBuffer::SL_VertexBuffer(SL_VertexBuffer&& v) noexcept :
    mNumBytes{v.mNumBytes},
    mBuffer{std::move(v.mBuffer)}
{
    v.mNumBytes = 0;
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SL_VertexBuffer& SL_VertexBuffer::operator=(const SL_VertexBuffer& v) noexcept
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
SL_VertexBuffer& SL_VertexBuffer::operator=(SL_VertexBuffer&& v) noexcept
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
int SL_VertexBuffer::init(size_t numBytes, const void* pData) noexcept
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
void SL_VertexBuffer::terminate() noexcept
{
    mNumBytes = 0;
    mBuffer.reset();
}
