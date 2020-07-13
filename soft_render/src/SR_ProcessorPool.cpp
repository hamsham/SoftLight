
#include <utility> // std::move()

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"
#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/vec4.h"

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
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
    mBinsReady{_aligned_alloc<std::atomic_int_fast32_t>(numThreads)},
    mBinsUsed{_aligned_alloc<uint32_t>(numThreads)},
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
}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_ProcessorPool::SR_ProcessorPool(const SR_ProcessorPool& p) noexcept :
    mFragSemaphore{0},
    mShadingSemaphore{0},
    mBinsReady{_aligned_alloc<std::atomic_int_fast32_t>(p.mNumThreads)},
    mBinsUsed{_aligned_alloc<uint32_t>(p.mNumThreads)},
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
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
    {
        mWorkers[threadId].wait();
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

    mBinsReady.reset(_aligned_alloc<std::atomic_int_fast32_t>(inNumThreads));
    mBinsUsed.reset(_aligned_alloc<uint32_t>(inNumThreads));
    mFragBins.reset(_aligned_alloc<SR_FragmentBin>(inNumThreads * SR_SHADER_MAX_BINNED_PRIMS));
    mVaryings.reset(_aligned_alloc<ls::math::vec4>(inNumThreads * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS));
    mFragQueues.reset(_aligned_alloc<SR_FragCoord>(inNumThreads));
    mWorkers.reset(inNumThreads > 1 ? _aligned_alloc<SR_ProcessorPool::ThreadedWorker>(inNumThreads - 1) : nullptr);

    for (unsigned i = 0; i < inNumThreads-1u; ++i)
    {
        new (&mWorkers[i]) ThreadedWorker{};
    }

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
