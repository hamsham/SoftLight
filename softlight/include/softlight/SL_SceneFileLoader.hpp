
#ifndef SL_SCENE_GRAPH_LOADER_HPP
#define SL_SCENE_GRAPH_LOADER_HPP

#include <string>
#include <unordered_map>
#include <utility> // std::pair
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "lightsky/math/mat4.h"

#include "softlight/SL_AnimationProperty.hpp" // SL_AnimPrecision
#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_SceneGraph.hpp"



struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiNodeAnim;
struct aiScene;


namespace Assimp
{
    class Importer;
} // end Assimp namespace



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
struct SL_AnimationChannel;
enum   SL_CommonVertType : unsigned; // VertexUtils.h
class  SL_ImgFile;
struct SL_Material;
struct SL_Mesh;
struct SL_SceneNode;



/**----------------------------------------------------------------------------
 * A VboGroup is an intermediate structure to help determine which group of
 * vertices in an Assimp mesh belong to which section of a VBO's memory buffer.
-----------------------------------------------------------------------------*/
struct SL_VaoGroup
{
    SL_CommonVertType vertType;
    unsigned numVboBytes;
    unsigned vboOffset;
    unsigned meshOffset;
    unsigned baseVert;

    ~SL_VaoGroup() noexcept;

    SL_VaoGroup() noexcept;

    SL_VaoGroup(const SL_VaoGroup& v) noexcept;

    SL_VaoGroup(SL_VaoGroup&& v) noexcept;

    SL_VaoGroup& operator=(const SL_VaoGroup& v) noexcept;

    SL_VaoGroup& operator=(SL_VaoGroup&& v) noexcept;
};


/**----------------------------------------------------------------------------
 * Condensed meta-information about a scene file.
-----------------------------------------------------------------------------*/
struct SL_SceneFileMeta
{
    uint32_t totalVboBytes = 0;
    uint32_t totalVertices = 0;
    uint32_t totalIboBytes = 0;
    uint32_t totalIndices = 0;
    SL_DataType indexType = SL_DataType::VERTEX_DATA_INVALID;
};



/*-----------------------------------------------------------------------------
 * @brief SL_SceneLoadOpts Structure
 *
 * This structure can be passed into the scene loader to adjust the output
 * data of a scene being loaded.
-----------------------------------------------------------------------------*/
struct SL_SceneLoadOpts
{
    // UVs are usually stored in two 32-bit floats. Use this flag to compress
    // UV data into two 16-bit floats.
    bool packUvs;

    // Vertex normals will be compressed from a math::vec3_t<float> type into
    // an int32_t type. They can be unpacked using either
    // "sl_unpack_vertex_vec3()" or "sl_unpack_vertex_vec4()." This option
    // does nothing if no normals exist or are generated.
    bool packNormals;

    // Use 16-bit bone IDs (4 per vertex) rather than 32-bit bone IDs.
    bool packBoneIds;

    // Use 16-bit floats for bone weights rather than 32-bit floats.
    bool packBoneWeights;

    // Use this to generate normals for meshes which don't have them (can be
    // superseded by genSmoothNormals).
    bool genFlatNormals;

    // Use this to generate normals for meshes which don't have them
    // (supersedes genFlatNormals).
    bool genSmoothNormals;

    // Implies "genSmoothNormals." This will generate tangents and bitangents
    // for normal mapping.
    bool genTangents;

    // Use texel swizzling on loaded textures to increase the chance a
    // transformed UV mapping is in the CPU cache (will increase CPU cycles
    // spent calculating UVs while potentially decreasing memory bandwidth).
    bool swizzleTexels;
};



/**
 * @brief Retrieve the default scene loading options.
 *
 * @note The following options are set by default:
 *     packUvs:          FALSE
 *     packNormals:      FALSE
 *     packBoneIds:      FALSE
 *     packBoneWeights:  FALSE
 *     genFlatNormals:   FALSE
 *     genSmoothNormals: TRUE
 *     genTangents:      FALSE
 *     swizzleTexels:    FALSE
 *
 * @return A SL_SceneLoadOpts structure, containing standard data-modification
 * options which will affect a scene being loaded.
 */
SL_SceneLoadOpts sl_default_scene_load_opts() noexcept;



/**----------------------------------------------------------------------------
 * Preloading structure which allows a file to load in a separate thread.
-----------------------------------------------------------------------------*/
class SL_SceneFilePreload
{

    friend class SL_SceneFileLoader;

  private:
    std::string mFilepath;

    SL_SceneLoadOpts mLoadOpts;

    ls::utils::Pointer<Assimp::Importer> mImporter;

    SL_SceneFileMeta mSceneInfo;

    SL_SceneGraph mSceneData;

    std::string mBaseFileDir;

    std::vector<SL_VaoGroup> mVaoGroups;

    std::unordered_map<std::string, unsigned> mTexPaths;

    std::unordered_map<uint32_t, SL_BoneData> mBones;

    std::unordered_map<std::string, ls::math::mat4_t<float>> mBoneOffsets;

    const aiScene* preload_mesh_data() noexcept;

    bool allocate_cpu_data(const aiScene* const pScene) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Unloads all data contain within *this.
     */
    ~SL_SceneFilePreload() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all members contained within *this.
     */
    SL_SceneFilePreload() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to reduce excess memory allocations.
     */
    SL_SceneFilePreload(const SL_SceneFilePreload&) noexcept = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param r
     * An r-value reference to a temporary SceneFilePreLoader object.
     */
    SL_SceneFilePreload(SL_SceneFilePreload&& s) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to reduce excess memory allocations.
     */
    SL_SceneFilePreload& operator=(const SL_SceneFilePreload& s) noexcept = delete;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param s
     * An r-value reference to a temporary SceneFilePreLoader object.
     *
     * @return a reference to *this.
     */
    SL_SceneFilePreload& operator=(SL_SceneFilePreload&& s) noexcept;

    /**
     * @brief Unload/free all memory used by *this.
     */
    void unload() noexcept;

    /**
     * @brief Load a 3D mesh file
     *
     * @param filename
     * A string object containing the relative path name to a file that
     * should be loadable into memory.
     *
     * @return true if the file was successfully loaded into memory. False
     * if not.
     */
    bool load(const std::string& filename, SL_SceneLoadOpts opts = sl_default_scene_load_opts()) noexcept;

    /**
     * @brief Verify that data loaded successfully.
     *
     * @return TRUE if *this object contains data on he CPU which can be
     * passed to a SceneFileLoader object for GPU loading, FALSE if not.
     */
    bool is_loaded() const noexcept;
};


/**----------------------------------------------------------------------------
 * The scene file loader  can be used to load a 3D scene from a file.
-----------------------------------------------------------------------------*/
class SL_SceneFileLoader
{

    // Private Variables
  private:
    SL_SceneFilePreload mPreloader;

    std::unordered_map<std::string, const SL_Texture*> mLoadedTextures;

    // Private functions
  private:
    bool load_scene(const aiScene* const pScene, SL_SceneLoadOpts opts) noexcept;

    bool allocate_gpu_data() noexcept;

    int import_materials(const aiScene* const pScene) noexcept;

    void import_texture_path(
        const aiMaterial* const pMaterial,
        const int slotType,
        const SL_Texture* pTextures[SL_MaterialProperty::SL_MATERIAL_MAX_TEXTURES],
        SL_ImgFile& imgLoader,
        std::unordered_map<std::string, const SL_Texture*>& loadedTextures
    ) noexcept;

    SL_Texture* load_texture_at_path(const std::string& path, SL_ImgFile& imgLoader) noexcept;

    bool import_mesh_data(const aiScene* const pScene, const SL_SceneLoadOpts& opts) noexcept;

    bool import_bone_data(const aiMesh* const pMesh, unsigned baseVertex, const SL_SceneLoadOpts& opts) noexcept;

    char* upload_mesh_indices(const aiMesh* const pMesh, char* pIbo, const size_t baseIndex, const size_t baseVertex, SL_Mesh& outMesh, size_t& outNumIndices) noexcept;

    size_t get_mesh_group_marker(const SL_CommonVertType vertType, const std::vector<SL_VaoGroup>& markers) const noexcept;

    /**
     * @brief Recursively reads and imports scene graph data from Assimp.
     *
     * @param pScene
     * A constant pointer to a constant aiScene object from ASSIMP.
     *
     * @param pNode
     * A pointer to a node in an Assimp scene graph.
     *
     * @param parentId
     * An index value of the parent sceneNode contained within an internal
     * array of nodes.
     *
     * @param invGlobalTransform
     * The inverse of the global bone transformation.
     *
     * @return The index of the the last child node recursively placed into
     * the scene node list.
     */
    void read_node_hierarchy(
        const aiScene* const     pScene,
        const aiNode* const      pNode,
        const size_t             parentId,
        ls::math::mat4_t<float>& invGlobalTransform
    ) noexcept;

    /**
     * @brief Import a mesh node if an ASSIMP node contains meshes.
     *
     * @param pNode
     * A constant pointer to an assimp mesh node.
     *
     * @param outNode
     * A reference to the SceneNode object which will have the remainder of
     * its data initialized through this method.
     */
    void import_mesh_node(const aiNode* const pNode, SL_SceneNode& outNode) noexcept;

    /**
     * @brief Import/convert a Camera node from ASSIMP.
     *
     * @param pScene
     * A constant pointer to an assimp scene from which camera and node
     * information will be loaded.
     *
     * @param camIndex
     * An index value of the camera object in ASSIMP which the imported
     * SceneNode should draw its data from.
     *
     * @param outNode
     * A reference to the SceneNode object which will have the remainder of
     * its data initialized through this method.
     */
    void import_camera_node(const aiScene* const pScene, const size_t camIndex, SL_SceneNode& outNode) noexcept;

    /**
     * @brief Import all animations contained within a 3D scene file,
     * provided by assimp.
     *
     * @param pScene
     * A constant pointer to a constant ASSIMP scene structure.
     *
     * @return TRUE if all animations were successfully imported, FALSE if
     * not.
     */
    bool import_animations(const aiScene* const pScene) noexcept;

    /**
     * @brief Import a single Animation track from ASSIMP.
     *
     * @param pInAnim
     * A constant pointer to a constant Animation track.
     *
     * @param outAnim
     * A reference to the animationReel object which is to contain batched
     * Animation data for a single track.
     *
     * @param animDuration
     * Contains the total duration of the animation, in ticks.
     *
     * @return The ID of a node who's track was successfully imported,
     * UINT_MAX if not.
     */
    unsigned import_animation_track(
        const aiNodeAnim* const pInAnim,
        SL_AnimationChannel& outAnim,
        const SL_AnimPrecision animDuration
    ) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Unloads all data contain within *this.
     */
    ~SL_SceneFileLoader() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all members contained within *this.
     */
    SL_SceneFileLoader() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to reduce excess memory allocations.
     */
    SL_SceneFileLoader(const SL_SceneFileLoader& s) noexcept = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param r
     * An r-value reference to a temporary SceneFileLoader object.
     */
    SL_SceneFileLoader(SL_SceneFileLoader&& s) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to reduce excess memory allocations.
     */
    SL_SceneFileLoader& operator=(const SL_SceneFileLoader& s) noexcept = delete;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param s
     * An r-value reference to a temporary SceneFileLoader object.
     *
     * @return a reference to *this.
     */
    SL_SceneFileLoader& operator=(SL_SceneFileLoader&& s) noexcept;

    /**
     * @brief Unload/free all memory used by *this.
     */
    void unload() noexcept;

    /**
     * @brief Load a 3D mesh file
     *
     * @param filename
     * A string object containing the relative path name to a file that
     * should be loadable into memory.
     *
     * @return true if the file was successfully loaded. False if not.
     */
    bool load(const std::string& filename, SL_SceneLoadOpts opts = sl_default_scene_load_opts()) noexcept;

    /**
     * @brief Import in-memory mesh data, preloaded from a file.
     *
     * @param preload
     * An r-value reference to a scene preloader which is ready to be sent
     * to the GPU.
     *
     * @return true if the data was successfully loaded. False if not.
     */
    bool load(SL_SceneFilePreload&& preload) noexcept;

    /**
     * @brief get_loaded_data() allows the loaded scene graph to be
     * retrieved by reference.
     *
     * @return A constant reference to the loaded scene grpah object which
     * can be used for validation, rendering, or something else.
     */
    const SL_SceneGraph& data() const noexcept;

    /**
     * @brief get_loaded_data() allows the loaded scene graph to be
     * retrieved by reference.
     *
     * @return A reference to the loaded scene grpah object which can be
     * used for validation, rendering, or something else.
     */
    SL_SceneGraph& data() noexcept;

    /**
     * @brief Retrieve the mapping of texture paths to SL_Texture objects.
     *
     * @note This mapping only contains path-to-texture mappings for textures
     * which were successfully loaded.
     *
     * @return A mapping of external texture file paths to their in-memory
     * SL_Texture objects.
     */
    const std::unordered_map<std::string, const SL_Texture*>& texture_path_mappings() const;

    /**
     * @brief Retrieve the types of VAOs loaded into memory.
     *
     * @return A std::vector of SL_VaoGroup types which represent the loaded
     * VAO types of various meshes loaded from a file.
     */
    const std::vector<SL_VaoGroup>& vao_types() const;
};



/*------------------- objects and their paths.------------------
 * Retrieve the loaded scene data (const)
-------------------------------------*/
inline const SL_SceneGraph& SL_SceneFileLoader::data() const noexcept
{
    return mPreloader.mSceneData;
}


/*-------------------------------------
 * Retrieve the loaded scene data
-------------------------------------*/
inline SL_SceneGraph& SL_SceneFileLoader::data() noexcept
{
    return mPreloader.mSceneData;
}



/*-------------------------------------
 * Retrieve loaded texture objects and their paths.
-------------------------------------*/
inline const std::unordered_map<std::string, const SL_Texture*>& SL_SceneFileLoader::texture_path_mappings() const
{
    return mLoadedTextures;
}



/*-------------------------------------
 * Retrieve loaded vao types
-------------------------------------*/
inline const std::vector<SL_VaoGroup>& SL_SceneFileLoader::vao_types() const
{
    return mPreloader.mVaoGroups;
}



#endif /* SL_SCENE_GRAPH_LOADER_HPP */
