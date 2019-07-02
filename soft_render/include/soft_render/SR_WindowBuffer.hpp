
#ifndef SR_WINDOW_BUFFER_HPP
#define SR_WINDOW_BUFFER_HPP

#include <cstdint> // uint8_t

#include "lightsky/utils/Pointer.h"

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

class SR_RenderWindow;
class SR_Texture;



/**----------------------------------------------------------------------------
 * @brief The SR_WindowBuffer class encapsulates the native windowing system's
 * backbuffer. This class gets passed into an SR_RenderWindow for blitting to
 * the front buffer.
-----------------------------------------------------------------------------*/
class SR_WindowBuffer
{
    friend class SR_Context;
    friend class SR_ShaderProcessor;

  public:
    static ls::utils::Pointer<SR_WindowBuffer> create() noexcept;

  protected:
    SR_Texture mTexture;

  public:
    virtual ~SR_WindowBuffer() noexcept = 0;

    SR_WindowBuffer() noexcept;

    SR_WindowBuffer(const SR_WindowBuffer&) = delete;

    SR_WindowBuffer(SR_WindowBuffer&&) noexcept;

    SR_WindowBuffer& operator=(const SR_WindowBuffer&) = delete;

    SR_WindowBuffer& operator=(SR_WindowBuffer&&) noexcept;

    virtual int init(SR_RenderWindow& w, unsigned width, unsigned height) noexcept = 0;

    virtual int terminate() noexcept = 0;

    virtual unsigned width() const noexcept = 0;

    virtual unsigned height() const noexcept = 0;

    virtual const void* native_handle() const noexcept = 0;

    virtual void* native_handle() noexcept = 0;

    virtual const ls::math::vec4_t<uint8_t>* buffer() const noexcept = 0;

    virtual ls::math::vec4_t<uint8_t>* buffer() noexcept = 0;

    inline SR_ColorDataType type() const noexcept;

    inline const SR_Texture& texture() const noexcept;
};



/*-------------------------------------
 * Retrieve the native color type of the backbuffer.
-------------------------------------*/
inline SR_ColorDataType SR_WindowBuffer::type() const noexcept
{
    return SR_ColorDataType::SR_COLOR_RGBA_8U;
}



/*-------------------------------------
 * Retrieve the texture contained within the backbuffer.
-------------------------------------*/
inline const SR_Texture& SR_WindowBuffer::texture() const noexcept
{
    return mTexture;
}



#endif /* SR_WINDOW_BUFFER_HPP */
