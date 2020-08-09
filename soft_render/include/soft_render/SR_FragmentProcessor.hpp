
#ifndef SR_FRAGMENT_PROCESSOR_HPP
#define SR_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "soft_render/SR_Mesh.hpp" // SR_RenderMode



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

struct SR_FragCoord; // SR_ShaderProcessor.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp
class SR_Framebuffer;
class SR_Shader;
class SR_Texture;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
 *
 * Point rasterization will divide the output framebuffer into equal parts,
 * so all threads will be assigned a specific region of the screen.
 *
 * Line rasterization will
-----------------------------------------------------------------------------*/
struct SR_FragmentProcessor
{
    // 16 bits
    uint16_t mThreadId;

    // 32 bits
    SR_RenderMode mMode;

    // 32-bits
    uint32_t mNumProcessors;

    // 64 bits
    uint_fast32_t mNumBins;

    // 256 bits
    const SR_Shader* mShader;
    SR_Framebuffer* mFbo;
    const SR_FragmentBin* mBins;
    ls::math::vec4* mVaryings;
    SR_FragCoord* mQueues;

    // 432 bits = 54 bytes

    template <typename depth_type>
    void render_point(
        const uint32_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    template <typename depth_type>
    void render_line(
        const uint32_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    template <typename depth_type>
    void render_wireframe(const SR_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle(const SR_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void render_triangle_simd(const SR_Texture* depthBuffer) const noexcept;

    template <typename depth_type>
    void flush_fragments(const SR_FragmentBin* pBin, uint32_t numQueuedFrags, const SR_FragCoord* outCoords) const noexcept;

    void execute() noexcept;
};



extern template void SR_FragmentProcessor::render_point<ls::math::half>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SR_FragmentProcessor::render_point<float>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SR_FragmentProcessor::render_point<double>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;

extern template void SR_FragmentProcessor::render_line<ls::math::half>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SR_FragmentProcessor::render_line<float>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
extern template void SR_FragmentProcessor::render_line<double>(const uint32_t, SR_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;

extern template void SR_FragmentProcessor::render_wireframe<ls::math::half>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_wireframe<float>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_wireframe<double>(const SR_Texture*) const noexcept;

extern template void SR_FragmentProcessor::render_triangle<ls::math::half>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_triangle<float>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_triangle<double>(const SR_Texture*) const noexcept;

extern template void SR_FragmentProcessor::render_triangle_simd<ls::math::half>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_triangle_simd<float>(const SR_Texture*) const noexcept;
extern template void SR_FragmentProcessor::render_triangle_simd<double>(const SR_Texture*) const noexcept;

extern template void SR_FragmentProcessor::flush_fragments<ls::math::half>(const SR_FragmentBin*, uint32_t, const SR_FragCoord*) const noexcept;
extern template void SR_FragmentProcessor::flush_fragments<float>(const SR_FragmentBin*, uint32_t, const SR_FragCoord*) const noexcept;
extern template void SR_FragmentProcessor::flush_fragments<double>(const SR_FragmentBin*, uint32_t, const SR_FragCoord*) const noexcept;



#endif /* SR_FRAGMENT_PROCESSOR_HPP */
