
#ifndef SL_LINE_RASTERIZER_HPP
#define SL_LINE_RASTERIZER_HPP

#include "softlight/SL_FragmentProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
struct half;
}
}

template <typename data_t>
union SL_BinCounter;

struct SL_FragCoord; // SL_ShaderProcessor.hpp
struct SL_FragmentBin; // SL_ShaderProcessor.hpp
class SL_Framebuffer;
class SL_Shader;
class SL_Texture;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing for lines.
-----------------------------------------------------------------------------*/
struct SL_LineRasterizer final : public SL_FragmentProcessor
{
    template <typename depth_type>
    void render_line(const uint32_t binId, SL_Framebuffer* const fbo, const ls::math::vec4_t<int32_t> dimens) noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_LineRasterizer::render_line<ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SL_LineRasterizer::render_line<float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SL_LineRasterizer::render_line<double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;



#endif /* SL_LINE_RASTERIZER_HPP */
