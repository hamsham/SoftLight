
#include "lightsky/setup/Macros.h"

#include "lightsky/setup/OS.h"

#ifdef LS_OS_UNIX
    #include <time.h> // nanosleep, time_spec
#endif

#include "lightsky/utils/Assertions.h" // LS_DEBUG_ASSERT
#include "lightsky/utils/Sort.hpp" // utils::sort_quick

#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_Config.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_ShaderProcessor.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;



namespace
{



#if SR_VERTEX_CACHING_ENABLED
class SR_PTVCache
{
  public:
    static constexpr size_t PTV_CACHE_SIZE = SR_VERTEX_CACHE_SIZE;

    static constexpr size_t PTV_CACHE_MISS = ~(size_t)0;

    size_t lruIndices[PTV_CACHE_SIZE];

    #if SR_VERTEX_CACHE_TYPE_LRU
    long long lruCounts[PTV_CACHE_SIZE];
    #endif

    math::vec4 lruVertices[PTV_CACHE_SIZE];

    math::vec4 lruVaryings[PTV_CACHE_SIZE][SR_SHADER_MAX_VARYING_VECTORS];

    SR_VertexParam lruParam;

    ls::math::vec4_t<float> (*shader)(SR_VertexParam&);

    SR_PTVCache(const SR_PTVCache&) = delete;
    SR_PTVCache(SR_PTVCache&&) = delete;
    SR_PTVCache& operator=(const SR_PTVCache&) = delete;
    SR_PTVCache& operator=(SR_PTVCache&&) = delete;

    SR_PTVCache(ls::math::vec4_t<float> (*pShader)(SR_VertexParam&), const SR_VertexParam& inParam) noexcept
    {
        for (size_t& index : lruIndices)
        {
            index = PTV_CACHE_MISS;
        }

        #if SR_VERTEX_CACHE_TYPE_LRU
            for (long long& count : lruCounts)
            {
                count = -1;
            }
        #endif

        lruParam = inParam;
        shader = pShader;
    }

    #if SR_VERTEX_CACHE_TYPE_LRU
    inline void query_and_update(size_t key, math::vec4& outVert, math::vec4* outVaryings, size_t numVaryings) noexcept
    {
        size_t ret = PTV_CACHE_MISS;
        size_t cacheIndex = 0;
        long long maxCount = 0;

        for (size_t i = PTV_CACHE_SIZE; i--;)
        {
            if (maxCount > lruCounts[i])
            {
                maxCount = lruCounts[i];
                cacheIndex = i;
            }

            if (lruIndices[i] == key)
            {
                ret = key;
                lruCounts[i] = 0;
                outVert = lruVertices[i];

                for (size_t v = numVaryings; v--;)
                {
                    outVaryings[v] = lruVaryings[i][v];
                }
            }
            else
            {
                if (lruCounts[i] >= 0)
                {
                    ++lruCounts[i];
                }
            }
        }

        if (ret == PTV_CACHE_MISS)
        {
            lruIndices[cacheIndex] = key;
            lruCounts[cacheIndex] = 0;
            lruParam.vertId = key;
            lruParam.pVaryings = outVaryings[cacheIndex];
            outVert = lruVertices[cacheIndex] = shader(lruParam);

            for (size_t v = numVaryings; v--;)
            {
                lruVaryings[cacheIndex][v] = outVaryings[v];
            }
        }
    }

    #else // SR_VERTEX_CACHE_TYPE_LRU

    inline void query_and_update(size_t key, math::vec4& outVert, math::vec4* outVaryings, size_t numVaryings) noexcept
    {
        size_t i = key & (PTV_CACHE_SIZE-1);

        if (lruIndices[i] == key)
        {
            outVert = lruVertices[i];

            for (size_t v = numVaryings; v--;)
            {
                outVaryings[v] = lruVaryings[i][v];
            }

            return;
        }

        lruIndices[i] = key;
        lruParam.vertId = key;
        lruParam.pVaryings = outVaryings;
        outVert = lruVertices[i] = shader(lruParam);

        for (size_t v = numVaryings; v--;)
        {
            lruVaryings[i][v] = outVaryings[v];
        }
    }

    #endif // SR_VERTEX_CACHE_TYPE_LRU
};

#endif // SR_VERTEX_CACHING_ENABLED



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline LS_INLINE math::vec4 sr_perspective_divide(const math::vec4 v) noexcept
{
    #if !defined(LS_ARCH_X86)
        const math::vec4&& wInv = math::rcp(math::vec4{v[3]});
        const math::vec4&& vMul = v * wInv;
        return math::vec4{vMul[0], vMul[1], vMul[2], wInv[0]};
    #else
        const __m128 wInv = _mm_rcp_ps(_mm_set1_ps(v[3]));
        const __m128 vMul = _mm_mul_ps(v.simd, wInv);
        return math::vec4{_mm_blend_ps(wInv, vMul, 0x07)};
    #endif
}



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline LS_INLINE void sr_perspective_divide3(math::vec4* v) noexcept
{
    #if !defined(LS_ARCH_X86)
        const math::vec4&& wInv0 = math::rcp(math::vec4{v[0][3]});
        const math::vec4&& wInv1 = math::rcp(math::vec4{v[1][3]});
        const math::vec4&& wInv2 = math::rcp(math::vec4{v[2][3]});
        const math::vec4&& vMul0 = v[0] * wInv0;
        const math::vec4&& vMul1 = v[1] * wInv1;
        const math::vec4&& vMul2 = v[2] * wInv2;
        v[0] = {vMul0[0], vMul0[1], vMul0[2], wInv0[0]};
        v[1] = {vMul1[0], vMul1[1], vMul1[2], wInv1[0]};
        v[2] = {vMul2[0], vMul2[1], vMul2[2], wInv2[0]};
    #else
        const __m128 wInv0 = _mm_rcp_ps(_mm_load1_ps(reinterpret_cast<const float*>(v)+3));
        const __m128 wInv1 = _mm_rcp_ps(_mm_load1_ps(reinterpret_cast<const float*>(v)+7));
        const __m128 wInv2 = _mm_rcp_ps(_mm_load1_ps(reinterpret_cast<const float*>(v)+11));

        const __m128 vMul0 = _mm_mul_ps(v[0].simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v[1].simd, wInv1);
        const __m128 vMul2 = _mm_mul_ps(v[2].simd, wInv2);

        _mm_store_ps(reinterpret_cast<float*>(v+0), _mm_blend_ps(wInv0, vMul0, 0x07));
        _mm_store_ps(reinterpret_cast<float*>(v+1), _mm_blend_ps(wInv1, vMul1, 0x07));
        _mm_store_ps(reinterpret_cast<float*>(v+2), _mm_blend_ps(wInv2, vMul2, 0x07));
    #endif
}



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline LS_INLINE void sr_world_to_screen_coords_divided(math::vec4& v, const float widthScale, const float heightScale) noexcept
{
    #if !defined(LS_ARCH_X86)
        v[0] = math::max(0.f, math::floor(math::fmadd(widthScale,  v[0], widthScale)));
        v[1] = math::max(0.f, math::floor(math::fmadd(heightScale, v[1], heightScale)));
        //v = math::fmadd(math::vec4{widthScale, heightScale, 1.f, 1.f}, v, math::vec4{widthScale, heightScale, 0.f, 0.f});
    #else
        const __m128 wh0 = _mm_set_ps(0.f, 0.f, heightScale, widthScale);
        const __m128 wh1 = _mm_set_ps(1.f, 1.f, heightScale, widthScale);

        __m128 scl = _mm_fmadd_ps(wh1, v.simd, wh0);
        scl = _mm_max_ps(_mm_floor_ps(scl), _mm_setzero_ps());
        v.simd = _mm_blend_ps(scl, v.simd, 0x0C);
    #endif
}



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline LS_INLINE void sr_world_to_screen_coords_divided3(math::vec4* v, const float widthScale, const float heightScale) noexcept
{
    #if !defined(LS_ARCH_X86)
        v[0][0] = math::max(0.f, math::floor(math::fmadd(widthScale,  v[0][0], widthScale)));
        v[0][1] = math::max(0.f, math::floor(math::fmadd(heightScale, v[0][1], heightScale)));

        v[1][0] = math::max(0.f, math::floor(math::fmadd(widthScale,  v[1][0], widthScale)));
        v[1][1] = math::max(0.f, math::floor(math::fmadd(heightScale, v[1][1], heightScale)));

        v[2][0] = math::max(0.f, math::floor(math::fmadd(widthScale,  v[2][0], widthScale)));
        v[2][1] = math::max(0.f, math::floor(math::fmadd(heightScale, v[2][1], heightScale)));
        //v = math::fmadd(math::vec4{widthScale, heightScale, 1.f, 1.f}, v, math::vec4{widthScale, heightScale, 0.f, 0.f});
    #else

        const __m128 wh0 = _mm_set_ps(0.f, 0.f, heightScale, widthScale);
        const __m128 wh1 = _mm_set_ps(1.f, 1.f, heightScale, widthScale);

        const __m128 v0 = _mm_load_ps(reinterpret_cast<float*>(v+0));
        const __m128 v1 = _mm_load_ps(reinterpret_cast<float*>(v+1));
        const __m128 v2 = _mm_load_ps(reinterpret_cast<float*>(v+2));

        __m128 scl0 = _mm_fmadd_ps(wh1, v0, wh0);
        __m128 scl1 = _mm_fmadd_ps(wh1, v1, wh0);
        __m128 scl2 = _mm_fmadd_ps(wh1, v2, wh0);

        scl0 = _mm_max_ps(_mm_floor_ps(scl0), _mm_setzero_ps());
        scl1 = _mm_max_ps(_mm_floor_ps(scl1), _mm_setzero_ps());
        scl2 = _mm_max_ps(_mm_floor_ps(scl2), _mm_setzero_ps());

        _mm_store_ps(reinterpret_cast<float*>(v+0), _mm_blend_ps(scl0, v0, 0x0C));
        _mm_store_ps(reinterpret_cast<float*>(v+1), _mm_blend_ps(scl1, v1, 0x0C));
        _mm_store_ps(reinterpret_cast<float*>(v+2), _mm_blend_ps(scl2, v2, 0x0C));
    #endif
}



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline LS_INLINE void sr_world_to_screen_coords(math::vec4& v, const float widthScale, const float heightScale) noexcept
{
    const float wInv = 1.f / v.v[3];
    math::vec4&& temp = v * wInv;

    temp[0] = widthScale  + temp[0] * widthScale;
    temp[1] = heightScale + temp[1] * heightScale;

    v[0] = temp[0];
    v[1] = temp[1];
    v[2] = temp[2];
    v[3] = wInv;
}



/*--------------------------------------
 * Get the next vertex from an IBO
--------------------------------------*/
inline size_t get_next_vertex(const SR_IndexBuffer* pIbo, size_t vId) noexcept
{
    switch (pIbo->type())
    {
        case VERTEX_DATA_BYTE:  return *reinterpret_cast<const unsigned char*>(pIbo->element(vId));
        case VERTEX_DATA_SHORT: return *reinterpret_cast<const unsigned short*>(pIbo->element(vId));
        case VERTEX_DATA_INT:   return *reinterpret_cast<const unsigned int*>(pIbo->element(vId));
        default:
            LS_DEBUG_ASSERT(false);
            break;
    }
    return vId;
}



inline LS_INLINE math::vec3_t<size_t> get_next_vertex3(const SR_IndexBuffer* pIbo, size_t vId) noexcept
{
    math::vec3_t<unsigned char> byteIds;
    math::vec3_t<unsigned short> shortIds;
    math::vec3_t<unsigned int> intIds;

    switch (pIbo->type())
    {
        case VERTEX_DATA_BYTE:
            byteIds = *reinterpret_cast<const decltype(byteIds)*>(pIbo->element(vId));
            return (math::vec3_t<size_t>)byteIds;
            break;

        case VERTEX_DATA_SHORT:
            shortIds = *reinterpret_cast<const decltype(shortIds)*>(pIbo->element(vId));
            return (math::vec3_t<size_t>)shortIds;
            break;

        case VERTEX_DATA_INT:
            intIds = *reinterpret_cast<const decltype(intIds)*>(pIbo->element(vId));
            return (math::vec3_t<size_t>)intIds;
            break;

        default:
            LS_DEBUG_ASSERT(false);
    }

    return math::vec3_t<size_t>{0, 0, 0};
}



/*--------------------------------------
 * Cull backfaces of a triangle
--------------------------------------*/
inline int back_face_visible(const math::vec4* screenCoords) noexcept
{
    //return (0.f >= math::dot(math::vec4{0.f, 0.f, 1.f, 1.f}, math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0])));

    #if !defined(LS_ARCH_X86)
        const math::mat3 det{
            math::vec3{screenCoords[0][0], screenCoords[0][1], screenCoords[0][3]},
            math::vec3{screenCoords[1][0], screenCoords[1][1], screenCoords[1][3]},
            math::vec3{screenCoords[2][0], screenCoords[2][1], screenCoords[2][3]}
        };
        return (0.f > math::determinant(det));
    #else
        // Swap the z and w components for each vector. Z will be discarded later
        const __m128 col4 = _mm_permute_ps(screenCoords[0].simd, 0xB4);

        constexpr int shuffleMask120 = 0x8D; // indices: <base> + (2, 0, 3, 1): 10001101
        constexpr int shuffleMask201 = 0x93; // indices: <base> + (2, 1, 0, 3): 10010011

        const __m128 col3 = _mm_permute_ps(screenCoords[2].simd, shuffleMask120);
        const __m128 col2 = _mm_permute_ps(screenCoords[1].simd, shuffleMask201);
        const __m128 col1 = _mm_permute_ps(screenCoords[2].simd, shuffleMask201);
        const __m128 col0 = _mm_permute_ps(screenCoords[1].simd, shuffleMask120);

        const __m128 sub0 = _mm_fmsub_ps(col0, col1, _mm_mul_ps(col2, col3));

        // Remove the Z component which was shuffled earlier
        const __m128 mul2 = _mm_blend_ps(_mm_mul_ps(sub0, col4), _mm_setzero_ps(), 0x08);

        // horizontal add: swap the words of each vector, add, then swap each
        // half of the vectors and perform a final add.
        const __m128 swap = _mm_add_ps(mul2, _mm_permute_ps(mul2, 0xB1));
        const __m128 sum  = _mm_add_ps(swap, _mm_permute_ps(swap, 0x0F));

        return 0.f > _mm_cvtss_f32(sum);
    #endif
}



/*--------------------------------------
 * Cull frontfaces of a triangle
--------------------------------------*/
inline int front_face_visible(const math::vec4* screenCoords) noexcept
{
    //return (0.f >= math::dot(math::vec4{0.f, 0.f, 1.f, 1.f}, math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0])));

    #if !defined(LS_ARCH_X86)
        const math::mat3 det{
            math::vec3{screenCoords[0][0], screenCoords[0][1], screenCoords[0][3]},
            math::vec3{screenCoords[1][0], screenCoords[1][1], screenCoords[1][3]},
            math::vec3{screenCoords[2][0], screenCoords[2][1], screenCoords[2][3]}
        };
        return (0.f < math::determinant(det));
    #else
        // Swap the z and w components for each vector. Z will be discarded later
        const __m128 col4 = _mm_permute_ps(screenCoords[0].simd, 0xB4);

        constexpr int shuffleMask120 = 0x8D; // indices: <base> + (2, 0, 3, 1): 10001101
        constexpr int shuffleMask201 = 0x93; // indices: <base> + (2, 1, 0, 3): 10010011

        const __m128 col3 = _mm_permute_ps(screenCoords[2].simd, shuffleMask120);
        const __m128 col2 = _mm_permute_ps(screenCoords[1].simd, shuffleMask201);
        const __m128 col1 = _mm_permute_ps(screenCoords[2].simd, shuffleMask201);
        const __m128 col0 = _mm_permute_ps(screenCoords[1].simd, shuffleMask120);

        const __m128 sub0 = _mm_fmsub_ps(col0, col1, _mm_mul_ps(col2, col3));

        // Remove the Z component which was shuffled earlier
        const __m128 mul2 = _mm_blend_ps(_mm_mul_ps(sub0, col4), _mm_setzero_ps(), 0x08);

        // horizontal add: swap the words of each vector, add, then swap each
        // half of the vectors and perform a final add.
        const __m128 swap = _mm_add_ps(mul2, _mm_permute_ps(mul2, 0xB1));
        const __m128 sum  = _mm_add_ps(swap, _mm_permute_ps(swap, 0x0F));

        return 0.f < _mm_cvtss_f32(sum);
    #endif
}



/*--------------------------------------
 * Cull only triangle outside of the screen
--------------------------------------*/
inline LS_INLINE SR_ClipStatus face_visible(const math::vec4 clipCoords[SR_SHADER_MAX_WORLD_COORDS]) noexcept
{
    #if SR_PRIMITIVE_CLIPPING_ENABLED != 0
        const float w0p = clipCoords[0][3];
        const float w1p = clipCoords[1][3];
        const float w2p = clipCoords[2][3];

        const float w0n = -clipCoords[0][3];
        const float w1n = -clipCoords[1][3];
        const float w2n = -clipCoords[2][3];

        if (clipCoords[0] >= w0n && clipCoords[0] <= w0p
        &&  clipCoords[1] >= w1n && clipCoords[1] <= w1p
        &&  clipCoords[2] >= w2n && clipCoords[2] <= w2p)
        {
            return SR_TRIANGLE_FULLY_VISIBLE;
        }

        if (w0p >= 1.f || w1p >= 1.f || w2p >= 1.f)
        {
            return SR_TRIANGLE_PARTIALLY_VISIBLE;
        }
    #else
        if (math::min(clipCoords[0][3], clipCoords[1][3], clipCoords[2][3]) >= 0.f)
        {
            // something's visible, but near-plane clipping might say otherwise
            return SR_TRIANGLE_PARTIALLY_VISIBLE;
        }
    #endif /* SR_VERTEX_CLIPPING_ENABLED */

    return SR_TRIANGLE_NOT_VISIBLE;
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SR_VertexProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute a fragment processor
-------------------------------------*/
void SR_VertexProcessor::flush_bins() const noexcept
{
    // Sync Point 1 indicates that all triangles have been sorted.
    // Since we only have two sync points for the bins to process, a negative
    // value for mFragProcessors will indicate all threads are ready to process
    // fragments. A zero-value for mFragProcessors indicates we can continue
    // processing vertices.
    const int_fast64_t   syncPoint1 = -mNumThreads-1;
    int_fast64_t         tileId     = mFragProcessors->fetch_add(1, std::memory_order_acq_rel);

    #if defined(LS_OS_UNIX)
        struct timespec sleepAmt;
        sleepAmt.tv_sec = 0;
        sleepAmt.tv_nsec = 1;
        (void)sleepAmt;
    #endif

    // Sort the bins based on their depth.
    if (tileId == mNumThreads-1u)
    {
        #if !SR_TUNE_HIGH_POLY
        const uint_fast64_t maxElements = math::min<uint64_t>(mBinsUsed->load(std::memory_order_consume), SR_SHADER_MAX_BINNED_PRIMS);

        // Blended fragments get sorted back-to-front for correct coloring.
        if (mShader->fragment_shader().blend == SR_BLEND_OFF)
        {
            ls::utils::sort_quick_iterative<uint32_t>(mBinIds, maxElements, [&](uint32_t a, uint32_t b)->bool {
                return mFragBins[a] > mFragBins[b];
            });
        }
        else
        {
            // Sort opaque objects from front-to-back to fortify depth testing.
            ls::utils::sort_quick_iterative<uint32_t>(mBinIds, maxElements, [&](uint32_t a, uint32_t b)->bool {
                return mFragBins[a] < mFragBins[b];
            });
        }
        #endif

        // Let all threads know they can process fragments.
        mFragProcessors->store(syncPoint1, std::memory_order_release);
    }
    else
    {
        while (LS_LIKELY(mFragProcessors->load(std::memory_order_consume) > 0))
        {
            // Wait until the bins are sorted
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #elif defined(LS_OS_LINUX) || defined(LS_OS_ANDROID)
                clock_nanosleep(CLOCK_MONOTONIC, 0, &sleepAmt, nullptr);
            #elif defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
                nanosleep(&sleepAmt, nullptr);
            #else
                std::this_thread::yield();
            #endif
        }
    }

    SR_FragmentProcessor fragTask{
        (uint16_t)tileId,
        mRenderMode,
        (uint32_t)mNumThreads,
        math::min<uint64_t>(mBinsUsed->load(std::memory_order_relaxed), SR_SHADER_MAX_BINNED_PRIMS),
        mShader,
        mFbo,
        mBinIds,
        mFragBins,
        mVaryings + tileId * SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_QUEUED_FRAGS,
        mFragQueues + tileId
    };

    fragTask.execute();

    // Indicate to all threads we can now process more vertices
    if (-2 == mFragProcessors->fetch_add(1, std::memory_order_acq_rel))
    {
        mBinsUsed->store(0, std::memory_order_release);
        mFragProcessors->store(0, std::memory_order_release);
        return;
    }

    // Wait for the last thread to reset the number of available bins.
    while (LS_LIKELY(mFragProcessors->load(std::memory_order_consume) < 0))
    {
        // wait until all fragments are rendered across the other threads.
        // High-poly models should rely only on _mm_pause() and not sleep
        // in a hyper-threaded environment.
        #if SR_TUNE_HIGH_POLY
            #if defined(LS_ARCH_X86)
                _mm_pause();
            #elif defined(LS_OS_LINUX) || defined(LS_OS_ANDROID)
                clock_nanosleep(CLOCK_MONOTONIC, 0, &sleepAmt, nullptr);
            #elif defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
                nanosleep(&sleepAmt, nullptr);
            #else
                std::this_thread::yield();
            #endif
        #else
            #if defined(LS_OS_LINUX) || defined(LS_OS_ANDROID)
                clock_nanosleep(CLOCK_MONOTONIC, 0, &sleepAmt, nullptr);
            #elif defined(LS_OS_OSX) || defined(LS_OS_IOS) || defined(LS_OS_IOS_SIM)
                nanosleep(&sleepAmt, nullptr);
            #elif defined(LS_ARCH_X86)
                _mm_pause();
            #else
                std::this_thread::yield();
            #endif
        #endif
    }
}



/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
template <int renderMode>
inline void SR_VertexProcessor::push_bin(
    float fboW,
    float fboH,
    math::vec4* const screenCoords,
    const math::vec4* varyings
) const noexcept
{
    const uint_fast32_t numVaryings = mShader->get_num_varyings();
    unsigned numVerts;
    std::atomic_uint_fast64_t* const pLocks = mBinsUsed;

    const math::vec4& p0 = screenCoords[0];
    const math::vec4& p1 = screenCoords[1];
    const math::vec4& p2 = screenCoords[2];
    float bboxMinX;
    float bboxMinY;
    float bboxMaxX;
    float bboxMaxY;

    // calculate the bounds of the tile which a certain thread will be
    // responsible for

    // render points through whichever tile/thread they appear in
    switch (renderMode)
    {
        case RENDER_MODE_POINTS:
            numVerts = 1;
            bboxMinX = p0[0];
            bboxMaxX = p0[0];
            bboxMinY = p0[1];
            bboxMaxY = p0[1];
            break;

        case RENDER_MODE_LINES:
            // establish a bounding box to detect overlap with a thread's tiles
            numVerts = 2;
            bboxMinX = math::min(p0[0], p1[0]);
            bboxMinY = math::min(p0[1], p1[1]);
            bboxMaxX = math::max(p0[0], p1[0]);
            bboxMaxY = math::max(p0[1], p1[1]);
            break;

        case RENDER_MODE_TRIANGLES:
            // establish a bounding box to detect overlap with a thread's tiles
            numVerts = 3;
            bboxMinX = math::min(p0[0], p1[0], p2[0]);
            bboxMinY = math::min(p0[1], p1[1], p2[1]);
            bboxMaxX = math::max(p0[0], p1[0], p2[0]);
            bboxMaxY = math::max(p0[1], p1[1], p2[1]);
            break;

        default:
            return;
    }

    int isPrimHidden = (bboxMaxX < 0.f || bboxMaxY < 0.f || fboW < bboxMinX || fboH < bboxMinY);
    isPrimHidden = isPrimHidden || (bboxMaxX-bboxMinX < 1.f) || (bboxMaxY-bboxMinY < 1.f);
    if (isPrimHidden)
    {
        return;
    }

    SR_FragmentBin* const pFragBins = mFragBins;

    // Check if the output bin is full
    uint_fast64_t binId;

    // Attempt to grab a bin index. Flush the bins if they've filled up.
    while (true)
    {
        #if defined(LS_ARCH_X86)
            _mm_lfence();
        #endif

        binId = pLocks->fetch_add(1, std::memory_order_acq_rel);

        #if defined(LS_ARCH_X86)
            _mm_sfence();
        #endif

        if (LS_UNLIKELY(binId >= SR_SHADER_MAX_BINNED_PRIMS))
        {
            flush_bins();
            continue;
        }

        break;
    }

    // place a triangle into the next available bin
    mBinIds[binId] = (uint32_t)binId;

    // Copy all per-vertex coordinates and varyings to the fragment bins
    // which will need the data for interpolation. The perspective-divide
    // is only used for rendering triangles.
    SR_FragmentBin& bin = pFragBins[binId];
    bin.mScreenCoords[0] = p0;
    bin.mScreenCoords[1] = p1;
    bin.mScreenCoords[2] = p2;

    if (renderMode == SR_RenderMode::RENDER_MODE_TRIANGLES)
    {
        const float denom = 1.f / ((p0[0]-p2[0])*(p1[1]-p0[1]) - (p0[0]-p1[0])*(p2[1]-p0[1]));
        bin.mBarycentricCoords[0] = denom*math::vec4(p1[1]-p2[1], p2[1]-p0[1], p0[1]-p1[1], 0.f);
        bin.mBarycentricCoords[1] = denom*math::vec4(p2[0]-p1[0], p0[0]-p2[0], p1[0]-p0[0], 0.f);
        bin.mBarycentricCoords[2] = denom*math::vec4(
            p1[0]*p2[1] - p2[0]*p1[1],
            p2[0]*p0[1] - p0[0]*p2[1],
            p0[0]*p1[1] - p1[0]*p0[1],
            0.f
        );
    }

    while (numVerts--)
    {
        const unsigned offset = numVerts * SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* pInVar = varyings+offset;
        math::vec4* pOutVar = bin.mVaryings+offset;

        for (unsigned i = numVaryings; i--;)
        {
            *pOutVar++ = *pInVar++;
        }
    }
}



template void SR_VertexProcessor::push_bin<SR_RenderMode::RENDER_MODE_POINTS>(float, float, math::vec4* const, const math::vec4*) const noexcept;
template void SR_VertexProcessor::push_bin<SR_RenderMode::RENDER_MODE_LINES>(float, float, math::vec4* const, const math::vec4*) const noexcept;
template void SR_VertexProcessor::push_bin<SR_RenderMode::RENDER_MODE_TRIANGLES>(float, float, math::vec4* const, const math::vec4*) const noexcept;



/*-------------------------------------
 * Ensure only visible triangles get rendered. Triangles should have already
 * been tested for visibility within clip-space. Now we need to clip the
 * remaining tris and generate new ones.
 *
 * The basic clipping algorithm is as follows:
 *
 *  for each clipping edge do
 *      for (i = 0; i < Polygon.length; i++)
 *          Pi = Polygon.vertex[i];
 *          Pi+1 = Polygon.vertex[i+1];
 *          if (Pi is inside clipping region)
 *              if (Pi+1 is inside clipping region)
 *                  clippedPolygon.add(Pi+1)
 *              else
 *                  clippedPolygon.add(intersectionPoint(Pi, Pi+1, currentEdge)
 *          else
 *              if (Pi+1 is inside clipping region)
 *                  clippedPolygon.add(intersectionPoint(Pi, Pi+1, currentEdge)
 *                  clippedPolygon.add(Pi+1)
 *      end for
 *      Polygon = clippedPolygon     // keep on working with the new polygon
 *  end for
-------------------------------------*/
void SR_VertexProcessor::clip_and_process_tris(
    float fboW,
    float fboH,
    ls::math::vec4 vertCoords[SR_SHADER_MAX_SCREEN_COORDS],
    ls::math::vec4 pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS]) noexcept
{
    const SR_VertexShader vertShader    = mShader->mVertShader;
    const unsigned        numVarys      = (unsigned)vertShader.numVaryings;
    const float           widthScale    = fboW * 0.5f;
    const float           heightScale   = fboH * 0.5f;
    constexpr unsigned    numTempVerts  = 9; // at most 9 vertices should be generated
    unsigned              numTotalVerts = 3;
    math::vec4            tempVerts     [numTempVerts];
    math::vec4            newVerts      [numTempVerts];
    math::vec4            tempVarys     [numTempVerts * SR_SHADER_MAX_VARYING_VECTORS];
    math::vec4            newVarys      [numTempVerts * SR_SHADER_MAX_VARYING_VECTORS];
    const math::vec4      clipEdges[]  = {
        { 0.f,  0.f, -1.f, 1.f},
        { 0.f, -1.f,  0.f, 1.f},
        {-1.f,  0.f,  0.f, 1.f},
        { 0.f,  0.f,  1.f, 1.f},
        { 0.f,  1.f,  0.f, 1.f},
        { 1.f,  0.f,  0.f, 1.f}
    };

    const auto _copy_verts = [](int maxVerts, const math::vec4* inVerts, math::vec4* outVerts) noexcept->void
    {
        for (unsigned i = maxVerts; i--;)
        {
            outVerts[i] = inVerts[i];
        }
    };

    const auto _interpolate_varyings = [&numVarys](const math::vec4* inVarys, math::vec4* outVarys, int fromIndex, int toIndex, float amt) noexcept->void
    {
        const math::vec4* pV0 = inVarys + fromIndex * SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* pV1 = inVarys + toIndex * SR_SHADER_MAX_VARYING_VECTORS;

        for (unsigned i = numVarys; i--;)
        {
            *outVarys++ = math::mix(*pV0++, *pV1++, amt);
        }
    };

    _copy_verts(3, vertCoords, newVerts);
    _copy_verts(numVarys, pVaryings, newVarys);
    _copy_verts(numVarys, pVaryings+SR_SHADER_MAX_VARYING_VECTORS, newVarys+SR_SHADER_MAX_VARYING_VECTORS);
    _copy_verts(numVarys, pVaryings+SR_SHADER_MAX_VARYING_VECTORS*2, newVarys+SR_SHADER_MAX_VARYING_VECTORS*2);

    for (const math::vec4& edge : clipEdges)
    {
        // caching
        unsigned   numNewVerts = 0;
        unsigned   j           = numTotalVerts-1;
        math::vec4 p0          = newVerts[numTotalVerts-1];
        float      t0          = math::dot(p0, edge);
        int        visible0    = t0 >= 0.f;

        for (unsigned k = 0; k < numTotalVerts; ++k)
        {
            const math::vec4& p1 = newVerts[k];
            const float t1       = math::dot(p1, edge);
            const int   visible1 = t1 >= 0.f;

            if (visible0 ^ visible1)
            {
                const float t = t0 / (t0-t1);//math::clamp(t0 / (t0-t1), 0.f, 1.f);
                tempVerts[numNewVerts] = math::mix(p0, p1, t);
                _interpolate_varyings(newVarys, tempVarys+(numNewVerts*SR_SHADER_MAX_VARYING_VECTORS), j, k, t);

                ++numNewVerts;
            }

            if (visible1)
            {
                tempVerts[numNewVerts] = p1;
                _copy_verts(numVarys, newVarys+(k*SR_SHADER_MAX_VARYING_VECTORS), tempVarys+(numNewVerts*SR_SHADER_MAX_VARYING_VECTORS));
                ++numNewVerts;
            }

            j           = k;
            p0          = p1;
            t0          = t1;
            visible0    = visible1;
        }

        if (!numNewVerts)
        {
            return;
        }

        numTotalVerts = numNewVerts;
        _copy_verts(numNewVerts, tempVerts, newVerts);

        for (unsigned i = numNewVerts; i--;)
        {
            const unsigned offset = i*SR_SHADER_MAX_VARYING_VECTORS;
            _copy_verts(numNewVerts, tempVarys+offset, newVarys+offset);
        }
    }

    if (numTotalVerts < 3)
    {
        return;
    }

    LS_DEBUG_ASSERT(numTotalVerts <= numTempVerts);

    for (unsigned i = numTotalVerts; i--;)
    {
        newVerts[i] = sr_perspective_divide(newVerts[i]);
        sr_world_to_screen_coords_divided(newVerts[i], widthScale, heightScale);
    }

    tempVerts[0] = newVerts[0];
    _copy_verts(numVarys, newVarys, tempVarys);

    for (unsigned i = numTotalVerts-2; i--;)
    {
        const unsigned j = i+1;
        const unsigned k = i+2;

        tempVerts[1] = newVerts[j];
        tempVerts[2] = newVerts[k];

        _copy_verts(numVarys, newVarys+(j*SR_SHADER_MAX_VARYING_VECTORS), tempVarys+(1*SR_SHADER_MAX_VARYING_VECTORS));
        _copy_verts(numVarys, newVarys+(k*SR_SHADER_MAX_VARYING_VECTORS), tempVarys+(2*SR_SHADER_MAX_VARYING_VECTORS));

        push_bin<SR_RenderMode::RENDER_MODE_TRIANGLES>(fboW, fboH, tempVerts, tempVarys);
    }
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
void SR_VertexProcessor::process_points(const SR_Mesh& m, size_t instanceId) noexcept
{
    alignas(alignof(math::vec4)) math::vec4 vertCoords[SR_SHADER_MAX_SCREEN_COORDS];
    alignas(alignof(math::vec4)) math::vec4 pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    const SR_VertexShader   vertShader  = mShader->mVertShader;
    const auto              shader      = vertShader.shader;
    const SR_VertexArray&   vao         = mContext->vao(m.vaoId);
    const float             fboW        = (float)mFbo->width();
    const float             fboH        = (float)mFbo->height();
    const float             widthScale  = fboW * 0.5f;
    const float             heightScale = fboH * 0.5f;
    size_t                  begin       = m.elementBegin;
    const size_t            end         = m.elementEnd;
    const SR_IndexBuffer*   pIbo        = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;

    const bool usingIndices = m.mode == RENDER_MODE_INDEXED_POINTS;

    SR_VertexParam params;
    params.pUniforms = mShader->mUniforms;
    params.instanceId = instanceId;
    params.pVao = &vao;
    params.pVbo = &mContext->vbo(vao.get_vertex_buffer());

    #if SR_VERTEX_CACHING_ENABLED
        SR_PTVCache ptvCache{shader, params};
        const size_t numVaryings = vertShader.numVaryings;
    #endif

    begin += mThreadId;
    const size_t step = mNumThreads;

    for (size_t i = begin; i < end; i += step)
    {
        #if SR_VERTEX_CACHING_ENABLED
            const size_t vertId = usingIndices ? get_next_vertex(pIbo, i) : i;
            ptvCache.query_and_update(vertId, vertCoords[0], pVaryings, numVaryings);
        #else
            params.vertId    = usingIndices ? get_next_vertex(pIbo, i) : i;
            params.pVaryings = pVaryings;
            vertCoords[0]    = shader(params);
        #endif

        if (vertCoords[0][3] > 0.f)
        {
            sr_world_to_screen_coords(vertCoords[0], widthScale, heightScale);
            push_bin<SR_RenderMode::RENDER_MODE_POINTS>(fboW, fboH, vertCoords, pVaryings);
        }
    }
}



/*--------------------------------------
 * Process Lines
--------------------------------------*/
void SR_VertexProcessor::process_lines(const SR_Mesh& m, size_t instanceId) noexcept
{
    alignas(alignof(math::vec4)) math::vec4 vertCoords[SR_SHADER_MAX_SCREEN_COORDS];
    alignas(alignof(math::vec4)) math::vec4 pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    const SR_VertexShader   vertShader  = mShader->mVertShader;
    const auto              shader      = vertShader.shader;
    const SR_VertexArray&   vao         = mContext->vao(m.vaoId);
    const float             fboW        = (float)mFbo->width();
    const float             fboH        = (float)mFbo->height();
    const float             widthScale  = fboW * 0.5f;
    const float             heightScale = fboH * 0.5f;
    size_t                  begin       = m.elementBegin;
    const size_t            end         = m.elementEnd;
    const SR_IndexBuffer*   pIbo        = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;

    const bool usingIndices = m.mode == RENDER_MODE_INDEXED_LINES;

    SR_VertexParam params;
    params.pUniforms = mShader->mUniforms;
    params.instanceId = instanceId;
    params.pVao = &vao;
    params.pVbo = &mContext->vbo(vao.get_vertex_buffer());

    #if SR_VERTEX_CACHING_ENABLED
        SR_PTVCache ptvCache{shader, params};
        const size_t numVaryings = vertShader.numVaryings;
    #endif

    begin += mThreadId * 2u;
    const size_t step = mNumThreads * 2u;

    LS_PREFETCH(params.pUniforms, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
    LS_PREFETCH(pVaryings,        LS_PREFETCH_ACCESS_RW, LS_PREFETCH_LEVEL_L1);

    for (size_t i = begin; i < end; i += step)
    {
        const size_t index0  = i;
        const size_t index1  = i + 1;

        #if SR_VERTEX_CACHING_ENABLED
            const size_t vertId0 = usingIndices ? get_next_vertex(pIbo, index0) : index0;
            const size_t vertId1 = usingIndices ? get_next_vertex(pIbo, index1) : index1;
            ptvCache.query_and_update(vertId0, vertCoords[0], pVaryings, numVaryings);
            ptvCache.query_and_update(vertId1, vertCoords[1], pVaryings + SR_SHADER_MAX_VARYING_VECTORS, numVaryings);
        #else
            params.vertId    = usingIndices ? get_next_vertex(pIbo, index0) : index0;
            params.pVaryings = pVaryings;
            vertCoords[0]    = shader(params);

            params.vertId    = usingIndices ? get_next_vertex(pIbo, index1) : index1;
            params.pVaryings = pVaryings + SR_SHADER_MAX_VARYING_VECTORS;
            vertCoords[1]    = shader(params);
        #endif

        if (vertCoords[0][3] >= 0.f && vertCoords[1][3] >= 0.f)
        {
            sr_world_to_screen_coords(vertCoords[0], widthScale, heightScale);
            sr_world_to_screen_coords(vertCoords[1], widthScale, heightScale);

            push_bin<SR_RenderMode::RENDER_MODE_LINES>(fboW, fboH, vertCoords, pVaryings);
        }
    }
}



/*--------------------------------------
 * Process Triangles
--------------------------------------*/
void SR_VertexProcessor::process_tris(const SR_Mesh& m, size_t instanceId) noexcept
{
    alignas(alignof(math::vec4)) math::vec4 vertCoords[SR_SHADER_MAX_SCREEN_COORDS];
    alignas(alignof(math::vec4)) math::vec4 pVaryings[SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    const SR_VertexShader   vertShader  = mShader->mVertShader;
    const SR_CullMode       cullMode    = vertShader.cullMode;
    const auto              shader      = vertShader.shader;
    const SR_VertexArray&   vao         = mContext->vao(m.vaoId);
    const float             fboW        = (float)mFbo->width();
    const float             fboH        = (float)mFbo->height();
    const float             widthScale  = fboW * 0.5f;
    const float             heightScale = fboH * 0.5f;
    size_t                  begin       = m.elementBegin;
    const size_t            end         = m.elementEnd;
    const SR_IndexBuffer*   pIbo        = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;

    const int usingIndices = m.mode & ((RENDER_MODE_INDEXED_TRIANGLES | RENDER_MODE_INDEXED_TRI_WIRE) ^ (RENDER_MODE_TRIANGLES | RENDER_MODE_TRI_WIRE));

    SR_VertexParam params;
    params.pUniforms = mShader->mUniforms;
    params.instanceId = instanceId;
    params.pVao = &vao;
    params.pVbo = &mContext->vbo(vao.get_vertex_buffer());

    #if SR_VERTEX_CACHING_ENABLED
        SR_PTVCache ptvCache{shader, params};
        const size_t numVaryings = vertShader.numVaryings;
    #endif

    LS_PREFETCH(params.pUniforms, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
    //LS_PREFETCH(pVaryings,        LS_PREFETCH_ACCESS_RW, LS_PREFETCH_LEVEL_L1);

    begin += mThreadId * 3u;
    const size_t step = mNumThreads * 3u;

    for (size_t i = begin; i < end; i += step)
    {
        const math::vec3_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : math::vec3_t<size_t>{i, i+1, i+2};

        #if SR_VERTEX_CACHING_ENABLED
            ptvCache.query_and_update(vertId[0], vertCoords[0], pVaryings, numVaryings);
            ptvCache.query_and_update(vertId[1], vertCoords[1], pVaryings + SR_SHADER_MAX_VARYING_VECTORS, numVaryings);
            ptvCache.query_and_update(vertId[2], vertCoords[2], pVaryings + SR_SHADER_MAX_VARYING_VECTORS * 2, numVaryings);
        #else
            params.vertId    = vertId[0];
            params.pVaryings = pVaryings;
            vertCoords[0]    = shader(params);

            params.vertId    = vertId[1];
            params.pVaryings = pVaryings + SR_SHADER_MAX_VARYING_VECTORS;
            vertCoords[1]    = shader(params);

            params.vertId    = vertId[2];
            params.pVaryings = pVaryings + SR_SHADER_MAX_VARYING_VECTORS * 2;
            vertCoords[2]    = shader(params);
        #endif

        if (cullMode == SR_CULL_BACK_FACE)
        {
            if (back_face_visible(vertCoords))
            {
                continue;
            }
        }
        else if (cullMode == SR_CULL_FRONT_FACE)
        {
            if (front_face_visible(vertCoords))
            {
                continue;
            }
        }

        // Clip-space culling
        const SR_ClipStatus visStatus = face_visible(vertCoords);
        if (visStatus == SR_TRIANGLE_NOT_VISIBLE)
        {
            continue;
        }

        #if SR_PRIMITIVE_CLIPPING_ENABLED == 0
            sr_perspective_divide3(vertCoords);
            sr_world_to_screen_coords_divided3(vertCoords, widthScale, heightScale);
            push_bin<SR_RenderMode::RENDER_MODE_TRIANGLES>(fboW, fboH, vertCoords, pVaryings);
        #else
            if (visStatus == SR_TRIANGLE_FULLY_VISIBLE)
            {
                sr_perspective_divide3(vertCoords);
                sr_world_to_screen_coords_divided3(vertCoords, widthScale, heightScale);
                push_bin<SR_RenderMode::RENDER_MODE_TRIANGLES>(fboW, fboH, vertCoords, pVaryings);
            }
            else
            {
                clip_and_process_tris(fboW, fboH, vertCoords, pVaryings);
            }
        #endif
    }
}



/*--------------------------------------
 * Process Vertices
--------------------------------------*/
void SR_VertexProcessor::execute() noexcept
{
    if (mNumInstances == 1)
    {
        if (mRenderMode & (RENDER_MODE_POINTS | RENDER_MODE_INDEXED_POINTS))
        {
            for (size_t i = 0; i < mNumMeshes; ++i)
            {
                process_points(mMeshes[i], 0);
            }
        }
        else if (mRenderMode & (RENDER_MODE_LINES | RENDER_MODE_INDEXED_LINES))
        {
            for (size_t i = 0; i < mNumMeshes; ++i)
            {
                process_lines(mMeshes[i], 0);
            }
        }
        else if (mRenderMode & (RENDER_MODE_TRIANGLES | RENDER_MODE_INDEXED_TRIANGLES | RENDER_MODE_TRI_WIRE | RENDER_MODE_INDEXED_TRI_WIRE))
        {
            for (size_t i = 0; i < mNumMeshes; ++i)
            {
                process_tris(mMeshes[i], 0);
            }
        }
    }
    else
    {
        if (mRenderMode & (RENDER_MODE_POINTS | RENDER_MODE_INDEXED_POINTS))
        {
            for (size_t i = 0; i < mNumInstances; ++i)
            {
                process_points(mMeshes[0], i);
            }
        }
        else if (mRenderMode & (RENDER_MODE_LINES | RENDER_MODE_INDEXED_LINES))
        {
            for (size_t i = 0; i < mNumInstances; ++i)
            {
                process_lines(mMeshes[0], i);
            }
        }
        else if (mRenderMode & (RENDER_MODE_TRIANGLES | RENDER_MODE_INDEXED_TRIANGLES | RENDER_MODE_TRI_WIRE | RENDER_MODE_INDEXED_TRI_WIRE))
        {
            for (size_t i = 0; i < mNumInstances; ++i)
            {
                process_tris(mMeshes[0], i);
            }
        }
    }

    mBusyProcessors->fetch_sub(1, std::memory_order_acq_rel);
    while (mBusyProcessors->load(std::memory_order_consume))
    {
        #if defined(LS_ARCH_X86)
            _mm_pause();
        #endif

        if (mFragProcessors->load(std::memory_order_consume))
        {
            flush_bins();
        }
    }

    if (mBinsUsed->load(std::memory_order_consume))
    {
        flush_bins();
    }
}
