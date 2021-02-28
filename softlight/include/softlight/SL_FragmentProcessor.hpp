
#ifndef SL_FRAGMENT_PROCESSOR_HPP
#define SL_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "softlight/SL_Mesh.hpp" // SL_RenderMode



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace math
    {
        struct Half;
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
struct SL_FragmentProcessor
{
    // 16 bits
    uint16_t mThreadId;

    // 32 bits
    SL_RenderMode mMode;

    // 32-bits
    uint32_t mNumProcessors;

    // 64 bits
    uint_fast32_t mNumBins;

    // 256 bits
    const SL_Shader* mShader;
    SL_Framebuffer* mFbo;
    SL_BinCounter<uint32_t>* mBinIds;
    const SL_FragmentBin* mBins;
    SL_FragCoord* mQueues;

    // 432 bits = 54 bytes

    template <typename depth_type>
    void render_point(const uint32_t binId, SL_Framebuffer* const fbo) noexcept;

    template <typename depth_type>
    void render_line(
        const uint32_t binId,
        SL_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    template <typename depth_type>
    void render_wireframe(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle_simd(const SL_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void flush_fragments(const SL_FragmentBin* pBin, uint32_t numQueuedFrags, const SL_FragCoord* outCoords) const noexcept;

    void execute() noexcept;
};



extern template void SL_FragmentProcessor::render_point<ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
extern template void SL_FragmentProcessor::render_point<float>(const uint32_t, SL_Framebuffer* const) noexcept;
extern template void SL_FragmentProcessor::render_point<double>(const uint32_t, SL_Framebuffer* const) noexcept;

extern template void SL_FragmentProcessor::render_line<ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SL_FragmentProcessor::render_line<float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SL_FragmentProcessor::render_line<double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;

extern template void SL_FragmentProcessor::render_wireframe<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_wireframe<float>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_wireframe<double>(const SL_Texture*) const noexcept;

extern template void SL_FragmentProcessor::render_triangle<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_triangle<float>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_triangle<double>(const SL_Texture*) const noexcept;

extern template void SL_FragmentProcessor::render_triangle_simd<ls::math::half>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_triangle_simd<float>(const SL_Texture*) const noexcept;
extern template void SL_FragmentProcessor::render_triangle_simd<double>(const SL_Texture*) const noexcept;

extern template void SL_FragmentProcessor::flush_fragments<ls::math::half>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
extern template void SL_FragmentProcessor::flush_fragments<float>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
extern template void SL_FragmentProcessor::flush_fragments<double>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;



#endif /* SL_FRAGMENT_PROCESSOR_HPP */
