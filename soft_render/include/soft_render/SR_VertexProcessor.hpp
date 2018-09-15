
#ifndef SR_VERTEX_PROCESSOR_HPP
#define SR_VERTEX_PROCESSOR_HPP

#include <vector>

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
struct SR_FragmentBin; // SR_FragmentProcessor.hpp



/*--------------------------------------
 * Calculate the optimal tiling for the fragment shader threads
--------------------------------------*/
template <typename data_type>
inline void sr_calc_frag_tiles(data_type numThreads, data_type& numHoriz, data_type& numVert)
{
    // Create a set of horizontal and vertical tiles. This method will create
    // more horizontal tiles than vertical ones.
    const float tileCountf = ls::math::fast_sqrt((float)numThreads);
    data_type   tileCount  = (data_type)tileCountf;

    tileCount += (numThreads % tileCount) != 0;

    numHoriz = ls::math::gcd(numThreads, tileCount);
    numVert = numThreads / numHoriz;
}



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_VertexProcessor
{
    typedef ls::utils::SpinLock LockType;
    //typedef std::mutex LockType;

    // 64-128 bits
    const SR_Shader*  mShader;
    const SR_Context* mContext;

    // 32 bits
    uint16_t mTileId;
    uint16_t mNumTiles;

    // 32-bits
    uint16_t mFboW;
    uint16_t mFboH;

    // 128 bits
    SR_Mesh mMesh;

    // 64-128 bits
    LockType* mLocks;
    std::vector<SR_FragmentBin>* mFragBins;


    // 448 bits (56 bytes) max, padding not included

    void push_fragments(
        uint32_t fboW,
        uint32_t fboH,
        ls::math::vec4_t<float>*       screenCoords,
        ls::math::vec4_t<float>*       worldCoords,
        const ls::math::vec4_t<float>* varyings
    ) const noexcept;

    void execute() noexcept;
};



#endif /* SR_VERTEX_PROCESSOR_HPP */
