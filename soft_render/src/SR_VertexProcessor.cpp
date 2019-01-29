
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
inline math::vec4 world_to_screen_3(const math::vec4& v, const float widthScale, const float heightScale) noexcept
{
    const float wInv = 1.f / v.v[3];
    math::vec4&& temp = v * wInv;

    temp[0] = widthScale  + temp[0] * widthScale;
    temp[1] = heightScale + temp[1] * heightScale;

    return math::vec4{temp[0], temp[1], temp[2], 0.f};
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

    switch (pIbo->type())
    {
        case VERTEX_DATA_BYTE:
            ret[0] = *reinterpret_cast<const unsigned char*>(pIbo->element(vId+0));
            ret[1] = *reinterpret_cast<const unsigned char*>(pIbo->element(vId+1));
            ret[2] = *reinterpret_cast<const unsigned char*>(pIbo->element(vId+2));
            break;

        case VERTEX_DATA_SHORT:
            ret[0] = *reinterpret_cast<const unsigned short*>(pIbo->element(vId+0));
            ret[1] = *reinterpret_cast<const unsigned short*>(pIbo->element(vId+1));
            ret[2] = *reinterpret_cast<const unsigned short*>(pIbo->element(vId+2));
            break;

        case VERTEX_DATA_INT:
            ret[0] = *reinterpret_cast<const unsigned int*>(pIbo->element(vId+0));
            ret[1] = *reinterpret_cast<const unsigned int*>(pIbo->element(vId+1));
            ret[2] = *reinterpret_cast<const unsigned int*>(pIbo->element(vId+2));
            break;

        default:
            abort();
            break;
    }

    return ret;
}



/*--------------------------------------
 * Methods for clipping a line segment
 *
 * This method was adapted from Stephen M. Cameron's implementation of the
 * Liang-Barsky line clipping algorithm:
 *     https://github.com/smcameron/liang-barsky-in-c
 *
 * His method was also adapted from Hin Jang's C++ implementation:
 *     http://hinjang.com/articles/04.html#eight
--------------------------------------*/
bool clip_segment(float num, float denom, float& tE, float& tL)
{
    if (math::abs(denom) < LS_EPSILON)
        return num < 0.f;

    const float t = num / denom;

    if (denom > 0.f)
    {
        if (t > tL)
        {
            return false;
        }

        if (t > tE)
        {
            tE = t;
        }
    }
    else
    {
        if (t < tE)
        {
            return false;
        }

        if (t < tL)
        {
            tL = t;
        }
    }

    return true;
}

bool sr_clip_liang_barsky(math::vec4 screenCoords[SR_SHADER_MAX_SCREEN_COORDS], float xMax, float yMax)
{
    float tE, tL;
    float& x1 = screenCoords[0][0];
    float& y1 = screenCoords[0][1];
    float& x2 = screenCoords[1][0];
    float& y2 = screenCoords[1][1];
    const float dx = x2 - x1;
    const float dy = y2 - y1;

    if (math::abs(dx) < LS_EPSILON && math::abs(dy) < LS_EPSILON)
    {
        if (x1 >= 0.f && x1 <= xMax && y1 >= 0.f && y1 <= yMax)
        {
            return true;
        }
    }

    tE = 0.f;
    tL = 1.f;

    if (clip_segment(-x1,      dx, tE, tL) &&
        clip_segment(x1-xMax, -dx, tE, tL) &&
        clip_segment(-y1,      dy, tE, tL) &&
        clip_segment(y1-yMax, -dy, tE, tL))
    {
        if (tL < 1.f)
        {
            x2 = x1 + tL * dx;
            y2 = y1 + tL * dy;
        }

        if (tE > 0.f)
        {
            x1 += tE * dx;
            y1 += tE * dy;
        }

        return true;
    }

    return false;
}



/*--------------------------------------
 * Cull backfaces of a triangle
--------------------------------------*/
inline bool backface_visible(math::vec4 screenCoords[SR_SHADER_MAX_SCREEN_COORDS], const math::vec4 worldCoords[SR_SHADER_MAX_WORLD_COORDS]) noexcept
{
    return math::min(worldCoords[0][3], worldCoords[1][3], worldCoords[2][3]) > 0.f &&
           (0.f < math::dot(math::vec4{0.f, 0.f, 1.f, 0.f}, math::normalize(math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0]))));
}



/*--------------------------------------
 * Cull frontfaces of a triangle
--------------------------------------*/
inline bool frontface_visible(const math::vec4 screenCoords[SR_SHADER_MAX_SCREEN_COORDS], const math::vec4 worldCoords[SR_SHADER_MAX_WORLD_COORDS]) noexcept
{
    return math::min(worldCoords[0][3], worldCoords[1][3], worldCoords[2][3]) >= 0.f &&
           (0.f > math::dot(math::vec4{0.f, 0.f, 1.f, 0.f}, math::normalize(math::cross(screenCoords[1]-screenCoords[0], screenCoords[2]-screenCoords[0]))));
}



/*--------------------------------------
 * Cull only triangle outside of the screen
--------------------------------------*/
inline bool face_visible(const math::vec4 worldCoords[SR_SHADER_MAX_WORLD_COORDS]) noexcept
{
    return math::min(worldCoords[0][3], worldCoords[1][3], worldCoords[2][3]) > 0.f;
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
    // Sync Point 1 indicates that all triangles have been sorted
    const uint_fast64_t syncPoint1 = 2u * mNumThreads;

    // Sync point 2 indicates that all fragments have been rasterized and the
    // vertex processors can now take over again
    const uint_fast64_t syncPoint2 = 3u * mNumThreads - 1;

    // sync before running the fragment shaders. The fragment processor will
    // return this number back to 0
    uint_fast64_t tileId = mFragProcessors->fetch_add(1, std::memory_order_acq_rel);

    SR_FragmentProcessor fragTask;
    fragTask.mThreadId      = (uint16_t)tileId;
    fragTask.mMode          = mMesh.mode;
    fragTask.mNumProcessors = mNumThreads;
    fragTask.mNumBins       = math::min<uint64_t>(mBinsUsed->load(std::memory_order_relaxed), SR_SHADER_MAX_FRAG_BINS);
    fragTask.mShader        = mShader;
    fragTask.mFbo           = mFbo;
    fragTask.mBins          = mFragBins;
    fragTask.mQueues        = mFragQueues[tileId].data();
    fragTask.mFboW          = (float)(mFboW - 1);
    fragTask.mFboH          = (float)(mFboH - 1);

    // Sort the bins based on their depth. Closer objects should be rendered
    // first to allow for fragment rejection during the depth test.
    if (tileId == mNumThreads-1u)
    {
        // Alpha-blended fragments get sorted from nearest to furthest for
        // corrected coloring
        if (mShader->fragment_shader().blend == SR_BLEND_OFF)
        {
            ls::utils::sort_quick<SR_FragmentBin, ls::utils::IsGreater<SR_FragmentBin>>(mFragBins, math::min<uint64_t>(mBinsUsed->load(), SR_SHADER_MAX_FRAG_BINS));
        }
        else
        {
            ls::utils::sort_quick<SR_FragmentBin, ls::utils::IsLess<SR_FragmentBin>>(mFragBins, math::min<uint64_t>(mBinsUsed->load(), SR_SHADER_MAX_FRAG_BINS));
        }
        mFragProcessors->store(syncPoint1, std::memory_order_release);
    }

    while (mFragProcessors->load(std::memory_order_relaxed) < syncPoint1)
    {
        std::this_thread::yield();
    }

    fragTask.execute();

    // indicate the bins are available for pushing
    tileId = mFragProcessors->fetch_add(1, std::memory_order_acq_rel);

    // Wait for the last thread to reset the number of available bins.
    if (tileId == syncPoint2)
    {
        mBinsUsed->store(0, std::memory_order_release);
        mFragProcessors->store(0, std::memory_order_release);
    }

    // Sync all threads
    while (mFragProcessors->load(std::memory_order_relaxed) != 0)
    {
        std::this_thread::yield();
    }
}



/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SR_VertexProcessor::push_fragments(
    float fboW,
    float fboH,
    math::vec4* const screenCoords,
    math::vec4* const worldCoords,
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

    const int isFragVisible = (bboxMaxX >= 0.f && fboW >= bboxMinX && bboxMaxY >= 0.f && fboH >= bboxMinY);

    if (isFragVisible)
    {
        SR_FragmentBin* const pFragBins = mFragBins;

        // Copy all per-vertex coordinates and varyings to the fragment bins which
        // will need the data for interpolation
        SR_FragmentBin bin;
        bin.mScreenCoords[0] = p0;
        bin.mScreenCoords[1] = p1;
        bin.mScreenCoords[2] = p2;
        bin.mPerspDivide     = math::rcp(math::vec4{worldCoords[0][3], worldCoords[1][3], worldCoords[2][3], 1.f});

        for (unsigned i = 0; i < LS_ARRAY_SIZE(bin.mVaryings); ++i)
        {
            bin.mVaryings[i] = varyings[i];
        }

        // Check if the output bin is full
        uint_fast64_t binId = pLocks->fetch_add(1, std::memory_order_relaxed);

        // flush the bins if they've filled up
        if (binId >= SR_SHADER_MAX_FRAG_BINS)
        {
            flush_fragments();

            // Attempt to grab another bin index
            binId = pLocks->fetch_add(1, std::memory_order_relaxed);
        }

        // place a triangle into the next available bin
        pFragBins[binId] = bin;
    }

    if (pLocks->load() >= SR_SHADER_MAX_FRAG_BINS)
    {
        flush_fragments();
    }
}



/*--------------------------------------
 * Process Vertices
--------------------------------------*/
void SR_VertexProcessor::execute() noexcept
{
    const SR_VertexShader   vertShader   = mShader->mVertShader;
    const SR_CullMode       cullMode     = vertShader.cullMode;
    const auto              shader       = vertShader.shader;
    const SR_UniformBuffer* pUniforms    = mShader->mUniforms.get();
    const size_t            numVaryings  = vertShader.numVaryings;
    const SR_VertexArray&   vao          = mContext->vao(mMesh.vaoId);
    const SR_VertexBuffer&  vbo          = mContext->vbo(vao.get_vertex_buffer());
    const float             fboW         = (float)mFboW;
    const float             fboH         = (float)mFboH;
    const float             widthScale   = fboW * 0.5f;
    const float             heightScale  = fboH * 0.5f;
    size_t                  begin        = mMesh.elementBegin;
    const size_t            end          = mMesh.elementEnd;
    const SR_IndexBuffer*   pIbo         = nullptr;
    ls::math::vec4          worldCoords  [SR_SHADER_MAX_WORLD_COORDS];
    ls::math::vec4          screenCoords [SR_SHADER_MAX_SCREEN_COORDS];
    math::vec4              pVaryings    [SR_SHADER_MAX_VARYING_VECTORS * SR_SHADER_MAX_SCREEN_COORDS];

    if (vao.has_index_buffer())
    {
        pIbo = &mContext->ibo(vao.get_index_buffer());
    }

    if (mMesh.mode == RENDER_MODE_INDEXED_POINTS)
    {
        begin += mThreadId;
        const size_t step = mNumThreads;
        for (size_t i = begin; i < end; i += step)
        {
            const size_t vertId = get_next_vertex(pIbo, i);
            worldCoords[0] = shader(vertId, vao, vbo, pUniforms, pVaryings);

            if (worldCoords[0][3] > 0.f)
            {
                screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
                push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_POINTS)
    {
        begin += mThreadId;
        const size_t step = mNumThreads;

        for (size_t i = begin; i < end; i += step)
        {
            worldCoords[0] = shader(i, vao, vbo, pUniforms, pVaryings);

            if (worldCoords[0][3] > 0.f)
            {
                screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
                push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_INDEXED_LINES)
    {
        // 3 vertices per set of lines
        begin += mThreadId * 3u;
        const size_t step = mNumThreads * 3u;

        for (size_t i = begin; i < end; i += step)
        {
            for (size_t j = 0; j < 3; ++j)
            {
                const size_t   index0  = i + j;
                const size_t   index1  = i + ((j + 1) % 3);
                const size_t   vertId0 = get_next_vertex(pIbo, index0);
                const size_t   vertId1 = get_next_vertex(pIbo, index1);
                worldCoords[0]         = shader(vertId0, vao, vbo, pUniforms, pVaryings);
                worldCoords[1]         = shader(vertId1, vao, vbo, pUniforms, pVaryings + numVaryings);

                if (worldCoords[0][3] > 0.f && worldCoords[1][3] > 0.f)
                {
                    screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
                    screenCoords[1] = world_to_screen_3(worldCoords[1], widthScale, heightScale);

                    if (sr_clip_liang_barsky(screenCoords, fboW, fboH))
                    {
                        push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
                    }
                }
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_LINES)
    {
        // 3 vertices per set of lines
        begin += mThreadId * 3u;
        const size_t step = mNumThreads * 3u;
        for (size_t i = begin; i < end; i += step)
        {
            for (size_t j = 0; j < 3; ++j)
            {
                const size_t   vertId0 = i + j;
                const size_t   vertId1 = i + ((j + 1) % 3);
                worldCoords[0]         = shader(vertId0, vao, vbo, pUniforms, pVaryings);
                worldCoords[1]         = shader(vertId1, vao, vbo, pUniforms, pVaryings + numVaryings);

                if (worldCoords[0][3] > 0.f && worldCoords[1][3] > 0.f)
                {
                    screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
                    screenCoords[1] = world_to_screen_3(worldCoords[1], widthScale, heightScale);

                    if (sr_clip_liang_barsky(screenCoords, fboW, fboH))
                    {
                        push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
                    }
                }
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_INDEXED_TRIANGLES)
    {
        // 3 vertices per triangle
        begin += mThreadId * 3u;
        const size_t step = mNumThreads * 3u;
        for (size_t i = begin; i < end; i += step)
        {
            const math::vec3_t<size_t>&& vertId = get_next_vertex3(pIbo, i);

            worldCoords[0]  = shader(vertId[0], vao, vbo, pUniforms, pVaryings);
            worldCoords[1]  = shader(vertId[1], vao, vbo, pUniforms, pVaryings + numVaryings);
            worldCoords[2]  = shader(vertId[2], vao, vbo, pUniforms, pVaryings + (numVaryings << 1));

            screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
            screenCoords[1] = world_to_screen_3(worldCoords[1], widthScale, heightScale);
            screenCoords[2] = world_to_screen_3(worldCoords[2], widthScale, heightScale);

            if ((cullMode == SR_CULL_BACK_FACE  && backface_visible(screenCoords, worldCoords))
            ||  (cullMode == SR_CULL_FRONT_FACE && frontface_visible(screenCoords, worldCoords))
            ||  (cullMode == SR_CULL_OFF        && face_visible(worldCoords)))
            {
                push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
            }
        }
    }
    else if (mMesh.mode == RENDER_MODE_TRIANGLES)
    {
        // 3 vertices per triangle
        begin += mThreadId * 3u;
        const size_t step = mNumThreads * 3u;
        for (size_t i = begin; i < end; i += step)
        {
            const size_t vertId0 = i;
            const size_t vertId1 = i+1;
            const size_t vertId2 = i+2;

            worldCoords[0]  = shader(vertId0, vao, vbo, pUniforms, pVaryings);
            worldCoords[1]  = shader(vertId1, vao, vbo, pUniforms, pVaryings + numVaryings);
            worldCoords[2]  = shader(vertId2, vao, vbo, pUniforms, pVaryings + (numVaryings << 1));

            screenCoords[0] = world_to_screen_3(worldCoords[0], widthScale, heightScale);
            screenCoords[1] = world_to_screen_3(worldCoords[1], widthScale, heightScale);
            screenCoords[2] = world_to_screen_3(worldCoords[2], widthScale, heightScale);

            if ((cullMode == SR_CULL_BACK_FACE  && backface_visible(screenCoords, worldCoords))
            ||  (cullMode == SR_CULL_FRONT_FACE && frontface_visible(screenCoords, worldCoords))
            ||  (cullMode == SR_CULL_OFF        && face_visible(worldCoords)))
            {
                push_fragments(fboW, fboH, screenCoords, worldCoords, pVaryings);
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
