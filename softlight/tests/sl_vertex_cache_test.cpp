
#include <iomanip>
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"



namespace utils = ls::utils;



struct CacheTestData
{
    bool enabled;
    std::string sceneName;
    std::string sceneFile;
    std::vector<size_t> numHits;
    std::vector<size_t> numIndices;
    double millisElapsedCached;
    double millisElapsedUncached;
};



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> load_scene(const std::string& fileName)
{
    int retCode = 0;
    (void)retCode;

    SL_SceneFileLoader meshLoader;
    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};

    retCode = meshLoader.load(fileName);
    LS_ASSERT(retCode != 0);

    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 0);

    return pGraph;
}



/*--------------------------------------
 * Load a grouping of vertex element IDs (mostly copied from the tri processor
--------------------------------------*/
inline LS_INLINE ls::math::vec4_t<size_t> get_next_vertex3(const SL_IndexBuffer* LS_RESTRICT_PTR pIbo, size_t vId) noexcept
{
    union
    {
        ls::math::vec4_t<unsigned char> asBytes;
        ls::math::vec4_t<unsigned short> asShorts;
        ls::math::vec4_t<unsigned int> asInts;
    } ids;

    ids = *reinterpret_cast<const decltype(ids)*>(pIbo->element(vId));

    switch (pIbo->type())
    {
        case VERTEX_DATA_BYTE:
            return (ls::math::vec4_t<size_t>)ids.asBytes;

        case VERTEX_DATA_SHORT:
            return (ls::math::vec4_t<size_t>)ids.asShorts;

        case VERTEX_DATA_INT:
            return (ls::math::vec4_t<size_t>)ids.asInts;

        default:
            LS_UNREACHABLE();
    }

    return ls::math::vec4_t<size_t>{0, 0, 0, 0};
}



/*-----------------------------------------------------------------------------
 * Query PTV cache Info
-----------------------------------------------------------------------------*/
void process_tris_cached(CacheTestData& testData, const utils::Pointer<SL_SceneGraph>& pGraph) noexcept
{
    const SL_Context& context = pGraph->mContext;
    SL_TransformedVert pVert;
    pVert.varyings[0] = ls::math::vec4{1.f};
    pVert.varyings[1] = ls::math::vec4{2.f};
    pVert.varyings[2] = ls::math::vec4{3.f};
    pVert.varyings[3] = ls::math::vec4{4.f};

    size_t totalIndices = 0;
    size_t hitCount = 0;

    const auto&& vertTransform = [&](size_t key, SL_TransformedVert& tv)->void {
        ++hitCount;
        ls::math::mat4 m{
            pVert.varyings[0],
            pVert.varyings[1],
            pVert.varyings[2],
            pVert.varyings[3],
        };
        tv.vert = m * ls::math::vec4{(float)key};
    };

    ls::utils::Clock<double, std::chrono::milliseconds::period> timer;
    timer.stop();
    timer.start();

    for (const SL_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t           nodeMeshId  = meshIds[meshId];
            const SL_Mesh&         m           = pGraph->mMeshes[nodeMeshId];
            const SL_VertexArray& vao          = context.vao(m.vaoId);
            const SL_IndexBuffer* pIbo         = vao.has_index_buffer() ? &context.ibo(vao.get_index_buffer()) : nullptr;
            const int             usingIndices = (m.mode == RENDER_MODE_INDEXED_TRIANGLES) || (m.mode == RENDER_MODE_INDEXED_TRI_WIRE);

            SL_PTVCache ptvCache{};
            const size_t threadId = 0;
            const size_t numThreads = 4;//std::thread::hardware_concurrency();

            #if 1
                size_t begin;
                size_t end;
                constexpr size_t step = 3;
                sl_calc_indexed_parition<3, true>(m.elementEnd-m.elementBegin, numThreads, threadId, begin, end);
                begin += m.elementBegin;
                end += m.elementBegin;

            #else
                //const size_t begin = m.elementBegin + mThreadId * 3u;
                const size_t begin = m.elementBegin + (threadId % numThreads) * 3u;
                const size_t end   = m.elementEnd;
                const size_t step  = numThreads * 3u;
            #endif

            totalIndices = 0;
            hitCount = 0;

            for (size_t i = begin; i < end; i += step)
            {
                const ls::math::vec4_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : ls::math::vec4_t<size_t>{i+0, i+1, i+2, i+3};
                sl_cache_query_or_update(ptvCache, vertId[0], pVert, vertTransform);
                sl_cache_query_or_update(ptvCache, vertId[1], pVert, vertTransform);
                sl_cache_query_or_update(ptvCache, vertId[2], pVert, vertTransform);

                totalIndices += step;
            }

            testData.numHits.push_back(hitCount);
            testData.numIndices.push_back(totalIndices);
        }
    }

    timer.tick();
    testData.millisElapsedCached = timer.tick_time().count();
}



/*-----------------------------------------------------------------------------
 * Query PTV cache Info
-----------------------------------------------------------------------------*/
void process_tris_uncached(CacheTestData& testData, const utils::Pointer<SL_SceneGraph>& pGraph) noexcept
{
    const SL_Context& context = pGraph->mContext;
    SL_TransformedVert pVert;
    pVert.varyings[0] = ls::math::vec4{1.f};
    pVert.varyings[1] = ls::math::vec4{2.f};
    pVert.varyings[2] = ls::math::vec4{3.f};
    pVert.varyings[3] = ls::math::vec4{4.f};

    ls::utils::Clock<double, std::chrono::milliseconds::period> timer;
    const auto&& vertShader = [&](const SL_VertexParam& params)->ls::math::vec4 {
        ls::math::mat4 m{
            params.pVaryings[0],
            params.pVaryings[1],
            params.pVaryings[2],
            params.pVaryings[3],
        };
        return m * ls::math::vec4{(float)params.vertId};
    };

    SL_VertexParam params;
    params.pUniforms  = nullptr;
    params.instanceId = 0;
    params.pVao       = nullptr;
    params.pVbo       = nullptr;
    params.pVaryings  = pVert.varyings;

    timer.stop();
    timer.start();

    for (const SL_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t           nodeMeshId  = meshIds[meshId];
            const SL_Mesh&         m           = pGraph->mMeshes[nodeMeshId];
            const SL_VertexArray& vao          = context.vao(m.vaoId);
            const SL_IndexBuffer* pIbo         = vao.has_index_buffer() ? &context.ibo(vao.get_index_buffer()) : nullptr;
            const int             usingIndices = (m.mode == RENDER_MODE_INDEXED_TRIANGLES) || (m.mode == RENDER_MODE_INDEXED_TRI_WIRE);

            const size_t threadId = 0;
            const size_t numThreads = 4;//std::thread::hardware_concurrency();

            #if 0
                size_t begin;
                size_t end;
                constexpr size_t step = 3;
                sl_calc_indexed_parition<3, true>(m.elementEnd-m.elementBegin, numThreads, threadId, begin, end);
                begin += m.elementBegin;
                end += m.elementBegin;

            #else
                //const size_t begin = m.elementBegin + mThreadId * 3u;
                const size_t begin = m.elementBegin + (threadId % numThreads) * 3u;
                const size_t end   = m.elementEnd;
                const size_t step  = numThreads * 3u;
            #endif

            for (size_t i = begin; i < end; i += step)
            {
                const ls::math::vec4_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : ls::math::vec4_t<size_t>{i+0, i+1, i+2, i+3};

                params.vertId = vertId[0];
                pVert.varyings[(i+0)%3] = vertShader(params);

                params.vertId = vertId[1];
                pVert.varyings[(i+1)%3] = vertShader(params);

                params.vertId = vertId[2];
                pVert.varyings[(i+2)%3] = vertShader(params);
            }
        }
    }

    timer.tick();
    testData.millisElapsedUncached = timer.tick_time().count();
}



/*-----------------------------------------------------------------------------
 * Print PTV cache Info
-----------------------------------------------------------------------------*/
void print_cache_info(const CacheTestData& testData, bool summarize) noexcept
{
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << testData.sceneName << " Cache Statistics: " << std::endl;
    double totalPercent = 0.0;
    size_t totalHits = 0;
    size_t totalIndices = 0;
    size_t numMeshes = testData.numHits.size();

    std::streamsize oldPrecision = std::cout.precision(std::numeric_limits<double>::digits10);

    for (size_t i = 0; i < numMeshes; ++i)
    {
        double percentHit = testData.numIndices[i] ? ((double)testData.numHits[i] / (double)testData.numIndices[i]) : 0;
        totalPercent += percentHit;
        totalHits += testData.numHits[i];
        totalIndices += testData.numIndices[i];

        if (!summarize)
        {
            std::cout
                << "\tSubMesh " << i << ':'
                << "\n\t\tIndices:   " << testData.numIndices[i]
                << "\n\t\tHit Count: " << testData.numHits[i]
                << "\n\t\tHit Rate:  " << 100.0 * percentHit
                << std::endl;
        }
    }

    std::cout
        << "\tSummary"
        << "\n\t\tTotal Indices:      " << totalIndices
        << "\n\t\tTotal Hit Count:    " << totalHits
        << "\n\t\tAverage Hit Rate:   " << 100.0 * (totalPercent / (double)numMeshes)
        << "\n\t\tCached Time (ms):   " << testData.millisElapsedCached
        << "\n\t\tUncached Time (ms): " << testData.millisElapsedUncached
        << std::endl;

    std::cout.precision(oldPrecision);
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    std::string sceneFile;
    utils::Pointer<SL_SceneGraph> pGraph;

    std::vector<CacheTestData> testList{
        {
            true,
            "Sibenik Cathedral",
            "testdata/sibenik/sibenik.obj",
            {}, {},
            0.0,
            0.0
        },
        {
            true,
            "Bob",
            "testdata/bob/Bob.md5mesh",
            {}, {},
            0.0,
            0.0
        },
        {
            true,
            "Mars Rover",
            "testdata/rover/testmesh.dae",
            {}, {},
            0.0,
            0.0
        },
        {
            true,
            "Zelda Heart",
            "testdata/heart/heart.obj",
            {}, {},
            0.0,
            0.0
        },
        {
            true,
            "Someone's Head",
            "testdata/african_head/african_head.obj",
            {}, {},
            0.0,
            0.0
        },
        {
            true,
            "Houdini Castle",
            "testdata/towerG.obj",
            {}, {},
            0.0,
            0.0
        },
    };

    for (CacheTestData& testData : testList)
    {
        testData.numHits.clear();
        testData.numIndices.clear();

        if (testData.enabled)
        {
            pGraph = std::move(load_scene(testData.sceneFile));
            process_tris_cached(testData, pGraph);
            process_tris_uncached(testData, pGraph);
        }
    }

    for (CacheTestData& testData : testList)
    {
        if (testData.enabled)
        {
            print_cache_info(testData, true);
        }
    }

    return 0;
}
