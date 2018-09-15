
#ifndef SR_SHADER_PROCESSOR_HPP
#define SR_SHADER_PROCESSOR_HPP

#include <cstdlib> // std::size_t
#include <vector>

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/WorkerThread.hpp"

#include "soft_render/SR_FragmentProcessor.hpp" // SR_FragmentBin
#include "soft_render/SR_Mesh.hpp"



struct SR_BlitProcessor;
class SR_Context;
class SR_Framebuffer;
class SR_Shader;
struct SR_ShaderProcessor;
class SR_Texture;
class SR_WindowBuffer;



/*-----------------------------------------------------------------------------
 * Wrapper around Shader Processors which allows operations on binned
 * Fragments
-----------------------------------------------------------------------------*/
class SR_ProcessorPool
{
  public:
    typedef ls::utils::Worker<SR_ShaderProcessor> Worker;
    typedef ls::utils::WorkerThread<SR_ShaderProcessor> ThreadedWorker;
    typedef ls::utils::SpinLock LockType;
    //typedef std::mutex LockType;

  private:
    ls::utils::Pointer<LockType[]> mLocks;

    ls::utils::Pointer<std::vector<SR_FragmentBin>[]> mFragBins;

    ls::utils::Pointer<Worker*[]> mThreads;

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

    void push_fragment_bin(unsigned threadId, const SR_FragmentBin& bin) noexcept;

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
 * Wait for the threads to finish
-------------------------------------*/
inline void SR_ProcessorPool::wait() noexcept
{
    // Each thread will pause except for the main thread.
    for (unsigned threadId = 0; threadId < mNumThreads-1u; ++threadId)
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



/*-------------------------------------
 * Run the processor threads
-------------------------------------*/
inline void SR_ProcessorPool::execute() noexcept
{
    this->flush();
    this->wait();
}



/*-------------------------------------
 * Push a fragment bin to the task list
-------------------------------------*/
inline void SR_ProcessorPool::push_fragment_bin(unsigned threadId, const SR_FragmentBin& bin) noexcept
{
    mLocks[threadId].lock();
    mFragBins[threadId].push_back(bin);
    mLocks[threadId].unlock();
}



/*-------------------------------------
 * Remove all bins from potential processing
-------------------------------------*/
inline void SR_ProcessorPool::clear_fragment_bins() noexcept
{
    for (uint16_t i = 0; i < mNumThreads; ++i)
    {
        mFragBins[i].clear();
    }
}



#endif /* SR_SHADER_PROCESSOR_HPP */
