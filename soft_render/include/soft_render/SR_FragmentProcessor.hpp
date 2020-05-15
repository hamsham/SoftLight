
#ifndef SR_FRAGMENT_PROCESSOR_HPP
#define SR_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "soft_render/SR_Mesh.hpp" // SR_RenderMode



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
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
    uint_fast64_t mNumBins;

    // 256 bits
    const SR_Shader* mShader;
    SR_Framebuffer* mFbo;
    SR_FragmentBin* mBins;
    ls::math::vec4* mVaryings;
    SR_FragCoord* mQueues;

    // 432 bits = 54 bytes

    void render_point(
        const uint_fast64_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    void render_line(
        const uint_fast64_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    void render_wireframe(const SR_Texture* depthBuffer) const noexcept;

    void render_triangle(const SR_Texture* depthBuffer) const noexcept;

    void flush_fragments(const SR_FragmentBin* pBin, uint_fast32_t numQueuedFrags, const SR_FragCoord* outCoords) const noexcept;

    void execute() noexcept;
};



#endif /* SR_FRAGMENT_PROCESSOR_HPP */
