
#include <algorithm> // std::replace
#include <utility> // std::move
#include <memory> // std::nothrow
#include <string>
#include <cstring> // strcmp()
#include <lightsky/math/Math.h>

#include "lightsky/setup/OS.h"

#include "lightsky/utils/Log.h"

#include "lightsky/math/scalar_utils.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_ImgFile.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneFileUtility.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"



namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * SR_VaoGroup Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_VaoGroup::~SR_VaoGroup() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_VaoGroup::SR_VaoGroup() noexcept :
    vertType{(SR_CommonVertType)0},
    numVboBytes{0},
    vboOffset{0},
    meshOffset{0},
    baseVert{0}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_VaoGroup::SR_VaoGroup(const SR_VaoGroup& v) noexcept :
    vertType{v.vertType},
    numVboBytes{v.numVboBytes},
    vboOffset{v.vboOffset},
    meshOffset{v.meshOffset},
    baseVert{v.baseVert}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_VaoGroup::SR_VaoGroup(SR_VaoGroup&& v) noexcept :
    vertType{v.vertType},
    numVboBytes{v.numVboBytes},
    vboOffset{v.vboOffset},
    meshOffset{v.meshOffset},
    baseVert{v.baseVert}
{
    v.vertType = (SR_CommonVertType)0;
    v.numVboBytes = 0;
    v.vboOffset = 0;
    v.meshOffset = 0;
    v.baseVert = 0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_VaoGroup& SR_VaoGroup::operator=(const SR_VaoGroup& v) noexcept
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
SR_VaoGroup& SR_VaoGroup::operator=(SR_VaoGroup&& v) noexcept
{
    vertType = v.vertType;
    v.vertType = (SR_CommonVertType)0;

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
 * SR_SceneFilePreload Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_SceneFilePreload::~SR_SceneFilePreload() noexcept
{
    unload();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_SceneFilePreload::SR_SceneFilePreload() noexcept :
    mFilepath{},
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
SR_SceneFilePreload::SR_SceneFilePreload(SR_SceneFilePreload&& s) noexcept :
    mFilepath{std::move(s.mFilepath)},
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
SR_SceneFilePreload& SR_SceneFilePreload::operator=(SR_SceneFilePreload&& s) noexcept
{
    unload();

    mFilepath = std::move(s.mFilepath);
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
void SR_SceneFilePreload::unload() noexcept
{
    mFilepath.clear();

    mImporter.reset();

    mSceneInfo = SR_SceneFileMeta{};

    mSceneData.terminate();

    mBaseFileDir = "./";

    mVaoGroups.clear();

    mTexPaths.clear();
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SR_SceneFilePreload::load(const std::string& filename) noexcept
{
    unload();

    LS_LOG_MSG("Attempting to load 3D mesh file ", filename, '.');

    // load
    mImporter.reset(new Assimp::Importer);

    Assimp::Importer& fileImporter = *mImporter;
    fileImporter.SetPropertyBool(AI_CONFIG_FAVOUR_SPEED, AI_TRUE);
    fileImporter.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, (int)SR_BONE_MAX_WEIGHTS);
    fileImporter.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, AI_TRUE); // remove degenerate triangles
    fileImporter.SetPropertyInteger(AI_CONFIG_FAVOUR_SPEED, AI_TRUE);
    //fileImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, AI_SLM_DEFAULT_MAX_TRIANGLES);
    //fileImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, std::numeric_limits<uint8_t>::max());

    if (!fileImporter.ReadFile(filename.c_str(), SCENE_FILE_IMPORT_FLAGS))
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
        LS_LOG_MSG("\tMesh file successfully imported. Running post-process optimization.");

        const std::string::size_type baseDirIndex = filename.find_last_of(u8R"(\/)");
        if (baseDirIndex != std::string::npos)
        {
            mBaseFileDir = filename.substr(0, baseDirIndex + 1);
        }
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
 * Verify *this contains data to pass to a SR_SceneFileLoader
-------------------------------------*/
bool SR_SceneFilePreload::is_loaded() const noexcept
{
    return mImporter.get() && mImporter->GetScene() != nullptr;
}


/*-------------------------------------
 * Perform a pre-processing step in
 * order to ensure a one-time
 * allocation of vertex+index data.
-------------------------------------*/
const aiScene* SR_SceneFilePreload::preload_mesh_data() noexcept
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
        const SR_CommonVertType inVertType = sr_convert_assimp_verts(pMesh);
        SR_VaoGroup* outMeshMarker = sr_get_matching_marker(inVertType, mVaoGroups);

        // Keep track of where in the output VBO a mesh's data should be placed.
        // Use the following information to contiguously group all mesh
        // vertices of similar types within the output VBO
        if (!outMeshMarker)
        {
            mVaoGroups.emplace_back(SR_VaoGroup{});
            outMeshMarker = &mVaoGroups[mVaoGroups.size() - 1];
            outMeshMarker->vertType = inVertType;
            outMeshMarker->numVboBytes = 0;
            outMeshMarker->vboOffset = 0;
            outMeshMarker->meshOffset = 0;
            outMeshMarker->baseVert = 0;
        }

        const unsigned numMeshVerts = pMesh->mNumVertices;
        mSceneInfo.totalVertices += numMeshVerts;

        const unsigned numMeshBytes = numMeshVerts * sr_vertex_byte_size(outMeshMarker->vertType);
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

    mSceneInfo.indexType = sr_required_index_type(mSceneInfo.totalIndices);
    mSceneInfo.totalIboBytes = sr_index_byte_size(mSceneInfo.indexType) * mSceneInfo.totalIndices;

    // calculate all of the vertex strides
    unsigned totalVboOffset = 0;
    for (SR_VaoGroup& m : this->mVaoGroups)
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
        SR_VaoGroup& m = mVaoGroups[i];
        LS_LOG_MSG("\t\t                 VAO ", i, ": 0x", std::hex, m.vertType, std::dec);

        for (unsigned j = 0; j < SR_NUM_COMMON_VERTEX_FLAGS; ++j)
        {
            if (!!(m.vertType & SR_COMMON_VERTEX_FLAGS[j]))
            {
                LS_LOG_MSG("\t\t                        ", sr_common_vertex_names()[j]);
            }
        }
    }

    return pScene;
}


/*-------------------------------------
 * Allocate all required CPU-side memory for a scene.
-------------------------------------*/
bool SR_SceneFilePreload::allocate_cpu_data(const aiScene* const pScene) noexcept
{
    mSceneData.mMeshes.resize(pScene->mNumMeshes);
    mSceneData.mMaterials.resize(pScene->mNumMaterials);

    for (SR_Material& m : mSceneData.mMaterials)
    {
        sr_reset(m);
    }

    // reserve space for textures using ASSIMP's materials count, the
    // "pScene->mNumTextures" member only counts textures saved into the file
    // itself. Materials store file paths.
    SR_Context& renderData = mSceneData.mContext;
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
    const unsigned numSceneNodes = sr_count_assimp_nodes(pScene->mRootNode);
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
 * SR_SceneFileLoader Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_SceneFileLoader::~SR_SceneFileLoader() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_SceneFileLoader::SR_SceneFileLoader() noexcept :
    mPreloader{}
{}



/*-------------------------------------
 * SceneResource Move Constructor
-------------------------------------*/
SR_SceneFileLoader::SR_SceneFileLoader(SR_SceneFileLoader&& s) noexcept :
    mPreloader{std::move(s.mPreloader)}
{}



/*-------------------------------------
 * Mesh Loader move operator
-------------------------------------*/
SR_SceneFileLoader& SR_SceneFileLoader::operator=(SR_SceneFileLoader&& s) noexcept
{
    mPreloader = std::move(s.mPreloader);

    return *this;
}



/*-------------------------------------
 * SceneResource Destructor
-------------------------------------*/
void SR_SceneFileLoader::unload() noexcept
{
    mPreloader.unload();
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SR_SceneFileLoader::load(const std::string& filename) noexcept
{
    unload();

    if (!mPreloader.load(filename))
    {
        return false;
    }

    return load_scene(mPreloader.mImporter->GetScene());
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SR_SceneFileLoader::load(SR_SceneFilePreload&& p) noexcept
{
    LS_DEBUG_ASSERT(&p != &mPreloader);

    unload();

    if (p.is_loaded())
    {
        mPreloader = std::move(p);
        return load_scene(mPreloader.mImporter->GetScene());
    }

    return false;
}



/*-------------------------------------
 * Load a set of meshes from a file
-------------------------------------*/
bool SR_SceneFileLoader::load_scene(const aiScene* const pScene) noexcept
{
    const std::string& filename = mPreloader.mFilepath;
    SR_SceneGraph& sceneData = mPreloader.mSceneData;

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
    math::mat4 invGlobalTransform = sr_convert_assimp_matrix(pScene->mRootNode->mTransformation);
    invGlobalTransform = math::inverse(invGlobalTransform);

    for (unsigned i = 0; i < pScene->mNumMeshes; ++i)
    {
        const aiMesh* const pMesh = pScene->mMeshes[i];
        for (unsigned j = 0; j < pMesh->mNumBones; ++j)
        {
            const aiBone* pBone = pMesh->mBones[j];
            const std::string nodeName{pBone->mName.C_Str()};
            offsets[nodeName] = sr_convert_assimp_matrix(pBone->mOffsetMatrix);
        }
    }

    read_node_hierarchy(pScene, pScene->mRootNode, SCENE_NODE_ROOT_ID, invGlobalTransform);

    for (const SR_SceneNode n : sceneData.mNodes)
    {
        const size_t nId = n.nodeId;
        LS_LOG_MSG(nId, ' ', sceneData.mCurrentTransforms[nId].mParentId);
    }

    // to properly get the correct match of bones to vertex IDs, we need to
    // ensure the mesh base vertex is applied to all vertices affected by bones
    {
        std::vector<SR_VaoGroup> tempVaoGroups = mPreloader.mVaoGroups;
        for (unsigned i = 0; i < pScene->mNumMeshes; ++i)
        {
            const aiMesh* const pMesh = pScene->mMeshes[i];
            const SR_CommonVertType inVertType = sr_convert_assimp_verts(pMesh);
            SR_VaoGroup* outMeshMarker = sr_get_matching_marker(inVertType, tempVaoGroups);

            if (!import_bone_data(pMesh, outMeshMarker->baseVert))
            {
                LS_LOG_ERR("\t\tUnable to import bone data for the mesh ", pMesh->mName.C_Str());
                return false;
            }

            outMeshMarker->baseVert += pMesh->mNumVertices;
        }
    }

    if (!import_mesh_data(pScene))
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
bool SR_SceneFileLoader::allocate_gpu_data() noexcept
{
    SR_SceneGraph&           sceneData  = mPreloader.mSceneData;
    std::vector<SR_VaoGroup> vboMarkers = mPreloader.mVaoGroups;
    SR_SceneFileMeta&        sceneInfo  = mPreloader.mSceneInfo;
    SR_Context&              renderData = sceneData.mContext;

    SR_VertexBuffer vbo;
    SR_IndexBuffer ibo;

    if (vboMarkers.empty())
    {
        LS_LOG_MSG("\t\tNo Vertex types available to load.");
        return true;
    }

    size_t totalMeshTypes = vboMarkers.size();
    utils::Pointer<SR_CommonVertType[]> vertTypes{new SR_CommonVertType[totalMeshTypes]};

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
    std::vector<SR_VertexArray>& vaos = renderData.mVaos;
    vaos.clear();
    vaos.reserve(totalMeshTypes);

    for (unsigned i = 0; i < totalMeshTypes; ++i)
    {
        SR_VertexArray    vao{};
        SR_CommonVertType inAttribs          = vertTypes[i];
        size_t            currentVaoAttribId = 0;
        SR_VaoGroup&      m                  = vboMarkers[i];
        const int         numBindings        = math::count_set_bits(inAttribs);

        if (vao.set_num_bindings(numBindings) != numBindings)
        {
            LS_LOG_ERR("\t\tFailed to allocate memory for VAO bindings.");
            return false;
        }
        else
        {
            LS_LOG_MSG("\t\tAllocated ", numBindings, " bindings for VAO ", i, '.');
        }

        for (unsigned k = 0; k < SR_NUM_COMMON_VERTEX_FLAGS; ++k)
        {
            const SR_CommonVertType currentVertType = SR_COMMON_VERTEX_FLAGS[k];
            if (0 != (inAttribs & currentVertType))
            {
                const ptrdiff_t offset = m.vboOffset + sr_vertex_attrib_offset(inAttribs, currentVertType);

                vao.set_binding(
                    currentVaoAttribId,
                    offset,
                    sr_vertex_stride(inAttribs),
                    sr_dimens_of_vertex(currentVertType),
                    sr_type_of_vertex(currentVertType)
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
int SR_SceneFileLoader::import_materials(const aiScene* const pScene) noexcept
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
    utils::Pointer<SR_ImgFile> imgLoader{new SR_ImgFile{}};

    std::cout << "\tImporting " << numMaterials << " materials from the imported mesh." << std::endl;

    if (!numMaterials)
    {
        std::cout << "\t\tDone." << std::endl;
        return 0;
    }

    std::unordered_map<std::string, const SR_Texture*> loadedTextures;

    for (unsigned i = 0; i < numMaterials; ++i)
    {
        const aiMaterial* const pMaterial = pScene->mMaterials[i];

        SR_Material& newMaterial = mPreloader.mSceneData.mMaterials[i];
        sr_reset(newMaterial);

        for (unsigned j = 0; j < LS_ARRAY_SIZE(texTypes); ++j)
        {
            import_texture_path(pMaterial, texTypes[j], newMaterial, *imgLoader, loadedTextures);
        }
    }
    std::cout << "\t\tDone." << std::endl;

    return 0;
}


/*-------------------------------------
    Read and import a single texture path
-------------------------------------*/
void SR_SceneFileLoader::import_texture_path(
    const aiMaterial* const pMaterial,
    const int slotType,
    SR_Material& outMaterial,
    SR_ImgFile& imgLoader,
    std::unordered_map<std::string, const SR_Texture*>& loadedTextures
) noexcept
{
    imgLoader.unload();

    const unsigned maxTexCount = math::min<unsigned>(SR_MATERIAL_MAX_TEXTURES, pMaterial->GetTextureCount((aiTextureType)slotType));

    switch (slotType)
    {
        case aiTextureType::aiTextureType_DIFFUSE:
            std::cout << "\t\tLocated " << maxTexCount << " diffuse textures." << std::endl;
            break;

        case aiTextureType::aiTextureType_HEIGHT:
            std::cout << "\t\tLocated " << maxTexCount << " height maps." << std::endl;
            break;

        case aiTextureType::aiTextureType_NORMALS:
            std::cout << "\t\tLocated " << maxTexCount << " normal maps." << std::endl;
            break;

        case aiTextureType::aiTextureType_SPECULAR:
            std::cout << "\t\tLocated " << maxTexCount << " specular maps." << std::endl;
            break;

        case aiTextureType::aiTextureType_AMBIENT:
            std::cout << "\t\tLocated " << maxTexCount << " ambient textures." << std::endl;
            break;

            // TODO: other textures are unsupported at the moment.
        default:
            std::cout << "\t\tLocated " << maxTexCount << " miscellaneous textures (" << slotType << ")." << std::endl;
            break;
    }

    // Get an offset to the next texture's index within the current material
    unsigned materialTexOffset = 0;
    for (unsigned i = 0; i < SR_MATERIAL_MAX_TEXTURES; ++i)
    {
        if (outMaterial.pTextures[i] == nullptr)
        {
            materialTexOffset = i;
            break;
        }
    }

    // iterate
    aiString inPath;
    aiTextureMapMode inWrapMode[3] = {aiTextureMapMode::aiTextureMapMode_Wrap};
    aiColor3D inMatColor;
    float inShininess;
    const SR_Texture* pTexture = nullptr;

    for (unsigned i = 0; i < maxTexCount; ++i)
    {
        const unsigned outTexSlot = materialTexOffset+i;
        if (i + materialTexOffset > maxTexCount)
        {
            std::cerr << "\t\t\tExceeded material texture count." << std::endl;
        }

        // Assimp won't clear this by default
        inPath.Clear();

        if (pMaterial->GetTexture((aiTextureType)slotType, i, &inPath, nullptr, nullptr, nullptr, nullptr, inWrapMode) != aiReturn_SUCCESS)
        {
            std::cerr << "\t\t\tFailed to locate the texture " << inPath.C_Str() << std::endl;
            continue;
        }

        // add the imported texture to the appropriate array in textureSet.
        std::string&& texPath = mPreloader.mBaseFileDir + inPath.C_Str();

        #ifndef LS_OS_WINDOWS
        std::replace(texPath.begin(), texPath.end(), '\\', '/');
        #endif

        if (loadedTextures.count(texPath) > 0)
        {
            outMaterial.pTextures[outTexSlot] = loadedTextures[texPath];
        }
        else
        {
            pTexture = load_texture_at_path(texPath, imgLoader, inWrapMode[0]);

            if (pTexture != nullptr)
            {
                loadedTextures[texPath] = pTexture;
                outMaterial.pTextures[outTexSlot] = pTexture;
            }
            else
            {
                std::cerr << "\t\t\tFailed to load a texture: " << texPath << std::endl;
                outMaterial.pTextures[outTexSlot] = nullptr;
            }
        }
    }

    if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, inMatColor))
    {
        outMaterial.ambient[0] = inMatColor.r;
        outMaterial.ambient[1] = inMatColor.g;
        outMaterial.ambient[2] = inMatColor.b;
    }

    if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, inMatColor))
    {
        outMaterial.diffuse[0] = inMatColor.r;
        outMaterial.diffuse[1] = inMatColor.g;
        outMaterial.diffuse[2] = inMatColor.b;
    }

    if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, inMatColor))
    {
        outMaterial.specular[0] = inMatColor.r;
        outMaterial.specular[1] = inMatColor.g;
        outMaterial.specular[2] = inMatColor.b;
    }

    if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS, inShininess))
    {
        outMaterial.shininess = inShininess;
    }
}



/*-------------------------------------
 * Attempt to load a texture from the local filesystem
-------------------------------------*/
SR_Texture* SR_SceneFileLoader::load_texture_at_path(
    const std::string& path,
    SR_ImgFile& imgLoader,
    int wrapMode) noexcept
{
    if (imgLoader.load(path.c_str()) != SR_ImgFile::img_status_t::FILE_LOAD_SUCCESS)
    {
        return nullptr;
    }

    SR_SceneGraph& graph = mPreloader.mSceneData;
    const size_t loadedTexture = graph.mContext.create_texture();
    SR_Texture& t = graph.mContext.texture(loadedTexture);

    if (t.init(imgLoader) != 0)
    {
        graph.mContext.destroy_texture(loadedTexture);
        return nullptr;
    }

    t.set_wrap_mode(sr_convert_assimp_tex_wrap((aiTextureMapMode)wrapMode));

    return &t;
}



/*-------------------------------------
 * Use all information from the
 * preprocess step to allocate and
 * import mesh information
-------------------------------------*/
bool SR_SceneFileLoader::import_mesh_data(const aiScene* const pScene) noexcept
{
    LS_LOG_MSG("\tImporting vertices and indices of individual meshes from a file.");

    std::vector<SR_VaoGroup>& tempVboMarks = mPreloader.mVaoGroups;
    SR_SceneGraph&           sceneData    = mPreloader.mSceneData;
    SR_Context&              renderData   = sceneData.mContext;
    std::vector<SR_Mesh>&    meshes       = sceneData.mMeshes;
    std::vector<SR_BoundingBox>& bounds   = sceneData.mMeshBounds;
    SR_VertexBuffer&         vbo          = renderData.vbo(renderData.vbos().size()-1);
    SR_IndexBuffer&          ibo          = renderData.ibo(renderData.ibos().size()-1);
    size_t                   baseIndex    = 0;
    char* const              pVbo         = reinterpret_cast<char*>(vbo.data());
    char*                    pIbo         = reinterpret_cast<char*>(ibo.data());

    // vertex data in ASSIMP is not interleaved. It has to be converted into
    // the internally used vertex format which is recommended for use on mobile
    // devices.
    for (unsigned meshId = 0; meshId < pScene->mNumMeshes; ++meshId)
    {
        const aiMesh* const     pMesh       = pScene->mMeshes[meshId];
        const SR_CommonVertType vertType    = sr_convert_assimp_verts(pMesh);
        const size_t            meshGroupId = get_mesh_group_marker(vertType, mPreloader.mVaoGroups);
        SR_VaoGroup&            meshGroup   = tempVboMarks[meshGroupId];
        size_t                  numIndices  = 0;

        LS_DEBUG_ASSERT(meshGroup.vertType == vertType);

        SR_Mesh& mesh   = meshes[meshId];
        SR_BoundingBox& box = bounds[meshId];
        mesh.materialId = (uint32_t)pMesh->mMaterialIndex;
        mesh.vaoId      = meshGroupId;

        // get the offset to the current mesh so it can be sent to a VBO
        const size_t meshOffset = meshGroup.vboOffset + meshGroup.meshOffset;
        sr_upload_mesh_vertices(pMesh, meshGroup.baseVert, pVbo + meshOffset, meshGroup.vertType, mPreloader.mBones);

        // increment the mesh offset for the next mesh
        meshGroup.meshOffset += sr_vertex_stride(meshGroup.vertType) * pMesh->mNumVertices;
        pIbo = upload_mesh_indices(pMesh, pIbo, baseIndex, meshGroup.baseVert, mesh, numIndices);
        meshGroup.baseVert += pMesh->mNumVertices;
        baseIndex += numIndices;

        sr_update_mesh_bounds(pMesh, box);
    }

    LS_LOG_MSG("\t\tDone.");

    return true;
}



/*-------------------------------------
 * Use all information from the preprocess step to allocate and import bone
 * information.
-------------------------------------*/
bool SR_SceneFileLoader::import_bone_data(const aiMesh* const pMesh, unsigned baseVert) noexcept
{
    LS_LOG_MSG("\tImporting bones from a file.");
    std::vector<std::string>&                    nodeNames = mPreloader.mSceneData.mNodeNames;
    std::unordered_map<uint32_t, SR_BoneData>&   boneData  = mPreloader.mBones;

    for (unsigned i = 0; i < pMesh->mNumBones; ++i)
    {
        const aiBone*     pBone      = pMesh->mBones[i];
        const std::string boneName   = {pBone->mName.C_Str()};
        const uint32_t    numWeights = pBone->mNumWeights;
        size_t            boneId     = 0;

        std::vector<std::string>::iterator&& nameIter = std::find(nodeNames.begin(), nodeNames.end(), boneName);
        if (nameIter != nodeNames.end())
        {
            boneId = std::distance(nodeNames.begin(), nameIter);
        }

        // gather all weights from this bone
        for (uint32_t j = 0; j < numWeights; ++j)
        {
            const uint32_t vertId = (uint32_t)(baseVert+pBone->mWeights[j].mVertexId);
            const float    weight = pBone->mWeights[j].mWeight;

            // Apply up to SR_BONE_MAX_WEIGHTS weights for a single vertex
            std::unordered_map<uint32_t, SR_BoneData>::iterator&& iter = boneData.find(vertId);

            // If we have already accounted for a vertex, apply this weight to
            // the first non-zero value available for that vertex
            if (iter != boneData.end())
            {
                for (uint32_t k = 1; k < SR_BONE_MAX_WEIGHTS; ++k)
                {
                    if (iter->second.ids[k] == 0)
                    {
                        iter->second.ids[k]     = (int32_t)boneId;
                        iter->second.weights[k] = weight;
                        break;
                    }
                }
            }
            else
            {
                // The vertex we're checking does not have any weights
                // currently associated with it, add one.
                SR_BoneData newBone;
                newBone.ids        = math::vec4_t<int32_t>{0, 0, 0, 0};
                newBone.weights    = math::vec4_t<float>{0.f, 0.f, 0.f, 0.f};
                newBone.ids[0]     = (int32_t)boneId;
                newBone.weights[0] = weight;
                boneData[vertId]   = newBone;
            }
        }
    }

    return true;
}



/*-------------------------------------
    Read all face data (triangles)
-------------------------------------*/
char* SR_SceneFileLoader::upload_mesh_indices(
    const aiMesh* const pMesh,
    char* pIbo,
    const size_t baseIndex,
    const size_t baseVertex,
    SR_Mesh& outMesh,
    size_t& outNumIndices
) noexcept
{
    const SR_SceneFileMeta& sceneInfo = mPreloader.mSceneInfo;
    const ptrdiff_t numBytesPerIndex = sr_index_byte_size(sceneInfo.indexType);

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
    outMesh.mode = sr_convert_assimp_draw_mode(pMesh);
    outMesh.elementBegin = baseIndex;
    outMesh.elementEnd = baseIndex + outNumIndices;

    return pIbo;
}



/*-------------------------------------
 * Retrieve a single VBO Marker
-------------------------------------*/
size_t SR_SceneFileLoader::get_mesh_group_marker(
    const SR_CommonVertType vertType,
    const std::vector<SR_VaoGroup>& markers
) const noexcept
{
    for (size_t i = 0; i < markers.size(); ++i)
    {
        if (vertType == markers[i].vertType)
        {
            return i;
        }
    }

    LS_DEBUG_ASSERT(false);

    return (size_t)-1;
}



/*-------------------------------------
    Read and import all nodes in a scene
-------------------------------------*/
void SR_SceneFileLoader::read_node_hierarchy(
    const aiScene* const pScene,
    const aiNode* const pInNode,
    const size_t parentId,
    const math::mat4& invGlobalTransform
) noexcept
{
    // use the size of the node list as an index which should be returned to
    // the parent node's child indices.
    SR_SceneGraph&               sceneData      = mPreloader.mSceneData;
    std::vector<SR_SceneNode>&   nodeList       = sceneData.mNodes;
    std::vector<std::string>&    nodeNames      = sceneData.mNodeNames;
    std::vector<math::mat4>&     baseTransforms = sceneData.mBaseTransforms;
    std::vector<SR_Transform>&   currTransforms = sceneData.mCurrentTransforms;
    std::vector<math::mat4>&     modelMatrices  = sceneData.mModelMatrices;

    //LS_LOG_MSG("\tImporting Scene Node ", nodeList.size(), ": ", pInNode->mName.C_Str());

    nodeList.emplace_back(SR_SceneNode());
    SR_SceneNode& currentNode = nodeList.back();

    // initialize
    sr_reset(currentNode);
    currentNode.nodeId = nodeList.size() - 1;

    // import the node name
    const std::string nodeName{pInNode->mName.C_Str()};
    nodeNames.push_back(nodeName);

    // import the node transformation
    // This is also needed for camera nodes to be imported properly
    {
        SR_Transform baseTrans{};
        aiVector3D inPos, inScale;
        aiQuaternion inRotation;

        pInNode->mTransformation.Decompose(inScale, inRotation, inPos);

        baseTrans.set_position(sr_convert_assimp_vector(inPos));
        baseTrans.set_scale(sr_convert_assimp_vector(inScale));
        baseTrans.set_orientation(sr_convert_assimp_quaternion(inRotation));

        // Store the transform as specified by the scene graph hierarchy.
        currTransforms.push_back(baseTrans);

        // Store the original, unmodified, unparented transform
        baseTrans.apply_transform();
        baseTransforms.push_back(baseTrans.get_transform());
    }

    const size_t camIndex = sr_is_node_type<aiCamera>(pInNode, pScene->mCameras, pScene->mNumCameras);

    if (camIndex != SCENE_NODE_ROOT_ID)
    {
        currentNode.type = NODE_TYPE_CAMERA;
        import_camera_node(pScene, camIndex, currentNode);
    }
    else if (mPreloader.mBoneOffsets.find(nodeName) != mPreloader.mBoneOffsets.end())
    {
        currentNode.type = NODE_TYPE_BONE;
        currentNode.dataId = mPreloader.mSceneData.mBoneOffsets.size();
        mPreloader.mSceneData.mInvBoneTransforms.push_back(invGlobalTransform);
        mPreloader.mSceneData.mBoneOffsets.push_back(mPreloader.mBoneOffsets[nodeName]);
    }
    else if (sr_is_node_type<aiMesh>(pInNode, pScene->mMeshes, pScene->mNumMeshes))
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
        SR_Transform& nodeTransform = currTransforms.back();
        nodeTransform.mParentId = parentId;

        // Only apply parented transforms here. Applying non-parented
        // transforms will cause root objects to swap X & Z axes.
        if (parentId != SCENE_NODE_ROOT_ID)
        {
            nodeTransform.apply_pre_transform(currTransforms[parentId].get_transform());
        }

        modelMatrices.push_back(currTransforms.back().get_transform());
    }

    // CYA
    LS_DEBUG_ASSERT(nodeList.size() == nodeNames.size());
    LS_DEBUG_ASSERT(nodeList.size() == baseTransforms.size());
    LS_DEBUG_ASSERT(nodeList.size() == currTransforms.size());
    LS_DEBUG_ASSERT(nodeList.size() == modelMatrices.size());

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
void SR_SceneFileLoader::import_mesh_node(const aiNode* const pNode, SR_SceneNode& outNode) noexcept
{
    SR_SceneGraph& graph = mPreloader.mSceneData;
    std::vector<size_t>& nodeMeshCounts = graph.mNumNodeMeshes;
    std::vector<utils::Pointer<size_t[]>>& meshList = graph.mNodeMeshes;

    // The check for how many meshes a scene node has must have already been
    // performed.
    const unsigned numMeshes = pNode->mNumMeshes;
    LS_DEBUG_ASSERT(numMeshes > 0);

    utils::Pointer<size_t[]> meshIds{new size_t[numMeshes]};
    LS_DEBUG_ASSERT(meshIds.get());

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
void SR_SceneFileLoader::import_camera_node(
    const aiScene* const pScene,
    const size_t camIndex,
    SR_SceneNode& outNode
) noexcept
{
    SR_SceneGraph& graph = mPreloader.mSceneData;

    // Very important for the scene graph to keep track of what camera node
    // owns which camera
    outNode.dataId = camIndex;

    const aiCamera* const pInCam = pScene->mCameras[camIndex];
    std::vector<SR_Camera>& camList = graph.mCameras;

    camList.emplace_back(SR_Camera());
    SR_Camera& outCam = graph.mCameras.back();

    outCam.set_fov(pInCam->mHorizontalFOV);
    outCam.set_aspect_ratio(pInCam->mAspect, 1.f);
    outCam.set_near_plane(pInCam->mClipPlaneNear);
    outCam.set_far_plane(pInCam->mClipPlaneFar);
    outCam.set_projection_type(SR_ProjectionType::SR_PROJECTION_LOGARITHMIC_PERSPECTIVE);
    outCam.update();

    // A Transform object must have been added by the parent function
    SR_Transform& camTrans = graph.mCurrentTransforms.back();
    camTrans.set_type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_FPS); // ASSIMP never specified a default

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

    const math::vec3&& finalPos = sr_convert_assimp_vector(inPos);
    const math::vec3&& finalDir = sr_convert_assimp_vector(inDir);
    const math::vec3&& finalUp = sr_convert_assimp_vector(inUp);
    camTrans.look_at(finalPos, finalDir, finalUp);

    const math::vec3& camPos = camTrans.get_position();
    const math::vec3&& camUp = math::vec3{0.f, 1.f, 0.f};//nodeCam.get_up_direction();

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
bool SR_SceneFileLoader::import_animations(const aiScene* const pScene) noexcept
{
    bool ret = true;
    SR_SceneGraph&                  graph                      = mPreloader.mSceneData;
    const aiAnimation* const* const pAnimations                = pScene->mAnimations;
    std::vector<SR_Animation>&      animations                 = graph.mAnimations;
    const unsigned                  totalAnimations            = pScene->mNumAnimations;
    std::vector<std::vector<SR_AnimationChannel>>& allChannels = graph.mNodeAnims;

    for (unsigned i = 0; i < totalAnimations; ++i)
    {
        const aiAnimation* const pInAnim = pAnimations[i];

        animations.emplace_back(SR_Animation{});
        SR_Animation& anim = animations.back();

        // The animation as a whole needs to have its properties imported from
        // ASSIMP.
        SR_Animation&& tempAnim = sr_setup_imported_animation(
            pInAnim->mName.C_Str(),
            pInAnim->mDuration,
            pInAnim->mTicksPerSecond,
            pInAnim->mNumChannels
        );

        anim = std::move(tempAnim);

        for (unsigned c = 0; c < pInAnim->mNumChannels; ++c)
        {
            SR_AnimationChannel track;
            const aiNodeAnim* const pInTrack = pInAnim->mChannels[c];
            const unsigned nodeId = import_animation_track(pInTrack, track, anim.get_duration());

            if (nodeId == UINT_MAX)
            {
                // failing to load an Animation track is not an error.
                ret = false;
                continue;
            }

            // Grab the scene node which contains the data ID to map with a
            // transformation and animation channel
            SR_SceneNode& node = graph.mNodes[nodeId];

            // If a list of animation tracks doesn't exist for the current node
            // then be sure to add it.
            if (node.animListId == SCENE_NODE_ROOT_ID)
            {
                node.animListId = allChannels.size();
                allChannels.emplace_back(std::vector<SR_AnimationChannel>{});
            }

            std::vector<SR_AnimationChannel>& nodeChannels = allChannels[node.animListId];
            nodeChannels.emplace_back(std::move(track));

            // Add the node's imported track to the current animation
            anim.add_anim_channel(node, nodeChannels.size()-1);
        }

        std::cout
            << "\tLoaded Animation " << i+1 << '/' << totalAnimations
            << "\n\t\tName:      " << anim.get_anim_name()
            << "\n\t\tDuration:  " << anim.get_duration()
            << "\n\t\tTicks/Sec: " << anim.get_ticks_per_sec()
            << "\n\t\tChannels:  " << anim.get_num_anim_channels()
            << std::endl;
    }

    if (totalAnimations > 0)
    {
        SR_Animation& initialState = animations[0];
        initialState.init(graph);
    }

    std::cout << "\tSuccessfully loaded " << animations.size() << " animations." << std::endl;

    return ret;
}



/*-------------------------------------
 * Import a single Animation track
-------------------------------------*/
unsigned SR_SceneFileLoader::import_animation_track(
    const aiNodeAnim* const pInAnim,
    SR_AnimationChannel& outAnim,
    const SR_AnimPrecision animDuration
) noexcept {
    const unsigned posFrames                    = pInAnim->mNumPositionKeys;
    const unsigned sclFrames                    = pInAnim->mNumScalingKeys;
    const unsigned rotFrames                    = pInAnim->mNumRotationKeys;
    const std::vector<std::string>& nodeNames   = mPreloader.mSceneData.mNodeNames;
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

    if (!outAnim.set_num_frames(posFrames, sclFrames, rotFrames))
    {
        std::cout << "Unable to import the Animation \"" << pInAnim->mNodeName.C_Str() << "\"." << std::endl;
        return UINT_MAX;
    }

    SR_AnimationKeyListVec3& outPosFrames = outAnim.mPosFrames;
    SR_AnimationKeyListVec3& outSclFrames = outAnim.mScaleFrames;
    SR_AnimationKeyListQuat& outRotFrames = outAnim.mOrientFrames;

    // Convert all animation times into percentages for the internal animation
    // system.
    // Positions
    for (unsigned p=0; p < outPosFrames.size(); ++p)
    {
        const aiVectorKey& posKey = pInAnim->mPositionKeys[p];
        outPosFrames.set_frame(p, posKey.mTime/animDuration, sr_convert_assimp_vector(posKey.mValue));
    }

    // Scalings
    for (unsigned s=0; s < outSclFrames.size(); ++s)
    {
        const aiVectorKey& sclKey = pInAnim->mScalingKeys[s];
        outSclFrames.set_frame(s, sclKey.mTime/animDuration, sr_convert_assimp_vector(sclKey.mValue));
    }

    // Rotations
    for (unsigned r=0; r < outRotFrames.size(); ++r)
    {
        const aiQuatKey& rotKey = pInAnim->mRotationKeys[r];
        outRotFrames.set_frame(r, rotKey.mTime/animDuration, sr_convert_assimp_quaternion(rotKey.mValue));
    }

    // Convert ASSIMP animation flags into internal ones.
    // ASSIMP allows for animation behavior to change between the starting and
    // ending animation frames. Those are currently unsupported, all imported
    // animations share the same flags from start to finish.
    unsigned animFlags = SR_ANIM_FLAG_NONE;
    const auto flagFill = [&](const aiAnimBehaviour inFlag, const unsigned outFlag)->void
    {
        if (pInAnim->mPreState & inFlag || pInAnim->mPostState & inFlag)
        {
            animFlags |= outFlag;
        }
    };

    flagFill(aiAnimBehaviour_CONSTANT,  SR_ANIM_FLAG_IMMEDIATE);
    flagFill(aiAnimBehaviour_DEFAULT,   SR_ANIM_FLAG_INTERPOLATE);
    flagFill(aiAnimBehaviour_LINEAR,    SR_ANIM_FLAG_INTERPOLATE);
    flagFill(aiAnimBehaviour_REPEAT,    SR_ANIM_FLAG_REPEAT);

    outAnim.mAnimMode = (SR_AnimationFlag)animFlags;

    std::cout
        << "\tSuccessfully imported the Animation \"" << pInAnim->mNodeName.C_Str() << '\"'
        << "\n\t\tPosition Keys: " << outPosFrames.size() << " @ " << outPosFrames.get_duration()
        << "\n\t\tScaling Keys:  " << outSclFrames.size() << " @ " << outSclFrames.get_duration()
        << "\n\t\tRotation Keys: " << outRotFrames.size() << " @ " << outRotFrames.get_duration()
        << std::endl;

    return nodeId;
}
