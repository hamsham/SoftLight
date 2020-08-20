
#ifndef SL_UNIFORM_BUFFER_HPP
#define SL_UNIFORM_BUFFER_HPP



/**----------------------------------------------------------------------------
 * Specify limits for the uniform buffer class.
-----------------------------------------------------------------------------*/
enum SL_UniformLimits : unsigned
{
    SL_MAX_UNIFORM_BUFFER_SIZE = 1024u
};



/**----------------------------------------------------------------------------
 * A common uniform type which can be shared across shaders.
-----------------------------------------------------------------------------*/
class alignas(sizeof(float)*4) SL_UniformBuffer
{
  private:
    unsigned char mBytes[SL_MAX_UNIFORM_BUFFER_SIZE];

  public:
    ~SL_UniformBuffer() noexcept = default;

    SL_UniformBuffer() noexcept = default;

    SL_UniformBuffer(const SL_UniformBuffer&) noexcept = default;

    SL_UniformBuffer(SL_UniformBuffer&&) noexcept = default;

    SL_UniformBuffer& operator=(const SL_UniformBuffer&) noexcept = default;

    SL_UniformBuffer& operator=(SL_UniformBuffer&&) noexcept = default;

    const unsigned char* buffer() const noexcept;

    inline unsigned char* buffer() noexcept;

    template <typename data_t>
    const data_t* as() const noexcept;

    template <typename data_t>
    data_t* as() noexcept;

    template <typename data_t>
    void assign(const data_t* pBytes, unsigned long long offset) noexcept;

    void assign(const void* pBytes, unsigned long long offset, unsigned long long numBytes) noexcept;

    void fill(unsigned char fillByte, unsigned long long offset, unsigned long long numBytes) noexcept;

    void clear() noexcept;
};



inline const unsigned char* SL_UniformBuffer::buffer() const noexcept
{
    return mBytes;
}



inline unsigned char* SL_UniformBuffer::buffer() noexcept
{
    return mBytes;
}



template <typename data_t>
inline const data_t* SL_UniformBuffer::as() const noexcept
{
    static_assert(sizeof(data_t) < SL_MAX_UNIFORM_BUFFER_SIZE, "Cannot access uniform due to insufficient memory.");
    return reinterpret_cast<const data_t*>(mBytes);
}



template <typename data_t>
inline data_t* SL_UniformBuffer::as() noexcept
{
    static_assert(sizeof(data_t) < SL_MAX_UNIFORM_BUFFER_SIZE, "Cannot access uniform due to insufficient memory.");
    return reinterpret_cast<data_t*>(mBytes);
}



template <typename data_t>
inline void SL_UniformBuffer::assign(const data_t* pBytes, unsigned long long offset) noexcept
{
    static_assert(sizeof(data_t)+offset < SL_MAX_UNIFORM_BUFFER_SIZE, "Cannot store uniform due to insufficient memory.");

    data_t* pDst = reinterpret_cast<data_t*>(&mBytes[0] + offset);
    *pDst = *pBytes;
}



inline void SL_UniformBuffer::assign(const void* pBytes, unsigned long long offset, unsigned long long numBytes) noexcept
{
    const unsigned char* pSrc = reinterpret_cast<const unsigned char*>(pBytes);
    unsigned char* pDst = &mBytes[0] + offset;

    while (numBytes--)
    {
        *pDst++ = *pSrc++;
    }
}



inline void SL_UniformBuffer::fill(const unsigned char fillByte, unsigned long long offset, unsigned long long numBytes) noexcept
{
    unsigned char* pDst = &mBytes[0] + offset;

    while (numBytes--)
    {
        *pDst++ = fillByte;
    }
}



inline void SL_UniformBuffer::clear() noexcept
{
    this->fill((unsigned char)'\0', 0, SL_MAX_UNIFORM_BUFFER_SIZE);
}



#endif /* SL_UNIFORM_BUFFER_HPP */
