
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

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_ProcessorPool.hpp"
#include "soft_render/SR_ShaderProcessor.hpp"
#include "soft_render/SR_ShaderUtil.hpp" // SR_FragmentBin
#include "soft_render/SR_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_Framebuffer;




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
 * SR_ProcessorPool Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Destructor
--------------------------------------*/
SR_ProcessorPool::~SR_ProcessorPool() noexcept
{
    for (unsigned i = 0; i < mNumThreads - 1; ++i)
    {
        mWorkers[i].~WorkerThread();
    }
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_ProcessorPool::SR_ProcessorPool(unsigned numThreads) noexcept :
    mFragSemaphore{0},
    mShadingSemaphore{0},
    mBinsReady{_aligned_alloc<SR_BinCounterAtomic<int32_t>>(numThreads)},
    mBinsUsed{_aligned_alloc<SR_BinCounter<uint32_t>>(numThreads)},
    mFragBins{_aligned_alloc<SR_FragmentBin>(numThreads * SR_SHADER_MAX_BINNED_PRIMS)},
    mVaryings{_aligned_alloc<ls::math::vec4>(numThreads * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS)},
    mFragQueues{_aligned_alloc<SR_FragCoord>(numThreads)},
    mWorkers{numThreads > 1 ? _aligned_alloc<SR_ProcessorPool::ThreadedWorker>(numThreads - 1) : nullptr},
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
SR_ProcessorPool::SR_ProcessorPool(const SR_ProcessorPool& p) noexcept :
    mFragSemaphore{0},
    mShadingSemaphore{0},
    mBinsReady{_aligned_alloc<SR_BinCounterAtomic<int32_t>>(p.mNumThreads)},
    mBinsUsed{_aligned_alloc<SR_BinCounter<uint32_t>>(p.mNumThreads)},
    mFragBins{_aligned_alloc<SR_FragmentBin>(p.mNumThreads * SR_SHADER_MAX_BINNED_PRIMS)},
    mVaryings{_aligned_alloc<ls::math::vec4>(p.mNumThreads * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS)},
    mFragQueues{_aligned_alloc<SR_FragCoord>(p.mNumThreads)},
    mWorkers{p.mNumThreads > 1 ? _aligned_alloc<SR_ProcessorPool::ThreadedWorker>(p.mNumThreads - 1) : nullptr},
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
SR_ProcessorPool::SR_ProcessorPool(SR_ProcessorPool&& p) noexcept :
    mFragSemaphore{0},
    mShadingSemaphore{0},
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
SR_ProcessorPool& SR_ProcessorPool::operator=(const SR_ProcessorPool& p) noexcept
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
SR_ProcessorPool& SR_ProcessorPool::operator=(SR_ProcessorPool&& p) noexcept
{
    if (this == &p)
    {
        return *this;
    }

    mFragSemaphore.store(p.mFragSemaphore.load());
    p.mFragSemaphore.store(0);

    mShadingSemaphore.store(p.mShadingSemaphore.load());
    p.mShadingSemaphore.store(0);

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
void SR_ProcessorPool::flush() noexcept
{
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
    {
        mWorkers[threadId].flush();
    }
}



/*-------------------------------------
 * Wait for the threads to finish
-------------------------------------*/
void SR_ProcessorPool::wait() noexcept
{
    // Each thread will pause except for the main thread.
    /*
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
    {
        mWorkers[threadId].wait();
    }
    */

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
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SR_ProcessorPool::concurrency(unsigned inNumThreads) noexcept
{
    // always use at least the main thread.
    inNumThreads = ls::math::max<unsigned>(1u, inNumThreads);

    for (unsigned i = 0; i < mNumThreads - 1; ++i)
    {
        mWorkers[i].~WorkerThread();
    }

    mBinsReady.reset(_aligned_alloc<SR_BinCounterAtomic<int32_t>>(inNumThreads));
    mBinsUsed.reset(_aligned_alloc<SR_BinCounter<uint32_t>>(inNumThreads));
    mFragBins.reset(_aligned_alloc<SR_FragmentBin>(inNumThreads * SR_SHADER_MAX_BINNED_PRIMS));
    mVaryings.reset(_aligned_alloc<ls::math::vec4>(inNumThreads * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS));
    mFragQueues.reset(_aligned_alloc<SR_FragCoord>(inNumThreads));
    mWorkers.reset(inNumThreads > 1 ? _aligned_alloc<SR_ProcessorPool::ThreadedWorker>(inNumThreads - 1) : nullptr);

    for (unsigned i = 0; i < inNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{};
    }

    clear_fragment_bins();
    mNumThreads = inNumThreads;

    LS_LOG_MSG(
        "Rendering threads updated:"
        "\n\tThread Count:       ", inNumThreads,
        "\n\tBytes per Task:     ", sizeof(SR_ShaderProcessor),
        "\n\tBytes of Task Pool: ", sizeof(SR_ProcessorPool),
        "\n\tVertex Task Size:   ", sizeof(SR_VertexProcessor),
        "\n\tFragment Task Size: ", sizeof(SR_FragmentProcessor),
        "\n\tFragment Bin Size:  ", sizeof(SR_FragmentBin),
        "\n\tBlitter Task Size:  ", sizeof(SR_BlitProcessor));

    return inNumThreads;
}



/*-------------------------------------
-------------------------------------*/
void SR_ProcessorPool::run_shader_processors(const SR_Context& c, const SR_Mesh& m, size_t numInstances, const SR_Shader& s, SR_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore.store(0);
    mShadingSemaphore.store(mNumThreads);
    clear_fragment_bins();

    SR_ShaderProcessor task;
    task.mType = SR_VERTEX_SHADER;

    SR_VertexProcessor& vertTask = task.mVertProcessor;
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
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
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
void SR_ProcessorPool::run_shader_processors(const SR_Context& c, const SR_Mesh* meshes, size_t numMeshes, const SR_Shader& s, SR_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore.store(0);
    mShadingSemaphore.store(mNumThreads);
    clear_fragment_bins();

    SR_ShaderProcessor task;
    task.mType = SR_VERTEX_SHADER;

    SR_VertexProcessor& vertTask = task.mVertProcessor;
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
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
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
void SR_ProcessorPool::clear_fragment_bins() noexcept
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
void SR_ProcessorPool::run_blit_processors(
    const SR_Texture* inTex,
    SR_Texture* outTex,
    uint16_t srcX0,
    uint16_t srcY0,
    uint16_t srcX1,
    uint16_t srcY1,
    uint16_t dstX0,
    uint16_t dstY0,
    uint16_t dstX1,
    uint16_t dstY1) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_BLIT_SHADER;

    SR_BlitProcessor& blitter = processor.mBlitter;
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

        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
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
void SR_ProcessorPool::run_clear_processors(const void* inColor, SR_Texture* outTex) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_CLEAR_SHADER;

    SR_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;
    blitter.mTexture          = inColor;
    blitter.mBackBuffer       = outTex;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;

        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
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
void SR_ProcessorPool::run_clear_processors(const void* inColor, const void* depth, SR_Texture* colorBuf, SR_Texture* depthBuf) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_CLEAR_SHADER;

    SR_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

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
void SR_ProcessorPool::run_clear_processors(const std::array<const void*, 2>& inColors, const void* depth, const std::array<SR_Texture*, 2>& colorBufs, SR_Texture* depthBuf) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_CLEAR_SHADER;

    SR_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

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
void SR_ProcessorPool::run_clear_processors(const std::array<const void*, 3>& inColors, const void* depth, const std::array<SR_Texture*, 3>& colorBufs, SR_Texture* depthBuf) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_CLEAR_SHADER;

    SR_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

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
void SR_ProcessorPool::run_clear_processors(const std::array<const void*, 4>& inColors, const void* depth, const std::array<SR_Texture*, 4>& colorBufs, SR_Texture* depthBuf) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_CLEAR_SHADER;

    SR_ClearProcessor& blitter = processor.mClear;
    blitter.mThreadId         = 0;
    blitter.mNumThreads       = (uint16_t)mNumThreads;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads - 1; ++threadId)
    {
        blitter.mThreadId = threadId;
        SR_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];

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
