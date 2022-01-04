
#ifndef SL_FRAGMENT_PROCESSOR_HPP
#define SL_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "softlight/SL_Mesh.hpp" // SL_RenderMode



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace math
    {
        struct half;
    }
}

template <typename data_t>
union SL_BinCounter;

struct SL_FragCoord; // SL_ShaderProcessor.hpp
struct SL_FragmentBin; // SL_ShaderProcessor.hpp
class SL_Framebuffer;
class SL_ViewportState;
struct SL_Shader;
class SL_Texture;



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
 *
 * Point rasterization will divide the output framebuffer into equal parts,
 * so all threads will be assigned a specific region of the screen.
 *
 * Line rasterization will
-----------------------------------------------------------------------------*/
struct SL_FragmentProcessor
{
    uint16_t mThreadId;
    SL_RenderMode mMode;
    uint32_t mNumProcessors;
    uint_fast32_t mNumBins;
    const SL_Shader* mShader;
    SL_Framebuffer* mFbo;
    const SL_ViewportState* mViewState;
    SL_BinCounter<uint32_t>* mBinIds;
    const SL_FragmentBin* mBins;
    SL_FragCoord* mQueues;

    virtual ~SL_FragmentProcessor() noexcept {}

    virtual void execute() noexcept = 0;
};



#endif /* SL_FRAGMENT_PROCESSOR_HPP */
