
#ifndef SR_WINDOW_BUFFER_HPP
#define SR_WINDOW_BUFFER_HPP

#include <cstdint> // uint8_t

#include "lightsky/utils/Pointer.h"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
template <typename color_type>
struct SR_ColorRGBAType;
class SR_RenderWindow;
class SR_Texture;



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SR_WindowBuffer
{
  public:
    static ls::utils::Pointer<SR_WindowBuffer> create() noexcept;

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

    virtual const SR_ColorRGBAType<uint8_t>* buffer() const noexcept = 0;

    virtual SR_ColorRGBAType<uint8_t>* buffer() noexcept = 0;
};



#endif /* SR_WINDOW_BUFFER_HPP */
