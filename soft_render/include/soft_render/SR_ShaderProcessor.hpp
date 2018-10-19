
#ifndef SR_SHADER_PROCESSOR_HPP
#define SR_SHADER_PROCESSOR_HPP

#include <array>
#include <cstdlib> // std::size_t

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/vec4.h"

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
    SR_SHADER_MAX_FRAG_QUEUES     = 1024,

    // Maximum number of vertex groups which get binned before being sent to a
    // fragment processor. About 16 MB per thread
    // (multiplied by sizeof(SR_FragmentBin)).
    SR_SHADER_MAX_FRAG_BINS       = 32768
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

// Comparison operator for sorting fragments by depth
constexpr bool operator > (const SR_FragmentBin& a, const SR_FragmentBin& b)
{
    return a.mPerspDivide[0] > b.mPerspDivide[0];
}



/*-----------------------------------------------------------------------------
 * Helper structure to put a pixel on the screen
-----------------------------------------------------------------------------*/
struct alignas(sizeof(ls::math::vec4)) SR_FragCoord
{
    ls::math::vec4 bc; // 32*4
    uint16_t       x; // 16
    uint16_t       y; // 16
    float          xf; // 32
    float          yf; // 32
    float          zf; // 32

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

    struct ProcessorDeleter
    {
        inline void operator()(void* p)
        {
            #ifdef LS_ARCH_X86
                _mm_free(p);
            #else
                free(p);
            #endif
        }
    };

  private:
    std::atomic_uint_fast32_t mFragSemaphore;

    ls::utils::Pointer<std::atomic_uint_fast32_t[], ProcessorDeleter> mBinsUsed;

    ls::utils::Pointer<std::array<SR_FragmentBin, SR_SHADER_MAX_FRAG_BINS>[], ProcessorDeleter> mFragBins;

    ls::utils::Pointer<std::array<SR_FragCoord, SR_SHADER_MAX_FRAG_QUEUES>[], ProcessorDeleter> mFragQueues;

    ls::utils::Pointer<Worker*[], ProcessorDeleter> mThreads;

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

    void run_blit_processors(const SR_Texture* t, SR_WindowBuffer* b) noexcept;
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
        mBinsUsed[i].store(0);
    }
}



#endif /* SR_SHADER_PROCESSOR_HPP */
