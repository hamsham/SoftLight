
#include <utility> // std::move()

#include "lightsky/setup/Compiler.h"
#include "lightsky/setup/Macros.h"
#include "lightsky/setup/OS.h"

#ifdef LS_OS_UNIX
    #include <time.h> // nanosleep, time_spec
#endif

#if defined(LS_COMPILER_CLANG) && defined(LS_ARCH_AARCH64)
    #include <arm_acle.h> // __yield()
#endif

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"
#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/vec4.h"

#include "softlight/SL_BlitProcesor.hpp"
#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_ProcessorPool.hpp"
#include "softlight/SL_ShaderProcessor.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_FragmentBin
#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_Framebuffer;




/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{


/*--------------------------------------
 * Custom aligned-allocator to resolve allocation alignment issues between
 * GCC and MSVC.
--------------------------------------*/
template <typename data_t>
static inline data_t* _aligned_alloc(size_t numElements) noexcept
{
    return reinterpret_cast<data_t*>(ls::utils::aligned_malloc(sizeof(data_t) * numElements));
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_ProcessorPool Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Destructor
--------------------------------------*/
SL_ProcessorPool::~SL_ProcessorPool() noexcept
{
    for (unsigned i = 0; i < mNumThreads - 1; ++i)
    {
        mWorkers[i].~WorkerThread();
    }
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool(unsigned numThreads) noexcept :
    mFragSemaphore{{0}, {}},
    mShadingSemaphore{{0}, {}},
    mBinsReady{_aligned_alloc<SL_BinCounterAtomic<int32_t>>(numThreads)},
    mBinsUsed{_aligned_alloc<SL_BinCounter<uint32_t>>(numThreads)},
    mFragBins{_aligned_alloc<SL_FragmentBin>(numThreads * SL_SHADER_MAX_BINNED_PRIMS)},
    mVaryings{_aligned_alloc<ls::math::vec4>(numThreads * SL_SHADER_MAX_VARYING_VECTORS * SL_SHADER_MAX_QUEUED_FRAGS)},
    mFragQueues{_aligned_alloc<SL_FragCoord>(numThreads)},
    mWorkers{numThreads > 1 ? _aligned_alloc<SL_ProcessorPool::ThreadedWorker>(numThreads - 1) : nullptr},
    mNumThreads{numThreads}
{
    LS_ASSERT(numThreads > 0);

    for (unsigned i = 0; i < numThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{};
    }

    clear_fragment_bins();
}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool(const SL_ProcessorPool& p) noexcept :
    mFragSemaphore{{0}, {}},
    mShadingSemaphore{{0}, {}},
    mBinsReady{_aligned_alloc<SL_BinCounterAtomic<int32_t>>(p.mNumThreads)},
    mBinsUsed{_aligned_alloc<SL_BinCounter<uint32_t>>(p.mNumThreads)},
    mFragBins{_aligned_alloc<SL_FragmentBin>(p.mNumThreads * SL_SHADER_MAX_BINNED_PRIMS)},
    mVaryings{_aligned_alloc<ls::math::vec4>(p.mNumThreads * SL_SHADER_MAX_VARYING_VECTORS * SL_SHADER_MAX_QUEUED_FRAGS)},
    mFragQueues{_aligned_alloc<SL_FragCoord>(p.mNumThreads)},
    mWorkers{p.mNumThreads > 1 ? _aligned_alloc<SL_ProcessorPool::ThreadedWorker>(p.mNumThreads - 1) : nullptr},
    mNumThreads{p.mNumThreads}
{
    for (unsigned i = 0; i < p.mNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{};
    }

    clear_fragment_bins();
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool(SL_ProcessorPool&& p) noexcept :
    mFragSemaphore{{0}, {}},
    mShadingSemaphore{{0}, {}},
    mBinsReady{std::move(p.mBinsReady)},
    mBinsUsed{std::move(p.mBinsUsed)},
    mFragBins{std::move(p.mFragBins)},
    mVaryings{std::move(p.mVaryings)},
    mFragQueues{std::move(p.mFragQueues)},
    mWorkers{std::move(p.mWorkers)},
    mNumThreads{p.mNumThreads}
{
    p.mNumThreads = 0;
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SL_ProcessorPool& SL_ProcessorPool::operator=(const SL_ProcessorPool& p) noexcept
{
    if (this == &p || concurrency() == p.concurrency())
    {
        return *this;
    }

    concurrency(p.concurrency());

    return *this;
}



/*--------------------------------------
 * Move Operator
--------------------------------------*/
SL_ProcessorPool& SL_ProcessorPool::operator=(SL_ProcessorPool&& p) noexcept
{
    if (this == &p)
    {
        return *this;
    }

    mFragSemaphore.count.store(p.mFragSemaphore.count.load());
    p.mFragSemaphore.count.store(0);

    mShadingSemaphore.count.store(p.mShadingSemaphore.count.load());
    p.mShadingSemaphore.count.store(0);

    mBinsReady = std::move(p.mBinsReady);
    mBinsUsed = std::move(p.mBinsUsed);
    mFragBins = std::move(p.mFragBins);
    mVaryings = std::move(p.mVaryings);
    mFragQueues = std::move(p.mFragQueues);

    for (unsigned i = 0; i < mNumThreads-1u; ++i)
    {
        mWorkers[i].~WorkerThread();
    }

    mWorkers = std::move(p.mWorkers);

    mNumThreads = p.mNumThreads;
    p.mNumThreads = 0;

    return *this;
}



/*-------------------------------------
 * Flush the thread pool
-------------------------------------*/
void SL_ProcessorPool::flush() noexcept
{
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
    {
        mWorkers[threadId].flush();
    }
}



/*-------------------------------------
 * Wait for the threads to finish
-------------------------------------*/
void SL_ProcessorPool::wait() noexcept
{
    // Each thread will pause except for the main thread.
    #if 0
        for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
        {
            mWorkers[threadId].wait();
        }

        #else
        #if defined(LS_OS_UNIX) && !defined(LS_ARCH_X86)
            struct timespec sleepAmt;
            sleepAmt.tv_sec = 0;
            sleepAmt.tv_nsec = 1;
            (void)sleepAmt;
        #endif

        for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
        {
            while (!mWorkers[threadId].ready())
            {
                #if defined(LS_ARCH_X86)
                    _mm_pause();
                #elif defined(LS_COMPILER_CLANG) && defined(LS_ARCH_AARCH64)
                    __yield();
                #elif defined(LS_OS_LINUX) || defined(LS_OS_ANDROID)
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &sleepAmt, nullptr);
                #elif defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
                    nanosleep(&sleepAmt, nullptr);
                #else
                    std::this_thread::yield();
                #endif
            }
        }

    #endif
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SL_ProcessorPool::concurrency(unsigned inNumThreads) noexcept
{
    // always use at least the main thread.
    inNumThreads = ls::math::max<unsigned>(1u, inNumThreads);

    for (unsigned i = 0; i < mNumThreads - 1; ++i)
    {
        mWorkers[i].~WorkerThread();
    }

    mBinsReady.reset(_aligned_alloc<SL_BinCounterAtomic<int32_t>>(inNumThreads));
    mBinsUsed.reset(_aligned_alloc<SL_BinCounter<uint32_t>>(inNumThreads));
    mFragBins.reset(_aligned_alloc<SL_FragmentBin>(inNumThreads * SL_SHADER_MAX_BINNED_PRIMS));
    mVaryings.reset(_aligned_alloc<ls::math::vec4>(inNumThreads * SL_SHADER_MAX_VARYING_VECTORS * SL_SHADER_MAX_QUEUED_FRAGS));
    mFragQueues.reset(_aligned_alloc<SL_FragCoord>(inNumThreads));
    mWorkers.reset(inNumThreads > 1 ? _aligned_alloc<SL_ProcessorPool::ThreadedWorker>(inNumThreads - 1) : nullptr);

    for (unsigned i = 0; i < inNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{};
    }

    mNumThreads = inNumThreads;
    clear_fragment_bins();

    LS_LOG_MSG(
        "Rendering threads updated:"
        "\n\tThread Count:       ", inNumThreads,
        "\n\tBytes per Task:     ", sizeof(SL_ShaderProcessor),
        "\n\tBytes of Task Pool: ", sizeof(SL_ProcessorPool),
        "\n\tVertex Task Size:   ", sizeof(SL_VertexProcessor),
        "\n\tFragment Task Size: ", sizeof(SL_FragmentProcessor),
        "\n\tFragment Bin Size:  ", sizeof(SL_FragmentBin),
        "\n\tBlitter Task Size:  ", sizeof(SL_BlitProcessor));

    return inNumThreads;
}



/*-------------------------------------
-------------------------------------*/
void SL_ProcessorPool::run_shader_processors(const SL_Context& c, const SL_Mesh& m, size_t numInstances, const SL_Shader& s, SL_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore.count.store(0);
    mShadingSemaphore.count.store(mNumThreads);
    clear_fragment_bins();

    SL_ShaderProcessor task;
    task.mType = SL_VERTEX_SHADER;

    SL_VertexProcessor& vertTask = task.mVertProcessor;
    vertTask.mThreadId       = 0;
    vertTask.mNumThreads     = (int16_t)mNumThreads;
    vertTask.mFragProcessors = &mFragSemaphore;
    vertTask.mBusyProcessors = &mShadingSemaphore;
    vertTask.mShader         = &s;
    vertTask.mContext        = &c;
    vertTask.mFbo            = &fbo;
    vertTask.mRenderMode     = m.mode;
    vertTask.mNumMeshes      = 1;
    vertTask.mNumInstances   = numInstances;
    vertTask.mMeshes         = &m;
    vertTask.mBinsReady      = mBinsReady;
    vertTask.mBinsUsed       = mBinsUsed;
    vertTask.mFragBins       = mFragBins.get();
    vertTask.mVaryings       = mVaryings.get();
    vertTask.mFragQueues     = mFragQueues.get();

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads-1; ++threadId)
    {
        vertTask.mThreadId = threadId;

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
        worker.busy_waiting(false);
        worker.push(task);
    }

    flush();
    vertTask.mThreadId = (uint16_t)(mNumThreads-1u);
    vertTask.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
-------------------------------------*/
void SL_ProcessorPool::run_shader_processors(const SL_Context& c, const SL_Mesh* meshes, size_t numMeshes, const SL_Shader& s, SL_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore.count.store(0);
    mShadingSemaphore.count.store(mNumThreads);
    clear_fragment_bins();

    SL_ShaderProcessor task;
    task.mType = SL_VERTEX_SHADER;

    SL_VertexProcessor& vertTask = task.mVertProcessor;
    vertTask.mThreadId       = 0;
    vertTask.mNumThreads     = (int16_t)mNumThreads;
    vertTask.mFragProcessors = &mFragSemaphore;
    vertTask.mBusyProcessors = &mShadingSemaphore;
    vertTask.mShader         = &s;
    vertTask.mContext        = &c;
    vertTask.mFbo            = &fbo;
    vertTask.mRenderMode     = meshes->mode;
    vertTask.mNumMeshes      = numMeshes;
    vertTask.mNumInstances   = 1;
    vertTask.mMeshes         = meshes;
    vertTask.mBinsReady      = mBinsReady;
    vertTask.mBinsUsed       = mBinsUsed;
    vertTask.mFragBins       = mFragBins.get();
    vertTask.mVaryings       = mVaryings.get();
    vertTask.mFragQueues     = mFragQueues.get();

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads-1; ++threadId)
    {
        vertTask.mThreadId = threadId;

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
        worker.busy_waiting(false);
        worker.push(task);
    }

    flush();
    vertTask.mThreadId = (uint16_t)(mNumThreads-1u);
    vertTask.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Remove all bins from potential processing
-------------------------------------*/
void SL_ProcessorPool::clear_fragment_bins() noexcept
{
    for (uint16_t t = 0; t < mNumThreads; ++t)
    {
        mBinsReady[t].count.store(-1, std::memory_order_release);
        mBinsUsed[t].count = 0;
    }
}


/*-------------------------------------
 * Execute a texture blit across threads
-------------------------------------*/
void SL_ProcessorPool::run_blit_processors(
    const SL_Texture* inTex,
    SL_Texture* outTex,
    uint16_t srcX0,
    uint16_t srcY0,
    uint16_t srcX1,
    uint16_t srcY1,
    uint16_t dstX0,
    uint16_t dstY0,
    uint16_t dstX1,
    uint16_t dstY1) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_BLIT_SHADER;

    SL_BlitProcessor& blitter = processor.mBlitter;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;
    blitter.srcX0             = srcX0;
    blitter.srcY0             = srcY0;
    blitter.srcX1             = srcX1;
    blitter.srcY1             = srcY1;
    blitter.dstX0             = dstX0;
    blitter.dstY0             = dstY0;
    blitter.dstX1             = dstX1;
    blitter.dstY1             = dstY1;
    blitter.mTexture          = inTex;
    blitter.mBackBuffer       = outTex;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;

        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
        worker.busy_waiting(false);
        worker.push(processor);
    }

    flush();
    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Clear a framebuffer's attachment across threads
-------------------------------------*/
void SL_ProcessorPool::run_clear_processors(const void* inColor, SL_Texture* outTex) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_CLEAR_SHADER;

    SL_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;
    blitter.mTexture          = inColor;
    blitter.mBackBuffer       = outTex;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;

        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
        worker.busy_waiting(false);
        worker.push(processor);
    }

    flush();
    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Clear a framebuffer across threads
-------------------------------------*/
void SL_ProcessorPool::run_clear_processors(const void* inColor, const void* depth, SL_Texture* colorBuf, SL_Texture* depthBuf) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_CLEAR_SHADER;

    SL_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

        blitter.mBackBuffer = colorBuf;
        blitter.mTexture = inColor;
        worker.push(processor);

        blitter.mBackBuffer = depthBuf;
        blitter.mTexture = depth;
        worker.push(processor);

        worker.busy_waiting(false);
    }

    flush();

    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.mBackBuffer = colorBuf;
    blitter.mTexture = inColor;
    blitter.execute();

    blitter.mBackBuffer = depthBuf;
    blitter.mTexture = depth;
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Clear a framebuffer across threads (2 attachments)
-------------------------------------*/
void SL_ProcessorPool::run_clear_processors(const std::array<const void*, 2>& inColors, const void* depth, const std::array<SL_Texture*, 2>& colorBufs, SL_Texture* depthBuf) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_CLEAR_SHADER;

    SL_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

        blitter.mBackBuffer = colorBufs[0];
        blitter.mTexture = inColors[0];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[1];
        blitter.mTexture = inColors[1];
        worker.push(processor);

        blitter.mBackBuffer = depthBuf;
        blitter.mTexture = depth;
        worker.push(processor);

        worker.busy_waiting(false);
    }

    flush();

    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.mBackBuffer = colorBufs[0];
    blitter.mTexture = inColors[0];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[1];
    blitter.mTexture = inColors[1];
    blitter.execute();

    blitter.mBackBuffer = depthBuf;
    blitter.mTexture = depth;
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Clear a framebuffer across threads (3 attachments)
-------------------------------------*/
void SL_ProcessorPool::run_clear_processors(const std::array<const void*, 3>& inColors, const void* depth, const std::array<SL_Texture*, 3>& colorBufs, SL_Texture* depthBuf) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_CLEAR_SHADER;

    SL_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

        blitter.mBackBuffer = colorBufs[0];
        blitter.mTexture = inColors[0];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[1];
        blitter.mTexture = inColors[1];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[2];
        blitter.mTexture = inColors[2];
        worker.push(processor);

        blitter.mBackBuffer = depthBuf;
        blitter.mTexture = depth;
        worker.push(processor);

        worker.busy_waiting(false);
    }

    flush();

    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.mBackBuffer = colorBufs[0];
    blitter.mTexture = inColors[0];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[1];
    blitter.mTexture = inColors[1];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[2];
    blitter.mTexture = inColors[2];
    blitter.execute();

    blitter.mBackBuffer = depthBuf;
    blitter.mTexture = depth;
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Clear a framebuffer across threads (4 attachments)
-------------------------------------*/
void SL_ProcessorPool::run_clear_processors(const std::array<const void*, 4>& inColors, const void* depth, const std::array<SL_Texture*, 4>& colorBufs, SL_Texture* depthBuf) noexcept
{
    SL_ShaderProcessor processor;
    processor.mType = SL_CLEAR_SHADER;

    SL_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

        blitter.mBackBuffer = colorBufs[0];
        blitter.mTexture = inColors[0];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[1];
        blitter.mTexture = inColors[1];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[2];
        blitter.mTexture = inColors[2];
        worker.push(processor);

        blitter.mBackBuffer = colorBufs[3];
        blitter.mTexture = inColors[3];
        worker.push(processor);

        blitter.mBackBuffer = depthBuf;
        blitter.mTexture = depth;
        worker.push(processor);

        worker.busy_waiting(false);
    }

    flush();

    blitter.mThreadId = (uint16_t)(mNumThreads - 1u);
    blitter.mBackBuffer = colorBufs[0];
    blitter.mTexture = inColors[0];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[1];
    blitter.mTexture = inColors[1];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[2];
    blitter.mTexture = inColors[2];
    blitter.execute();

    blitter.mBackBuffer = colorBufs[3];
    blitter.mTexture = inColors[3];
    blitter.execute();

    blitter.mBackBuffer = depthBuf;
    blitter.mTexture = depth;
    blitter.execute();

    // Each thread should now pause except for the main thread.
    wait();
}
