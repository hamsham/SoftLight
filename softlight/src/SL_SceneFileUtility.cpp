
#include <type_traits>

#include "lightsky/utils/Log.h"

#include "lightsky/math/Math.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneFileUtility.hpp"
#include "softlight/SL_Texture.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Utility Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Convert Assimp draw types to internal ones
-------------------------------------*/
SL_RenderMode sl_convert_assimp_draw_mode(const aiMesh* const pMesh) noexcept
{
    if (0 != (pMesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_POINT))
    {
        return RENDER_MODE_INDEXED_POINTS;
    }

    if (0 != (pMesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_LINE))
    {
        return RENDER_MODE_INDEXED_LINES;
    }

    return RENDER_MODE_INDEXED_TRIANGLES;
}



/*-------------------------------------
 * Convert Assimp vertex attributes into internal enumerations
-------------------------------------*/
SL_CommonVertType sl_convert_assimp_verts(const aiMesh* const pMesh, const SL_SceneLoadOpts& opts) noexcept
{
    std::underlying_type<SL_CommonVertType>::type vertTypes = 0;

    if (pMesh->HasFaces())
    {
        vertTypes |= SL_CommonVertType::POSITION_VERTEX;
    }

    if (pMesh->HasTextureCoords(aiTextureType_NONE))
    {
        vertTypes |= opts.packUvs ? SL_CommonVertType::PACKED_TEXTURE_VERTEX : SL_CommonVertType::TEXTURE_VERTEX;
    }

    if (pMesh->HasVertexColors(0))
    {
        vertTypes |= SL_CommonVertType::COLOR_VERTEX;
    }

    if (pMesh->HasNormals())
    {
        vertTypes |= opts.packNormals ? SL_CommonVertType::PACKED_NORMAL_VERTEX : SL_CommonVertType::NORMAL_VERTEX;
    }

    if (pMesh->HasTangentsAndBitangents())
    {
        vertTypes |= opts.packNormals
            ? (SL_CommonVertType::PACKED_TANGENT_VERTEX | SL_CommonVertType::PACKED_BITANGENT_VERTEX)
            : (SL_CommonVertType::TANGENT_VERTEX        | SL_CommonVertType::BITANGENT_VERTEX);
    }

    if (pMesh->HasBones())
    {
        vertTypes |= opts.packBoneIds ? SL_CommonVertType::PACKED_BONE_ID_VERTEX : SL_CommonVertType::BONE_ID_VERTEX;
        vertTypes |= opts.packBoneWeights ? SL_CommonVertType::PACKED_BONE_WEIGHT_VERTEX : SL_CommonVertType::BONE_WEIGHT_VERTEX;
    }

    if (!vertTypes)
    {
        LS_LOG_ERR("Warning: No vertex data found for the imported submesh \"", pMesh->mName.C_Str(), ".\"");
    }

    return (SL_CommonVertType)vertTypes;
}



/*-------------------------------------
 * Convert Assimp's texture mapping to internally recognized ones
------------------------------------*/
SL_SamplerWrap sl_convert_assimp_tex_wrap(const aiTextureMapMode inWrapMode) noexcept
{
    SL_SamplerWrap outWrapping = SL_SamplerWrap::DEFAULT;

    switch(inWrapMode)
    {
        case aiTextureMapMode::aiTextureMapMode_Decal:
            outWrapping = SL_SamplerWrap::BORDER;
            break;

        case aiTextureMapMode::aiTextureMapMode_Clamp:
            outWrapping = SL_SamplerWrap::CLAMP;
            break;

        case aiTextureMapMode::aiTextureMapMode_Mirror:
        case aiTextureMapMode::aiTextureMapMode_Wrap:
            outWrapping = SL_SamplerWrap::REPEAT;
            break;

        default:
            break;
    }

    return outWrapping;
}



/*-------------------------------------
 * Retreive the next VBO Group marker in a list of markers.
-------------------------------------*/
SL_VaoGroup* sl_get_matching_marker(const SL_CommonVertType inVertType, std::vector<SL_VaoGroup>& markers) noexcept
{
    for (SL_VaoGroup& m : markers)
    {
        if (inVertType == m.vertType)
        {
            return &m;
        }
    }

    return nullptr;
}



/*-------------------------------------
 * Calculate a portion of vertex data that a glyph should contain (unaligned
 * vec4 store).
-------------------------------------*/
inline char* set_mesh_vertex_data(char* const pVert, const math::vec4& data) noexcept
{
    // It seems some vertex configurations may attempt an unaligned store and
    // crash SSE/NEON-optimized builds. This store procedure must be
    // explicitly fixed using scalar assignments to avoid crashing.
    float* const pOut = reinterpret_cast<float*>(pVert);
    pOut[0] = data[0];
    pOut[1] = data[1];
    pOut[2] = data[2];
    pOut[3] = data[3];

    return pVert + sizeof(math::vec4);
}



/*-------------------------------------
 * Calculate a portion of vertex data that a glyph should contain.
-------------------------------------*/
template<typename data_t>
inline char* set_mesh_vertex_data(char* const pVert, const data_t& data) noexcept
{
    *reinterpret_cast<data_t* const> (pVert) = data;
    return pVert + sizeof(data_t);
}



/*-------------------------------------
 * Calculate the vertex positions for a mesh.
-------------------------------------*/
char* sl_calc_mesh_geometry_pos(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiVector3D* const pInVerts = pMesh->mVertices;
    const aiVector3D&       inVert   = pInVerts[index];

    return set_mesh_vertex_data(pVbo, sl_convert_assimp_vector(inVert));
}



/*-------------------------------------
 * Convert Assimp UVs to Internal Uvs.
-------------------------------------*/
char* sl_calc_mesh_geometry_uvs(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiVector3D* const inUvs = pMesh->mTextureCoords[aiTextureType_NONE];
    const aiVector3D&       inUv  = inUvs[index];
    const float             uvx   = inUv.x;
    const float             uvy   = inUv.y;

    return set_mesh_vertex_data(pVbo, math::vec2{uvx, uvy});
}



/*-------------------------------------
 * Convert Assimp UVs to Internal Uvs (packed).
-------------------------------------*/
char* sl_calc_mesh_geometry_uvs_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiVector3D* const inUvs = pMesh->mTextureCoords[aiTextureType_NONE];
    const aiVector3D&       inUv  = inUvs[index];
    const math::half&&      uvx   = (math::half)inUv.x;
    const math::half&&      uvy   = (math::half)inUv.y;

    return set_mesh_vertex_data(pVbo, math::vec2h{uvx, uvy});
}



/*-------------------------------------
 * Convert Assimp Colors to Internal Colors.
-------------------------------------*/
char* sl_calc_mesh_geometry_colors(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiColor4D* const pInColors = pMesh->mColors[aiTextureType_NONE];

    const aiColor4D& inColor = pInColors[index];
    return set_mesh_vertex_data(pVbo, sl_convert_assimp_color(inColor));
}



/*-------------------------------------
 * Convert Assimp Normals to Internal Normals.
-------------------------------------*/
char* sl_calc_mesh_geometry_norm(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiVector3D* const pInNorms = pMesh->mNormals;

    const aiVector3D&  inNorm = pInNorms[index];
    const math::vec3&& n      = sl_convert_assimp_vector(inNorm);

    return set_mesh_vertex_data(pVbo, math::normalize(n));
}



/*-------------------------------------
 * Convert Assimp Normals to Internal Normals.
-------------------------------------*/
char* sl_calc_mesh_geometry_norm_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo
) noexcept
{
    const aiVector3D* const pInNorms = pMesh->mNormals;

    const aiVector3D&  inNorm = pInNorms[index];
    const math::vec3&& n      = sl_convert_assimp_vector(inNorm);

    return set_mesh_vertex_data(pVbo, sl_pack_vec3_10_10_10_2I(math::normalize(n)));
}



/*-------------------------------------
 * Convert Assimp Tangents & BiTangents to Internal ones.
 * Add an index for each submesh to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_tangent(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo,
    const SL_CommonVertType tangentType
) noexcept
{
    const aiVector3D&  inTng = (tangentType == SL_CommonVertType::TANGENT_VERTEX) ? pMesh->mTangents[index] : pMesh->mBitangents[index];
    const math::vec3&& t     = sl_convert_assimp_vector(inTng);

    return set_mesh_vertex_data(pVbo, math::normalize(t));
}



/*-------------------------------------
 * Convert Assimp Tangents & BiTangents to Internal ones.
 * Add an index for each submesh to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_tangent_packed(
    const unsigned index,
    const aiMesh* const pMesh,
    char* pVbo,
    const SL_CommonVertType tangentType
) noexcept
{
    const aiVector3D&  inTng = (tangentType == SL_CommonVertType::PACKED_TANGENT_VERTEX) ? pMesh->mTangents[index] : pMesh->mBitangents[index];
    const math::vec3&& t     = sl_convert_assimp_vector(inTng);

    return set_mesh_vertex_data(pVbo, sl_pack_vec3_10_10_10_2I(math::normalize(t)));
}



/*-------------------------------------
 * Convert Assimp bone IDs to Internal ones. Add an index for each submesh to
 * the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_id(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept
{
    const SL_BoneData& bone = boneData[index];
    return set_mesh_vertex_data(pVbo, bone.ids32);
}



/*-------------------------------------
 * Convert Assimp bone IDs to Internal ones. Add an index for each submesh to
 * the VBO (16-bits per bone ID).
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_id_packed(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept
{
    const SL_BoneData& bone = boneData[index];
    return set_mesh_vertex_data(pVbo, bone.ids16);
}



/*-------------------------------------
 * Convert Assimp bone weights to Internal ones. Add an index for each submesh
 * to the VBO.
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_weight(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept
{
    const SL_BoneData& bone = boneData[index];
    return set_mesh_vertex_data(pVbo, bone.weights32);
}



/*-------------------------------------
 * Convert Assimp bone weights to Internal ones. Add an index for each submesh
 * to the VBO (16-bits per bone weight).
-------------------------------------*/
char* sl_calc_mesh_geometry_bone_weight_packed(
    const uint32_t index,
    char* pVbo,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept
{
    const SL_BoneData& bone = boneData[index];
    return set_mesh_vertex_data(pVbo, bone.weights16);
}



/*-------------------------------------
 * Function to dispatch all text-loading responsibilities to their respective loaders.
-------------------------------------*/
unsigned sl_upload_mesh_vertices(
    const aiMesh* const pMesh,
    const uint32_t baseVert,
    char* const pVbo,
    const SL_CommonVertType vertTypes,
    std::unordered_map<uint32_t, SL_BoneData>& boneData
) noexcept
{
    //const unsigned vertStride  = sl_vertex_stride(vertTypes);
    const unsigned numVertices = pMesh->mNumVertices;
    char*          pVboIter    = pVbo;

    for (unsigned i = 0; i < numVertices; ++i)
    {
        if (vertTypes & SL_CommonVertType::POSITION_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_pos(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::TEXTURE_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_uvs(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::PACKED_TEXTURE_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_uvs_packed(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::COLOR_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_colors(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::NORMAL_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_norm(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::TANGENT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_tangent(i, pMesh, pVboIter, SL_CommonVertType::TANGENT_VERTEX);
        }

        if (vertTypes & SL_CommonVertType::BITANGENT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_tangent(i, pMesh, pVboIter, SL_CommonVertType::BITANGENT_VERTEX);
        }

        if (vertTypes & SL_CommonVertType::PACKED_NORMAL_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_norm_packed(i, pMesh, pVboIter);
        }

        if (vertTypes & SL_CommonVertType::PACKED_TANGENT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_tangent_packed(i, pMesh, pVboIter, SL_CommonVertType::PACKED_TANGENT_VERTEX);
        }

        if (vertTypes & SL_CommonVertType::PACKED_BITANGENT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_tangent_packed(i, pMesh, pVboIter, SL_CommonVertType::PACKED_BITANGENT_VERTEX);
        }

        if (vertTypes & SL_CommonVertType::BONE_ID_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_bone_id((uint32_t)(i+baseVert), pVboIter, boneData);
        }

        if (vertTypes & SL_CommonVertType::PACKED_BONE_ID_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_bone_id_packed((uint32_t)(i+baseVert), pVboIter, boneData);
        }

        if (vertTypes & SL_CommonVertType::BONE_WEIGHT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_bone_weight((uint32_t)(i+baseVert), pVboIter, boneData);
        }

        if (vertTypes & SL_CommonVertType::PACKED_BONE_WEIGHT_VERTEX)
        {
            pVboIter = sl_calc_mesh_geometry_bone_weight_packed((uint32_t)(i+baseVert), pVboIter, boneData);
        }
    }

    const unsigned bytesWritten = numVertices * sl_vertex_byte_size(vertTypes);

    return bytesWritten;
}



/*-------------------------------------
 * Function to load a bounding box for a mesh
-------------------------------------*/
void sl_update_mesh_bounds(const aiMesh* const pMesh, SL_BoundingBox& box) noexcept
{
    for (unsigned i = 0; i < pMesh->mNumVertices; ++i)
    {
        box.compare_and_update(sl_convert_assimp_vector(pMesh->mVertices[i]));
    }
}



/*-------------------------------------
 * Count all scene nodes in an aiScene
-------------------------------------*/
unsigned sl_count_assimp_nodes(const aiNode* const pNode) noexcept
{
    if (!pNode)
    {
        return 0;
    }

    const unsigned numChildren = pNode->mNumChildren;
    unsigned numNodes = 1 + numChildren;

    for (unsigned i = 0; i < numChildren; ++i)
    {
        numNodes += sl_count_assimp_nodes(pNode->mChildren[i]);
    }

    return numNodes;
}



/*-------------------------------------
 * Setup an animation for importing
-------------------------------------*/
SL_Animation sl_setup_imported_animation(
    const char* const name,
    const SL_AnimPrecision duration,
    const SL_AnimPrecision ticksPerSec,
    const unsigned numChannels
) noexcept
{
    SL_Animation anim{};

    anim.duration(duration);
    anim.name(std::string{name});
    anim.ticks_per_sec(ticksPerSec > 0.0 ? ticksPerSec : 23.976);
    anim.reserve(numChannels);

    return anim;
}
