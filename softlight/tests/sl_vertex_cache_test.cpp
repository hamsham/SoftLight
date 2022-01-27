
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
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"



namespace utils = ls::utils;



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
void query_cache_info(std::vector<size_t>& numHits, std::vector<size_t>& numIndices, const utils::Pointer<SL_SceneGraph>& pGraph) noexcept
{
    const ls::math::mat4 identity{1.f};
    const SL_Context& context = pGraph->mContext;
    const auto vertShader = [](SL_VertexParam&)->ls::math::vec4
    {
        return ls::math::vec4{};
    };
    SL_VertexParam params;
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

            SL_PTVCache ptvCache{vertShader, params};
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

                if (ptvCache.have_key(vertId[0]))
                {
                    ++hitCount;
                }
                ptvCache.query_and_update(vertId[0], identity, pVert);

                if (ptvCache.have_key(vertId[1]))
                {
                    ++hitCount;
                }
                ptvCache.query_and_update(vertId[1], identity, pVert);

                if (ptvCache.have_key(vertId[2]))
                {
                    ++hitCount;
                }
                ptvCache.query_and_update(vertId[2], identity, pVert);

                totalIndices += step;
            }

            numHits.push_back(hitCount);
            numIndices.push_back(totalIndices ? totalIndices : 1);
        }
    }
}



/*-----------------------------------------------------------------------------
 * Print PTV cache Info
-----------------------------------------------------------------------------*/
void print_cache_info(std::vector<size_t>& numHits, std::vector<size_t>& numIndices, const char* sceneName) noexcept
{
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << sceneName << " Cache Statistics: " << std::endl;
    for (size_t i = 0; i < numHits.size(); ++i)
    {
        std::cout
            << "\n\tTotal Indices: " << numIndices[i]
            << "\n\tHits (total):  " << numHits[i]
            << "\n\tHits (%):      " << (100.0 * ((double)numHits[i] / (double)numIndices[i]))
            << std::endl;
    }
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    std::string sceneFile;
    utils::Pointer<SL_SceneGraph> pGraph;

    bool testSibenik = true;
    bool testBob = true;
    bool testRover = true;

    std::vector<size_t> numHits0;
    std::vector<size_t> numIndices0;
    if (testSibenik)
    {
        pGraph = std::move(load_scene("testdata/sibenik/sibenik.obj"));
        query_cache_info(numHits0, numIndices0, pGraph);
    }

    std::vector<size_t> numHits1;
    std::vector<size_t> numIndices1;
    if (testBob)
    {
        pGraph = std::move(load_scene("testdata/bob/Bob.md5mesh"));
        query_cache_info(numHits1, numIndices1, pGraph);
    }

    std::vector<size_t> numHits2;
    std::vector<size_t> numIndices2;
    if (testRover)
    {
        pGraph = std::move(load_scene("testdata/rover/testmesh.dae"));
        query_cache_info(numHits2, numIndices2, pGraph);
    }

    print_cache_info(numHits0, numIndices0, "Sibenik");
    print_cache_info(numHits1, numIndices1, "Bob");
    print_cache_info(numHits2, numIndices2, "Mars Rover");

    return 0;
}
