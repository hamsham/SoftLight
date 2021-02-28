
#ifndef SL_TRI_RASTERIZER_HPP
#define SL_TRI_RASTERIZER_HPP

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
 * Encapsulation of fragment processing on another thread.
 *
 * Point rasterization will divide the output framebuffer into equal parts,
 * so all threads will be assigned a specific region of the screen.
 *
 * Line rasterization will
-----------------------------------------------------------------------------*/
struct SL_TriRasterizer final : public SL_FragmentProcessor
{
    template <typename depth_type>
    void render_wireframe(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle_simd(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void flush_fragments(const SL_FragmentBin* pBin, uint32_t numQueuedFrags, const SL_FragCoord* outCoords) const noexcept;

    virtual void execute() noexcept override;
};



extern template void SL_TriRasterizer::render_wireframe<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<float>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_wireframe<double>(const SL_Texture*) const noexcept;

extern template void SL_TriRasterizer::render_triangle<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_triangle<float>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_triangle<double>(const SL_Texture*) const noexcept;

extern template void SL_TriRasterizer::render_triangle_simd<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<float>(const SL_Texture*) const noexcept;
extern template void SL_TriRasterizer::render_triangle_simd<double>(const SL_Texture*) const noexcept;

extern template void SL_TriRasterizer::flush_fragments<ls::math::half>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
extern template void SL_TriRasterizer::flush_fragments<float>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
extern template void SL_TriRasterizer::flush_fragments<double>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;



#endif /* SL_TRI_RASTERIZER_HPP */
