
#include <algorithm> // std::replace
#include <utility> // std::move
#include <string>

#include "lightsky/setup/OS.h" // LS_OS_WINDOWS

#include "lightsky/utils/Log.h"

#include "lightsky/math/Math.h"
#include "lightsky/math/scalar_utils.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Config.hpp" // SL_VERTEX_CACHING_ENABLED
#include "softlight/SL_ImgFile.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneFileUtility.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"



namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * SL_VaoGroup Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_VaoGroup::~SL_VaoGroup() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_VaoGroup::SL_VaoGroup() noexcept :
    vertType{(SL_CommonVertType)0},
    numVboBytes{0},
    vboOffset{0},
    meshOffset{0},
    baseVert{0}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_VaoGroup::SL_VaoGroup(const SL_VaoGroup& v) noexcept :
    vertType{v.vertType},
    numVboBytes{v.numVboBytes},
    vboOffset{v.vboOffset},
    meshOffset{v.meshOffset},
    baseVert{v.baseVert}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_VaoGroup::SL_VaoGroup(SL_VaoGroup&& v) noexcept :
    vertType{v.vertType},
    numVboBytes{v.numVboBytes},
    vboOffset{v.vboOffset},
    meshOffset{v.meshOffset},
    baseVert{v.baseVert}
{
    v.vertType = (SL_CommonVertType)0;
    v.numVboBytes = 0;
    v.vboOffset = 0;
    v.meshOffset = 0;
    v.baseVert = 0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_VaoGroup& SL_VaoGroup::operator=(const SL_VaoGroup& v) noexcept
{
    vertType = v.vertType;
    numVboBytes = v.numVboBytes;
    vboOffset = v.vboOffset;
    meshOffset = v.meshOffset;
    baseVert = v.baseVert;
    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_VaoGroup& SL_VaoGroup::operator=(SL_VaoGroup&& v) noexcept
{
    vertType = v.vertType;
    v.vertType = (SL_CommonVertType)0;

    numVboBytes = v.numVboBytes;
    v.numVboBytes = 0;

    vboOffset = v.vboOffset;
    v.vboOffset = 0;

    meshOffset = v.meshOffset;
    v.meshOffset = 0;

    baseVert = v.baseVert;
    v.baseVert = 0;

    return *this;
}



/*-----------------------------------------------------------------------------
 * SL_SceneLoadOpts Structure
-----------------------------------------------------------------------------*/
SL_SceneLoadOpts sl_default_scene_load_opts() noexcept
{
    SL_SceneLoadOpts opts;
    opts.packUvs = false;
    opts.packNormals = false;
    opts.packBoneIds = false;
    opts.packBoneWeights = false;
    opts.genFlatNormals = false;
    opts.genSmoothNormals = true;
    opts.genTangents = false;
    opts.swizzleTexels = false;

    return opts;
}



/*-----------------------------------------------------------------------------
 * SL_SceneFilePreload Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_SceneFilePreload::~SL_SceneFilePreload() noexcept
{
    unload();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_SceneFilePreload::SL_SceneFilePreload() noexcept :
    mFilepath{},
    mLoadOpts{sl_default_scene_load_opts()},
    mImporter{nullptr},
    mSceneInfo{},
    mSceneData{},
    mBaseFileDir{"./"},
    mVaoGroups{},
    mTexPaths{}
{}



/*-------------------------------------
 * SceneResource Move Constructor
-------------------------------------*/
SL_SceneFilePreload::SL_SceneFilePreload(SL_SceneFilePreload&& s) noexcept :
    mFilepath{std::move(s.mFilepath)},
    mLoadOpts{std::move(s.mLoadOpts)},
    mImporter{std::move(s.mImporter)},
    mSceneInfo{std::move(s.mSceneInfo)},
    mSceneData{std::move(s.mSceneData)},
    mBaseFileDir{std::move(s.mBaseFileDir)},
    mVaoGroups{std::move(s.mVaoGroups)},
    mTexPaths{std::move(s.mTexPaths)}
{}



/*-------------------------------------
 * Mesh Loader move operator
-------------------------------------*/
SL_SceneFilePreload& SL_SceneFilePreload::operator=(SL_SceneFilePreload&& s) noexcept
{
    unload();

    mFilepath = std::move(s.mFilepath);
    mLoadOpts = std::move(s.mLoadOpts);
    mImporter = std::move(s.mImporter);
    mSceneInfo = std::move(s.mSceneInfo);
    mSceneData = std::move(s.mSceneData);
    mBaseFileDir = std::move(s.mBaseFileDir);
    mVaoGroups = std::move(s.mVaoGroups);
    mTexPaths = std::move(s.mTexPaths);

    return *this;
}



/*-------------------------------------
 * SceneResource Destructor
-------------------------------------*/
void SL_SceneFilePreload::unload() noexcept
{
    mFilepath.clear();

    mLoadOpts = sl_default_scene_load_opts();

    mImporter.reset();

    mSceneInfo = SL_SceneFileMeta{};

    mSceneData.terminate();

    mBaseFileDir = "./";

    mVaoGroups.clear();

    mTexPaths.clear();
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SL_SceneFilePreload::load(const std::string& filename, SL_SceneLoadOpts opts) noexcept
{
    unload();

    LS_LOG_MSG("Attempting to load 3D mesh file ", filename, '.');

    mLoadOpts = opts;

    // load
    mImporter.reset(new Assimp::Importer);

    Assimp::Importer& fileImporter = *mImporter;
    fileImporter.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, (int)SL_BONE_MAX_WEIGHTS);
    fileImporter.SetPropertyInteger(AI_CONFIG_FAVOUR_SPEED, 1);
    fileImporter.SetPropertyBool(AI_CONFIG_IMPORT_NO_SKELETON_MESHES, AI_TRUE);
    fileImporter.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, 1); // remove degenerate triangles
    fileImporter.SetPropertyBool(AI_CONFIG_PP_FD_CHECKAREA, AI_FALSE); // During degenerate removal, don't remove small triangles
    //fileImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, AI_SLM_DEFAULT_MAX_TRIANGLES);
    fileImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, std::numeric_limits<int32_t>::max());
    //fileImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, std::numeric_limits<uint16_t>::max());

    fileImporter.SetPropertyInteger(AI_CONFIG_PP_ICL_PTCACHE_SIZE, SL_VERTEX_CACHE_SIZE);

    unsigned defaultFlags = SCENE_FILE_IMPORT_FLAGS;
    if (opts.genTangents)
    {
        opts.genSmoothNormals = !(opts.genFlatNormals || opts.genSmoothNormals);
        defaultFlags |= aiProcess_CalcTangentSpace;
    }

    if (opts.genFlatNormals)
    {
        defaultFlags |= aiProcess_GenNormals;
    }

    if (opts.genSmoothNormals)
    {
        // Ensure we're only using one "normals" flag
        defaultFlags = (defaultFlags | aiProcess_GenNormals) ^ aiProcess_GenNormals;
        defaultFlags |= aiProcess_GenSmoothNormals;
    }

    if (!fileImporter.ReadFile(filename.c_str(), defaultFlags))
    {
        LS_LOG_ERR(
            "\tError: Unable to load the mesh file ", filename,
            " due to an import error:\n\t", fileImporter.GetErrorString(), '\n'
        );
        unload();
        return false;
    }
    else
    {
        const std::string::size_type baseDirIndex = filename.find_last_of(u8R"(\/)");
        if (baseDirIndex != std::string::npos)
        {
            mBaseFileDir = filename.substr(0, baseDirIndex + 1);
        }

        // remove duplicate backslashes
        for (std::string::size_type i = baseDirIndex; i > 0; --i)
        {
            if ((mBaseFileDir[i] == u'\\' || mBaseFileDir[i] == u'/') && mBaseFileDir[i] == mBaseFileDir[i-1])
            {
                mBaseFileDir.pop_back();
            }
        }

        LS_LOG_MSG("\tMesh file successfully imported from ", mBaseFileDir, ". Running post-process optimization.");
    }

    const aiScene* const pScene = preload_mesh_data();
    if (!pScene)
    {
        LS_LOG_ERR(
            "\tError: Failed to process the 3D mesh file ",
            filename, " in memory.\n"
        );
        unload();
        return false;
    }

    if (!allocate_cpu_data(pScene))
    {
        LS_LOG_ERR(
            "\tError: Failed to allocate data for the 3D mesh file ",
            filename, ".\n"
        );
        unload();
        return false;
    }

    LS_LOG_MSG(
        "\tDone. Successfully loaded the scene file \"", filename, ".\"",
        "\n\t\tTotal Meshes:     ", mSceneData.mMeshes.size(),
        "\n\t\tTotal Textures:   ", mSceneData.mContext.textures().size(),
        "\n\t\tTotal Nodes:      ", mSceneData.mNodes.size(),
        "\n\t\tTotal Cameras:    ", mSceneData.mCameras.size(),
        "\n\t\tTotal Animations: ", mSceneData.mAnimations.size(),
        '\n'
    );

    mFilepath = filename;

    return true;
}



/*-------------------------------------
 * Verify *this contains data to pass to a SL_SceneFileLoader
-------------------------------------*/
bool SL_SceneFilePreload::is_loaded() const noexcept
{
    return mImporter.get() && mImporter->GetScene() != nullptr;
}


/*-------------------------------------
 * Perform a pre-processing step in
 * order to ensure a one-time
 * allocation of vertex+index data.
-------------------------------------*/
const aiScene* SL_SceneFilePreload::preload_mesh_data() noexcept
{
    const aiScene* const pScene = mImporter->GetScene();

    if (!pScene)
    {
        LS_LOG_ERR("\tERROR: Unable to pre-process a scene file in-memory.");
        return nullptr;
    }

    for (unsigned meshIter = 0; meshIter < pScene->mNumMeshes; ++meshIter)
    {
        const aiMesh* const pMesh = pScene->mMeshes[meshIter];
        const SL_CommonVertType inVertType = sl_convert_assimp_verts(pMesh, mLoadOpts);
        SL_VaoGroup* outMeshMarker = sl_get_matching_marker(inVertType, mVaoGroups);

        // Keep track of where in the output VBO a mesh's data should be placed.
        // Use the following information to contiguously group all mesh
        // vertices of similar types within the output VBO
        if (!outMeshMarker)
        {
            mVaoGroups.emplace_back(SL_VaoGroup{});
            outMeshMarker = &mVaoGroups[mVaoGroups.size() - 1];
            outMeshMarker->vertType = inVertType;
            outMeshMarker->numVboBytes = 0;
            outMeshMarker->vboOffset = 0;
            outMeshMarker->meshOffset = 0;
            outMeshMarker->baseVert = 0;
        }

        const unsigned numMeshVerts = pMesh->mNumVertices;
        mSceneInfo.totalVertices += numMeshVerts;

        const unsigned numMeshBytes = numMeshVerts * sl_vertex_byte_size(outMeshMarker->vertType);
        outMeshMarker->numVboBytes += numMeshBytes;
        mSceneInfo.totalVboBytes += numMeshBytes;

        // NOTE: I don't believe grouping indices by type in a single IBO
        // should matter, it's possible they can take advantage of the GPU's
        // caches but we'll hold off on that for now.
        unsigned numIndices = 0;
        for (unsigned faceIter = 0; faceIter < pMesh->mNumFaces; ++faceIter)
        {
            numIndices += pMesh->mFaces[faceIter].mNumIndices;
        }
        mSceneInfo.totalIndices += numIndices;
    }

    mSceneInfo.indexType = sl_required_index_type(mSceneInfo.totalIndices);
    mSceneInfo.totalIboBytes = sl_index_byte_size(mSceneInfo.indexType) * mSceneInfo.totalIndices;

    // calculate all of the vertex strides
    unsigned totalVboOffset = 0;
    for (SL_VaoGroup& m : this->mVaoGroups)
    {
        m.vboOffset = totalVboOffset;
        totalVboOffset += m.numVboBytes;
    }

    LS_LOG_MSG(
        "\tScene File Memory requirements:",
        "\n\t\tVBO Byte Size:   ", mSceneInfo.totalVboBytes,
        "\n\t\tVertex Count:    ", mSceneInfo.totalVertices,
        "\n\t\tIBO Byte Size:   ", mSceneInfo.totalIboBytes,
        "\n\t\tIndex Count:     ", mSceneInfo.totalIndices,
        "\n\t\tVAO Count:       ", mVaoGroups.size()
    );
    for (unsigned i = 0; i < mVaoGroups.size(); ++i)
    {
        SL_VaoGroup& m = mVaoGroups[i];
        LS_LOG_MSG("\t\t                 VAO ", i, ": 0x", std::hex, m.vertType, std::dec);

        for (unsigned j = 0; j < SL_NUM_COMMON_VERTEX_FLAGS; ++j)
        {
            if (!!(m.vertType & SL_COMMON_VERTEX_FLAGS[j]))
            {
                LS_LOG_MSG("\t\t                        ", sl_common_vertex_names()[j]);
            }
        }
    }

    return pScene;
}


/*-------------------------------------
 * Allocate all required CPU-side memory for a scene.
-------------------------------------*/
bool SL_SceneFilePreload::allocate_cpu_data(const aiScene* const pScene) noexcept
{
    mSceneData.mMeshes.resize(pScene->mNumMeshes);
    mSceneData.mMaterials.resize(pScene->mNumMaterials);

    for (SL_Material& m : mSceneData.mMaterials)
    {
        sl_reset(m);
    }

    // reserve space for textures using ASSIMP's materials count, the
    // "pScene->mNumTextures" member only counts textures saved into the file
    // itself. Materials store file paths.
    SL_Context& renderData = mSceneData.mContext;
    renderData.mTextures.reserve(pScene->mNumMaterials);

    mTexPaths.reserve(pScene->mNumMaterials);

    size_t numBones = 0;
    for (unsigned i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh* pMesh = pScene->mMeshes[i];
        numBones += (size_t)pMesh->mNumBones;
    }

    // Reserve data here. There's no telling whether all nodes can be imported
    // or not while Assimp's bones and lights remain unsupported.
    const unsigned numSceneNodes = sl_count_assimp_nodes(pScene->mRootNode);
    mSceneData.mMeshBounds.resize(pScene->mNumMeshes);
    mSceneData.mNodes.reserve(numSceneNodes);
    mSceneData.mBaseTransforms.reserve(numSceneNodes);
    mSceneData.mCurrentTransforms.reserve(numSceneNodes);
    mSceneData.mInvBoneTransforms.reserve(numBones);
    mSceneData.mBoneOffsets.reserve(numBones);
    mSceneData.mNodeNames.reserve(numSceneNodes);
    mSceneData.mAnimations.reserve(pScene->mNumAnimations);
    mSceneData.mCameras.reserve(pScene->mNumCameras);
    mSceneData.mNumNodeMeshes.reserve(pScene->mNumMeshes);
    mSceneData.mNodeMeshes.reserve(pScene->mNumMeshes);

    return true;
}



/*-----------------------------------------------------------------------------
 * SL_SceneFileLoader Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_SceneFileLoader::~SL_SceneFileLoader() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_SceneFileLoader::SL_SceneFileLoader() noexcept :
    mPreloader{},
    mLoadedTextures{}
{}



/*-------------------------------------
 * SceneResource Move Constructor
-------------------------------------*/
SL_SceneFileLoader::SL_SceneFileLoader(SL_SceneFileLoader&& s) noexcept :
    mPreloader{std::move(s.mPreloader)},
    mLoadedTextures{std::move(s.mLoadedTextures)}
{}



/*-------------------------------------
 * Mesh Loader move operator
-------------------------------------*/
SL_SceneFileLoader& SL_SceneFileLoader::operator=(SL_SceneFileLoader&& s) noexcept
{
    mPreloader = std::move(s.mPreloader);
    mLoadedTextures = std::move(s.mLoadedTextures);

    return *this;
}



/*-------------------------------------
 * SceneResource Destructor
-------------------------------------*/
void SL_SceneFileLoader::unload() noexcept
{
    mPreloader.unload();
    mLoadedTextures.clear();
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SL_SceneFileLoader::load(const std::string& filename, SL_SceneLoadOpts opts) noexcept
{
    unload();

    if (!mPreloader.load(filename, opts))
    {
        return false;
    }

    return load_scene(mPreloader.mImporter->GetScene(), opts);
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SL_SceneFileLoader::load(SL_SceneFilePreload&& p) noexcept
{
    LS_ASSERT(&p != &mPreloader);

    unload();

    if (p.is_loaded())
    {
        mPreloader = std::move(p);
        return load_scene(mPreloader.mImporter->GetScene(), p.mLoadOpts);
    }

    return false;
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SL_SceneFileLoader::load_scene(const aiScene* const pScene, SL_SceneLoadOpts opts) noexcept
{
    const std::string& filename = mPreloader.mFilepath;
    SL_SceneGraph& sceneData = mPreloader.mSceneData;

    LS_LOG_MSG("\tAllocating GPU memory for 3D scene data.");
    if (!allocate_gpu_data())
    {
        unload();
        LS_LOG_ERR("\t\tUnable to initialize 3D scene data on the CPU.\n");
        return false;
    }

    if (import_materials(pScene) != 0)
    {
        LS_LOG_ERR("\tError: Unable to load materials for the 3D mesh ", filename, "!\n");
        unload();
        return false;
    }

    // Find all bone nodes and store their offset matrices
    std::unordered_map<std::string, math::mat4>& offsets = mPreloader.mBoneOffsets;

    for (unsigned i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh* const pMesh = pScene->mMeshes[i];
        for (unsigned j = 0; j < pMesh->mNumBones; ++j)
        {
            const aiBone* pBone = pMesh->mBones[j];
            const std::string nodeName{pBone->mName.C_Str()};
            offsets[nodeName] = sl_convert_assimp_matrix(pBone->mOffsetMatrix);
        }
    }

    math::mat4 invGlobalTransform{1.f};
    read_node_hierarchy(pScene, pScene->mRootNode, SCENE_NODE_ROOT_ID, invGlobalTransform);

    /*
    for (const SL_SceneNode n : sceneData.mNodes)
    {
        const size_t nId = n.nodeId;
        LS_LOG_MSG(nId, ' ', sceneData.mCurrentTransforms[nId].mParentId);
    }
    */

    // to properly get the correct match of bones to vertex IDs, we need to
    // ensure the mesh base vertex is applied to all vertices affected by bones
    {
        std::vector<SL_VaoGroup> tempVaoGroups = mPreloader.mVaoGroups;
        for (unsigned i = 0; i < pScene->mNumMeshes; ++i)
        {
            const aiMesh* const pMesh = pScene->mMeshes[i];
            const SL_CommonVertType inVertType = sl_convert_assimp_verts(pMesh, opts);
            SL_VaoGroup* outMeshMarker = sl_get_matching_marker(inVertType, tempVaoGroups);

            if (!import_bone_data(pMesh, outMeshMarker->baseVert, opts))
            {
                LS_LOG_ERR("\t\tUnable to import bone data for the mesh ", pMesh->mName.C_Str());
                return false;
            }

            outMeshMarker->baseVert += pMesh->mNumVertices;
        }
    }

    if (!import_mesh_data(pScene, opts))
    {
        LS_LOG_ERR("\tError: Failed to load the 3D mesh ", filename, "!\n");
        unload();
        return false;
    }

    if (!import_animations(pScene))
    {
        LS_LOG_ERR("\tWarning: Failed to animations from ", filename, "!\n");
    }

    LS_LOG_MSG(
        "\tDone. Successfully loaded the scene file \"", filename, ".\"",
        "\n\t\tTotal Meshes:     ", sceneData.mMeshes.size(),
        "\n\t\tTotal Bones:      ", sceneData.mBoneOffsets.size(),
        "\n\t\tTotal Textures:   ", sceneData.mContext.textures().size(),
        "\n\t\tTotal Nodes:      ", sceneData.mNodes.size(),
        "\n\t\tTotal Cameras:    ", sceneData.mCameras.size(),
        "\n\t\tTotal Animations: ", sceneData.mAnimations.size(),
        '\n'
    );

    return true;
}



/*-------------------------------------
 * Allocate all required GPU-side memory for a scene.
-------------------------------------*/
bool SL_SceneFileLoader::allocate_gpu_data() noexcept
{
    SL_SceneGraph&                  sceneData  = mPreloader.mSceneData;
    const std::vector<SL_VaoGroup>& vboMarkers = mPreloader.mVaoGroups;
    SL_SceneFileMeta&               sceneInfo  = mPreloader.mSceneInfo;
    SL_Context&                     renderData = sceneData.mContext;

    SL_VertexBuffer vbo;
    SL_IndexBuffer ibo;

    if (vboMarkers.empty())
    {
        LS_LOG_MSG("\t\tNo Vertex types available to load.");
        return true;
    }

    size_t totalMeshTypes = vboMarkers.size();
    utils::Pointer<SL_CommonVertType[]> vertTypes{new SL_CommonVertType[totalMeshTypes]};

    // initialize the VBO attributes
    for (size_t i = 0; i < vboMarkers.size(); ++i)
    {
        vertTypes[i] = vboMarkers[i].vertType;
    }

    if (sceneInfo.totalVertices)
    {
        if (vbo.init(sceneInfo.totalVboBytes) != 0)
        {
            LS_LOG_ERR("\t\tFailed to initialize a VBO to hold all mesh data for the currently loading scene file.");
            return false;
        }

        LS_LOG_MSG("\t\tAllocated ", sceneInfo.totalVboBytes, " bytes for ", vboMarkers.size(), " types of vertices.");
    }

    if (sceneInfo.totalIndices)
    {
        // We're only creating one IBO for loading mesh data
        if (ibo.init(sceneInfo.totalIndices, sceneInfo.indexType) != 0)
        {
            vbo.terminate();
            LS_LOG_ERR("\t\tFailed to initialize a IBO to hold all mesh data for the currently loading scene file.");
            return false;
        }

        LS_LOG_MSG("\t\tAllocated ", sceneInfo.totalIboBytes, " bytes for indices.");
    }

    // Start adding the mesh descriptors and GL handles
    SL_AlignedVector<SL_VertexArray>& vaos = renderData.mVaos;
    vaos.clear();
    vaos.reserve(totalMeshTypes);

    for (unsigned i = 0; i < totalMeshTypes; ++i)
    {
        SL_VertexArray     vao{};
        SL_CommonVertType  inAttribs          = vertTypes[i];
        size_t             currentVaoAttribId = 0;
        const SL_VaoGroup& m                  = vboMarkers[i];
        const int          numBindings        = inAttribs ? math::popcnt_i32((int32_t)inAttribs) : 0;//math::count_set_bits(inAttribs);

        if (vao.set_num_bindings((size_t)numBindings) != numBindings)
        {
            LS_LOG_ERR("\t\tFailed to allocate memory for VAO bindings.");
            return false;
        }
        else
        {
            LS_LOG_MSG("\t\tAllocated ", numBindings, " bindings for VAO ", i, '.');
        }

        for (unsigned k = 0; k < SL_NUM_COMMON_VERTEX_FLAGS; ++k)
        {
            const SL_CommonVertType currentVertType = SL_COMMON_VERTEX_FLAGS[k];
            if (0 != (inAttribs & currentVertType))
            {
                const ptrdiff_t offset = m.vboOffset + sl_vertex_attrib_offset(inAttribs, currentVertType);

                vao.set_binding(
                    currentVaoAttribId,
                    offset,
                    sl_vertex_stride(inAttribs),
                    sl_dimens_of_vertex(currentVertType),
                    sl_type_of_vertex(currentVertType)
                );
                currentVaoAttribId++;
            }
        }

        vao.set_vertex_buffer(renderData.mVbos.size());
        vao.set_index_buffer(renderData.mIbos.size());

        vaos.emplace_back(std::move(vao));
    }

    renderData.mVbos.emplace_back(std::move(vbo));
    renderData.mIbos.emplace_back(std::move(ibo));

    return true;
}


/*-------------------------------------
 *
-------------------------------------*/
int SL_SceneFileLoader::import_materials(const aiScene* const pScene) noexcept
{
    static constexpr aiTextureType texTypes[] = {
        aiTextureType_DIFFUSE,
        aiTextureType_HEIGHT,
        aiTextureType_SPECULAR,
        aiTextureType_AMBIENT,
        aiTextureType_EMISSIVE,
        aiTextureType_NORMALS,
        aiTextureType_SHININESS,
        aiTextureType_OPACITY,
        aiTextureType_DISPLACEMENT,
        aiTextureType_LIGHTMAP,
        aiTextureType_REFLECTION,
        aiTextureType_UNKNOWN
    };

    const unsigned numMaterials = pScene->mNumMaterials;
    utils::Pointer<SL_ImgFile> imgLoader{new SL_ImgFile{}};

    LS_LOG_MSG("\tImporting ", numMaterials, " materials from the imported mesh.");

    if (!numMaterials)
    {
        LS_LOG_MSG("\t\tDone.");
        return 0;
    }

    for (unsigned i = 0; i < numMaterials; ++i)
    {
        const aiMaterial* const pMaterial = pScene->mMaterials[i];

        SL_Material& newMaterial = mPreloader.mSceneData.mMaterials[i];
        sl_reset(newMaterial);

        for (unsigned j = 0; j < LS_ARRAY_SIZE(texTypes); ++j)
        {
            import_texture_path(pMaterial, texTypes[j], newMaterial.pTextures, *imgLoader, mLoadedTextures);
        }

        aiColor3D inMatColor;
        float inTransparent = 1.f;
        float inShininess = 0.f;

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, inMatColor))
        {
            LS_LOG_MSG("\t\t\tUsing transparent material: ", inMatColor.r, ", ", inMatColor.g, ", ", inMatColor.b);
            inTransparent = (float)math::min<ai_real>(1.f, math::max<ai_real>(0.f, inMatColor.r, inMatColor.g, inMatColor.b));
        }

        // Assimp uses BGR formatting for material colors. We must convert back to RGB
        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, inMatColor))
        {
            LS_LOG_MSG("\t\t\tUsing ambient material: ", inMatColor.r, ", ", inMatColor.g, ", ", inMatColor.b);
            newMaterial.ambient[0] = inMatColor.b;
            newMaterial.ambient[1] = inMatColor.g;
            newMaterial.ambient[2] = inMatColor.r;
            newMaterial.ambient[3] = inTransparent;
        }

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, inMatColor))
        {
            LS_LOG_MSG("\t\t\tUsing diffuse material: ", inMatColor.r, ", ", inMatColor.g, ", ", inMatColor.b);
            newMaterial.diffuse[0] = inMatColor.b;
            newMaterial.diffuse[1] = inMatColor.g;
            newMaterial.diffuse[2] = inMatColor.r;
            newMaterial.diffuse[3] = inTransparent;
        }

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, inMatColor))
        {
            LS_LOG_MSG("\t\t\tUsing specular material: ", inMatColor.r, ", ", inMatColor.g, ", ", inMatColor.b);
            newMaterial.specular[0] = inMatColor.b;
            newMaterial.specular[1] = inMatColor.g;
            newMaterial.specular[2] = inMatColor.r;
            newMaterial.specular[3] = inTransparent;
        }

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS, inShininess))
        {
            LS_LOG_MSG("\t\t\tLoading shininess: ", inShininess);
            newMaterial.shininess = inShininess;
        }
    }
    LS_LOG_MSG("\t\tDone.");

    return 0;
}


/*-------------------------------------
    Read and import a single texture path
-------------------------------------*/
void SL_SceneFileLoader::import_texture_path(
    const aiMaterial* const pMaterial,
    const int slotType,
    const SL_Texture* pTextures[SL_MaterialProperty::SL_MATERIAL_MAX_TEXTURES],
    SL_ImgFile& imgLoader,
    std::unordered_map<std::string, const SL_Texture*>& loadedTextures
) noexcept
{
    imgLoader.unload();

    const unsigned maxTexCount = math::min<unsigned>(SL_MATERIAL_MAX_TEXTURES, pMaterial->GetTextureCount((aiTextureType)slotType));
    unsigned materialTexOffset = 0;

    if (maxTexCount)
    {
        switch (slotType)
        {
            case aiTextureType::aiTextureType_AMBIENT:
                materialTexOffset = SL_MATERIAL_TEXTURE_AMBIENT;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " ambient textures.");
                break;

            case aiTextureType::aiTextureType_DIFFUSE:
                materialTexOffset = SL_MATERIAL_TEXTURE_DIFFUSE;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " diffuse textures.");
                break;

            case aiTextureType::aiTextureType_NORMALS:
                materialTexOffset = SL_MATERIAL_TEXTURE_NORMAL;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " normal maps.");
                break;

            case aiTextureType::aiTextureType_HEIGHT:
                materialTexOffset = SL_MATERIAL_TEXTURE_HEIGHT;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " height maps.");
                break;

            case aiTextureType::aiTextureType_SPECULAR:
                materialTexOffset = SL_MATERIAL_TEXTURE_SPECULAR;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " specular maps.");
                break;

            case aiTextureType::aiTextureType_OPACITY:
                materialTexOffset = SL_MATERIAL_TEXTURE_OPACITY;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " opacity maps.");
                break;

                // TODO: other textures are unsupported at the moment.
            default:
                materialTexOffset = SL_MATERIAL_TEXTURE_MISC0;
                LS_LOG_MSG("\t\tLocated ", maxTexCount, " miscellaneous textures (", slotType, ").");
                break;
        }
    }

    // iterate
    aiString inPath;
    aiTextureMapMode inWrapMode[3] = {aiTextureMapMode::aiTextureMapMode_Wrap};
    const SL_Texture* pTexture = nullptr;

    for (unsigned i = 0, j = 0; i < maxTexCount; ++i)
    {
        if (materialTexOffset != SL_MATERIAL_TEXTURE_MISC0 && pTextures[materialTexOffset])
        {
            materialTexOffset = SL_MATERIAL_TEXTURE_MISC0;
        }

        // SL_MATERIAL_TEXTURE_MISC0 is used as an offset. Here we check if
        // a texture slot at SL_MATERIAL_TEXTURE_MISC0+N is available.
        if (materialTexOffset == SL_MATERIAL_TEXTURE_MISC0)
        {
            for (unsigned k = SL_MATERIAL_TEXTURE_MISC0; k <= SL_MATERIAL_MAX_TEXTURES; ++k)
            {
                if (k == SL_MATERIAL_MAX_TEXTURES)
                {
                    LS_LOG_ERR("\t\t\tExceeded material texture count (", k, " > ", SL_MATERIAL_MAX_TEXTURES, ").");
                    return;
                }

                if (!pTextures[k]) {
                    materialTexOffset = k;
                    break;
                }
            }
        }

        // Assimp won't clear this by default
        inPath.Clear();

        if (pMaterial->GetTexture((aiTextureType)slotType, i, &inPath, nullptr, nullptr, nullptr, nullptr, inWrapMode) != aiReturn_SUCCESS)
        {
            LS_LOG_ERR("\t\t\tFailed to locate the texture ", inPath.C_Str());
            continue;
        }

        // add the imported texture to the appropriate array in textureSet.
        std::string&& texPath = mPreloader.mBaseFileDir + inPath.C_Str();

        #ifndef LS_OS_WINDOWS
        std::replace(texPath.begin(), texPath.end(), '\\', '/');
        #endif

        if (loadedTextures.count(texPath) > 0)
        {
            pTextures[materialTexOffset] = loadedTextures[texPath];
            LS_LOG_MSG("\t\t\tAlready loaded texture: ", texPath);
        }
        else
        {
            pTexture = load_texture_at_path(texPath, imgLoader);

            if (pTexture != nullptr)
            {
                loadedTextures[texPath] = pTexture;
                pTextures[materialTexOffset] = pTexture;
            }
            else
            {
                LS_LOG_ERR("\t\t\tFailed to load a texture: ", texPath);
                pTextures[materialTexOffset] = nullptr;
                continue;
            }
        }

        ++j;
    }
}



/*-------------------------------------
 * Attempt to load a texture from the local filesystem
-------------------------------------*/
SL_Texture* SL_SceneFileLoader::load_texture_at_path(const std::string& path, SL_ImgFile& imgLoader) noexcept
{
    if (imgLoader.load(path.c_str()) != SL_ImgFile::ImgStatus::FILE_LOAD_SUCCESS)
    {
        return nullptr;
    }

    SL_SceneGraph& graph = mPreloader.mSceneData;
    const size_t loadedTexture = graph.mContext.create_texture();
    SL_Texture& t = graph.mContext.texture(loadedTexture);

    SL_TexelOrder texelOrder = mPreloader.mLoadOpts.swizzleTexels ? SL_TexelOrder::SWIZZLED : SL_TexelOrder::ORDERED;
    //if (t.init(imgLoader, SL_TexelOrder::ORDERED) != 0)
    if (t.init(imgLoader, texelOrder) != 0)
    {
        graph.mContext.destroy_texture(loadedTexture);
        return nullptr;
    }

    return &t;
}



/*-------------------------------------
 * Use all information from the
 * preprocess step to allocate and
 * import mesh information
-------------------------------------*/
bool SL_SceneFileLoader::import_mesh_data(const aiScene* const pScene, const SL_SceneLoadOpts& opts) noexcept
{
    LS_LOG_MSG("\tImporting vertices and indices of individual meshes from a file.");

    std::vector<SL_VaoGroup>&         tempVboMarks = mPreloader.mVaoGroups;
    SL_SceneGraph&                    sceneData    = mPreloader.mSceneData;
    SL_Context&                       renderData   = sceneData.mContext;
    SL_AlignedVector<SL_Mesh>&        meshes       = sceneData.mMeshes;
    SL_AlignedVector<SL_BoundingBox>& bounds       = sceneData.mMeshBounds;
    SL_VertexBuffer&                  vbo          = renderData.vbo(renderData.vbos().size()-1);
    SL_IndexBuffer&                   ibo          = renderData.ibo(renderData.ibos().size()-1);
    size_t                            baseIndex    = 0;
    char* const                       pVbo         = reinterpret_cast<char*>(vbo.data());
    char*                             pIbo         = reinterpret_cast<char*>(ibo.data());

    // vertex data in ASSIMP is not interleaved. It has to be converted into
    // the internally used vertex format which is recommended for use on mobile
    // devices.
    for (unsigned meshId = 0; meshId < pScene->mNumMeshes; ++meshId)
    {
        const aiMesh* const     pMesh       = pScene->mMeshes[meshId];
        const SL_CommonVertType vertType    = sl_convert_assimp_verts(pMesh, opts);
        const size_t            meshGroupId = get_mesh_group_marker(vertType, mPreloader.mVaoGroups);
        SL_VaoGroup&            meshGroup   = tempVboMarks[meshGroupId];
        size_t                  numIndices  = 0;

        LS_ASSERT(meshGroup.vertType == vertType);

        SL_Mesh& mesh   = meshes[meshId];
        SL_BoundingBox& box = bounds[meshId];
        mesh.materialId = (uint32_t)pMesh->mMaterialIndex;
        mesh.vaoId      = meshGroupId;

        // get the offset to the current mesh so it can be sent to a VBO
        const size_t meshOffset = meshGroup.vboOffset + meshGroup.meshOffset;
        sl_upload_mesh_vertices(
            pMesh, meshGroup.baseVert,
            pVbo + meshOffset,
            meshGroup.vertType,
            mPreloader.mBones);

        // increment the mesh offset for the next mesh
        meshGroup.meshOffset += sl_vertex_stride(meshGroup.vertType) * pMesh->mNumVertices;
        pIbo = upload_mesh_indices(pMesh, pIbo, baseIndex, meshGroup.baseVert, mesh, numIndices);
        meshGroup.baseVert += pMesh->mNumVertices;
        baseIndex += numIndices;

        sl_update_mesh_bounds(pMesh, box);
    }

    LS_LOG_MSG("\t\tDone.");

    return true;
}



/*-------------------------------------
 * Use all information from the preprocess step to allocate and import bone
 * information.
-------------------------------------*/
bool SL_SceneFileLoader::import_bone_data(const aiMesh* const pMesh, unsigned baseVert, const SL_SceneLoadOpts& opts) noexcept
{
    if (!pMesh->mNumBones)
    {
        return true;
    }

    LS_LOG_MSG("\tImporting bones from a file.");
    SL_AlignedVector<std::string>&             nodeNames = mPreloader.mSceneData.mNodeNames;
    std::unordered_map<uint32_t, SL_BoneData>& boneData  = mPreloader.mBones;

    for (unsigned i = 0; i < pMesh->mNumBones; ++i)
    {
        const aiBone*     pBone      = pMesh->mBones[i];
        const std::string boneName   = {pBone->mName.C_Str()};
        const uint32_t    numWeights = pBone->mNumWeights;
        size_t            boneId     = 0;

        SL_AlignedVector<std::string>::iterator&& nameIter = std::find(nodeNames.begin(), nodeNames.end(), boneName);
        if (nameIter != nodeNames.end())
        {
            boneId = std::distance(nodeNames.begin(), nameIter);
        }

        // gather all weights from this bone
        for (uint32_t j = 0; j < numWeights; ++j)
        {
            const uint32_t vertId = (uint32_t)(baseVert+pBone->mWeights[j].mVertexId);
            const float    weight = pBone->mWeights[j].mWeight;

            // Apply up to SL_BONE_MAX_WEIGHTS weights for a single vertex
            std::unordered_map<uint32_t, SL_BoneData>::iterator&& iter = boneData.find(vertId);

            // If we have already accounted for a vertex, apply this weight to
            // the first non-zero value available for that vertex
            if (iter != boneData.end())
            {
                if (opts.packBoneIds)
                {
                    for (uint32_t k = 1; k < SL_BONE_MAX_WEIGHTS; ++k)
                    {
                        if (iter->second.ids16[k] == 0)
                        {
                            iter->second.ids16[k] = (uint16_t)boneId;

                            if (opts.packBoneIds)
                            {
                                iter->second.weights16[k] = ls::math::half{weight};
                            }
                            else
                            {
                                iter->second.weights32[k] = weight;
                            }

                            break;
                        }
                    }
                }
                else
                {
                    for (uint32_t k = 1; k < SL_BONE_MAX_WEIGHTS; ++k)
                    {
                        if (iter->second.ids32[k] == 0)
                        {
                            iter->second.ids32[k] = (uint32_t)boneId;

                            if (opts.packBoneIds)
                            {
                                iter->second.weights16[k] = ls::math::half{weight};
                            }
                            else
                            {
                                iter->second.weights32[k] = weight;
                            }

                            break;
                        }
                    }
                }
            }
            else
            {
                // The vertex we're checking does not have any weights
                // currently associated with it, add one.
                SL_BoneData newBone;
                if (opts.packBoneIds)
                {
                    newBone.ids16    = math::vec4_t<uint16_t>{0, 0, 0, 0};
                    newBone.ids16[0] = (int16_t)boneId;
                }
                else
                {
                    newBone.ids32    = math::vec4_t<uint32_t>{0, 0, 0, 0};
                    newBone.ids32[0] = (int32_t)boneId;
                }

                if (opts.packBoneWeights)
                {
                    newBone.weights16 = math::vec4_t<math::half>{math::half{0x00, 0x00}};
                    newBone.weights16[0] = weight;
                }
                else
                {
                    newBone.weights32 = math::vec4_t<float>{0.f, 0.f, 0.f, 0.f};
                    newBone.weights32[0] = weight;
                }

                boneData[vertId] = newBone;
            }
        }
    }

    return true;
}



/*-------------------------------------
    Read all face data (triangles)
-------------------------------------*/
char* SL_SceneFileLoader::upload_mesh_indices(
    const aiMesh* const pMesh,
    char* pIbo,
    const size_t baseIndex,
    const size_t baseVertex,
    SL_Mesh& outMesh,
    size_t& outNumIndices
) noexcept
{
    const SL_SceneFileMeta& sceneInfo = mPreloader.mSceneInfo;
    const ptrdiff_t numBytesPerIndex = sl_index_byte_size(sceneInfo.indexType);

    // iterate through all faces in the mesh
    for (size_t faceIter = 0; faceIter < pMesh->mNumFaces; ++faceIter)
    {
        const aiFace& face = pMesh->mFaces[faceIter];

        for (size_t i = 0; i < face.mNumIndices; ++i)
        {
            const size_t idx = face.mIndices[i] + baseVertex;

            switch (sceneInfo.indexType)
            {
                case VERTEX_DATA_BYTE:
                    *reinterpret_cast<unsigned char*>(pIbo) = (unsigned char)(idx);
                    break;

                case VERTEX_DATA_SHORT:
                    *reinterpret_cast<unsigned short*>(pIbo) = (unsigned short)(idx);
                    break;

                case VERTEX_DATA_INT:
                    *reinterpret_cast<unsigned int*>(pIbo) = (unsigned int)(idx);
                    break;

                default:
                    LS_ASSERT(false && "Unknown index type.");
                    break;
            }

            pIbo += numBytesPerIndex;
        }

        outNumIndices += face.mNumIndices;
    }

    // store the first/last vertex indices
    outMesh.mode = sl_convert_assimp_draw_mode(pMesh);
    outMesh.elementBegin = baseIndex;
    outMesh.elementEnd = baseIndex + outNumIndices;

    return pIbo;
}



/*-------------------------------------
 * Retrieve a single VBO Marker
-------------------------------------*/
size_t SL_SceneFileLoader::get_mesh_group_marker(
    const SL_CommonVertType vertType,
    const std::vector<SL_VaoGroup>& markers
) const noexcept
{
    for (size_t i = 0; i < markers.size(); ++i)
    {
        if (vertType == markers[i].vertType)
        {
            return i;
        }
    }

    LS_ASSERT(false);

    return (size_t)-1;
}



/*-------------------------------------
    Read and import all nodes in a scene
-------------------------------------*/
void SL_SceneFileLoader::read_node_hierarchy(
    const aiScene* const pScene,
    const aiNode* const  pInNode,
    const size_t         parentId,
    math::mat4&          invGlobalTransform
) noexcept
{
    // use the size of the node list as an index which should be returned to
    // the parent node's child indices.
    SL_SceneGraph&                  sceneData      = mPreloader.mSceneData;
    SL_AlignedVector<SL_SceneNode>& nodeList       = sceneData.mNodes;
    SL_AlignedVector<std::string>&  nodeNames      = sceneData.mNodeNames;
    SL_AlignedVector<math::mat4>&   baseTransforms = sceneData.mBaseTransforms;
    SL_AlignedVector<SL_Transform>& currTransforms = sceneData.mCurrentTransforms;
    SL_AlignedVector<math::mat4>&   modelMatrices  = sceneData.mModelMatrices;

    //LS_LOG_MSG("\tImporting Scene Node ", nodeList.size(), ": ", pInNode->mName.C_Str());

    nodeList.emplace_back(SL_SceneNode());
    SL_SceneNode& currentNode = nodeList.back();

    // initialize
    sl_reset(currentNode);
    currentNode.nodeId = nodeList.size() - 1;

    // import the node name
    const std::string nodeName{pInNode->mName.C_Str()};
    nodeNames.push_back(nodeName);

    // import the node transformation
    // This is also needed for camera nodes to be imported properly
    {
        SL_Transform baseTrans{};
        aiVector3D inPos, inScale;
        aiQuaternion inRotation;

        pInNode->mTransformation.Decompose(inScale, inRotation, inPos);

        baseTrans.position(sl_convert_assimp_vector(inPos));
        baseTrans.scaling(sl_convert_assimp_vector(inScale));
        baseTrans.orientation(sl_convert_assimp_quaternion(inRotation));

        // Store the transform as specified by the scene graph hierarchy.
        currTransforms.push_back(baseTrans);

        // Store the original, unmodified, unparented transform
        baseTrans.apply_transform();
        baseTransforms.push_back(baseTrans.transform());
    }

    const size_t camIndex = sl_is_node_type<aiCamera>(pInNode, pScene->mCameras, pScene->mNumCameras);

    if (camIndex != SCENE_NODE_ROOT_ID)
    {
        currentNode.type = NODE_TYPE_CAMERA;
        import_camera_node(pScene, camIndex, currentNode);
    }
    else if (mPreloader.mBoneOffsets.find(nodeName) != mPreloader.mBoneOffsets.end())
    {
        if (pInNode->mParent == pScene->mRootNode)
        {
            invGlobalTransform = sl_convert_assimp_matrix(pScene->mRootNode->mTransformation);
            invGlobalTransform = math::inverse(invGlobalTransform);
        }

        currentNode.type = NODE_TYPE_BONE;
        currentNode.dataId = mPreloader.mSceneData.mBoneOffsets.size();
        mPreloader.mSceneData.mInvBoneTransforms.push_back(invGlobalTransform);
        mPreloader.mSceneData.mBoneOffsets.push_back(mPreloader.mBoneOffsets[nodeName]);
    }
    else if (sl_is_node_type<aiMesh>(pInNode, pScene->mMeshes, pScene->mNumMeshes))
    {
        currentNode.type = NODE_TYPE_MESH;
        import_mesh_node(pInNode, currentNode);
    }
    else
    {
        currentNode.type = NODE_TYPE_EMPTY;
    }

    // Remaining transformation information
    {
        SL_Transform& nodeTransform = currTransforms.back();
        nodeTransform.mParentId = parentId;

        // Only apply parented transforms here. Applying non-parented
        // transforms will cause root objects to swap X & Z axes.
        if (parentId != SCENE_NODE_ROOT_ID)
        {
            nodeTransform.apply_pre_transform(currTransforms[parentId].transform());
        }

        modelMatrices.push_back(currTransforms.back().transform());
    }

    // CYA
    LS_ASSERT(nodeList.size() == nodeNames.size());
    LS_ASSERT(nodeList.size() == baseTransforms.size());
    LS_ASSERT(nodeList.size() == currTransforms.size());
    LS_ASSERT(nodeList.size() == modelMatrices.size());

    // recursively load node children
    for (unsigned childId = 0; childId < pInNode->mNumChildren; ++childId)
    {
        const aiNode* const pChildNode = pInNode->mChildren[childId];

        read_node_hierarchy(pScene, pChildNode, currentNode.nodeId, invGlobalTransform);
    }
}



/*-------------------------------------
    Import a mesh node
-------------------------------------*/
void SL_SceneFileLoader::import_mesh_node(const aiNode* const pNode, SL_SceneNode& outNode) noexcept
{
    SL_SceneGraph& graph = mPreloader.mSceneData;
    SL_AlignedVector<size_t>& nodeMeshCounts = graph.mNumNodeMeshes;
    SL_AlignedVector<utils::Pointer<size_t[]>>& meshList = graph.mNodeMeshes;

    // The check for how many meshes a scene node has must have already been
    // performed.
    const unsigned numMeshes = pNode->mNumMeshes;
    LS_ASSERT(numMeshes > 0);

    utils::Pointer<size_t[]> meshIds{new size_t[numMeshes]};
    LS_ASSERT(meshIds.get());

    // map the internal indices to the assimp node's mesh list
    for (unsigned i = 0; i < numMeshes; ++i)
    {
        meshIds[i] = pNode->mMeshes[i];
    }

    // Very important for the scene graph to keep track of what mesh node owns
    // which set of meshes
    outNode.dataId = meshList.size();

    nodeMeshCounts.push_back(numMeshes);
    meshList.emplace_back(std::move(meshIds));
}


/*-------------------------------------
    Import a camera node
-------------------------------------*/
void SL_SceneFileLoader::import_camera_node(
    const aiScene* const pScene,
    const size_t camIndex,
    SL_SceneNode& outNode
) noexcept
{
    SL_SceneGraph& graph = mPreloader.mSceneData;

    // Very important for the scene graph to keep track of what camera node
    // owns which camera
    outNode.dataId = camIndex;

    const aiCamera* const pInCam = pScene->mCameras[camIndex];
    SL_AlignedVector<SL_Camera>& camList = graph.mCameras;

    camList.emplace_back(SL_Camera());
    SL_Camera& outCam = graph.mCameras.back();

    outCam.set_fov(pInCam->mHorizontalFOV);
    outCam.set_aspect_ratio(pInCam->mAspect, 1.f);
    outCam.set_near_plane(pInCam->mClipPlaneNear);
    outCam.set_far_plane(pInCam->mClipPlaneFar);
    outCam.set_projection_type(SL_ProjectionType::SL_PROJECTION_LOGARITHMIC_PERSPECTIVE);
    outCam.update();

    // A Transform object must have been added by the parent function
    SL_Transform& camTrans = graph.mCurrentTransforms.back();
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS); // ASSIMP never specified a default

    const aiNode* const pNode = pScene->mRootNode->FindNode(pInCam->mName);

    aiVector3D inPos, inDir, inUp;

    if (pNode)
    {
        const aiMatrix4x4& inMat = pNode->mTransformation;

        inPos = pInCam->mPosition;
        inPos *= inMat;

        inDir = pInCam->mLookAt;
        inDir *= inMat;

        inUp = pInCam->mUp;
        inUp *= inMat;
    }
    else
    {
        inPos = pInCam->mPosition;
        inDir = pInCam->mLookAt;
        inUp = pInCam->mUp;
    }

    const math::vec3&& finalPos = sl_convert_assimp_vector(inPos);
    const math::vec3&& finalDir = sl_convert_assimp_vector(inDir);
    const math::vec3&& finalUp = sl_convert_assimp_vector(inUp);
    camTrans.look_at(finalPos, finalDir, finalUp);

    const math::vec3& camPos = camTrans.position();
    const math::vec3&& camUp = math::vec3{0.f, 1.f, 0.f};//nodeCam.up_direction();

    std::cout
        << "\tLoaded the scene camera " << pInCam->mName.C_Str() << ':'
        << "\n\t\tField of View: " << LS_RAD2DEG(outCam.get_fov())
        << "\n\t\tAspect Ratio:  " << outCam.get_aspect_ratio()
        << "\n\t\tNear Plane:    " << outCam.get_near_plane()
        << "\n\t\tFar Plane:     " << outCam.get_far_plane()
        << "\n\t\tPosition:      {" << camPos[0] << ", " << camPos[1] << ", " << camPos[2] << '}'
        << "\n\t\tUp Direction:  {" << camUp[0] << ", " << camUp[1] << ", " << camUp[2] << '}'
        << std::endl;
}



/*-------------------------------------
 * Import ASSIMP animations
-------------------------------------*/
bool SL_SceneFileLoader::import_animations(const aiScene* const pScene) noexcept
{
    bool ret = true;
    SL_SceneGraph&                  graph                      = mPreloader.mSceneData;
    const aiAnimation* const* const pAnimations                = pScene->mAnimations;
    SL_AlignedVector<SL_Animation>& animations                 = graph.mAnimations;
    const unsigned                  totalAnimations            = pScene->mNumAnimations;

    SL_AlignedVector<SL_AlignedVector<SL_AnimationChannel>>& allChannels = graph.mNodeAnims;

    for (unsigned i = 0; i < totalAnimations; ++i)
    {
        const aiAnimation* const pInAnim = pAnimations[i];

        // The animation as a whole needs to have its properties imported from
        // ASSIMP.
        animations.emplace_back(sl_setup_imported_animation(
            pInAnim->mName.C_Str(),
            pInAnim->mDuration,
            pInAnim->mTicksPerSecond,
            pInAnim->mNumChannels
        ));

        SL_Animation& anim = animations.back();

        for (unsigned c = 0; c < pInAnim->mNumChannels; ++c)
        {
            SL_AnimationChannel track;
            const aiNodeAnim* const pInTrack = pInAnim->mChannels[c];
            const unsigned nodeId = import_animation_track(pInTrack, track, anim.duration());

            if (nodeId == UINT_MAX)
            {
                // failing to load an Animation track is not an error.
                ret = false;
                continue;
            }

            // Grab the scene node which contains the data ID to map with a
            // transformation and animation channel
            SL_SceneNode& node = graph.mNodes[nodeId];

            // If a list of animation tracks doesn't exist for the current node
            // then be sure to add it.
            if (node.animListId == SCENE_NODE_ROOT_ID)
            {
                node.animListId = allChannels.size();
                allChannels.emplace_back(SL_AlignedVector<SL_AnimationChannel>{});
            }

            SL_AlignedVector<SL_AnimationChannel>& nodeChannels = allChannels[node.animListId];
            nodeChannels.emplace_back(std::move(track));

            // Add the node's imported track to the current animation
            anim.add_channel(node, nodeChannels.size() - 1);
        }

        std::cout
            << "\tLoaded Animation " << i+1 << '/' << totalAnimations
            << "\n\t\tName:      " << anim.name()
            << "\n\t\tDuration:  " << anim.duration()
            << "\n\t\tTicks/Sec: " << anim.ticks_per_sec()
            << "\n\t\tChannels:  " << anim.size()
            << std::endl;
    }

    if (totalAnimations > 0)
    {
        SL_Animation& initialState = animations[0];
        initialState.init(graph);
    }

    std::cout << "\tSuccessfully loaded " << animations.size() << " animations." << std::endl;

    return ret;
}



/*-------------------------------------
 * Import a single Animation track
-------------------------------------*/
unsigned SL_SceneFileLoader::import_animation_track(
    const aiNodeAnim* const pInAnim,
    SL_AnimationChannel& outAnim,
    const SL_AnimPrecision animDuration
) noexcept {
    const unsigned posFrames                    = pInAnim->mNumPositionKeys;
    const unsigned sclFrames                    = pInAnim->mNumScalingKeys;
    const unsigned rotFrames                    = pInAnim->mNumRotationKeys;
    const SL_AlignedVector<std::string>& nodeNames = mPreloader.mSceneData.mNodeNames;
    const char* const pInName                   = pInAnim->mNodeName.C_Str();
    unsigned nodeId                             = 0;

    // Locate the node associated with the current animation track.
    for (; nodeId < nodeNames.size(); ++nodeId)
    {
        if (nodeNames[nodeId] == pInName)
        {
            break;
        }
    }

    if (nodeId == nodeNames.size())
    {
        std::cerr << "\tError: Unable to locate the animation track for a scene node: " << pInAnim << std::endl;
        outAnim.clear();
        return UINT_MAX;
    }

    if (!outAnim.size(posFrames, sclFrames, rotFrames))
    {
        std::cout << "Unable to import the Animation \"" << pInAnim->mNodeName.C_Str() << "\"." << std::endl;
        return UINT_MAX;
    }

    SL_AnimationKeyListVec3& outPosFrames = outAnim.mPosFrames;
    SL_AnimationKeyListVec3& outSclFrames = outAnim.mScaleFrames;
    SL_AnimationKeyListQuat& outRotFrames = outAnim.mOrientFrames;

    // Convert all animation times into percentages for the internal animation
    // system.
    // Positions
    for (unsigned p=0; p < outPosFrames.size(); ++p)
    {
        const aiVectorKey& posKey = pInAnim->mPositionKeys[p];
        outPosFrames.frame(p, posKey.mTime / animDuration, sl_convert_assimp_vector(posKey.mValue));
    }

    // Scalings
    for (unsigned s=0; s < outSclFrames.size(); ++s)
    {
        const aiVectorKey& sclKey = pInAnim->mScalingKeys[s];
        outSclFrames.frame(s, sclKey.mTime / animDuration, sl_convert_assimp_vector(sclKey.mValue));
    }

    // Rotations
    for (unsigned r=0; r < outRotFrames.size(); ++r)
    {
        const aiQuatKey& rotKey = pInAnim->mRotationKeys[r];
        outRotFrames.frame(r, rotKey.mTime / animDuration, sl_convert_assimp_quaternion(rotKey.mValue));
    }

    // Convert ASSIMP animation flags into internal ones.
    // ASSIMP allows for animation behavior to change between the starting and
    // ending animation frames. Those are currently unsupported, all imported
    // animations share the same flags from start to finish.
    unsigned animFlags = SL_ANIM_FLAG_NONE;
    const auto flagFill = [&](const aiAnimBehaviour inFlag, const unsigned outFlag)->void
    {
        if (pInAnim->mPreState & inFlag || pInAnim->mPostState & inFlag)
        {
            animFlags |= outFlag;
        }
    };

    flagFill(aiAnimBehaviour_CONSTANT,  SL_ANIM_FLAG_IMMEDIATE);
    flagFill(aiAnimBehaviour_DEFAULT,   SL_ANIM_FLAG_INTERPOLATE);
    flagFill(aiAnimBehaviour_LINEAR,    SL_ANIM_FLAG_INTERPOLATE);
    flagFill(aiAnimBehaviour_REPEAT,    SL_ANIM_FLAG_REPEAT);

    outAnim.mAnimMode = (SL_AnimationFlag)animFlags;

    std::cout
        << "\tSuccessfully imported the Animation \"" << pInAnim->mNodeName.C_Str() << '\"'
        << "\n\t\tPosition Keys: " << outPosFrames.size() << " @ " << outPosFrames.duration()
        << "\n\t\tScaling Keys:  " << outSclFrames.size() << " @ " << outSclFrames.duration()
        << "\n\t\tRotation Keys: " << outRotFrames.size() << " @ " << outRotFrames.duration()
        << std::endl;

    return nodeId;
}
