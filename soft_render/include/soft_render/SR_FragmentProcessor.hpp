
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

template<typename color_t>
struct SR_ColorRGBAType;

class SR_Context;
struct SR_FragCoord;
struct SR_ShaderProcessor;



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SR_ShaderLimits
{
    SR_SHADER_MAX_WORLD_COORDS    = 3,
    SR_SHADER_MAX_SCREEN_COORDS   = 3,
    SR_SHADER_MAX_VARYING_VECTORS = 4,
    SR_SHADER_MAX_FRAG_OUTPUTS    = 4,
    SR_SHADER_MAX_FRAG_QUEUES     = 32
};



/*-----------------------------------------------------------------------------
 * Intermediate Fragment Storage for Binning
-----------------------------------------------------------------------------*/
struct alignas(sizeof(ls::math::vec4)) SR_FragmentBin
{
    // 4-byte floats * 4-element vector * 3 vectors-per-tri = 48 bytes
    ls::math::vec4 mScreenCoords[SR_SHADER_MAX_SCREEN_COORDS];

    // 4-byte floats * 4-element vector = 16 bytes
    ls::math::vec4 mPerspDivide;

    // 4-byte floats * 4-element vector * 3-vectors-per-tri * 4 varyings-per-vertex = 192 bytes
    ls::math::vec4 mVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    // 256 bytes = 2048 bits
};

constexpr bool operator > (const SR_FragmentBin& a, const SR_FragmentBin& b)
{
    return a.mPerspDivide[0] > b.mPerspDivide[0];
}

constexpr bool operator < (const SR_FragmentBin& a, const SR_FragmentBin& b)
{
    return a.mPerspDivide[0] < b.mPerspDivide[0];
}



/*-----------------------------------------------------------------------------
 * Helper structure to put a pixel on the screen
-----------------------------------------------------------------------------*/
struct alignas(sizeof(ls::math::vec4)) SR_FragCoord
{
    ls::math::vec4 bc; // 32*4
    uint16_t       x; // 16
    uint16_t       y; // 16
    float          zf; // 32

    // 192 bits / 24 bytes
};



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_FragmentProcessor
{
    // 192 bits
    const SR_Shader* mShader;
    SR_Framebuffer* mFbo;
    SR_FragmentBin* mBins;

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

    void render_point(
        SR_Framebuffer* const    fbo,
        SR_ColorRGBAType<float>* pOutputs
    ) noexcept;

    void render_line(
        SR_Framebuffer* const    fbo,
        SR_ColorRGBAType<float>* pOutputs,
        ls::math::vec4*          outVaryings
    ) noexcept;

    void render_triangle(
        const SR_Texture*        depthBuffer,
        SR_ColorRGBAType<float>* pOutputs,
        ls::math::vec4*          outVaryings
    ) noexcept;

    void flush_fragments(
        uint_fast32_t            numQueuedFrags,
        const SR_FragCoord*      outCoords,
        SR_ColorRGBAType<float>* pOutputs,
        ls::math::vec4*          outVaryings) noexcept;

    void execute() noexcept;
};



#endif /* SR_FRAGMENT_PROCESSOR_HPP */
