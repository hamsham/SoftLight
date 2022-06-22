
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
 * Encapsulation of fragment processing for lines.
-----------------------------------------------------------------------------*/
struct SL_LineRasterizer final : public SL_FragmentProcessor
{
    template <typename depth_type>
    void flush_fragments(const SL_FragmentBin& bin, uint_fast32_t numQueuedFrags, SL_FragCoord* const outCoords) const noexcept;

    template <class DepthCmpFunc, typename depth_type>
    void render_line(const SL_FragmentBin& bin, SL_Framebuffer* const fbo) noexcept;

    template <class DepthCmpFunc>
    void dispatch_bins() noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_LineRasterizer::flush_fragments<ls::math::half>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
extern template void SL_LineRasterizer::flush_fragments<float>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
extern template void SL_LineRasterizer::flush_fragments<double>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;



extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
extern template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



extern template void SL_LineRasterizer::render_line<SL_DepthFuncLT, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncLT, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncLT, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncLE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncLE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncLE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncGT, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncGT, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncGT, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncGE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncGE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncGE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncNE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncNE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncNE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

extern template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
extern template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;



#endif /* SL_LINE_RASTERIZER_HPP */
