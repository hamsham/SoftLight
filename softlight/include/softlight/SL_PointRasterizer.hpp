
#ifndef SL_POINT_RASTERIZER_HPP
#define SL_POINT_RASTERIZER_HPP

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
 * Encapsulation of fragment processing for points.
-----------------------------------------------------------------------------*/
struct SL_PointRasterizer final : public SL_FragmentProcessor
{
    template <typename depth_type>
    void render_point(const uint32_t binId, SL_Framebuffer* const fbo) noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_PointRasterizer::render_point<ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<float>(const uint32_t, SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<double>(const uint32_t, SL_Framebuffer* const) noexcept;



#endif /* SL_POINT_RASTERIZER_HPP */
