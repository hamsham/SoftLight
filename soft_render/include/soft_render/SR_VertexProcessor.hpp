
#ifndef SR_VERTEX_PROCESSOR_HPP
#define SR_VERTEX_PROCESSOR_HPP

#include <array>
#include <atomic>

#include "soft_render/SR_Mesh.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace math
    {
        template<typename>
        union vec4_t;
    } // end math namespace
} // end ls namespace

class SR_Context; // SR_Context.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp
struct SR_FragCoord;
class SR_Framebuffer;
class SR_Shader; // SR_Shader.hpp



/*-----------------------------------------------------------------------------
 * Internal Enums
-----------------------------------------------------------------------------*/
enum SR_ClipStatus
{
    SR_TRIANGLE_NOT_VISIBLE,
    SR_TRIANGLE_PARTIALLY_VISIBLE,
    SR_TRIANGLE_FULLY_VISIBLE
};

enum SR_ClipPlane
{
    SR_CLIP_PLANE_LEFT   = 0x01,
    SR_CLIP_PLANE_RIGHT  = 0x02,
    SR_CLIP_PLANE_TOP    = 0x04,
    SR_CLIP_PLANE_BOTTOM = 0x08,
    SR_CLIP_PLANE_NEAR   = 0x10,
    SR_CLIP_PLANE_FAR    = 0x20
};



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_VertexProcessor
{
    // 32 bits
    uint16_t mThreadId;
    int16_t mNumThreads;

    // 128 bits
    std::atomic_int_fast64_t* mFragProcessors;
    std::atomic_uint_fast64_t* mBusyProcessors;

    // 128-256 bits
    const SR_Shader*  mShader;
    const SR_Context* mContext;
    SR_Framebuffer*   mFbo;

    // 32-bits
    uint16_t mFboW;
    uint16_t mFboH;

    // 128 bits
    SR_Mesh mMesh;

    // 64-128 bits
    std::atomic_uint_fast64_t* mBinsUsed;
    uint32_t* mBinIds;
    SR_FragmentBin* mFragBins;
    ls::math::vec4_t<float>* mVaryings;
    SR_FragCoord* mFragQueues;

    // 768 bits (96 bytes) max, padding not included

    void flush_bins() const noexcept;

    void push_bin(
        float fboW,
        float fboH,
        ls::math::vec4_t<float>* const screenCoords,
        const ls::math::vec4_t<float>* varyings
    ) const noexcept;

    void clip_and_process_tris(
        ls::math::vec4_t<float> vertCoords[SR_SHADER_MAX_SCREEN_COORDS],
        ls::math::vec4_t<float> pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS]) noexcept;

    void execute() noexcept;
};



#endif /* SR_VERTEX_PROCESSOR_HPP */
