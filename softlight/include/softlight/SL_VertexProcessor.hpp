
#ifndef SL_RASTER_PROCESSOR_HPP
#define SL_RASTER_PROCESSOR_HPP

#include <atomic>

#include "softlight/SL_Mesh.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{

template <typename T>
union vec4_t;

template <typename T>
struct mat4_t;

} // end math namespace
} // end ls namespace

template <typename data_t>
union SL_BinCounter;

template <typename data_t>
union SL_BinCounterAtomic;

class SL_Context; // SL_Context.hpp
struct SL_FragmentBin; // SL_ShaderProcessor.hpp
struct SL_FragCoord;
class SL_Framebuffer; // SL_Framebuffer.hpp
struct SL_PointRasterizer;
struct SL_LineRasterizer;
struct SL_Shader; // SL_Shader.hpp
struct SL_TransformedVert;
struct SL_TriRasterizer;



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_VertexProcessor
{
  public:
    uint16_t mThreadId;
    uint16_t mNumThreads;

    SL_BinCounterAtomic<int_fast64_t>* mFragProcessors;
    SL_BinCounterAtomic<uint_fast64_t>* mBusyProcessors;

    const SL_Shader*  mShader;
    const SL_Context* mContext;
    SL_Framebuffer*   mFbo;

    size_t mNumMeshes;
    size_t mNumInstances;

    SL_RenderMode mRenderMode;

    const SL_Mesh* mMeshes;

    SL_BinCounterAtomic<uint32_t>* mBinsUsed;
    SL_BinCounter<uint32_t>* mBinIds;
    SL_BinCounter<uint32_t>* mTempBinIds; // pre-allocated storage for a radix sort

    SL_FragmentBin* mFragBins;
    SL_FragCoord* mFragQueues;

    virtual ~SL_VertexProcessor() noexcept = default;
    SL_VertexProcessor() noexcept = default;
    SL_VertexProcessor(const SL_VertexProcessor&) noexcept = default;
    SL_VertexProcessor(SL_VertexProcessor&&) noexcept = default;
    SL_VertexProcessor& operator=(const SL_VertexProcessor&) noexcept = default;
    SL_VertexProcessor& operator=(SL_VertexProcessor&&) noexcept = default;

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
