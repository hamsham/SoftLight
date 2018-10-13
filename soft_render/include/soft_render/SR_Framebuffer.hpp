
#ifndef SR_FRAMEBUFFER_HPP
#define SR_FRAMEBUFFER_HPP

#include "lightsky/setup/Arch.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h" // utils::fast_memset, fast_fill

#include "soft_render/SR_Texture.hpp"




/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
template <typename color_type>
union vec4_t;
}
}



/*-----------------------------------------------------------------------------
 * Framebuffer Abstraction
-----------------------------------------------------------------------------*/
class SR_Framebuffer
{
  private:
    uint64_t mNumColors;

    SR_Texture** mColors;

    SR_Texture* mDepth;

  public:
    ~SR_Framebuffer() noexcept;

    SR_Framebuffer() noexcept;

    SR_Framebuffer(const SR_Framebuffer& f) noexcept;

    SR_Framebuffer(SR_Framebuffer&& f) noexcept;

    SR_Framebuffer& operator=(const SR_Framebuffer& f) noexcept;

    SR_Framebuffer& operator=(SR_Framebuffer&& f) noexcept;

    int reserve_color_buffers(uint64_t numColorBuffers) noexcept;

    int attach_color_buffer(uint64_t index, SR_Texture& t) noexcept;

    SR_Texture* detach_color_buffer(uint64_t index) noexcept;

    const SR_Texture* get_color_buffer(uint64_t index) const noexcept;

    SR_Texture* get_color_buffer(uint64_t index) noexcept;

    uint64_t num_color_buffers() const noexcept;

    template <typename index_type, typename color_type>
    void clear_color_buffer(const index_type index, const color_type& color) noexcept;

    void clear_color_buffers() noexcept;

    int attach_depth_buffer(SR_Texture& d) noexcept;

    SR_Texture* detach_depth_buffer() noexcept;

    const SR_Texture* get_depth_buffer() const noexcept;

    SR_Texture* get_depth_buffer() noexcept;

    template <typename float_type>
    void clear_depth_buffer(const float_type depthVal) noexcept;

    void clear_depth_buffer() noexcept;

    int valid() const noexcept;

    void terminate() noexcept;

    bool put_pixel(
        uint64_t targetId,
        uint16_t x,
        uint16_t y,
        uint16_t z,
        const ls::math::vec4_t<float>& rgba) noexcept;

    bool test_depth_pixel(
        uint16_t x,
        uint16_t y,
        float depth) noexcept;

    template <typename data_t>
    void put_depth_pixel(
        uint16_t x,
        uint16_t y,
        data_t depth) noexcept;

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    uint16_t depth() const noexcept;
};



/*-------------------------------------
 *
-------------------------------------*/
inline const SR_Texture* SR_Framebuffer::get_color_buffer(uint64_t index) const noexcept
{
    return mColors[index];
}



/*-------------------------------------
 *
-------------------------------------*/
inline SR_Texture* SR_Framebuffer::get_color_buffer(uint64_t index) noexcept
{
    return mColors[index];
}



/*-------------------------------------
 *
-------------------------------------*/
inline uint64_t SR_Framebuffer::num_color_buffers() const noexcept
{
    return mNumColors;
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename index_type, typename color_type>
void SR_Framebuffer::clear_color_buffer(const index_type i, const color_type& c) noexcept
{
    static_assert(std::is_integral<index_type>::value, "Error: Data type cannot be used for indexing.");

    SR_Texture* const pTex = mColors[i];

    if (pTex->data())
    {
        LS_DEBUG_ASSERT(pTex->bpp() == sizeof(color_type)); // insurance

        const size_t numItems = pTex->width() * pTex->height() * pTex->depth();

        ls::utils::fast_fill<color_type>(reinterpret_cast<color_type*>(pTex->data()), c, numItems);
    }
}



/*-------------------------------------
 *
-------------------------------------*/
template <typename float_type>
void SR_Framebuffer::clear_depth_buffer(const float_type depthVal) noexcept
{
    static_assert(std::is_floating_point<float_type>::value, "Error: Data type cannot be used for clearing a depth buffer.");

    if (!mDepth->data())
    {
        return;
    }

    LS_DEBUG_ASSERT(mDepth->bpp() == sizeof(float_type)); // insurance

    ls::utils::fast_fill<float_type>(reinterpret_cast<float_type*>(mDepth->data()), depthVal, mDepth->width()*mDepth->height());
}



/*-------------------------------------
 *
-------------------------------------*/
inline void SR_Framebuffer::clear_depth_buffer() noexcept
{
    if (mDepth->data())
    {
        const uint64_t numBytes = mDepth->bpp() * mDepth->width() * mDepth->height() * mDepth->depth();
        ls::utils::fast_memset(mDepth->data(), 0, numBytes);
    }
}



/*-------------------------------------
 *
-------------------------------------*/
inline const SR_Texture* SR_Framebuffer::get_depth_buffer() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 *
-------------------------------------*/
inline SR_Texture* SR_Framebuffer::get_depth_buffer() noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Perform a depth test
-------------------------------------*/
inline bool SR_Framebuffer::test_depth_pixel(
    uint16_t x,
    uint16_t y,
    float depth) noexcept
{
    return ((mDepth->type() == SR_COLOR_R_FLOAT) && depth >= mDepth->texel<float>(x, y))
    || ((mDepth->type() == SR_COLOR_R_DOUBLE) && depth > mDepth->texel<double>(x, y));
}



/*-------------------------------------
 * Place a single pixel onto the depth buffer
-------------------------------------*/
template <>
inline void SR_Framebuffer::put_depth_pixel<float>(uint16_t x, uint16_t y, float depth) noexcept
{
    mDepth->texel<float>(x, y) = depth;
}



template <>
inline void SR_Framebuffer::put_depth_pixel<double>(uint16_t x, uint16_t y, double depth) noexcept
{
    mDepth->texel<double>(x, y) = depth;
}



#endif /* SR_FRAMEBUFFER_HPP */
