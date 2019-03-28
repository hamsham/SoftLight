
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
class SR_Shader; // SR_Shader.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_VertexProcessor
{
    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 128 bits
    std::atomic_uint_fast64_t* mFragProcessors;
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
    SR_FragmentBin* mFragBins;
    SR_FragCoord* mFragQueues;

    // 768 bits (96 bytes) max, padding not included

    void flush_fragments() const noexcept;

    void push_fragments(
        float fboW,
        float fboH,
        ls::math::vec4_t<float>* const screenCoords,
        ls::math::vec4_t<float>* const worldCoords,
        const ls::math::vec4_t<float>* varyings
    ) const noexcept;

    void execute() noexcept;
};



#endif /* SR_VERTEX_PROCESSOR_HPP */
