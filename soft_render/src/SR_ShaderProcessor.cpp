
#include <iostream>
#include <utility> // std::move()

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_ShaderProcessor.hpp"
#include "soft_render/SR_VertexProcessor.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



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
    mLocks{},
    mFragBins{},
    mThreads{},
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

    mLocks = std::move(p.mLocks);
    mFragBins = std::move(p.mFragBins);
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

        if (pWorker->have_pending_tasks())
        {
            pWorker->flush();
        }
    }
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

    mNumThreads = inNumThreads;
    mThreads.reset(new SR_ProcessorPool::Worker*[mNumThreads]);

    for (unsigned i = 0; i < mNumThreads; ++i)
    {
        // The last thread is always the main thread
        if (i == (mNumThreads-1u))
        {
            mThreads[i] = new SR_ProcessorPool::Worker{};
        }
        else
        {
            mThreads[i] = new SR_ProcessorPool::ThreadedWorker{};
        }
    }

    mLocks.reset(new SR_ProcessorPool::LockType[mNumThreads]);
    mFragBins.reset(new std::vector<SR_FragmentBin>[mNumThreads]);

    uint16_t c, r;
    sr_calc_frag_tiles<uint16_t>(mNumThreads, c, r);

    std::cout
        << "Rendering threads updated:"
        << "\n\tThread Count:       " << mNumThreads
        << "\n\tBytes per Task:     " << sizeof(SR_ShaderProcessor)
        << "\n\tBytes of Task Pool: " << sizeof(SR_ProcessorPool)
        << "\n\tVertex Task Size:   " << sizeof(SR_VertexProcessor)
        << "\n\tFragment Task Size: " << sizeof(SR_FragmentProcessor)
        << "\n\tFragment Bin Size:  " << sizeof(SR_FragmentBin)
        << "\n\tBlitter Task Size:  " << sizeof(SR_BlitProcessor)
        << "\n\tFragment tiles:     " << c << 'x' << r
        << std::endl;

    return mNumThreads;
}



/*-------------------------------------
-------------------------------------*/
void SR_ProcessorPool::run_shader_processors(const SR_Context* c, const SR_Mesh* m, const SR_Shader* s, SR_Framebuffer* fbo) noexcept
{
    SR_ShaderProcessor task;
    task.mType = SR_VERTEX_SHADER;
    SR_VertexProcessor& vertTask = task.mVertProcessor;

    // Divide all vertex processing amongst the available worker threads. Let
    // The threads work out between themselves how to partition the data.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        vertTask.mShader   = s;
        vertTask.mContext  = c;
        vertTask.mTileId   = threadId;
        vertTask.mNumTiles = mNumThreads;
        vertTask.mFboW     = fbo->width();
        vertTask.mFboH     = fbo->height();
        vertTask.mMesh     = *m;
        vertTask.mLocks    = mLocks.get();
        vertTask.mFragBins = mFragBins.get();

        // Busy waiting will be enabled the moment the first flush occurs on each
        // thread.
        SR_ProcessorPool::Worker* pWorker = mThreads[threadId];
        pWorker->busy_waiting(false);
        pWorker->push(task);
        pWorker->flush();
    }

    // sync-point
    wait();

    // calculate fragment tiles
    uint16_t cols;
    uint16_t rows;
    sr_calc_frag_tiles<uint16_t>(mNumThreads, cols, rows);

    const uint16_t fboW = fbo->width() / cols;
    const uint16_t fboH = fbo->height() / rows;

    task.mType = SR_FRAGMENT_SHADER;
    SR_FragmentProcessor& fragTask = task.mFragProcessor;

    // Run all remaining fragment processors. Prevent them from busy waiting
    // after they have completed processing all fragments.
    for (uint16_t threadId = 0; threadId < mNumThreads; ++threadId)
    {
        fragTask.mShader  = s;
        fragTask.mFbo     = fbo;
        fragTask.mBins    = mFragBins[threadId].data();
        fragTask.mNumBins = mFragBins[threadId].size();
        fragTask.mMode    = m->mode;
        fragTask.mFboX0   = (float)(fboW * (threadId % cols));
        fragTask.mFboY0   = (float)(fboH * ((threadId / cols) % rows));
        fragTask.mFboX1   = ((float)fboW + fragTask.mFboX0 - 1);
        fragTask.mFboY1   = ((float)fboH + fragTask.mFboY0 - 1);
        fragTask.mBinId   = 0;

        SR_ProcessorPool::Worker* pWorker = mThreads[threadId];
        pWorker->push(task);
        pWorker->flush();
        pWorker->busy_waiting(false);
    }

    // Each thread should now pause except for the main thread.
    wait();
    clear_fragment_bins();
}



/*-------------------------------------
-------------------------------------*/
void SR_ProcessorPool::run_blit_processors(const SR_Texture* t, SR_WindowBuffer* b) noexcept
{
    SR_ShaderProcessor processor;
    processor.mType = SR_BLIT_SHADER;

    SR_BlitProcessor& blitter = processor.mBlitter;

    // Process most of the rendering on other threads first.
    for (uint16_t i = 0; i < mNumThreads; ++i)
    {
        blitter.mThreadId   = i;
        blitter.mNumThreads = mNumThreads;
        blitter.mTexture    = t;
        blitter.mBackBuffer = b;

        mThreads[i]->push(processor);
    }

    execute();
}
