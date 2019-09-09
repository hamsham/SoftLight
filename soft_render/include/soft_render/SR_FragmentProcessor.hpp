
#ifndef SR_FRAGMENT_PROCESSOR_HPP
#define SR_FRAGMENT_PROCESSOR_HPP

#include "lightsky/math/vec4.h"

#include "soft_render/SR_Mesh.hpp" // SR_RenderMode



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
struct SR_FragCoord; // SR_ShaderProcessor.hpp
struct SR_FragmentBin; // SR_ShaderProcessor.hpp
class SR_Framebuffer;
class SR_Shader;
class SR_Texture;



/*-----------------------------------------------------------------------------
 * Helper Functions
-----------------------------------------------------------------------------*/
/**
 * @brief Retrieve the offset to a threads first renderable scanline.
 *
 * @tparam data_t
 * The requested data type.
 *
 * @param numThreads
 * The number of threads which are currently being used for rendering.
 *
 * @param threadId
 * The current thread's ID (0-based index).
 *
 * @param fragmentY
 * The initial scanline for a line or triangle being rendered.
 */
template <typename data_t>
constexpr data_t sr_scanline_offset(
    const data_t numThreads,
    const data_t threadId,
    const data_t fragmentY)
{
    return numThreads - 1 - (((fragmentY % numThreads) + threadId) % numThreads);
}



/**
 * Calculate the optimal tiling for the fragment shader threads
 */
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



/**
 * Subdivide a rectangular region into equally spaced areas
 */
template <typename data_t>
inline ls::math::vec4_t<data_t> sr_subdivide_region(
    data_t w,
    data_t h,
    const data_t numThreads,
    const data_t threadId
) noexcept
{
    data_t cols;
    data_t rows;

    sr_calc_frag_tiles<data_t>(numThreads, cols, rows);
    w = w / cols;
    h = h / rows;

    const data_t x0 = w * (threadId % cols);
    const data_t y0 = h * ((threadId / cols) % rows);
    const data_t x1 = w + x0;
    const data_t y1 = h + y0;

    return ls::math::vec4_t<data_t>{x0, x1, y0, y1};
}



/*-----------------------------------------------------------------------------
 * Encapsulation of fragment processing on another thread.
 *
 * Point rasterization will divide the output framebuffer into equal parts,
 * so all threads will be assigned a specific region of the screen.
 *
 * Line rasterization will
-----------------------------------------------------------------------------*/
struct SR_FragmentProcessor
{
    // 16 bits
    uint16_t mThreadId;

    // 32 bits
    SR_RenderMode mMode;

    // 32-bits
    uint32_t mNumProcessors;

    // 64-bits
    float mFboW;
    float mFboH;

    // 64 bits
    uint_fast64_t mNumBins;

    // 256 bits
    const SR_Shader* mShader;
    SR_Framebuffer* mFbo;
    uint32_t* mBinIds;
    SR_FragmentBin* mBins;
    ls::math::vec4* mVaryings;
    SR_FragCoord* mQueues;

    // 432 bits = 54 bytes

    void render_point(
        const uint_fast64_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    void render_line(
        const uint_fast64_t binId,
        SR_Framebuffer* const fbo,
        const ls::math::vec4_t<int32_t> dimens) noexcept;

    void render_wireframe(const SR_FragmentBin* pBin, const SR_Texture* depthBuffer) const noexcept;

    void render_triangle(const SR_FragmentBin* pBin, const SR_Texture* depthBuffer) const noexcept;

    void flush_fragments(const SR_FragmentBin* pBin, uint_fast32_t numQueuedFrags, const SR_FragCoord* outCoords) const noexcept;

    void execute() noexcept;
};



#endif /* SR_FRAGMENT_PROCESSOR_HPP */
