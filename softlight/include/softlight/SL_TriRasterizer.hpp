
#ifndef SL_TRI_RASTERIZER_HPP
#define SL_TRI_RASTERIZER_HPP

#include "softlight/SL_Config.hpp"
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

struct SL_FragCoord; // SL_ShaderProcessor.hpp
struct SL_FragmentBin; // SL_ShaderProcessor.hpp
class SL_Framebuffer;
struct SL_Shader;
struct SL_TextureView;

struct SL_DepthFuncLT;
struct SL_DepthFuncLE;
struct SL_DepthFuncGT;
struct SL_DepthFuncGE;
struct SL_DepthFuncEQ;
struct SL_DepthFuncNE;
struct SL_DepthFuncOFF;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
 *
 * Point rasterization will divide the output framebuffer into equal parts,
 * so all threads will be assigned a specific region of the screen.
 *
 * Line rasterization will
-----------------------------------------------------------------------------*/
struct SL_TriRasterizer final : public SL_FragmentProcessor
{
    template <class DepthCmpFunc, typename depth_type>
    void render_wireframe(const SL_TextureView& depthBuffer) const noexcept;

    template <class DepthCmpFunc, typename depth_type>
    void render_triangle(const SL_TextureView& depthBuffer) const noexcept;

    template <class DepthCmpFunc, typename depth_type>
    void render_triangle_simd(const SL_TextureView& depthBuffer) const noexcept;

    template <class DepthCmpFunc>
    void dispatch_bins() noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
extern template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



#endif /* SL_TRI_RASTERIZER_HPP */
