
#include <utility> // std::move()

#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Compiler.h"

#include "lightsky/utils/Log.h"

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp" // SR_Framebuffer
#include "soft_render/SR_ShaderProcessor.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexProcessor.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 * I've been having issues with allocated allocation on GCC and MSVC.
-----------------------------------------------------------------------------*/
template <typename data_t>
static inline data_t* _aligned_alloc(size_t numElements) noexcept
{
    return reinterpret_cast<data_t*>(ls::utils::aligned_malloc(sizeof(data_t)*numElements));
}




/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SR_ShaderType : uint8_t
{
    SR_VERTEX_SHADER,
    SR_FRAGMENT_SHADER,
    SR_BLIT_SHADER
};



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex & fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_ShaderProcessor
{
    SR_ShaderType mType; // 32 bits

    // 2128 bits
    union
    {
        SR_VertexProcessor mVertProcessor;
        SR_FragmentProcessor mFragProcessor;
        SR_BlitProcessor mBlitter;
    };

    // 2144 bits (268 bytes), padding not included

    ~SR_ShaderProcessor() noexcept = default;

    SR_ShaderProcessor() noexcept;

    SR_ShaderProcessor(const SR_ShaderProcessor&) noexcept;

    SR_ShaderProcessor(SR_ShaderProcessor&&) noexcept;

    SR_ShaderProcessor& operator=(const SR_ShaderProcessor&) noexcept;

    SR_ShaderProcessor& operator=(SR_ShaderProcessor&&) noexcept;

    void operator()() noexcept;
};



/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor() noexcept :
    mType{SR_VERTEX_SHADER},
    mVertProcessor()
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor(const SR_ShaderProcessor& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SR_BLIT_SHADER:
            mBlitter = sp.mBlitter;
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor(SR_ShaderProcessor&& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SR_BLIT_SHADER:
            mBlitter = sp.mBlitter;
    }
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SR_ShaderProcessor& SR_ShaderProcessor::operator=(const SR_ShaderProcessor& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SR_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SR_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SR_BLIT_SHADER:
                mBlitter = sp.mBlitter;
        }
    }

    return *this;
}



/*--------------------------------------
 * Move Operator
--------------------------------------*/
SR_ShaderProcessor& SR_ShaderProcessor::operator=(SR_ShaderProcessor&& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SR_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SR_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SR_BLIT_SHADER:
                mBlitter = sp.mBlitter;
        }
    }

    return *this;
}



/*--------------------------------------
 * Thread Execution
--------------------------------------*/
void SR_ShaderProcessor::operator()() noexcept
{
    switch (mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor.execute();
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor.execute();
            break;

        case SR_BLIT_SHADER:
            mBlitter.execute();
            break;
    }
}



/*-----------------------------------------------------------------------------
 * SR_ProcessorPool Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Destructor
--------------------------------------*/
SR_ProcessorPool::~SR_ProcessorPool() noexcept
{
    for (unsigned i = 0; i < mNumThreads; ++i)
    {
        delete mThreads[i];
        mThreads[i] = nullptr;
    }
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_ProcessorPool::SR_ProcessorPool(unsigned numThreads) noexcept :
    mFragSemaphore{0},
    mShadingSemaphore{0},
    mBinsUsed{0},
    mBinIds{nullptr},
    mFragBins{nullptr},
    mVaryings{nullptr},
    mFragQueues{nullptr},
    mThreads{nullptr},
    mNumThreads{0}
{
    num_threads(numThreads);
}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_ProcessorPool::SR_ProcessorPool(const SR_ProcessorPool& p) noexcept
{
    *this = p;
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SR_ProcessorPool::SR_ProcessorPool(SR_ProcessorPool&& p) noexcept
{
    *this = std::move(p);
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SR_ProcessorPool& SR_ProcessorPool::operator=(const SR_ProcessorPool& p) noexcept
{
    if (this == &p || num_threads() == p.num_threads())
    {
        return *this;
    }

    num_threads(p.num_threads());

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

    mBinsUsed.store(0, std::memory_order_relaxed);
    p.mBinsUsed.store(0);

    mBinIds = std::move(p.mBinIds);
    mFragBins = std::move(p.mFragBins);
    mVaryings = std::move(p.mVaryings);
    mFragQueues = std::move(p.mFragQueues);

    for (unsigned i = 0; i < mNumThreads; ++i)
    {
        delete mThreads[i];
        mThreads[i] = nullptr;
    }

    mThreads = std::move(p.mThreads);

    mNumThreads = p.mNumThreads;
    p.mNumThreads = 0;

    return *this;
}



/*-------------------------------------
 * Flush the thread pool
-------------------------------------*/
void SR_ProcessorPool::flush() noexcept
{
    for (unsigned threadId = 0; threadId < mNumThreads; ++threadId)
    {
        SR_ProcessorPool::Worker* const pWorker = mThreads[threadId];

        if (pWorker->have_pending())
        {
            pWorker->flush();
        }
    }
}



/*-------------------------------------
 * Wait for the threads to finish
-------------------------------------*/
void SR_ProcessorPool::wait() noexcept
{
    // Each thread will pause except for the main thread.
    for (unsigned threadId = 0; threadId < mNumThreads - 1u; ++threadId)
    {
        SR_ProcessorPool::Worker* const pWorker = mThreads[threadId];
        pWorker->wait();
    }

    /*
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
    {
        SR_ProcessorPool::Worker* const pWorker = mThreads[threadId];

        while (!pWorker->ready())
        {
            continue;
        }
    }
    */
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SR_ProcessorPool::num_threads(unsigned inNumThreads) noexcept
{
    if (inNumThreads == mNumThreads)
    {
        return mNumThreads;
    }

    for (unsigned i = 0; i < mNumThreads; ++i)
    {
        delete mThreads[i];
        mThreads[i] = nullptr;
    }

    if (inNumThreads == 0)
    {
        // always use at least the main thread.
        inNumThreads = 1;
    }

    mBinsUsed.store(0, std::memory_order_relaxed);
    mBinIds.reset(_aligned_alloc<uint32_t>(SR_SHADER_MAX_BINNED_PRIMS));
    mFragBins.reset(_aligned_alloc<SR_FragmentBin>(SR_SHADER_MAX_BINNED_PRIMS));
    mVaryings.reset(_aligned_alloc<ls::math::vec4>(inNumThreads * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS));
    mFragQueues.reset(_aligned_alloc<SR_FragCoord>(inNumThreads));
    mThreads.reset(_aligned_alloc<SR_ProcessorPool::Worker*>(inNumThreads));

    mNumThreads = inNumThreads;

    for (unsigned i = 0; i < inNumThreads; ++i)
    {
        // The last thread is always the main thread
        if (i == (inNumThreads-1u))
        {
            mThreads[i] = new SR_ProcessorPool::Worker{};
        }
        else
        {
            mThreads[i] = new SR_ProcessorPool::ThreadedWorker{};
        }
    }

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
void SR_ProcessorPool::run_shader_processors(const SR_Context* c, const SR_Mesh* m, const SR_Shader* s, SR_Framebuffer* fbo) noexcept
{
    // Reserve enough space for each thread to contain all triangles
    mFragSemaphore.store(0, std::memory_order_relaxed);
    mShadingSemaphore.store(mNumThreads, std::memory_order_relaxed);
    mBinsUsed.store(0, std::memory_order_relaxed);

    SR_ShaderProcessor task;
    task.mType = SR_VERTEX_SHADER;

    SR_VertexProcessor& vertTask = task.mVertProcessor;
    vertTask.mThreadId       = 0;
    vertTask.mNumThreads     = (int16_t)mNumThreads;
    vertTask.mFragProcessors = &mFragSemaphore;
    vertTask.mBusyProcessors = &mShadingSemaphore;
    vertTask.mShader         = s;
    vertTask.mContext        = c;
    vertTask.mFbo            = fbo;
    vertTask.mFboW           = fbo->width();
    vertTask.mFboH           = fbo->height();
    vertTask.mMesh           = *m;
    vertTask.mBinsUsed       = &mBinsUsed;
    vertTask.mBinIds         = mBinIds.get();
    vertTask.mFragBins       = mFragBins.get();
    vertTask.mVaryings       = mVaryings.get();
    vertTask.mFragQueues     = mFragQueues.get();

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        vertTask.mThreadId = threadId;

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        SR_ProcessorPool::Worker* pWorker = mThreads[threadId];
        pWorker->busy_waiting(false);
        pWorker->push(task);
    }

    // Each thread should now pause except for the main thread.
    execute();
    clear_fragment_bins();
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
    blitter.mThreadId   = 0;
    blitter.mNumThreads = (uint16_t)mNumThreads;
    blitter.srcX0       = srcX0;
    blitter.srcY0       = srcY0;
    blitter.srcX1       = srcX1;
    blitter.srcY1       = srcY1;
    blitter.dstX0       = dstX0;
    blitter.dstY0       = dstY0;
    blitter.dstX1       = dstX1;
    blitter.dstY1       = dstY1;
    blitter.mTexture    = inTex;
    blitter.mBackBuffer = outTex;

    // Process most of the rendering on other threads first.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        blitter.mThreadId = threadId;

        SR_ProcessorPool::Worker* pWorker = mThreads[threadId];
        pWorker->busy_waiting(false);
        pWorker->push(processor);
    }

    execute();
}
