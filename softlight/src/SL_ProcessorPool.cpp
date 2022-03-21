
#include <utility> // std::move()

#include "lightsky/setup/CPU.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"
#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/vec4.h"

#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_ProcessorPool.hpp"
#include "softlight/SL_ShaderProcessor.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_FragmentBin



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_Framebuffer;



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
    mFragSemaphore{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<int_fast64_t>>()},
    mShadingSemaphore{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<uint_fast64_t>>()},
    mBinIds{ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS)},
    mTempBinIds{ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS)},
    mBinsUsed{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<uint32_t>>()},
    mFragBins{ls::utils::make_unique_aligned_array<SL_FragmentBin>(SL_SHADER_MAX_BINNED_PRIMS)},
    mFragQueues{ls::utils::make_unique_aligned_array<SL_FragCoord>(numThreads)},
    mWorkers{numThreads > 1 ? ls::utils::make_unique_aligned_array<SL_ProcessorPool::ThreadedWorker>(numThreads - 1) : nullptr},
    mNumThreads{numThreads}
{
    LS_ASSERT(numThreads > 0);

    ls::utils::set_thread_affinity(ls::utils::get_thread_id(), 0);

    for (unsigned i = 0; i < numThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{i+1};
    }

    clear_fragment_bins();
}



/*--------------------------------------
 * Default Constructor (delegates)
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool() noexcept :
    SL_ProcessorPool{1u}
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool(const SL_ProcessorPool& p) noexcept :
    mFragSemaphore{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<int_fast64_t>>()},
    mShadingSemaphore{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<uint_fast64_t>>()},
    mBinIds{ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS)},
    mTempBinIds{ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS)},
    mBinsUsed{ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<uint32_t>>()},
    mFragBins{ls::utils::make_unique_aligned_array<SL_FragmentBin>(SL_SHADER_MAX_BINNED_PRIMS)},
    mFragQueues{ls::utils::make_unique_aligned_array<SL_FragCoord>(p.mNumThreads)},
    mWorkers{p.mNumThreads > 1 ? ls::utils::make_unique_aligned_array<SL_ProcessorPool::ThreadedWorker>(p.mNumThreads - 1) : nullptr},
    mNumThreads{p.mNumThreads}
{
    ls::utils::set_thread_affinity(ls::utils::get_thread_id(), 0);

    for (unsigned i = 0; i < p.mNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{i+1};
    }

    clear_fragment_bins();
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_ProcessorPool::SL_ProcessorPool(SL_ProcessorPool&& p) noexcept :
    mFragSemaphore{std::move(p.mFragSemaphore)},
    mShadingSemaphore{std::move(p.mShadingSemaphore)},
    mBinIds{std::move(p.mBinIds)},
    mTempBinIds{std::move(p.mTempBinIds)},
    mBinsUsed{std::move(p.mBinsUsed)},
    mFragBins{std::move(p.mFragBins)},
    mFragQueues{std::move(p.mFragQueues)},
    mWorkers{std::move(p.mWorkers)},
    mNumThreads{p.mNumThreads}
{
    p.mNumThreads = 1;
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

    mFragSemaphore = std::move(p.mFragSemaphore);
    mShadingSemaphore = std::move(p.mShadingSemaphore);
    mBinIds = std::move(p.mBinIds);
    mTempBinIds = std::move(p.mTempBinIds);
    mBinsUsed = std::move(p.mBinsUsed);
    mFragBins = std::move(p.mFragBins);
    mFragQueues = std::move(p.mFragQueues);

    for (unsigned i = 0; i < mNumThreads-1u; ++i)
    {
        mWorkers[i].~WorkerThread();
    }

    mWorkers = std::move(p.mWorkers);

    mNumThreads = p.mNumThreads;
    p.mNumThreads = 1;

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
    constexpr unsigned maxIters = 32;
    unsigned currentIters;

    for (unsigned threadId = 0; threadId < mNumThreads - 1u; ++threadId)
    {
        currentIters = 1;

        while (!mWorkers[threadId].ready())
        {
            switch (currentIters)
            {
                case 32:
                    mWorkers[threadId].wait();
                    break;

                case 16:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 8:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 4:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 2:
                    ls::setup::cpu_yield();
                default:
                    ls::setup::cpu_yield();
            }

            currentIters = ls::math::min(currentIters+currentIters, maxIters);
        }
    }
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

    mBinIds = ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS);
    mTempBinIds = ls::utils::make_unique_aligned_array<SL_BinCounter<uint32_t>>(SL_SHADER_MAX_BINNED_PRIMS);
    mBinsUsed = ls::utils::make_unique_aligned_pointer<SL_BinCounterAtomic<uint32_t>>();
    mFragBins = ls::utils::make_unique_aligned_array<SL_FragmentBin>(SL_SHADER_MAX_BINNED_PRIMS);
    mFragQueues = ls::utils::make_unique_aligned_array<SL_FragCoord>(inNumThreads);

    mWorkers.reset();
    if (inNumThreads > 1)
    {
        mWorkers = ls::utils::make_unique_aligned_array<SL_ProcessorPool::ThreadedWorker>(inNumThreads - 1);
    }

    ls::utils::set_thread_affinity(ls::utils::get_thread_id(), 0);

    for (unsigned i = 0; i < inNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{i+1};
    }

    mNumThreads = inNumThreads;
    clear_fragment_bins();

    LS_LOG_MSG(
        "Rendering threads updated:"
        "\n\tThread Count:                    ", inNumThreads,
        "\n\tBytes per Task:                  ", sizeof(SL_ShaderProcessor),
        "\n\tBytes of Task Pool:              ", sizeof(SL_ProcessorPool),
        "\n\tVertex Task Size:                ", sizeof(SL_VertexProcessor),
        "\n\tFragment Task Size:              ", sizeof(SL_FragmentProcessor),
        "\n\tFragment Bin Size:               ", sizeof(SL_FragmentBin),
        "\n\tBlitter Task Size:               ", sizeof(SL_BlitProcessor),
        "\n\tBlitter (compressed) Task Size:  ", sizeof(SL_BlitCompressedProcessor)
    );

    return inNumThreads;
}



/*-------------------------------------
-------------------------------------*/
void SL_ProcessorPool::run_shader_processors(const SL_Context& c, const SL_Mesh& m, size_t numInstances, const SL_Shader& s, SL_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore->count.store(0);
    mShadingSemaphore->count.store(mNumThreads);
    clear_fragment_bins();

    const SL_RenderMode renderMode = m.mode;
    SL_ShaderProcessor task;
    task.mType = sl_processor_type_for_draw_mode(renderMode);

    SL_VertexProcessor* vertTask = task.processor_for_draw_mode(renderMode);
    vertTask->mThreadId       = 0;
    vertTask->mNumThreads     = (int16_t)mNumThreads;
    vertTask->mFragProcessors = mFragSemaphore.get();
    vertTask->mBusyProcessors = mShadingSemaphore.get();
    vertTask->mShader         = &s;
    vertTask->mContext        = &c;
    vertTask->mFbo            = &fbo;
    vertTask->mRenderMode     = m.mode;
    vertTask->mNumMeshes      = 1;
    vertTask->mNumInstances   = numInstances;
    vertTask->mMeshes         = &m;
    vertTask->mBinsUsed       = mBinsUsed.get();
    vertTask->mBinIds         = mBinIds.get();
    vertTask->mTempBinIds     = mTempBinIds.get();
    vertTask->mFragBins       = mFragBins.get();
    vertTask->mFragQueues     = mFragQueues.get();

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        vertTask->mThreadId = threadId;

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        if (threadId < mNumThreads-1)
        {
            SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
            worker.busy_waiting(false);
            worker.push(task);
        }
    }

    flush();
    task();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
-------------------------------------*/
void SL_ProcessorPool::run_shader_processors(const SL_Context& c, const SL_Mesh* meshes, size_t numMeshes, const SL_Shader& s, SL_Framebuffer& fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore->count.store(0);
    mShadingSemaphore->count.store(mNumThreads);
    clear_fragment_bins();

    const SL_RenderMode renderMode = meshes[0].mode;
    SL_ShaderProcessor task;
    task.mType = sl_processor_type_for_draw_mode(renderMode);

    SL_VertexProcessor* vertTask = task.processor_for_draw_mode(renderMode);
    vertTask->mThreadId       = 0;
    vertTask->mNumThreads     = (int16_t)mNumThreads;
    vertTask->mFragProcessors = mFragSemaphore.get();
    vertTask->mBusyProcessors = mShadingSemaphore.get();
    vertTask->mShader         = &s;
    vertTask->mContext        = &c;
    vertTask->mFbo            = &fbo;
    vertTask->mRenderMode     = meshes->mode;
    vertTask->mNumMeshes      = numMeshes;
    vertTask->mNumInstances   = 1;
    vertTask->mMeshes         = meshes;
    vertTask->mBinsUsed       = mBinsUsed.get();
    vertTask->mBinIds         = mBinIds.get();
    vertTask->mTempBinIds     = mTempBinIds.get();
    vertTask->mFragBins       = mFragBins.get();
    vertTask->mFragQueues     = mFragQueues.get();

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        vertTask->mThreadId = threadId;

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        if (threadId < mNumThreads-1)
        {
            SL_ProcessorPool::ThreadedWorker& worker = mWorkers[threadId];
            worker.busy_waiting(false);
            worker.push(task);
        }
    }

    flush();
    task();

    // Each thread should now pause except for the main thread.
    wait();
}



/*-------------------------------------
 * Remove all bins from potential processing
-------------------------------------*/
void SL_ProcessorPool::clear_fragment_bins() noexcept
{
    mBinsUsed->count = 0;
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
    LS_ASSERT(!sl_is_compressed_color(inTex->type()) && !sl_is_compressed_color(outTex->type()));
    processor.mType = SL_BLIT_PROCESSOR;

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
 * Execute a texture blit across threads
-------------------------------------*/
void SL_ProcessorPool::run_blit_compressed_processors(
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
    LS_ASSERT(sl_is_compressed_color(inTex->type()) || sl_is_compressed_color(outTex->type()));
    processor.mType = SL_BLIT_COMPRESSED_PROCESSOR;

    SL_BlitCompressedProcessor& blitter = processor.mBlitterCompressed;
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
    processor.mType = SL_CLEAR_PROCESSOR;

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
    processor.mType = SL_CLEAR_PROCESSOR;

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
    processor.mType = SL_CLEAR_PROCESSOR;

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
    processor.mType = SL_CLEAR_PROCESSOR;

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
    processor.mType = SL_CLEAR_PROCESSOR;

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
