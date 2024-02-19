
#ifndef SL_SWAPCHAIN_HPP
#define SL_SWAPCHAIN_HPP

#include <cstdint> // uint8_t

#include "lightsky/utils/Pointer.h"

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

enum class SL_WindowBackend;

class SL_RenderWindow;
class SL_Texture;



/**----------------------------------------------------------------------------
 * @brief The SL_Swapchain class encapsulates the native windowing system's
 * backbuffer. This class gets passed into an SL_RenderWindow for blitting to
 * the front buffer.
-----------------------------------------------------------------------------*/
class SL_Swapchain
{
    friend class SL_Context;
    friend struct SL_ShaderProcessor;

  public:
    static ls::utils::Pointer<SL_Swapchain> create(SL_WindowBackend backend) noexcept;

  protected:
    SL_Texture mTexture;

  public:
    virtual ~SL_Swapchain() noexcept = 0;

    SL_Swapchain() noexcept;

    SL_Swapchain(const SL_Swapchain&) = delete;

    SL_Swapchain(SL_Swapchain&&) noexcept;

    SL_Swapchain& operator=(const SL_Swapchain&) = delete;

    SL_Swapchain& operator=(SL_Swapchain&&) noexcept;

    virtual int init(SL_RenderWindow& w, unsigned width, unsigned height) noexcept = 0;

    virtual int terminate() noexcept = 0;

    virtual unsigned width() const noexcept = 0;

    virtual unsigned height() const noexcept = 0;

    virtual const void* native_handle() const noexcept = 0;

    virtual void* native_handle() noexcept = 0;

    virtual const ls::math::vec4_t<uint8_t>* buffer() const noexcept = 0;

    virtual ls::math::vec4_t<uint8_t>* buffer() noexcept = 0;

    inline SL_ColorDataType type() const noexcept;

    virtual inline const SL_Texture& texture() const noexcept;

    virtual inline SL_Texture& texture() noexcept;
};



/*-------------------------------------
 * Retrieve the native color type of the backbuffer.
-------------------------------------*/
inline SL_ColorDataType SL_Swapchain::type() const noexcept
{
    return SL_ColorDataType::SL_COLOR_RGBA_8U;
}



/*-------------------------------------
 * Retrieve the texture contained within the backbuffer.
-------------------------------------*/
inline const SL_Texture& SL_Swapchain::texture() const noexcept
{
    return mTexture;
}



/*-------------------------------------
 * Retrieve the texture contained within the backbuffer.
-------------------------------------*/
inline SL_Texture& SL_Swapchain::texture() noexcept
{
    return mTexture;
}



#endif /* SL_SWAPCHAIN_HPP */
