
#ifndef SL_SCENE_FILE_UTILITY_HPP
#define SL_SCENE_FILE_UTILITY_HPP

#include <string>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_SceneNode.hpp"



/*-----------------------------------------------------------------------------
 * Private local functions, structures, and enumerations
-----------------------------------------------------------------------------*/
struct SL_SceneLoadOpts;
struct SL_VaoGroup;
enum SL_TexWrapMode : uint16_t;



/*-------------------------------------
 * Assimp Import Flags & Enumerations
 *
 * These meshes were hand-selected through much trial & error. Do not touch
 * unless you want a swarm of bees hidden in your breakfast cereal box.
-------------------------------------*/
enum : unsigned int
{
    SCENE_FILE_IMPORT_FLAGS = 0
        | aiProcess_SplitLargeMeshes
        | aiProcess_OptimizeMeshes
        | aiProcess_LimitBoneWeights
        | aiProcess_FindInstances
        | aiProcess_SortByPType
        | aiProcess_RemoveComponent
        | aiProcess_JoinIdenticalVertices
        | aiProcess_FindDegenerates
        | aiProcess_FixInfacingNormals
        | aiProcess_FindInvalidData
        | aiProcess_ValidateDataStructure
        | aiProcess_TransformUVCoords
        | aiProcess_GenUVCoords
        | aiProcess_RemoveRedundantMaterials
        //| aiProcess_GenSmoothNormals
        //| aiProcess_GenNormals
        | aiProcess_Triangulate // the renderer can only handle triangles
        | aiProcess_ImproveCacheLocality
        | 0
};



/*-----------------------------------------------------------------------------
 * Utility Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Convert an Assimp vector to an internal one
-------------------------------------*/
template<typename num_t>
ls::math::vec2_t<num_t> sl_convert_assimp_vector(const aiVector2t<num_t>& inVec) noexcept
{
    return ls::math::vec2_t<num_t>{inVec.x, inVec.y};
}


/*-------------------------------------
 * Convert an Assimp vector to an internal one
-------------------------------------*/
template<typename num_t>
ls::math::vec3_t<num_t> sl_convert_assimp_vector(const aiVector3t<num_t>& inVec) noexcept
{
    return ls::math::vec3_t<num_t>{inVec.x, inVec.y, inVec.z};
}


/*-------------------------------------
 * Convert an Assimp vector to an internal one
-------------------------------------*/
template<typename num_t>
ls::math::quat_t<num_t> sl_convert_assimp_quaternion(const aiQuaterniont<num_t>& inQuat) noexcept
{
    return ls::math::quat_t<num_t>{inQuat.x, inQuat.y, inQuat.z, inQuat.w};
}


/*-------------------------------------
 * Convert an Assimp color to an internal one
-------------------------------------*/
template<typename num_t>
SL_ColorRGBAf sl_convert_assimp_color(const aiColor4t<num_t>& inColor) noexcept
{
    return SL_ColorRGBAf{inColor.r, inColor.g, inColor.b, inColor.a};
}



/*-------------------------------------
 * Convert an Assimp matrix to an internal one
-------------------------------------*/
template<typename num_t>
ls::math::mat3_t<num_t> sl_convert_assimp_matrix(const aiMatrix3x3t<num_t>& inMat) noexcept
{
    return ls::math::mat3_t<num_t>{
        inMat.a1, inMat.b1, inMat.c1,
        inMat.a2, inMat.b2, inMat.c2,
        inMat.a3, inMat.b3, inMat.c3,
    };
}



/*-------------------------------------
 * Convert an Assimp matrix to an internal one
-------------------------------------*/
template<typename num_t>
ls::math::mat4_t<num_t> sl_convert_assimp_matrix(const aiMatrix4x4t<num_t>& inMat) noexcept
{
    return ls::math::mat4_t<num_t>{
        inMat.a1, inMat.b1, inMat.c1, inMat.d1,
        inMat.a2, inMat.b2, inMat.c2, inMat.d2,
        inMat.a3, inMat.b3, inMat.c3, inMat.d3,
        inMat.a4, inMat.b4, inMat.c4, inMat.d4
    };
}



/*-------------------------------------
 * Convert Assimp draw types to internal ones
-------------------------------------*/
SL_RenderMode sl_convert_assimp_draw_mode(const aiMesh* const pMesh) noexcept;



/*-------------------------------------
 * Convert Assimp vertex attributes into internal enumerations
-------------------------------------*/
SL_CommonVertType sl_convert_assimp_verts(const aiMesh* const pMesh, const SL_SceneLoadOpts& opts) noexcept;



/*-------------------------------------
 * Convert Assimp's texture mapping to internally recognized ones
------------------------------------*/
SL_TexWrapMode sl_convert_assimp_tex_wrap(const aiTextureMapMode inWrapMode) noexcept;



/*-------------------------------------
 * Retreive the next VBO Group marker in a list of markers.
-------------------------------------*/
SL_VaoGroup* sl_get_matching_marker(const SL_CommonVertType inVertType, std::vector<SL_VaoGroup>& markers) noexcept;



/*-------------------------------------
 * Calculate the vertex positions for a mesh.
-------------------------------------*/
char* sl_calc_mesh_geometry_pos(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp UVs to Internal Uvs.
-------------------------------------*/
char* sl_calc_mesh_geometry_uvs(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp UVs to Internal Uvs (packed).
-------------------------------------*/
char* sl_calc_mesh_geometry_uvs_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp Colors to Internal Colors.
-------------------------------------*/
char* sl_calc_mesh_geometry_colors(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp Normals to Internal Normals.
-------------------------------------*/
char* sl_calc_mesh_geometry_norm(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp Normals to Internal Normals (packed).
-------------------------------------*/
char* sl_calc_mesh_geometry_norm_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept;



/*-------------------------------------
 * Convert Assimp Tangents & BiTangents to Internal ones.
 * Add an index for each submesh to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_tangent(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo,
    const SL_CommonVertType tangentType
) noexcept;



/*-------------------------------------
 * Convert Assimp Tangents & BiTangents to Internal ones (packed).
 * Add an index for each submesh to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_tangent_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo,
    const SL_CommonVertType tangentType
) noexcept;



/*-------------------------------------
 * Convert Assimp bone IDs to Internal ones. Add an index for each submesh to
 * the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_id(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept;



/*-------------------------------------
 * Convert Assimp bone IDs to Internal ones. Add an index for each submesh to
 * the VBO (16-bits per bone ID).
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_id_packed(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept;



/*-------------------------------------
 * Convert Assimp bone weights to Internal ones. Add an index for each submesh
 * to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_weight(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept;



/*-------------------------------------
 * Convert Assimp bone weights to Internal ones. Add an index for each submesh
 * to the VBO (16-bits per bone weight).
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_weight_packed(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept;



/*-------------------------------------
 * Function to dispatch all text-loading responsibilities to their respective loaders.
-------------------------------------*/
unsigned sl_upload_mesh_vertices(
    const aiMesh* const pMesh,
    const uint32_t baseVert,
    char* const pVbo,
    const SL_CommonVertType vertTypes,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept;



/*-------------------------------------
 * Function to load a bounding box for a mesh
-------------------------------------*/
void sl_update_mesh_bounds(const aiMesh* const pMesh, SL_BoundingBox& box) noexcept;



/*-------------------------------------
 * Check to see if a node is a mesh/camera/bone/point node
-------------------------------------*/
template<typename assimp_data_t>
uint64_t sl_is_node_type(
    const aiNode* const pNode,
    const assimp_data_t* const* const pItems,
    const uint64_t itemCount
) noexcept
{
    const aiString& nodeName = pNode->mName;

    for (uint64_t i = 0; i < itemCount; ++i)
    {
        const assimp_data_t* const pItem = pItems[i];
        const aiString& itemName = pItem->mName;

        if (strcmp(nodeName.C_Str(), itemName.C_Str()) == 0)
        {
            return i;
        }
    }

    return SCENE_NODE_ROOT_ID;
}



template<>
inline uint64_t sl_is_node_type<aiMesh>(const aiNode* const pNode, const aiMesh* const* const, const uint64_t) noexcept
{
    return pNode->mNumMeshes > 0;
}



/*-------------------------------------
 * Count all scene nodes in an aiScene
-------------------------------------*/
unsigned sl_count_assimp_nodes(const aiNode* const pNode) noexcept;



/*-------------------------------------
 * Setup Animation
-------------------------------------*/
SL_Animation sl_setup_imported_animation(
    const char* const name,
    const SL_AnimPrecision duration,
    const SL_AnimPrecision ticksPerSec,
    const unsigned numChannels
) noexcept;



#endif /* SL_SCENE_FILE_UTILITY_HPP */
