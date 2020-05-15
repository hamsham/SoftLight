
#ifndef SR_VERTEX_PROCESSOR_HPP
#define SR_VERTEX_PROCESSOR_HPP

#include <atomic>

#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_ShaderUtil.hpp"



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



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_VertexProcessor
{
    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    std::atomic_int_fast64_t* mFragProcessors;
    std::atomic_uint_fast64_t* mBusyProcessors;

    // 96-192 bits
    const SR_Shader*  mShader;
    const SR_Context* mContext;
    SR_Framebuffer*   mFbo;

    // 64-128 bits
    size_t mNumMeshes;
    size_t mNumInstances;

    // 32 bits
    SR_RenderMode mRenderMode;

    // 32-64 bits
    const SR_Mesh* mMeshes;

    // 32-64 bits
    uint32_t* mBinsUsed;
    uint32_t mHaveHighPoly;

    // 96-192 bits
    SR_FragmentBin* mFragBins;
    ls::math::vec4_t<float>* mVaryings;
    SR_FragCoord* mFragQueues;

    // 448 bits (56 bytes) in 32-bit mode
    // 800 bits (100 bytes) in 64-bit mode
    // Padding not included

    void flush_bins() const noexcept;

    template <int renderMode, int vertCount>
    void push_bin(
        float fboW,
        float fboH,
        ls::math::vec4_t<float>* const screenCoords,
        const ls::math::vec4_t<float>* varyings
    ) const noexcept;

    void clip_and_process_tris(
        float fboW,
        float fboH,
        ls::math::vec4_t<float> vertCoords[SR_SHADER_MAX_SCREEN_COORDS],
        ls::math::vec4_t<float> pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS]) noexcept;

    void process_points(const SR_Mesh& m, size_t instanceId) noexcept;

    void process_lines(const SR_Mesh& m, size_t instanceId) noexcept;

    void process_tris(const SR_Mesh& m, size_t instanceId) noexcept;

    void execute() noexcept;
};


extern template
void SR_VertexProcessor::push_bin<RENDER_MODE_POINTS, 1>(float, float, ls::math::vec4_t<float>* const, const ls::math::vec4_t<float>*) const noexcept;

extern template
void SR_VertexProcessor::push_bin<RENDER_MODE_LINES, 2>(float, float, ls::math::vec4_t<float>* const, const ls::math::vec4_t<float>*) const noexcept;

extern template
void SR_VertexProcessor::push_bin<RENDER_MODE_TRIANGLES, 3>(float, float, ls::math::vec4_t<float>* const, const ls::math::vec4_t<float>*) const noexcept;





#endif /* SR_VERTEX_PROCESSOR_HPP */
