
#ifndef SL_FRAMEBUFFER_HPP
#define SL_FRAMEBUFFER_HPP

#include "lightsky/setup/Arch.h"

#include "lightsky/math/half.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h" // utils::fast_memset, fast_fill

#include "softlight/SL_Texture.hpp"



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

struct SL_FragmentParam;
enum SL_BlendMode : uint8_t;



/*-----------------------------------------------------------------------------
 * Framebuffer Utilities
-----------------------------------------------------------------------------*/
enum SL_FboOutputMask
{
    SL_FBO_OUTPUT_NONE,

    SL_FBO_OUTPUT_ATTACHMENT_0,
    SL_FBO_OUTPUT_ATTACHMENT_0_1,
    SL_FBO_OUTPUT_ATTACHMENT_0_1_2,
    SL_FBO_OUTPUT_ATTACHMENT_0_1_2_3,

    SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0,
    SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1,
    SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2,
    SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2_3,
};



constexpr SL_FboOutputMask sl_calc_fbo_out_mask(unsigned numOutputs, bool blendEnabled) noexcept
{
    return static_cast<SL_FboOutputMask>(numOutputs + (blendEnabled ? (unsigned)SL_FBO_OUTPUT_ATTACHMENT_0_1_2_3 : (unsigned)SL_FBO_OUTPUT_NONE));
}



/*-----------------------------------------------------------------------------
 * Framebuffer Abstraction
-----------------------------------------------------------------------------*/
class SL_Framebuffer
{
  private:
    uint64_t mNumColors;

    SL_Texture** mColors;

    SL_Texture* mDepth;

  public:
    ~SL_Framebuffer() noexcept;

    SL_Framebuffer() noexcept;

    SL_Framebuffer(const SL_Framebuffer& f) noexcept;

    SL_Framebuffer(SL_Framebuffer&& f) noexcept;

    SL_Framebuffer& operator=(const SL_Framebuffer& f) noexcept;

    SL_Framebuffer& operator=(SL_Framebuffer&& f) noexcept;

    int reserve_color_buffers(uint64_t numColorBuffers) noexcept;

    int attach_color_buffer(uint64_t index, SL_Texture& t) noexcept;

    SL_Texture* detach_color_buffer(uint64_t index) noexcept;

    const SL_Texture* get_color_buffer(uint64_t index) const noexcept;

    SL_Texture* get_color_buffer(uint64_t index) noexcept;

    uint64_t num_color_buffers() const noexcept;

    template <typename index_type, typename color_type>
    void clear_color_buffer(const index_type index, const color_type& color) noexcept;

    void clear_color_buffers() noexcept;

    int attach_depth_buffer(SL_Texture& d) noexcept;

    SL_Texture* detach_depth_buffer() noexcept;

    const SL_Texture* get_depth_buffer() const noexcept;

    SL_Texture* get_depth_buffer() noexcept;

    template <typename float_type>
    void clear_depth_buffer(const float_type depthVal) noexcept;

    void clear_depth_buffer() noexcept;

    int valid() const noexcept;

    void terminate() noexcept;

    void put_pixel(
        uint64_t targetId,
        uint16_t x,
        uint16_t y,
        const ls::math::vec4_t<float>& rgba) noexcept;

    void put_pixel(SL_FboOutputMask outMask, SL_BlendMode blendMode, const SL_FragmentParam& fragParam) noexcept;

    void put_alpha_pixel(
        uint64_t targetId,
        uint16_t x,
        uint16_t y,
        const ls::math::vec4_t<float>& rgba,
        const SL_BlendMode blendMode) noexcept;

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
 * Retrieve an internal color buffer, or NULL if it doesn't exist.
-------------------------------------*/
inline const SL_Texture* SL_Framebuffer::get_color_buffer(uint64_t index) const noexcept
{
    return mColors[index];
}



/*-------------------------------------
 * Retrieve an internal color buffer, or NULL if it doesn't exist.
-------------------------------------*/
inline SL_Texture* SL_Framebuffer::get_color_buffer(uint64_t index) noexcept
{
    return mColors[index];
}



/*-------------------------------------
 * Retrieve the number of active color buffers.
-------------------------------------*/
inline uint64_t SL_Framebuffer::num_color_buffers() const noexcept
{
    return mNumColors;
}



/*-------------------------------------
 * Clear a color buffer
-------------------------------------*/
template <typename index_type, typename color_type>
void SL_Framebuffer::clear_color_buffer(const index_type i, const color_type& c) noexcept
{
    static_assert(std::is_integral<index_type>::value, "Error: Data type cannot be used for indexing.");

    SL_Texture* const pTex = mColors[i];

    if (pTex->data())
    {
        LS_DEBUG_ASSERT(pTex->bpp() == sizeof(color_type)); // insurance
        const size_t numItems = pTex->width() * pTex->height() * pTex->depth();

        if (sizeof(color_type) == sizeof(uint32_t))
        {
            const size_t numBytes = numItems * pTex->bpp();
            ls::utils::fast_memset_4(reinterpret_cast<void*>(pTex->data()), *reinterpret_cast<const uint32_t*>(&c), numBytes);
        }
        else
        {
            ls::utils::fast_fill<color_type>(reinterpret_cast<color_type*>(pTex->data()), c, numItems);
        }
    }
}



/*-------------------------------------
 * Clear the depth buffer
-------------------------------------*/
template <typename float_type>
void SL_Framebuffer::clear_depth_buffer(const float_type depthVal) noexcept
{
    static_assert(std::is_floating_point<float_type>::value, "Error: Data type cannot be used for clearing a depth buffer.");

    if (!mDepth->data())
    {
        return;
    }

    LS_DEBUG_ASSERT(mDepth->bpp() == sizeof(float_type)); // insurance

    if (sizeof(float_type) == sizeof(uint32_t))
    {
        const size_t numBytes = mDepth->width()*mDepth->height()* sizeof(float_type);
        union
        {
            float_type f;
            uint32_t i;
        } outVal{depthVal};
        ls::utils::fast_memset_4(reinterpret_cast<void*>(mDepth->data()), outVal.i, numBytes);
    }
    else if (sizeof(float_type) == sizeof(uint64_t))
    {
        const size_t numBytes = mDepth->width()*mDepth->height()* sizeof(float_type);
        union
        {
            float_type f;
            uint64_t i;
        } outVal{depthVal};
        ls::utils::fast_memset_8(reinterpret_cast<void*>(mDepth->data()), outVal.i, numBytes);
    }
    else
    {
        ls::utils::fast_fill<float_type>(reinterpret_cast<float_type*>(mDepth->data()), depthVal, mDepth->width()*mDepth->height());
    }
}



/*-------------------------------------
 * Clear the depth buffer
-------------------------------------*/
inline void SL_Framebuffer::clear_depth_buffer() noexcept
{
    if (mDepth->data())
    {
        const uint64_t numBytes = mDepth->bpp() * mDepth->width() * mDepth->height() * mDepth->depth();
        ls::utils::fast_memset(mDepth->data(), 0, numBytes);
    }
}



/*-------------------------------------
 * Retrieve the depth buffer
-------------------------------------*/
inline const SL_Texture* SL_Framebuffer::get_depth_buffer() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Retrieve the depth buffer
-------------------------------------*/
inline SL_Texture* SL_Framebuffer::get_depth_buffer() noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Place a single pixel onto the depth buffer
-------------------------------------*/
template <>
inline void SL_Framebuffer::put_depth_pixel<ls::math::half>(uint16_t x, uint16_t y, ls::math::half depth) noexcept
{
    mDepth->texel<ls::math::half>(x, y) = depth;
}



template <>
inline void SL_Framebuffer::put_depth_pixel<float>(uint16_t x, uint16_t y, float depth) noexcept
{
    mDepth->texel<float>(x, y) = depth;
}



template <>
inline void SL_Framebuffer::put_depth_pixel<double>(uint16_t x, uint16_t y, double depth) noexcept
{
    mDepth->texel<double>(x, y) = depth;
}



#endif /* SL_FRAMEBUFFER_HPP */
