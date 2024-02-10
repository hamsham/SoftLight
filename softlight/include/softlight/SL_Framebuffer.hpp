
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
enum SL_FboLimits
{
    SL_FBO_MIN_COLOR_ATTACHMENTS = 1,
    SL_FBO_MAX_COLOR_ATTACHMENTS = 4,
};



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



struct SL_FboOutputFunctions
{
    SL_FboOutputMask outputMask;
    SL_TextureView* pColorAttachments;
    SL_TextureView* pDepthAttachment;

    union
    {
        void (*pOutFunc[SL_FboLimits::SL_FBO_MAX_COLOR_ATTACHMENTS])(uint16_t, uint16_t, const ls::math::vec4&, SL_TextureView&);
        void (*pOutBlendedFunc[SL_FboLimits::SL_FBO_MAX_COLOR_ATTACHMENTS])(uint16_t, uint16_t, const ls::math::vec4&, SL_TextureView&, const SL_BlendMode);
    };

    void (*pOutDepthFunc)(uint16_t, uint16_t, const ls::math::vec4&, SL_TextureView&);
};



/*-----------------------------------------------------------------------------
 * Framebuffer Abstraction
-----------------------------------------------------------------------------*/
class SL_Framebuffer
{
  private:
    uint64_t mNumColors;

    SL_TextureView mColors[(unsigned)SL_FboLimits::SL_FBO_MAX_COLOR_ATTACHMENTS];

    SL_TextureView mDepth;

  public:
    ~SL_Framebuffer() noexcept;

    SL_Framebuffer() noexcept;

    SL_Framebuffer(const SL_Framebuffer& f) noexcept;

    SL_Framebuffer(SL_Framebuffer&& f) noexcept;

    SL_Framebuffer& operator=(const SL_Framebuffer& f) noexcept;

    SL_Framebuffer& operator=(SL_Framebuffer&& f) noexcept;

    int reserve_color_buffers(unsigned numColorBuffers) noexcept;

    int attach_color_buffer(unsigned index, SL_TextureView& t) noexcept;

    int detach_color_buffer(unsigned index) noexcept;

    const SL_TextureView& get_color_buffer(unsigned index) const noexcept;

    SL_TextureView& get_color_buffer(unsigned index) noexcept;

    unsigned num_color_buffers() const noexcept;

    template <typename index_type, typename color_type>
    void clear_color_buffer(const index_type index, const color_type& color) noexcept;

    void clear_color_buffers() noexcept;

    int attach_depth_buffer(SL_TextureView& d) noexcept;

    int detach_depth_buffer() noexcept;

    const SL_TextureView& get_depth_buffer() const noexcept;

    SL_TextureView& get_depth_buffer() noexcept;

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

    bool build_output_functions(SL_FboOutputFunctions& result, bool blendEnabled) noexcept;
};



/*-------------------------------------
 * Retrieve an internal color buffer, or NULL if it doesn't exist.
-------------------------------------*/
inline const SL_TextureView& SL_Framebuffer::get_color_buffer(unsigned index) const noexcept
{
    return mColors[index];
}



/*-------------------------------------
 * Retrieve an internal color buffer, or NULL if it doesn't exist.
-------------------------------------*/
inline SL_TextureView& SL_Framebuffer::get_color_buffer(unsigned index) noexcept
{
    return mColors[index];
}



/*-------------------------------------
 * Retrieve the number of active color buffers.
-------------------------------------*/
inline unsigned SL_Framebuffer::num_color_buffers() const noexcept
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

    SL_TextureView& pTex = mColors[i];

    if (pTex.pTexels)
    {
        LS_DEBUG_ASSERT(pTex.bytesPerTexel == sizeof(color_type)); // insurance
        const size_t numItems = pTex.width * pTex.height * pTex.depth;

        if (sizeof(color_type) == sizeof(uint32_t))
        {
            const size_t numBytes = numItems * pTex.bytesPerTexel;
            ls::utils::fast_memset_4(reinterpret_cast<void*>(pTex.pTexels), *reinterpret_cast<const uint32_t*>(&c), numBytes);
        }
        else
        {
            ls::utils::fast_fill<color_type>(reinterpret_cast<color_type*>(pTex.pTexels), c, numItems);
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

    if (mDepth.pTexels == nullptr)
    {
        return;
    }

    LS_DEBUG_ASSERT(mDepth.bytesPerTexel == sizeof(float_type)); // insurance

    if (sizeof(float_type) == sizeof(uint32_t))
    {
        const size_t numBytes = mDepth.width*mDepth.height* sizeof(float_type);
        union
        {
            float_type f;
            uint32_t i;
        } outVal{depthVal};
        ls::utils::fast_memset_4(reinterpret_cast<void*>(mDepth.pTexels), outVal.i, numBytes);
    }
    else if (sizeof(float_type) == sizeof(uint64_t))
    {
        const size_t numBytes = mDepth.width*mDepth.height* sizeof(float_type);
        union
        {
            float_type f;
            uint64_t i;
        } outVal{depthVal};
        ls::utils::fast_memset_8(reinterpret_cast<void*>(mDepth.pTexels), outVal.i, numBytes);
    }
    else
    {
        ls::utils::fast_fill<float_type>(reinterpret_cast<float_type*>(mDepth.pTexels), depthVal, mDepth.width*mDepth.height);
    }
}



/*-------------------------------------
 * Clear the depth buffer
-------------------------------------*/
inline void SL_Framebuffer::clear_depth_buffer() noexcept
{
    if (mDepth.pTexels)
    {
        const uint64_t numBytes = mDepth.bytesPerTexel * mDepth.width * mDepth.height * mDepth.depth;
        ls::utils::fast_memset(mDepth.pTexels, 0, numBytes);
    }
}



/*-------------------------------------
 * Retrieve the depth buffer
-------------------------------------*/
inline const SL_TextureView& SL_Framebuffer::get_depth_buffer() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Retrieve the depth buffer
-------------------------------------*/
inline SL_TextureView& SL_Framebuffer::get_depth_buffer() noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Place a single pixel onto the depth buffer
-------------------------------------*/
template <>
inline void SL_Framebuffer::put_depth_pixel<ls::math::half>(uint16_t x, uint16_t y, ls::math::half depth) noexcept
{
    ((ls::math::half*)mDepth.pTexels)[x + mDepth.width * y] = depth;
}



template <>
inline void SL_Framebuffer::put_depth_pixel<float>(uint16_t x, uint16_t y, float depth) noexcept
{
    ((float*)mDepth.pTexels)[x + mDepth.width * y] = depth;
}



template <>
inline void SL_Framebuffer::put_depth_pixel<double>(uint16_t x, uint16_t y, double depth) noexcept
{
    ((double*)mDepth.pTexels)[x + mDepth.width * y] = depth;
}



#endif /* SL_FRAMEBUFFER_HPP */
