
#ifndef SL_PROCESSORPOOL_HPP
#define SL_PROCESSORPOOL_HPP

#include <array>
#include <atomic>

#include "lightsky/utils/Pointer.h" // Pointer, AlignedPointerDeleter

#include "softlight/SL_ShaderUtil.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{
template <class T>
class WorkerThread;
} // utils namespace

namespace math
{
template <typename T>
union vec4_t;
} // math namespace
} // ls namespace

class SL_Context;
struct SL_FragCoord;
struct SL_FragmentBin;
class SL_Framebuffer;
struct SL_Mesh;
struct SL_Shader;
struct SL_ShaderProcessor;
class SL_Texture;



/*-----------------------------------------------------------------------------
 * Wrapper around Shader Processors which allows operations on binned
 * Fragments
-----------------------------------------------------------------------------*/
class SL_ProcessorPool
{
  public:
    typedef ls::utils::WorkerThread<SL_ShaderProcessor> ThreadedWorker;

  private:
    ls::utils::UniqueAlignedPointer<SL_BinCounterAtomic<int_fast64_t>> mFragSemaphore;

    ls::utils::UniqueAlignedPointer<SL_BinCounterAtomic<uint_fast64_t>> mShadingSemaphore;

    ls::utils::UniqueAlignedArray<SL_BinCounter<uint32_t>> mBinIds;

    ls::utils::UniqueAlignedArray<SL_BinCounter<uint32_t>> mTempBinIds;

    ls::utils::UniqueAlignedPointer<SL_BinCounterAtomic<uint32_t>> mBinsUsed;

    ls::utils::UniqueAlignedArray<SL_FragmentBin> mFragBins;

    ls::utils::UniqueAlignedArray<SL_FragCoord> mFragQueues;

    ls::utils::UniqueAlignedArray<ThreadedWorker> mWorkers;

    unsigned mNumThreads;

  public:
    ~SL_ProcessorPool() noexcept;

    SL_ProcessorPool() noexcept;

    SL_ProcessorPool(unsigned numThreads) noexcept;

    SL_ProcessorPool(const SL_ProcessorPool&) noexcept;

    SL_ProcessorPool(SL_ProcessorPool&&) noexcept;

    SL_ProcessorPool& operator=(const SL_ProcessorPool&) noexcept;

    SL_ProcessorPool& operator=(SL_ProcessorPool&&) noexcept;

    unsigned concurrency() const noexcept;

    unsigned concurrency(unsigned n) noexcept;

    void flush() noexcept;

    void wait() noexcept;

    void execute() noexcept;

    void run_shader_processors(const SL_Context& c, const SL_Mesh& m, size_t numInstances, const SL_Shader& s, SL_Framebuffer& fbo) noexcept;

    void run_shader_processors(const SL_Context& c, const SL_Mesh* meshes, size_t numMeshes, const SL_Shader& s, SL_Framebuffer& fbo) noexcept;

    void clear_fragment_bins() noexcept;

    void run_blit_processors(
        const SL_Texture* inTex,
        SL_Texture* outTex,
        uint16_t srcX0,
        uint16_t srcY0,
        uint16_t srcX1,
        uint16_t srcY1,
        uint16_t dstX0,
        uint16_t dstY0,
        uint16_t dstX1,
        uint16_t dstY1
    ) noexcept;

    void run_blit_compressed_processors(
        const SL_Texture* inTex,
        SL_Texture* outTex,
        uint16_t srcX0,
        uint16_t srcY0,
        uint16_t srcX1,
        uint16_t srcY1,
        uint16_t dstX0,
        uint16_t dstY0,
        uint16_t dstX1,
        uint16_t dstY1
    ) noexcept;

    void run_clear_processors(const void* inColor, SL_Texture* outTex) noexcept;

    void run_clear_processors(const void* inColor, const void* depth, SL_Texture* colorBuf, SL_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 2>& inColors, const void* depth, const std::array<SL_Texture*, 2>& colorBufs, SL_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 3>& inColors, const void* depth, const std::array<SL_Texture*, 3>& colorBufs, SL_Texture* depthBuf) noexcept;

    void run_clear_processors(const std::array<const void*, 4>& inColors, const void* depth, const std::array<SL_Texture*, 4>& colorBufs, SL_Texture* depthBuf) noexcept;
};



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
inline unsigned SL_ProcessorPool::concurrency() const noexcept
{
    return mNumThreads;
}



/*-------------------------------------
 * Run the processor threads
-------------------------------------*/
inline void SL_ProcessorPool::execute() noexcept
{
    this->flush();
    this->wait();
}


#endif /* SL_PROCESSORPOOL_HPP */
