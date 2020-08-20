
#ifndef SL_WINDOW_BUFFER_HPP
#define SL_WINDOW_BUFFER_HPP

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

class SL_RenderWindow;
class SL_Texture;



/**----------------------------------------------------------------------------
 * @brief The SL_WindowBuffer class encapsulates the native windowing system's
 * backbuffer. This class gets passed into an SL_RenderWindow for blitting to
 * the front buffer.
-----------------------------------------------------------------------------*/
class SL_WindowBuffer
{
    friend class SL_Context;
    friend struct SL_ShaderProcessor;

  public:
    static ls::utils::Pointer<SL_WindowBuffer> create() noexcept;

  protected:
    SL_Texture mTexture;

  public:
    virtual ~SL_WindowBuffer() noexcept = 0;

    SL_WindowBuffer() noexcept;

    SL_WindowBuffer(const SL_WindowBuffer&) = delete;

    SL_WindowBuffer(SL_WindowBuffer&&) noexcept;

    SL_WindowBuffer& operator=(const SL_WindowBuffer&) = delete;

    SL_WindowBuffer& operator=(SL_WindowBuffer&&) noexcept;

    virtual int init(SL_RenderWindow& w, unsigned width, unsigned height) noexcept = 0;

    virtual int terminate() noexcept = 0;

    virtual unsigned width() const noexcept = 0;

    virtual unsigned height() const noexcept = 0;

    virtual const void* native_handle() const noexcept = 0;

    virtual void* native_handle() noexcept = 0;

    virtual const ls::math::vec4_t<uint8_t>* buffer() const noexcept = 0;

    virtual ls::math::vec4_t<uint8_t>* buffer() noexcept = 0;

    inline SL_ColorDataType type() const noexcept;

    inline const SL_Texture& texture() const noexcept;

    inline SL_Texture& texture() noexcept;
};



/*-------------------------------------
 * Retrieve the native color type of the backbuffer.
-------------------------------------*/
inline SL_ColorDataType SL_WindowBuffer::type() const noexcept
{
    return SL_ColorDataType::SL_COLOR_RGBA_8U;
}



/*-------------------------------------
 * Retrieve the texture contained within the backbuffer.
-------------------------------------*/
inline const SL_Texture& SL_WindowBuffer::texture() const noexcept
{
    return mTexture;
}



/*-------------------------------------
 * Retrieve the texture contained within the backbuffer.
-------------------------------------*/
inline SL_Texture& SL_WindowBuffer::texture() noexcept
{
    return mTexture;
}



#endif /* SL_WINDOW_BUFFER_HPP */
