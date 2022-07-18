
/**
 * @file Internal functions, types, and helper modules shared between shader
 * pipelines.
 */
#ifndef SL_SHADERUTIL_HPP
#define SL_SHADERUTIL_HPP

#include <atomic>
#include <cstdlib> // size_t

#include "lightsky/setup/Api.h"
#include "lightsky/setup/Types.h"

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec4.h"

#include "softlight/SL_Config.hpp"



/*-----------------------------------------------------------------------------
 * Helper Functions
-----------------------------------------------------------------------------*/
/**
 * @brief Calculate the optimal tiling for the fragment shader threads.
 *
 * Given a number of threads, retrieve the optimal number of horizontal and
 * vertical subdivisions to divide a framebuffer.
 *
 * @param numThreads
 * The number of threads which will be rendering to a framebuffer.
 *
 * @param numHoriz
 * Output parameter to hold the number of horizontal subdivisions for a
 * framebuffer.
 *
 * @param numVert
 * Output parameter to hold the number of vertical subdivisions for a
 * framebuffer.
 */
template <typename data_type>
inline void sl_calc_frag_tiles(data_type numThreads, data_type& numHoriz, data_type& numVert) noexcept
{
    // Create a set of horizontal and vertical tiles. This method will create
    // more horizontal tiles than vertical ones.
    data_type tileCount = ls::math::fast_sqrt<data_type>(numThreads);
    tileCount += (numThreads % tileCount) != 0;
    numHoriz  = ls::math::gcd(numThreads, tileCount);
    numVert   = numThreads / numHoriz;
}



/**
 * @brief Subdivide a rectangular region into equally spaced areas.
 *
 * @param dimens
 * The x, y, width, and height height of the region to subdivide.
 *
 * @param numThreads
 * The number of threads to consider for subdivision.
 *
 * @param threadId
 * The current thread ID.
 *
 * @return A 4D vector, containing the following parameters for the current
 * thread, respectively:
 *     - Beginning X coordinate
 *     - Ending X coordinate
 *     - Beginning Y coordinate
 *     - Ending Y coordinate
 */
template <typename data_t>
inline ls::math::vec4_t<data_t> sl_subdivide_region(
    ls::math::vec4_t<data_t> dimens,
    const data_t numThreads,
    const data_t threadId
) noexcept
{
    data_t cols;
    data_t rows;

    sl_calc_frag_tiles<data_t>(numThreads, cols, rows);
    dimens[2] = dimens[2] / cols;
    dimens[3] = dimens[3] / rows;

    const data_t x0 = dimens[0] + (dimens[2] * (threadId % cols));
    const data_t y0 = dimens[1] + (dimens[3] * ((threadId / cols) % rows));
    const data_t x1 = dimens[2] + x0;
    const data_t y1 = dimens[3] + y0;

    return ls::math::vec4_t<data_t>{x0, x1, y0, y1};
}



/**
 * @brief Subdivide a rectangular region into equally spaced areas.
 *
 * @param w
 * The total width of the region to subdivide.
 *
 * @param h
 * The total height of the region to subdivide.
 *
 * @param numThreads
 * The number of threads to consider for subdivision.
 *
 * @param threadId
 * The current thread ID.
 *
 * @return A 4D vector, containing the following parameters for the current
 * thread, respectively:
 *     - Beginning X coordinate
 *     - Ending X coordinate
 *     - Beginning Y coordinate
 *     - Ending Y coordinate
 */
template <typename data_t>
inline ls::math::vec4_t<data_t> sl_subdivide_region(
    data_t w,
    data_t h,
    const data_t numThreads,
    const data_t threadId
) noexcept
{
    return sl_subdivide_region<data_t>(ls::math::vec4_t<data_t>{data_t{0}, data_t{0}, w, h}, numThreads, threadId);
}



/**
 * @brief Calculate a shader processor's start/end positions
 *
 * @tparam vertsPerPrim
 * The number of vertices which specify a primitive (i.e. 2 for lines, 3 for
 * triangles).
 *
 * @tparam lastThreadProcessesLess
 * Boolean value to determine if the final thread will be given additional
 * elements to process, while all other threads contain an even distribution
 * of work. If this is false, the last thread will be given less elements to
 * process while the other threads are given a larger even distribution of
 * work to process.
 *
 * @param totalVerts
 * The total number of vertices to be processed.
 *
 * @param numThreads
 * The number of threads available to process vertices.
 *
 * @param threadId
 * The current thread index.
 *
 * @param outBegin
 * Output to store the beginning vertex (inclusive) to process for the current
 * thread.
 *
 * @param outEnd
 * Output to store the final vertex (exclusive) to process for the current
 * thread.
 */
template <size_t vertsPerPrim, bool lastThreadProcessesLess = true>
inline void sl_calc_indexed_parition(size_t totalVerts, size_t numThreads, size_t threadId, size_t& outBegin, size_t& outEnd) noexcept
{
    size_t totalPrims = totalVerts / vertsPerPrim;
    size_t activeThreads = numThreads < totalPrims ? numThreads : totalPrims;
    size_t chunkSize = totalVerts / activeThreads;
    size_t remainder = chunkSize % vertsPerPrim;
    size_t beg;
    size_t end;

    // Set to 0 for the last thread to share chunk processing, plus remainder.
    // Set to 1 for the last thread to only process remaining values.
    if (lastThreadProcessesLess)
    {
        chunkSize += vertsPerPrim - remainder;
    }
    else
    {
        chunkSize -= remainder;
    }

    beg = threadId * chunkSize;
    end = beg + chunkSize;

    if (threadId == (numThreads - 1))
    {
        end += totalVerts - (chunkSize * activeThreads);
    }

    outBegin = beg;
    outEnd = end < totalVerts ? end : totalVerts;
}

/**
 * @brief Calculate a shader processor's start/end positions
 *
 * @tparam vertsPerPrim
 * The number of vertices which specify a primitive (i.e. 2 for lines, 3 for
 * triangles).
 *
 * @param totalVerts
 * The total number of vertices to be processed.
 *
 * @param numThreads
 * The number of threads available to process vertices.
 *
 * @param threadId
 * The current thread index.
 *
 * @param outBegin
 * Output to store the beginning vertex (inclusive) to process for the current
 * thread.
 *
 * @param outEnd
 * Output to store the final vertex (exclusive) to process for the current
 * thread.
 */
template <size_t vertsPerPrim>
void sl_calc_indexed_parition2(size_t count, size_t numThreads, size_t threadId, size_t& outBegin, size_t& outEnd)
{
    const size_t totalRange  = (count / numThreads);
    const size_t threadRange = totalRange + (vertsPerPrim - (totalRange % 3));
    const size_t begin       = threadRange * threadId;
    const size_t end         = begin + threadRange;

    outBegin = begin;
    outEnd   = end < count ? end : count;
}



/*-----------------------------------------------------------------------------
 * Depth-Test Operations
-----------------------------------------------------------------------------*/
/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncOFF
{
    constexpr int operator()(float, float) const noexcept
    {
        return true;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4&, const ls::math::vec4&) const noexcept
    {
        return 0x0F;
    }

    #if defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128, __m128) const noexcept
        {
            return _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF));
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t, float32x4_t) const noexcept
        {
            return vreinterpretq_f32_u32(vdupq_n_u32(0xFFFFFFFF));
        }

    #endif
};



/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncLT
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a < b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] < b[0],
            a[1] < b[1],
            a[2] < b[2],
            a[3] < b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_LT_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmplt_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vcltq_f32(a, b));
        }

    #endif
};

/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncLE
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a <= b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] <= b[0],
            a[1] <= b[1],
            a[2] <= b[2],
            a[3] <= b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_LE_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmple_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vcleq_f32(a, b));
        }

    #endif
};

/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncGT
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a > b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] > b[0],
            a[1] > b[1],
            a[2] > b[2],
            a[3] > b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_GT_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmpgt_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vcgtq_f32(a, b));
        }

    #endif
};

/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncGE
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a >= b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] >= b[0],
            a[1] >= b[1],
            a[2] >= b[2],
            a[3] >= b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_GE_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmpge_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vcgeq_f32(a, b));
        }

    #endif
};

/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncEQ
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a == b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] == b[0],
            a[1] == b[1],
            a[2] == b[2],
            a[3] == b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_EQ_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmpeq_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vceqq_f32(a, b));
        }

    #endif
};

/*-------------------------------------
-------------------------------------*/
struct SL_DepthFuncNE
{
    constexpr int operator()(float a, float b) const noexcept
    {
        return a != b;
    }

    inline LS_INLINE ls::math::vec4_t<int> operator()(const ls::math::vec4& a, const ls::math::vec4& b) const noexcept
    {
        return ls::math::vec4_t<int>{
            a[0] != b[0],
            a[1] != b[1],
            a[2] != b[2],
            a[3] != b[3]
        };
    }

    #if defined(LS_X86_AVX)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmp_ps(a, b, _CMP_NEQ_OQ);
        }

    #elif defined(LS_X86_SSE)
        inline LS_INLINE __m128 operator()(__m128 a, __m128 b) const noexcept
        {
            return _mm_cmpneq_ps(a, b);
        }

    #elif defined(LS_ARM_NEON)
        inline LS_INLINE float32x4_t operator()(float32x4_t a, float32x4_t b) const noexcept
        {
            return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(a, b)));
        }

    #endif
};



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SL_ShaderLimits
{
    SL_SHADER_MAX_WORLD_COORDS    = 3,
    SL_SHADER_MAX_SCREEN_COORDS   = 3,
    SL_SHADER_MAX_VARYING_VECTORS = 4,
    SL_SHADER_MAX_FRAG_OUTPUTS    = 4,

    // Maximum number of fragments that get queued before being placed on a
    // framebuffer.
    #if !SL_CONSERVE_MEMORY
    SL_SHADER_MAX_QUEUED_FRAGS    = 600,
    #else
    SL_SHADER_MAX_QUEUED_FRAGS    = 16,
    #endif /* !SL_CONSERVE_MEMORY */

    // Maximum number of vertex groups which get binned before being sent to a
    // fragment processor.
    SL_SHADER_MAX_BINNED_PRIMS    = 8192,
};



/*-----------------------------------------------------------------------------
 * Padded data types to avoid false sharing
-----------------------------------------------------------------------------*/
template <typename data_t>
union alignas(16) SL_BinCounter
{
    alignas(16) typename ls::setup::EnableIf<ls::setup::IsIntegral<data_t>::value, data_t>::type count;
    unsigned char padding[16];

    static_assert(sizeof(padding) == 16, "Invalid structure alignment.");

    ~SL_BinCounter() noexcept {}

    constexpr LS_INLINE SL_BinCounter(const data_t& n) noexcept :
        count{n}
    {}

    constexpr LS_INLINE SL_BinCounter(const SL_BinCounter& bc) noexcept :
        count{bc.count}
    {}

    constexpr LS_INLINE SL_BinCounter(SL_BinCounter&& bc) noexcept :
        count{bc.count}
    {}

    inline LS_INLINE SL_BinCounter& operator=(const data_t& n) noexcept
    {
        count = n;
        return *this;
    }

    inline LS_INLINE SL_BinCounter& operator=(const SL_BinCounter& bc) noexcept
    {
        count = bc.count;
        return *this;
    }

    inline LS_INLINE SL_BinCounter& operator=(SL_BinCounter&& bc) noexcept
    {
        count = bc.count;
        return *this;
    }
};



template <typename data_t>
union alignas(64) SL_BinCounterAtomic
{
    alignas(64) std::atomic<typename ls::setup::EnableIf<ls::setup::IsIntegral<data_t>::value, data_t>::type> count;
    unsigned char padding[64];

    static_assert(sizeof(padding) == 64, "Invalid structure alignment.");

    ~SL_BinCounterAtomic() noexcept {}

    constexpr LS_INLINE SL_BinCounterAtomic(const data_t& n) noexcept :
        count{n}
    {}

    constexpr LS_INLINE SL_BinCounterAtomic(const SL_BinCounterAtomic& bc) noexcept :
        count{bc.count}
    {}

    constexpr LS_INLINE SL_BinCounterAtomic(SL_BinCounterAtomic&& bc) noexcept :
        count{bc.count}
    {}

    inline LS_INLINE SL_BinCounterAtomic& operator=(const data_t& n) noexcept
    {
        count.store(n, std::memory_order_acq_rel);
        return *this;
    }

    inline LS_INLINE SL_BinCounterAtomic& operator=(const SL_BinCounterAtomic& bc) noexcept
    {
        count = bc.count;
        return *this;
    }

    inline LS_INLINE SL_BinCounterAtomic& operator=(SL_BinCounterAtomic&& bc) noexcept
    {
        count = bc.count;
        return *this;
    }
};



/*-----------------------------------------------------------------------------
 * Intermediate representation of a vertex after it's been run through a
 * shader.
-----------------------------------------------------------------------------*/
struct alignas(alignof(ls::math::vec4)) SL_TransformedVert
{
    ls::math::vec4 vert;
    ls::math::vec4 varyings[SL_SHADER_MAX_VARYING_VECTORS];
};

static_assert(sizeof(SL_TransformedVert) == sizeof(ls::math::vec4)*5, "Unexpected size of SL_TransformedVert. Please update the vertex cache copy routines.");



/*-----------------------------------------------------------------------------
 * Intermediate Fragment Storage for Binning
 *
 * Aligned to 32 bytes to ensure aligned loads/stores when using AVX
-----------------------------------------------------------------------------*/
struct alignas(sizeof(ls::math::vec4)*2) SL_FragmentBin
{
    // 4-byte floats * 4-element vector * 3 vectors-per-tri = 48 bytes
    ls::math::vec4 mScreenCoords[SL_SHADER_MAX_SCREEN_COORDS];

    // 4-byte floats * 4-element vector * 3 barycentric coordinates = 48 bytes
    ls::math::vec4 mBarycentricCoords[SL_SHADER_MAX_SCREEN_COORDS];

    // 4-byte floats * 4-element vector * 3-vectors-per-tri * 4 varyings-per-vertex = 192 bytes
    ls::math::vec4 mVaryings[SL_SHADER_MAX_SCREEN_COORDS * SL_SHADER_MAX_VARYING_VECTORS];

    // 8 bytes
    uint_fast64_t primIndex;

    // 8 bytes
    uint_fast64_t pad0;

    // 16 bytes of padding to reduce false-sharing
    ls::math::vec4 pad1;

    // 304 bytes = 2432 bits
};

static_assert(sizeof(SL_FragmentBin) == sizeof(ls::math::vec4)*20, "Unexpected size of SL_FragmentBin. Please update all varying memcpy routines.");



/*-----------------------------------------------------------------------------
 * Helper structure to put a pixel on the screen
-----------------------------------------------------------------------------*/
struct alignas(alignof(uint64_t)) SL_FragCoordXYZ
{
    uint16_t x;
    uint16_t y;
    float depth;
};

static_assert(sizeof(SL_FragCoordXYZ) == sizeof(uint64_t), "Unexpected size of SL_FragCoordXYZ. Please update all functions using this structure.");



struct SL_FragCoord
{
    union
    {
        ls::math::vec4 bc[SL_SHADER_MAX_QUEUED_FRAGS]; // 32*4
        float lineInterp[SL_SHADER_MAX_QUEUED_FRAGS]; // 32*4
    };

    SL_FragCoordXYZ coord[SL_SHADER_MAX_QUEUED_FRAGS];
    // 256 bits / 32 bytes
};



#endif /* SL_SHADERUTIL_HPP */
