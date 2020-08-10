
#ifndef SR_PROCESSORPOOL_HPP
#define SR_PROCESSORPOOL_HPP

#include <array>
#include <atomic>

#include "lightsky/utils/Pointer.h" // Pointer, AlignedPointerDeleter

#include "soft_render/SR_ShaderUtil.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{
template <class T>
class Worker;

template <class T>
class WorkerThread;
} // utils namespace

namespace math
{
template <typename T>
union vec4_t;
} // math namespace
} // ls namespace

template <typename data_t>
struct SR_BinCounter;

template <typename data_t>
struct SR_BinCounterAtomic;

class SR_Context;
struct SR_FragCoord;
struct SR_FragmentBin;
class SR_Framebuffer;
struct SR_Mesh;
class SR_Shader;
struct SR_ShaderProcessor;
class SR_Texture;



/*-----------------------------------------------------------------------------
 * Wrapper around Shader Processors which allows operations on binned
 * Fragments
-----------------------------------------------------------------------------*/
class SR_ProcessorPool
{
  public:
    typedef ls::utils::WorkerThread<SR_ShaderProcessor> ThreadedWorker;

  private:
    SR_BinCounterAtomic<uint_fast64_t> mFragSemaphore;

    SR_BinCounterAtomic<uint_fast64_t> mShadingSemaphore;

    ls::utils::Pointer<SR_BinCounterAtomic<int32_t>[], ls::utils::AlignedDeleter> mBinsReady;

    ls::utils::Pointer<SR_BinCounter<uint32_t>[], ls::utils::AlignedDeleter> mBinsUsed;

    ls::utils::Pointer<SR_FragmentBin[], ls::utils::AlignedDeleter> mFragBins;

    ls::utils::Pointer<ls::math::vec4_t<float>[], ls::utils::AlignedDeleter> mVaryings;

    ls::utils::Pointer<SR_FragCoord[], ls::utils::AlignedDeleter> mFragQueues;

    ls::utils::Pointer<ThreadedWorker[], ls::utils::AlignedDeleter> mWorkers;

    unsigned mNumThreads;

  public:
    ~SR_ProcessorPool() noexcept;

    SR_ProcessorPool(unsigned numThreads = 1) noexcept;

    SR_ProcessorPool(const SR_ProcessorPool&) noexcept;

    SR_ProcessorPool(SR_ProcessorPool&&) noexcept;

    SR_ProcessorPool& operator=(const SR_ProcessorPool&) noexcept;

    SR_ProcessorPool& operator=(SR_ProcessorPool&&) noexcept;

    unsigned concurrency() const noexcept;

    unsigned concurrency(unsigned n) noexcept;

    void flush() noexcept;

    void wait() noexcept;

    void execute() noexcept;

    void run_shader_processors(const SR_Context& c, const SR_Mesh& m, size_t numInstances, const SR_Shader& s, SR_Framebuffer& fbo) noexcept;

    void run_shader_processors(const SR_Context& c, const SR_Mesh* meshes, size_t numMeshes, const SR_Shader& s, SR_Framebuffer& fbo) noexcept;

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

    void run_clear_processors(const void* inColor, SR_Texture* outTex) noexcept;

    void run_clear_processors(const void* inColor, const void* depth, SR_Texture* colorBuf, SR_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 2>& inColors, const void* depth, const std::array<SR_Texture*, 2>& colorBufs, SR_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 3>& inColors, const void* depth, const std::array<SR_Texture*, 3>& colorBufs, SR_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 4>& inColors, const void* depth, const std::array<SR_Texture*, 4>& colorBufs, SR_Texture* depthBuf) noexcept;
};



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
inline unsigned SR_ProcessorPool::concurrency() const noexcept
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


#endif /* SR_PROCESSORPOOL_HPP */
