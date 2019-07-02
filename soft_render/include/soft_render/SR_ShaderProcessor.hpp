
#ifndef SR_SHADER_PROCESSOR_HPP
#define SR_SHADER_PROCESSOR_HPP

#include <array>
#include <cstdlib> // std::size_t

#include "lightsky/utils/Pointer.h" // Pointer, AlignedPointerDeleter
#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/vec4.h"
#include "lightsky/math/vec3.h"

#include "soft_render/SR_Mesh.hpp"



struct SR_BlitProcessor;
class SR_Context;
class SR_Framebuffer;
class SR_Shader;
struct SR_ShaderProcessor;
class SR_Texture;
class SR_WindowBuffer;



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SR_ShaderLimits
{
    SR_SHADER_MAX_WORLD_COORDS    = 3,
    SR_SHADER_MAX_SCREEN_COORDS   = 3,
    SR_SHADER_MAX_VARYING_VECTORS = 4,
    SR_SHADER_MAX_FRAG_OUTPUTS    = 4,

    // Maximum number of fragments that get queued before being placed on a
    // framebuffer.
    SR_SHADER_MAX_FRAG_QUEUES     = 4096,

    // Maximum number of vertex groups which get binned before being sent to a
    // fragment processor. About 16 MB (when multiplied by
    // sizeof(SR_FragmentBin)).
    SR_SHADER_MAX_FRAG_BINS       = 65535
};



/*-----------------------------------------------------------------------------
 * Intermediate Fragment Storage for Binning
-----------------------------------------------------------------------------*/
struct alignas(sizeof(ls::math::vec4)) SR_FragmentBin
{
    // 4-byte floats * 4-element vector * 3 vectors-per-tri = 48 bytes
    ls::math::vec4 mScreenCoords[SR_SHADER_MAX_SCREEN_COORDS];

    // 4-byte floats * 4-element vector * 3 barycentric coordinates = 48 bytes
    ls::math::vec4 mBarycentricCoords[3];

    // 4-byte floats * 4-element vector * 3-vectors-per-tri * 4 varyings-per-vertex = 192 bytes
    ls::math::vec4 mVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    // 256 bytes = 2048 bits
};

// Comparison operator for sorting blended fragments by depth
inline LS_INLINE bool operator < (const SR_FragmentBin& a, const SR_FragmentBin& b)
{
    return *reinterpret_cast<const int32_t*>(a.mScreenCoords[0].v+3) < *reinterpret_cast<const int32_t*>(b.mScreenCoords[0].v+3);
}

// Comparison operator for sorting fragments by depth
inline LS_INLINE bool operator > (const SR_FragmentBin& a, const SR_FragmentBin& b)
{
    return *reinterpret_cast<const int32_t*>(a.mScreenCoords[0].v+3) > *reinterpret_cast<const int32_t*>(b.mScreenCoords[0].v+3);
}



/*-----------------------------------------------------------------------------
 * Helper structure to put a pixel on the screen
-----------------------------------------------------------------------------*/
struct SR_FragCoord
{
    ls::math::vec4 bc[SR_SHADER_MAX_FRAG_QUEUES]; // 32*4
    ls::math::vec4 xyzw[SR_SHADER_MAX_FRAG_QUEUES]; // 32*4
    uint32_t       xy[SR_SHADER_MAX_FRAG_QUEUES]; // 32-bit bitmask of x & y

    // 256 bits / 32 bytes
};



/*-----------------------------------------------------------------------------
 * Wrapper around Shader Processors which allows operations on binned
 * Fragments
-----------------------------------------------------------------------------*/
class SR_ProcessorPool
{
  public:
    typedef ls::utils::Worker<SR_ShaderProcessor> Worker;
    typedef ls::utils::WorkerThread<SR_ShaderProcessor> ThreadedWorker;

  private:
    std::atomic_uint_fast64_t mFragSemaphore;

    std::atomic_uint_fast64_t mShadingSemaphore;

    std::atomic_uint_fast64_t mBinsUsed;

    ls::utils::Pointer<uint32_t[], ls::utils::AlignedDeleter> mBinIds;

    ls::utils::Pointer<SR_FragmentBin[], ls::utils::AlignedDeleter> mFragBins;

    ls::utils::Pointer<SR_FragCoord[], ls::utils::AlignedDeleter> mFragQueues;

    ls::utils::Pointer<Worker*[], ls::utils::AlignedDeleter> mThreads;

    unsigned mNumThreads;

  public:
    ~SR_ProcessorPool() noexcept;

    SR_ProcessorPool(unsigned numThreads = 1) noexcept;

    SR_ProcessorPool(const SR_ProcessorPool&) noexcept;

    SR_ProcessorPool(SR_ProcessorPool&&) noexcept;

    SR_ProcessorPool& operator=(const SR_ProcessorPool&) noexcept;

    SR_ProcessorPool& operator=(SR_ProcessorPool&&) noexcept;

    unsigned num_threads() const noexcept;

    unsigned num_threads(unsigned n) noexcept;

    void flush() noexcept;

    void wait() noexcept;

    void execute() noexcept;

    void run_shader_processors(const SR_Context* c, const SR_Mesh* m, const SR_Shader* s, SR_Framebuffer* fbo) noexcept;

    void clear_fragment_bins() noexcept;

    void run_blit_processors(
        const SR_Texture* inTex,
        SR_Texture* outTex,
        uint16_t srcX0,
        uint16_t srcY0,
        uint16_t srcX1,
        uint16_t srcY1,
        uint16_t dstX0,
        uint16_t dstY0,
        uint16_t dstX1,
        uint16_t dstY1
    ) noexcept;
};



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
inline unsigned SR_ProcessorPool::num_threads() const noexcept
{
    return mNumThreads;
}



/*-------------------------------------
 * Run the processor threads
-------------------------------------*/
inline void SR_ProcessorPool::execute() noexcept
{
    this->flush();
    this->wait();
}



/*-------------------------------------
 * Remove all bins from potential processing
-------------------------------------*/
inline void SR_ProcessorPool::clear_fragment_bins() noexcept
{
    for (uint16_t i = 0; i < mNumThreads; ++i)
    {
        //mFragBins[i].clear();
        mBinsUsed.store(0);
    }
}



#endif /* SR_SHADER_PROCESSOR_HPP */
