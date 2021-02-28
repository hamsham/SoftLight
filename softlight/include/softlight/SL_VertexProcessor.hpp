
#ifndef SL_RASTER_PROCESSOR_HPP
#define SL_RASTER_PROCESSOR_HPP

#include <atomic>

#include "softlight/SL_Mesh.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
template <typename data_t>
union SL_BinCounter;

template <typename data_t>
union SL_BinCounterAtomic;

class SL_Context; // SL_Context.hpp
struct SL_FragmentBin; // SL_ShaderProcessor.hpp
struct SL_FragCoord;
class SL_Framebuffer; // SL_Framebuffer.hpp
class SL_Shader; // SL_Shader.hpp
struct SL_TransformedVert;

struct SL_LineRasterizer;
struct SL_PointRasterizer;
struct SL_TriRasterizer;



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_VertexProcessor
{
  public:
    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    SL_BinCounterAtomic<int_fast64_t>* mFragProcessors;
    SL_BinCounterAtomic<uint_fast64_t>* mBusyProcessors;

    // 96-192 bits
    const SL_Shader*  mShader;
    const SL_Context* mContext;
    SL_Framebuffer*   mFbo;

    // 64-128 bits
    size_t mNumMeshes;
    size_t mNumInstances;

    // 32 bits
    SL_RenderMode mRenderMode;

    // 32-64 bits
    const SL_Mesh* mMeshes;

    // 64-128 bits
    SL_BinCounterAtomic<uint32_t>* mBinsUsed;
    SL_BinCounter<uint32_t>* mBinIds;
    SL_BinCounter<uint32_t>* mTempBinIds; // pre-allocated storage for a radix sort

    // 96-192 bits
    SL_FragmentBin* mFragBins;
    SL_FragCoord* mFragQueues;

    // 448 bits (56 bytes) in 32-bit mode
    // 800 bits (100 bytes) in 64-bit mode
    // Padding not included

    virtual ~SL_VertexProcessor() = 0;

    virtual void execute() noexcept = 0;

  protected:
    template <typename RasterizerType>
    void flush_rasterizer() const noexcept;

    template <typename RasterizerType>
    void cleanup() noexcept;
};

/*--------------------------------------
 * Raster Specializations
--------------------------------------*/
extern template void SL_VertexProcessor::flush_rasterizer<SL_PointRasterizer>() const noexcept;
extern template void SL_VertexProcessor::flush_rasterizer<SL_LineRasterizer>() const noexcept;
extern template void SL_VertexProcessor::flush_rasterizer<SL_TriRasterizer>() const noexcept;

extern template void SL_VertexProcessor::cleanup<SL_PointRasterizer>() noexcept;
extern template void SL_VertexProcessor::cleanup<SL_LineRasterizer>() noexcept;
extern template void SL_VertexProcessor::cleanup<SL_TriRasterizer>() noexcept;



#endif /* SL_RASTER_PROCESSOR_HPP */
