
#ifndef SR_SCENE_GRAPH_LOADER_HPP
#define SR_SCENE_GRAPH_LOADER_HPP

#include <string>
#include <unordered_map>
#include <utility> // std::pair
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "lightsky/math/mat4.h"

#include "soft_render/SR_AnimationProperty.hpp" // SR_AnimPrecision
#include "soft_render/SR_Geometry.hpp"
#include "soft_render/SR_SceneGraph.hpp"



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
struct SR_AnimationChannel;
enum   SR_CommonVertType : unsigned; // VertexUtils.h
class  SR_ImgFile;
struct SR_Material;
struct SR_Mesh;
struct SR_SceneNode;



/**----------------------------------------------------------------------------
 * A VboGroup is an intermediate structure to help determine which group of
 * vertices in an Assimp mesh belong to which section of a VBO's memory buffer.
-----------------------------------------------------------------------------*/
struct SR_VaoGroup
{
    SR_CommonVertType vertType;
    unsigned numVboBytes;
    unsigned vboOffset;
    unsigned meshOffset;
    unsigned baseVert;

    ~SR_VaoGroup() noexcept;

    SR_VaoGroup() noexcept;

    SR_VaoGroup(const SR_VaoGroup& v) noexcept;

    SR_VaoGroup(SR_VaoGroup&& v) noexcept;

    SR_VaoGroup& operator=(const SR_VaoGroup& v) noexcept;

    SR_VaoGroup& operator=(SR_VaoGroup&& v) noexcept;
};


/**----------------------------------------------------------------------------
 * Condensed meta-information about a scene file.
-----------------------------------------------------------------------------*/
struct SR_SceneFileMeta
{
    uint32_t totalVboBytes = 0;
    uint32_t totalVertices = 0;
    uint32_t totalIboBytes = 0;
    uint32_t totalIndices = 0;
    SR_DataType indexType = SR_DataType::VERTEX_DATA_INVALID;
};



/*-----------------------------------------------------------------------------
 * @brief SR_SceneLoadOpts Structure
 *
 * This structure can be passed into the scene loader to adjust the output
 * data of a scene being loaded.
-----------------------------------------------------------------------------*/
struct SR_SceneLoadOpts
{
    // UVs are usually stored in two 32-bit floats. Use this flag to compress
    // UV data into two 16-bit floats.
    bool packUvs;

    // Vertex normals will be compressed from a math::vec3_t<float> type into
    // an int32_t type. They can be unpacked using either
    // "sr_unpack_vertex_vec3()" or "sr_unpack_vertex_vec4()." This option
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
 *
 * @return A SR_SceneLoadOpts structure, containing standard data-modification
 * options which will affect a scene being loaded.
 */
SR_SceneLoadOpts sr_default_scene_load_opts() noexcept;



/**----------------------------------------------------------------------------
 * Preloading structure which allows a file to load in a separate thread.
-----------------------------------------------------------------------------*/
class SR_SceneFilePreload
{

    friend class SR_SceneFileLoader;

  private:
    std::string mFilepath;

    SR_SceneLoadOpts mLoadOpts;

    ls::utils::Pointer<Assimp::Importer> mImporter;

    SR_SceneFileMeta mSceneInfo;

    SR_SceneGraph mSceneData;

    std::string mBaseFileDir;

    std::vector<SR_VaoGroup> mVaoGroups{};

    std::unordered_map<std::string, unsigned> mTexPaths;

    std::unordered_map<uint32_t, SR_BoneData> mBones;

    std::unordered_map<std::string, ls::math::mat4_t<float>> mBoneOffsets;

    const aiScene* preload_mesh_data() noexcept;

    bool allocate_cpu_data(const aiScene* const pScene) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Unloads all data contain within *this.
     */
    ~SR_SceneFilePreload() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all members contained within *this.
     */
    SR_SceneFilePreload() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to reduce excess memory allocations.
     */
    SR_SceneFilePreload(const SR_SceneFilePreload&) noexcept = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param r
     * An r-value reference to a temporary SceneFilePreLoader object.
     */
    SR_SceneFilePreload(SR_SceneFilePreload&& s) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to reduce excess memory allocations.
     */
    SR_SceneFilePreload& operator=(const SR_SceneFilePreload& s) noexcept = delete;

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
    SR_SceneFilePreload& operator=(SR_SceneFilePreload&& s) noexcept;

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
    bool load(const std::string& filename, SR_SceneLoadOpts opts = sr_default_scene_load_opts()) noexcept;

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
class SR_SceneFileLoader
{

    // Private Variables
  private:
    SR_SceneFilePreload mPreloader;

    // Private functions
  private:
    bool load_scene(const aiScene* const pScene, const SR_SceneLoadOpts& opts) noexcept;

    bool allocate_gpu_data() noexcept;

    int import_materials(const aiScene* const pScene) noexcept;

    void import_texture_path(
        const aiMaterial* const pMaterial,
        const int slotType,
        SR_Material& outMaterial,
        SR_ImgFile& imgLoader,
        std::unordered_map<std::string, const SR_Texture*>& loadedTextures
    ) noexcept;

    SR_Texture* load_texture_at_path(const std::string& path, SR_ImgFile& imgLoader, int wrapMode) noexcept;

    bool import_mesh_data(const aiScene* const pScene, const SR_SceneLoadOpts& opts) noexcept;

    bool import_bone_data(const aiMesh* const pMesh, unsigned baseVertex, const SR_SceneLoadOpts& opts) noexcept;

    char* upload_mesh_indices(const aiMesh* const pMesh, char* pIbo, const size_t baseIndex, const size_t baseVertex, SR_Mesh& outMesh, size_t& outNumIndices) noexcept;

    size_t get_mesh_group_marker(const SR_CommonVertType vertType, const std::vector<SR_VaoGroup>& markers) const noexcept;

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
    void import_mesh_node(const aiNode* const pNode, SR_SceneNode& outNode) noexcept;

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
    void import_camera_node(const aiScene* const pScene, const size_t camIndex, SR_SceneNode& outNode) noexcept;

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
        SR_AnimationChannel& outAnim,
        const SR_AnimPrecision animDuration
    ) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Unloads all data contain within *this.
     */
    ~SR_SceneFileLoader() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all members contained within *this.
     */
    SR_SceneFileLoader() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Deleted to reduce excess memory allocations.
     */
    SR_SceneFileLoader(const SR_SceneFileLoader& s) noexcept = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed during the operation.
     *
     * @param r
     * An r-value reference to a temporary SceneFileLoader object.
     */
    SR_SceneFileLoader(SR_SceneFileLoader&& s) noexcept;

    /**
     * @brief Copy Operator
     *
     * Deleted to reduce excess memory allocations.
     */
    SR_SceneFileLoader& operator=(const SR_SceneFileLoader& s) noexcept = delete;

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
    SR_SceneFileLoader& operator=(SR_SceneFileLoader&& s) noexcept;

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
    bool load(const std::string& filename, const SR_SceneLoadOpts& opts = sr_default_scene_load_opts()) noexcept;

    /**
     * @brief Import in-memory mesh data, preloaded from a file.
     *
     * @param preload
     * An r-value reference to a scene preloader which is ready to be sent
     * to the GPU.
     *
     * @return true if the data was successfully loaded. False if not.
     */
    bool load(SR_SceneFilePreload&& preload) noexcept;

    /**
     * @brief get_loaded_data() allows the loaded scene graph to be
     * retrieved by reference.
     *
     * @return A constant reference to the loaded scene grpah object which
     * can be used for validation, rendering, or something else.
     */
    const SR_SceneGraph& data() const noexcept;

    /**
     * @brief get_loaded_data() allows the loaded scene graph to be
     * retrieved by reference.
     *
     * @return A reference to the loaded scene grpah object which can be
     * used for validation, rendering, or something else.
     */
    SR_SceneGraph& data() noexcept;
};


/*-------------------------------------
 * Retrieve the loaded scene data (const)
-------------------------------------*/
inline const SR_SceneGraph& SR_SceneFileLoader::data() const noexcept
{
    return mPreloader.mSceneData;
}


/*-------------------------------------
 * Retrieve the loaded scene data
-------------------------------------*/
inline SR_SceneGraph& SR_SceneFileLoader::data() noexcept
{
    return mPreloader.mSceneData;
}



#endif /* SR_SCENE_GRAPH_LOADER_HPP */
