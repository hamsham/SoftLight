
#ifndef SL_POINT_RASTERIZER_HPP
#define SL_POINT_RASTERIZER_HPP

#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_ViewportState.hpp"



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
struct SL_Shader;
class SL_Texture;

struct SL_DepthFuncLT;
struct SL_DepthFuncLE;
struct SL_DepthFuncGT;
struct SL_DepthFuncGE;
struct SL_DepthFuncEQ;
struct SL_DepthFuncNE;
struct SL_DepthFuncOFF;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing for points.
-----------------------------------------------------------------------------*/
struct SL_PointRasterizer final : public SL_FragmentProcessor
{
    template <class DepthCmpFunc, typename depth_type>
    void render_point(SL_Framebuffer* const fbo) noexcept;

    template <class DepthCmpFunc>
    void dispatch_bins() noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
extern template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



extern template void SL_PointRasterizer::render_point<SL_DepthFuncLT, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncLT, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncLT, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncLE, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncLE, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncLE, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncGT, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncGT, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncGT, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncGE, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncGE, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncGE, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncNE, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncNE, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncNE, double>(SL_Framebuffer* const) noexcept;

extern template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, ls::math::half>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, float>(SL_Framebuffer* const) noexcept;
extern template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, double>(SL_Framebuffer* const) noexcept;



#endif /* SL_POINT_RASTERIZER_HPP */
