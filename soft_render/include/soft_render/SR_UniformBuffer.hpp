
#ifndef SR_UNIFORM_BUFFER_HPP
#define SR_UNIFORM_BUFFER_HPP



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
enum SR_UniformLimits : unsigned
{
    SR_MAX_UNIFORM_BUFFER_SIZE = 1024u
};



class alignas(sizeof(float)*4) SR_UniformBuffer
{
  private:
    unsigned char mBytes[SR_MAX_UNIFORM_BUFFER_SIZE];

  public:
    ~SR_UniformBuffer() noexcept = default;

    SR_UniformBuffer() noexcept = default;

    SR_UniformBuffer(const SR_UniformBuffer&) noexcept = default;

    SR_UniformBuffer(SR_UniformBuffer&&) noexcept = default;

    SR_UniformBuffer& operator=(const SR_UniformBuffer&) noexcept = default;

    SR_UniformBuffer& operator=(SR_UniformBuffer&&) noexcept = default;

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



inline const unsigned char* SR_UniformBuffer::buffer() const noexcept
{
    return mBytes;
}



inline unsigned char* SR_UniformBuffer::buffer() noexcept
{
    return mBytes;
}



template <typename data_t>
inline const data_t* SR_UniformBuffer::as() const noexcept
{
    return reinterpret_cast<const data_t*>(mBytes);
}



template <typename data_t>
inline data_t* SR_UniformBuffer::as() noexcept
{
    return reinterpret_cast<data_t*>(mBytes);
}



template <typename data_t>
inline void SR_UniformBuffer::assign(const data_t* pBytes, unsigned long long offset) noexcept
{
    static_assert(sizeof(data_t)+offset < SR_MAX_UNIFORM_BUFFER_SIZE, "Cannot store uniform due to insufficient memory.");

    data_t* pDst = reinterpret_cast<data_t*>(&mBytes[0] + offset);
    *pDst = *pBytes;
}



inline void SR_UniformBuffer::assign(const void* pBytes, unsigned long long offset, unsigned long long numBytes) noexcept
{
    const unsigned char* pSrc = reinterpret_cast<const unsigned char*>(pBytes);
    unsigned char* pDst = &mBytes[0] + offset;

    while (numBytes--)
    {
        *pDst++ = *pSrc++;
    }
}



inline void SR_UniformBuffer::fill(const unsigned char fillByte, unsigned long long offset, unsigned long long numBytes) noexcept
{
    unsigned char* pDst = &mBytes[0] + offset;

    while (numBytes--)
    {
        *pDst++ = fillByte;
    }
}



inline void SR_UniformBuffer::clear() noexcept
{
    this->fill((unsigned char)'\0', 0, SR_MAX_UNIFORM_BUFFER_SIZE);
}



#endif /* SR_UNIFORM_BUFFER_HPP */
