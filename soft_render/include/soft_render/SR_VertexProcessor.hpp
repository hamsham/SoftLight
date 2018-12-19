
#ifndef SR_VERTEX_PROCESSOR_HPP
#define SR_VERTEX_PROCESSOR_HPP

#include <array>

#include "lightsky/utils/WorkerThread.hpp"

#include "lightsky/math/scalar_utils.h"

#include "soft_render/SR_Mesh.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace utils
    {
        template<class WorkerTaskType>
        class Worker;

        template<class WorkerTaskType>
        class WorkerThread;
    } // end utils namespace

    namespace math
    {
        template<typename>
        union vec4_t;
    } // end math namespace
} // end ls namespace

class SR_Context; // SR_Context.hpp
class SR_Shader; // SR_Shader.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp



/*--------------------------------------
 * Calculate the optimal tiling for the fragment shader threads
--------------------------------------*/
template <typename data_type>
inline void sr_calc_frag_tiles(data_type numThreads, data_type& numHoriz, data_type& numVert) noexcept
{
    // Create a set of horizontal and vertical tiles. This method will create
    // more horizontal tiles than vertical ones.
    data_type tileCount = ls::math::fast_sqrt<data_type>(numThreads);
    tileCount += (numThreads % tileCount) != 0;
    numHoriz  = ls::math::gcd(numThreads, tileCount);
    numVert   = numThreads / numHoriz;
}



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_VertexProcessor
{
    // 128-256 bits
    const SR_Shader*  mShader;
    const SR_Context* mContext;
    SR_Framebuffer*   mFbo;
    std::atomic_uint_fast64_t* mFragProcessors;
    std::atomic_uint_fast64_t* mBusyProcessors;

    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 32-bits
    uint16_t mFboW;
    uint16_t mFboH;

    // 128 bits
    SR_Mesh mMesh;

    // 64-128 bits
    std::atomic_uint_fast64_t* mBinsUsed;
    SR_FragmentBin* mFragBins;
    std::array<SR_FragCoord, 4096>* mFragQueues;

    // 768 bits (96 bytes) max, padding not included

    void flush_fragments() const noexcept;

    void push_fragments(
        float fboW,
        float fboH,
        ls::math::vec4_t<float>*       screenCoords,
        ls::math::vec4_t<float>*       worldCoords,
        const ls::math::vec4_t<float>* varyings
    ) const noexcept;

    void execute() noexcept;
};



#endif /* SR_VERTEX_PROCESSOR_HPP */
