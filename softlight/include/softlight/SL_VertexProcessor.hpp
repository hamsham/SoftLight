
#ifndef SL_RASTER_PROCESSOR_HPP
#define SL_RASTER_PROCESSOR_HPP

#include <atomic>

#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter, SL_BinCounterAtomic



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



/*-------------------------------------
 * Enum to help with determining face visibility
-------------------------------------*/
enum SL_ClipStatus
{
    SL_CLIP_STATUS_NOT_VISIBLE       = 0x00,
    SL_CLIP_STATUS_PARTIALLY_VISIBLE = 0x01,
    SL_CLIP_STATUS_FULLY_VISIBLE     = 0x03,
};



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_VertexProcessor
{
  public:
    uint16_t mThreadId;
    uint16_t mNumThreads;
    uint32_t mProcessBufferIndex;
    SL_VertProcessBuffer* mProcessBuffer;

    SL_BinCounterAtomic<uint_fast64_t>* mBusyProcessors;

    const SL_Shader*  mShader;
    const SL_Context* mContext;
    SL_Framebuffer*   mFbo;

    size_t mNumMeshes;
    size_t mNumInstances;

    SL_RenderMode mRenderMode;

    const SL_Mesh* mMeshes;

    SL_FragCoord* mFragQueues;

    virtual ~SL_VertexProcessor() noexcept = default;
    SL_VertexProcessor() noexcept {}
    SL_VertexProcessor(const SL_VertexProcessor&) noexcept = default;
    SL_VertexProcessor(SL_VertexProcessor&&) noexcept = default;
    SL_VertexProcessor& operator=(const SL_VertexProcessor&) noexcept = default;
    SL_VertexProcessor& operator=(SL_VertexProcessor&&) noexcept = default;

    virtual void execute() noexcept = 0;

    uint32_t active_buffer_index() const noexcept { return mProcessBufferIndex; }
    uint32_t next_buffer_index() const noexcept { return (mProcessBufferIndex+1) % SL_VERT_PROCESSOR_MAX_BUFFERS; }

    void flip_process_buffers() { mProcessBufferIndex = next_buffer_index(); }

  public:
    const SL_BinCounterAtomic<int_fast64_t>& active_frag_processors() const noexcept { return mProcessBuffer[mProcessBufferIndex].mFragProcessors; }
    SL_BinCounterAtomic<int_fast64_t>& active_frag_processors() noexcept { return mProcessBuffer[mProcessBufferIndex].mFragProcessors; }

    const SL_BinCounterAtomic<uint32_t>& active_num_bins_used() const noexcept { return mProcessBuffer[mProcessBufferIndex].mBinsUsed; }
    SL_BinCounterAtomic<uint32_t>& active_num_bins_used() noexcept { return mProcessBuffer[mProcessBufferIndex].mBinsUsed; }

    const SL_BinCounter<uint32_t>* active_bin_indices() const noexcept { return mProcessBuffer[mProcessBufferIndex].mBinIds; }
    SL_BinCounter<uint32_t>* active_bin_indices() noexcept { return mProcessBuffer[mProcessBufferIndex].mBinIds; }

    const SL_BinCounter<uint32_t>* active_temp_bin_indices() const noexcept { return mProcessBuffer[mProcessBufferIndex].mTempBinIds; }
    SL_BinCounter<uint32_t>* active_temp_bin_indices() noexcept { return mProcessBuffer[mProcessBufferIndex].mTempBinIds; }

    const SL_BinCounter<uint32_t>& active_bin_index(uint_fast64_t index) const noexcept { return mProcessBuffer[mProcessBufferIndex].mBinIds[index]; }
    SL_BinCounter<uint32_t>& active_bin_index(uint_fast64_t index) noexcept { return mProcessBuffer[mProcessBufferIndex].mBinIds[index]; }

    const SL_FragmentBin* active_frag_bins() const noexcept { return mProcessBuffer[mProcessBufferIndex].mFragBins; }
    SL_FragmentBin* active_frag_bins() noexcept { return mProcessBuffer[mProcessBufferIndex].mFragBins; }

  protected:
    template <typename RasterizerType>
    void flush_rasterizer() noexcept;

    template <typename RasterizerType>
    void cleanup() noexcept;
};

/*--------------------------------------
 * Raster Specializations
--------------------------------------*/
extern template void SL_VertexProcessor::flush_rasterizer<SL_PointRasterizer>() noexcept;
extern template void SL_VertexProcessor::flush_rasterizer<SL_LineRasterizer>() noexcept;
extern template void SL_VertexProcessor::flush_rasterizer<SL_TriRasterizer>() noexcept;

extern template void SL_VertexProcessor::cleanup<SL_PointRasterizer>() noexcept;
extern template void SL_VertexProcessor::cleanup<SL_LineRasterizer>() noexcept;
extern template void SL_VertexProcessor::cleanup<SL_TriRasterizer>() noexcept;



#endif /* SL_RASTER_PROCESSOR_HPP */
