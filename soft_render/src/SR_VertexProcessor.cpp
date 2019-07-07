
#include <algorithm>

#include "lightsky/utils/Assertions.h" // LS_DEBUG_ASSERT
#include "lightsky/utils/Log.h"
#include "lightsky/utils/Sort.hpp" // utils::sort_quick

#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
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



/*--------------------------------------
 * Convert world coordinates to screen coordinates (temporary until NDC clipping is added)
--------------------------------------*/
inline void sr_world_to_screen_coords(math::vec4& v, const float widthScale, const float heightScale) noexcept
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
            assert(false);
            break;
    }
    return vId;
}



inline math::vec3_t<size_t> get_next_vertex3(const SR_IndexBuffer* pIbo, size_t vId) noexcept
{
    math::vec3_t<size_t> ret;
    math::vec3_t<unsigned char> byteIds;
    math::vec3_t<unsigned short> shortIds;
    math::vec3_t<unsigned int> intIds;

    switch (pIbo->type())
    {
        case VERTEX_DATA_BYTE:
            byteIds = *reinterpret_cast<const decltype(byteIds)*>(pIbo->element(vId));
            ret = (math::vec3_t<size_t>)byteIds;
            break;

        case VERTEX_DATA_SHORT:
            shortIds = *reinterpret_cast<const decltype(shortIds)*>(pIbo->element(vId));
            ret = (math::vec3_t<size_t>)shortIds;
            break;

        case VERTEX_DATA_INT:
            intIds = *reinterpret_cast<const decltype(intIds)*>(pIbo->element(vId));
            ret = (math::vec3_t<size_t>)intIds;
            break;

        default:
            LS_DEBUG_ASSERT(false);
            break;
    }

    return ret;
}



/*--------------------------------------
 * Cull backfaces of a triangle
--------------------------------------*/
inline LS_INLINE bool backface_visible(const math::vec4 screenCoords[SR_SHADER_MAX_SCREEN_COORDS]) noexcept
{
    return (0.f <= math::dot(math::vec4{0.f, 0.f, 1.f, 0.f}, math::normalize(math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0]))));
}



/*--------------------------------------
 * Cull frontfaces of a triangle
--------------------------------------*/
inline LS_INLINE bool frontface_visible(const math::vec4 screenCoords[SR_SHADER_MAX_SCREEN_COORDS]) noexcept
{
    return (0.f >= math::dot(math::vec4{0.f, 0.f, 1.f, 0.f}, math::normalize(math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0]))));
}



/*--------------------------------------
 * Cull only triangle outside of the screen
--------------------------------------*/
inline LS_INLINE bool face_visible(const math::vec4 clipCoords[SR_SHADER_MAX_WORLD_COORDS]) noexcept
{
    return math::min(clipCoords[0][3], clipCoords[1][3], clipCoords[2][3]) >= 0.f;
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SR_VertexProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute a fragment processor
-------------------------------------*/
void SR_VertexProcessor::flush_fragments() const noexcept
{
    // Sync Point 1 indicates that all triangles have been sorted.
    // Since we only have two sync points for the bins to process, a negative
    // value for mFragProcessors will indicate all threads are ready to process
    // fragments. A zero-value for mFragProcessors indicates we can continue
    // processing vertices.
    const int_fast64_t   syncPoint1 = -mNumThreads-1;
    int_fast64_t         tileId     = mFragProcessors->fetch_add(1, std::memory_order_acq_rel);

    // Sort the bins based on their depth.
    if (tileId == mNumThreads-1u)
    {
        const uint_fast64_t maxElements = math::min<uint64_t>(mBinsUsed->load(std::memory_order_consume), SR_SHADER_MAX_FRAG_BINS);

        // Blended fragments get sorted back-to-front for correct coloring.
        if (mShader->fragment_shader().blend == SR_BLEND_OFF)
        {
            ls::utils::sort_quick<uint32_t>(mBinIds, maxElements, [&](uint32_t a, uint32_t b)->bool {
                return mFragBins[a] > mFragBins[b];
            });
        }
        else
        {
            // Sort opaque objects from front-to-back to fortify depth testing.
            ls::utils::sort_quick<uint32_t>(mBinIds, maxElements, [&](uint32_t a, uint32_t b)->bool {
                return mFragBins[a] < mFragBins[b];
            });
        }

        // Let all threads know they can process fragments.
        mFragProcessors->store(syncPoint1, std::memory_order_release);
    }
    else
    {
        while (mFragProcessors->load(std::memory_order_consume) > 0)
        {
            #if LS_ARCH_X86
                _mm_pause();
            #endif
            //std::this_thread::yield();
        }
    }

    SR_FragmentProcessor fragTask{
        (uint16_t)tileId,
        mMesh.mode,
        (uint32_t)mNumThreads,
        (float)(mFboW - 1),
        (float)(mFboH - 1),
        math::min<uint64_t>(mBinsUsed->load(std::memory_order_consume), SR_SHADER_MAX_FRAG_BINS),
        mShader,
        mFbo,
        mBinIds,
        mFragBins,
        mFragQueues + tileId
    };

    fragTask.execute();

    // Indicate to all threads we can now process more vertices
    tileId = mFragProcessors->fetch_add(1, std::memory_order_acq_rel);

    // Wait for the last thread to reset the number of available bins.
    while (mFragProcessors->load(std::memory_order_consume) < 0)
    {
        if (tileId == -2)
        {
            mBinsUsed->store(0, std::memory_order_release);
            mFragProcessors->store(0, std::memory_order_acq_rel);

        }
        else
        {
            std::this_thread::yield();
        }
    }
}



/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SR_VertexProcessor::push_fragments(
    float fboW,
    float fboH,
    math::vec4* const screenCoords,
    const math::vec4* varyings
) const noexcept
{
    const SR_Mesh m = mMesh;
    std::atomic_uint_fast64_t* const pLocks = mBinsUsed;

    const math::vec4& p0 = screenCoords[0];
    const math::vec4& p1 = screenCoords[1];
    const math::vec4& p2 = screenCoords[2];
    float bboxMinX = -1.f;
    float bboxMinY = -1.f;
    float bboxMaxX = -1.f;
    float bboxMaxY = -1.f;

    // calculate the bounds of the tile which a certain thread will be
    // responsible for

    // render points through whichever tile/thread they appear in
    if (m.mode & RENDER_MODE_POINTS)
    {
        bboxMinX = p0[0];
        bboxMaxX = p0[0];
        bboxMinY = p0[1];
        bboxMaxY = p0[1];
    }
    else if (m.mode & RENDER_MODE_LINES)
    {
        // establish a bounding box to detect overlap with a thread's tiles
        bboxMinX = math::min(p0[0], p1[0]);
        bboxMinY = math::min(p0[1], p1[1]);
        bboxMaxX = math::max(p0[0], p1[0]);
        bboxMaxY = math::max(p0[1], p1[1]);
    }
    else if (m.mode & RENDER_MODE_TRIANGLES)
    {
        // establish a bounding box to detect overlap with a thread's tiles
        bboxMinX = math::min(p0[0], p1[0], p2[0]);
        bboxMinY = math::min(p0[1], p1[1], p2[1]);
        bboxMaxX = math::max(p0[0], p1[0], p2[0]);
        bboxMaxY = math::max(p0[1], p1[1], p2[1]);
    }

    //LS_LOG_MSG(bboxMinX, ' ', bboxMinY, " x ", bboxMaxX, ' ', bboxMaxY);

    const int isFragVisible = (bboxMaxX >= 0.f && fboW >= bboxMinX && bboxMaxY >= 0.f && fboH >= bboxMinY);

    if (isFragVisible)
    {
        SR_FragmentBin* const pFragBins = mFragBins;

        // Copy all per-vertex coordinates and varyings to the fragment bins
        // which will need the data for interpolation. The perspective-divide
        // is only used for rendering triangles.
        SR_FragmentBin bin;
        bin.mScreenCoords[0] = p0;
        bin.mScreenCoords[1] = p1;
        bin.mScreenCoords[2] = p2;

        const float denom = 1.f / ((p0[0]-p2[0])*(p1[1]-p0[1]) - (p0[0]-p1[0])*(p2[1]-p0[1]));
        bin.mBarycentricCoords[0] = denom*math::vec4(p1[1]-p2[1], p2[1]-p0[1], p0[1]-p1[1], 0.f);
        bin.mBarycentricCoords[1] = denom*math::vec4(p2[0]-p1[0], p0[0]-p2[0], p1[0]-p0[0], 0.f);
        bin.mBarycentricCoords[2] = denom*math::vec4(
            p1[0]*p2[1] - p2[0]*p1[1],
            p2[0]*p0[1] - p0[0]*p2[1],
            p0[0]*p1[1] - p1[0]*p0[1],
            0.f
        );

        for (unsigned i = 0; i < LS_ARRAY_SIZE(bin.mVaryings); ++i)
        {
            bin.mVaryings[i] = varyings[i];
        }

        // Check if the output bin is full
        uint_fast64_t binId;

        // Attempt to grab a bin index. Flush the bins if they've filled up.
        while ((binId = pLocks->fetch_add(1, std::memory_order_acq_rel)) >= SR_SHADER_MAX_FRAG_BINS)
        {
            flush_fragments();
        }

        // place a triangle into the next available bin
        mBinIds[binId] = (uint32_t)binId;
        pFragBins[binId] = bin;
    }

    while (pLocks->load(std::memory_order_consume) >= SR_SHADER_MAX_FRAG_BINS)
    {
        flush_fragments();
    }
}



/*--------------------------------------
 * Process Vertices
--------------------------------------*/
void SR_VertexProcessor::execute() noexcept
{
    const SR_VertexShader   vertShader  = mShader->mVertShader;
    const SR_CullMode       cullMode    = vertShader.cullMode;
    const auto              shader      = vertShader.shader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const SR_VertexArray&   vao         = mContext->vao(mMesh.vaoId);
    const SR_VertexBuffer&  vbo         = mContext->vbo(vao.get_vertex_buffer());
    const float             fboW        = (float)mFboW;
    const float             fboH        = (float)mFboH;
    const float             widthScale  = fboW * 0.5f;
    const float             heightScale = fboH * 0.5f;
    size_t                  begin       = mMesh.elementBegin;
    const size_t            end         = mMesh.elementEnd;
    const SR_IndexBuffer*   pIbo        = nullptr;
    ls::math::vec4          vertCoords  [SR_SHADER_MAX_SCREEN_COORDS];
    math::vec4              pVaryings   [SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    if (vao.has_index_buffer())
    {
        pIbo = &mContext->ibo(vao.get_index_buffer());
    }

    if (mMesh.mode & (RENDER_MODE_POINTS | RENDER_MODE_INDEXED_POINTS))
    {
        begin += mThreadId;
        const size_t step = mNumThreads;
        const bool usingIndices = mMesh.mode == RENDER_MODE_INDEXED_POINTS;

        for (size_t i = begin; i < end; i += step)
        {
            const size_t vertId0 = usingIndices ? get_next_vertex(pIbo, i) : i;

            vertCoords[0] = shader(vertId0, vao, vbo, pUniforms, pVaryings);

            if (vertCoords[0][3] > 0.f)
            {
                sr_world_to_screen_coords(vertCoords[0], widthScale, heightScale);
                push_fragments(fboW, fboH, vertCoords, pVaryings);
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_LINES || mMesh.mode == RENDER_MODE_INDEXED_LINES)
    {
        // 3 vertices per set of lines
        begin += mThreadId * 2u;
        const size_t step = mNumThreads * 2u;
        const bool usingIndices = mMesh.mode == RENDER_MODE_INDEXED_LINES;
        size_t index0;
        size_t index1;
        size_t vertId0;
        size_t vertId1;

        for (size_t i = begin; i < end; i += step)
        {
            index0  = i;
            index1  = i + 1;
            vertId0 = usingIndices ? get_next_vertex(pIbo, index0) : index0;
            vertId1 = usingIndices ? get_next_vertex(pIbo, index1) : index1;

            vertCoords[0] = shader(vertId0, vao, vbo, pUniforms, pVaryings);
            vertCoords[1] = shader(vertId1, vao, vbo, pUniforms, pVaryings + SR_SHADER_MAX_VARYING_VECTORS);

            if (vertCoords[0][3] >= 0.f && vertCoords[1][3] >= 0.f)
            {
                sr_world_to_screen_coords(vertCoords[0], widthScale, heightScale);
                sr_world_to_screen_coords(vertCoords[1], widthScale, heightScale);

                push_fragments(fboW, fboH, vertCoords, pVaryings);
            }
        }
    }
    else if (mMesh.mode & (RENDER_MODE_TRIANGLES | RENDER_MODE_INDEXED_TRIANGLES | RENDER_MODE_TRI_WIRE | RENDER_MODE_INDEXED_TRI_WIRE))
    {
        // 3 vertices per set of lines
        begin += mThreadId * 3u;
        const size_t step = mNumThreads * 3u;
        const int usingIndices = mMesh.mode & ((RENDER_MODE_INDEXED_TRIANGLES | RENDER_MODE_INDEXED_TRI_WIRE) ^ (RENDER_MODE_TRIANGLES | RENDER_MODE_TRI_WIRE));

        for (size_t i = begin; i < end; i += step)
        {
            const math::vec3_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : math::vec3_t<size_t>{i, i+1, i+2};

            vertCoords[0] = shader(vertId[0], vao, vbo, pUniforms, pVaryings);
            vertCoords[1] = shader(vertId[1], vao, vbo, pUniforms, pVaryings + SR_SHADER_MAX_VARYING_VECTORS);
            vertCoords[2] = shader(vertId[2], vao, vbo, pUniforms, pVaryings + (SR_SHADER_MAX_VARYING_VECTORS * 2));

            // Clip-space culling
            if (!face_visible(vertCoords))
            {
                continue;
            }

            sr_world_to_screen_coords(vertCoords[0], widthScale, heightScale);
            sr_world_to_screen_coords(vertCoords[1], widthScale, heightScale);
            sr_world_to_screen_coords(vertCoords[2], widthScale, heightScale);

            if ((cullMode == SR_CULL_BACK_FACE && backface_visible(vertCoords))
            || (cullMode == SR_CULL_FRONT_FACE && frontface_visible(vertCoords))
            || (cullMode == SR_CULL_OFF))
            {
                push_fragments(fboW, fboH, vertCoords, pVaryings);
            }
        }
    }

    mBusyProcessors->fetch_sub(1, std::memory_order_acq_rel);
    while (mBusyProcessors->load(std::memory_order_consume) > 0)
    {
        if (mFragProcessors->load(std::memory_order_consume))
        {
            flush_fragments();
        }
    }

    if (mBinsUsed->load(std::memory_order_consume))
    {
        flush_fragments();
    }
}
