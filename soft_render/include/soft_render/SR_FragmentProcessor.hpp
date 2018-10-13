
#ifndef SR_FRAGMENT_PROCESSOR_HPP
#define SR_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "soft_render/SR_Mesh.hpp"

#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_UniformBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace utils
    {
        template<class WorkerTaskType>
        class Worker;
    } // end utils namespace
} // end ls namespace

struct SR_FragCoord; // SR_ShaderProcessor.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp
struct SR_ShaderProcessor;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_FragmentProcessor
{
    // 192 bits
    const SR_Shader* mShader;
    SR_Framebuffer* mFbo;
    SR_FragmentBin* mBins;
    SR_FragCoord* mQueues;

    // 128 bits
    uint64_t mBinId;
    uint64_t mNumBins;

    // 128-bits
    float mFboX0;
    float mFboY0;
    float mFboX1;
    float mFboY1;

    // 16 bits
    SR_RenderMode mMode;

    // 464 bits = 58 bytes

    void render_point(SR_Framebuffer* const fbo) noexcept;

    void render_line(SR_Framebuffer* const fbo, ls::math::vec4* outVaryings) noexcept;

    void render_triangle(const SR_Texture* depthBuffer, ls::math::vec4* outVaryings) const noexcept;

    void flush_fragments(uint_fast32_t numQueuedFrags, const SR_FragCoord* outCoords, ls::math::vec4* outVaryings) const noexcept;

    void execute() noexcept;
};



#endif /* SR_FRAGMENT_PROCESSOR_HPP */
