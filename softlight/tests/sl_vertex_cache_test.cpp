
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/utils/Pointer.h"

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
void query_cache_info(CacheTestData& testData, const utils::Pointer<SL_SceneGraph>& pGraph) noexcept
{
    const SL_Context& context = pGraph->mContext;
    const auto&& vertTransform = [&](size_t key, SL_TransformedVert& tv)->void {
        tv.vert = ls::math::vec4{(float)key};
    };

    SL_TransformedVert pVert;

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
            const size_t numThreads = std::thread::hardware_concurrency();
            size_t begin;
            size_t end;
            constexpr size_t step = 3;

            sl_calc_indexed_parition<3, true>(m.elementEnd-m.elementBegin, numThreads, 0, begin, end);
            begin += m.elementBegin;
            end += m.elementBegin;

            size_t totalIndices = 0;
            size_t hitCount = 0;

            for (size_t i = begin; i < end; i += step)
            {
                const ls::math::vec4_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : ls::math::vec4_t<size_t>{i+0, i+1, i+2, i+3};

                if (ptvCache.query(vertId[0]))
                {
                    ++hitCount;
                }
                sl_cache_query_or_update(ptvCache, vertId[0], pVert, vertTransform);

                if (ptvCache.query(vertId[1]))
                {
                    ++hitCount;
                }
                sl_cache_query_or_update(ptvCache, vertId[1], pVert, vertTransform);

                if (ptvCache.query(vertId[2]))
                {
                    ++hitCount;
                }
                sl_cache_query_or_update(ptvCache, vertId[2], pVert, vertTransform);

                totalIndices += step;
            }

            testData.numHits.push_back(hitCount);
            testData.numIndices.push_back(totalIndices);
        }
    }
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
        << "\n\t\tTotal Indices:    " << totalIndices
        << "\n\t\tTotal Hit Count:  " << totalHits
        << "\n\t\tAverage Hit Rate: " << 100.0 * (totalPercent / (double)numMeshes)
        << std::endl;
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
            {}, {}
        },
        {
            true,
            "Bob",
            "testdata/bob/Bob.md5mesh",
            {}, {}
        },
        {
            true,
            "Mars Rover",
            "testdata/rover/testmesh.dae",
            {}, {}
        },
        {
            true,
            "Zelda Heart",
            "testdata/heart/heart.obj",
            {}, {}
        },
        {
            true,
            "Someone's Head",
            "testdata/african_head/african_head.obj",
            {}, {}
        },
        {
            true,
            "Houdini Castle",
            "testdata/towerG.obj",
            {}, {}
        },
    };

    for (CacheTestData& testData : testList)
    {
        testData.numHits.clear();
        testData.numIndices.clear();

        if (testData.enabled)
        {
            pGraph = std::move(load_scene(testData.sceneFile));
            query_cache_info(testData, pGraph);
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
