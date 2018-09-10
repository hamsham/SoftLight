
#include <type_traits>

#include "lightsky/utils/Log.h"

#include "lightsky/math/Math.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Geometry.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneFileUtility.hpp"
#include "soft_render/SR_Texture.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Utility Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Convert Assimp draw types to internal ones
-------------------------------------*/
SR_RenderMode sr_convert_assimp_draw_mode(const aiMesh* const pMesh) noexcept
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
SR_CommonVertType sr_convert_assimp_verts(const aiMesh* const pMesh) noexcept
{
    std::underlying_type<SR_CommonVertType>::type vertTypes = 0;

    if (pMesh->HasFaces())
    {
        vertTypes |= SR_CommonVertType::POSITION_VERTEX;
    }

    if (pMesh->HasTextureCoords(aiTextureType_NONE))
    {
        vertTypes |= SR_CommonVertType::TEXTURE_VERTEX;
    }

    if (pMesh->HasNormals())
    {
        vertTypes |= SR_CommonVertType::NORMAL_VERTEX;
    }

    if (pMesh->HasTangentsAndBitangents())
    {
        vertTypes |= SR_CommonVertType::TANGENT_VERTEX | SR_CommonVertType::BITANGENT_VERTEX;
    }

    if (pMesh->HasVertexColors(0))
    {
        vertTypes |= SR_CommonVertType::COLOR_VERTEX;
    }

    if (!vertTypes)
    {
        LS_LOG_ERR("Warning: No vertex data found for the imported submesh \"", pMesh->mName.C_Str(), ".\"");
    }

    return (SR_CommonVertType)vertTypes;
}



/*-------------------------------------
 * Convert Assimp's texture mapping to internally recognized ones
------------------------------------*/
SR_TexWrapMode sr_convert_assimp_tex_wrap(const aiTextureMapMode inWrapMode) noexcept
{
    SR_TexWrapMode outWrapping = SR_TexWrapMode::SR_TEXTURE_WRAP_DEFAULT;

    switch(inWrapMode)
    {
        case aiTextureMapMode::aiTextureMapMode_Clamp:
        case aiTextureMapMode::aiTextureMapMode_Decal:
            outWrapping = SR_TexWrapMode::SR_TEXTURE_WRAP_CLAMP;
            break;

        case aiTextureMapMode::aiTextureMapMode_Mirror:
        case aiTextureMapMode::aiTextureMapMode_Wrap:
            outWrapping = SR_TexWrapMode::SR_TEXTURE_WRAP_REPEAT;
            break;

        default:
            break;
    }

    return outWrapping;
}



/*-------------------------------------
 * Retreive the next VBO Group marker in a list of markers.
-------------------------------------*/
SR_VaoGroup* sr_get_matching_marker(const SR_CommonVertType inVertType, std::vector<SR_VaoGroup>& markers) noexcept
{
    for (SR_VaoGroup& m : markers)
    {
        if (inVertType == m.vertType)
        {
            return &m;
        }
    }

    return nullptr;
}



/*-------------------------------------
 * Calculate a portion of vertex data that a glyph should contain.
-------------------------------------*/
template<typename data_t>
inline char* set_mesh_vertex_data(char* const pVert, const data_t& data, const unsigned vertStride) noexcept
{
    *reinterpret_cast<data_t* const> (pVert) = data;
    return pVert + vertStride;
}



/*-------------------------------------
 * Calculate the vertex positions for a mesh.
-------------------------------------*/
unsigned sr_calc_mesh_geometry_pos(
    const aiMesh* const pMesh,
    char* pVbo,
    const unsigned vertStride
) noexcept
{
    const unsigned numVertices = pMesh->mNumVertices;
    const aiVector3D* const pInVerts = pMesh->mVertices;

    for (unsigned i = 0; i < numVertices; ++i)
    {
        const aiVector3D& inVert = pInVerts[i];
        pVbo = set_mesh_vertex_data(pVbo, sr_convert_assimp_vector(inVert), vertStride);
    }

    return numVertices * sr_vertex_byte_size(SR_CommonVertType::POSITION_VERTEX);
}



/*-------------------------------------
 * Convert Assimp UVs to Internal Uvs.
-------------------------------------*/
unsigned sr_calc_mesh_geometry_uvs(
    const aiMesh* const pMesh,
    char* pVbo,
    const unsigned vertStride
) noexcept
{
    const unsigned numVertices = pMesh->mNumVertices;
    const aiVector3D* const inUvs = pMesh->mTextureCoords[aiTextureType_NONE];

    for (unsigned i = 0; i < numVertices; ++i)
    {
        const aiVector3D& inUv = inUvs[i];
        const float uvx = inUv.x;
        const float uvy = inUv.y;
        pVbo = set_mesh_vertex_data(pVbo, math::vec2{uvx, uvy}, vertStride);
    }

    return numVertices * sr_vertex_byte_size(SR_CommonVertType::TEXTURE_VERTEX);
}



/*-------------------------------------
 * Convert Assimp Normals to Internal Normals.
-------------------------------------*/
unsigned sr_calc_mesh_geometry_norms(
    const aiMesh* const pMesh,
    char* pVbo,
    const unsigned vertStride
) noexcept
{
    const unsigned numVertices = pMesh->mNumVertices;
    const aiVector3D* const pInNorms = pMesh->mNormals;

    for (unsigned i = 0; i < numVertices; ++i)
    {
        const aiVector3D& inNorm = pInNorms[i];
        const math::vec3 n = sr_convert_assimp_vector(inNorm);
        pVbo = set_mesh_vertex_data(pVbo, math::normalize(n), vertStride);
    }

    return numVertices * sr_vertex_byte_size(SR_CommonVertType::NORMAL_VERTEX);
}



/*-------------------------------------
 * Convert Assimp Tangents & BiTangents to Internal ones.
 * Add an index for each submesh to the VBO.
-------------------------------------*/
unsigned sr_calc_mesh_geometry_tangent(
    const aiMesh* const pMesh,
    char* pVbo,
    const unsigned vertStride,
    const SR_CommonVertType tangentType
) noexcept
{
    const unsigned numVertices = pMesh->mNumVertices;

    for (unsigned i = 0; i < numVertices; ++i)
    {
        const aiVector3D& inTng = (tangentType == SR_CommonVertType::TANGENT_VERTEX) ? pMesh->mTangents[i] : pMesh->mBitangents[i];
        const math::vec3 t = sr_convert_assimp_vector(inTng);
        pVbo = set_mesh_vertex_data(pVbo, math::normalize(t), vertStride);
    }

    return numVertices * sr_vertex_byte_size(tangentType);
}



/*-------------------------------------
 * Convert Assimp Colors to Internal Colors.
-------------------------------------*/
unsigned sr_calc_mesh_geometry_colors(
    const aiMesh* const pMesh,
    char* pVbo,
    const unsigned vertStride
) noexcept
{
    const unsigned numVertices = pMesh->mNumVertices;
    const aiColor4D* const pInColors = pMesh->mColors[aiTextureType_NONE];

    for (unsigned i = 0; i < numVertices; ++i)
    {
        const aiColor4D& inColor = pInColors[i];
        pVbo = set_mesh_vertex_data(pVbo, sr_convert_assimp_color(inColor), vertStride);
    }

    return numVertices * sr_vertex_byte_size(SR_CommonVertType::COLOR_VERTEX);
}



/*-------------------------------------
 * Function to dispatch all text-loading responsibilities to their respective loaders.
-------------------------------------*/
unsigned sr_upload_mesh_vertices(
    const aiMesh* const pMesh,
    char* const pVbo,
    const SR_CommonVertType vertTypes
) noexcept
{
    const unsigned vertStride = sr_vertex_stride(vertTypes);
    unsigned offset = 0;
    unsigned bytesWritten = 0;

    if (vertTypes & SR_CommonVertType::POSITION_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_pos(pMesh, pVbo + offset, vertStride);
        offset += sr_vertex_byte_size(SR_CommonVertType::POSITION_VERTEX);
    }

    if (vertTypes & SR_CommonVertType::TEXTURE_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_uvs(pMesh, pVbo + offset, vertStride);
        offset += sr_vertex_byte_size(SR_CommonVertType::TEXTURE_VERTEX);
    }

    if (vertTypes & SR_CommonVertType::NORMAL_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_norms(pMesh, pVbo + offset, vertStride);
        offset += sr_vertex_byte_size(SR_CommonVertType::NORMAL_VERTEX);
    }

    if (vertTypes & SR_CommonVertType::TANGENT_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_tangent(pMesh, pVbo + offset, vertStride, SR_CommonVertType::TANGENT_VERTEX);
        offset += sr_vertex_byte_size(SR_CommonVertType::TANGENT_VERTEX);
    }

    if (vertTypes & SR_CommonVertType::BITANGENT_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_tangent(pMesh, pVbo + offset, vertStride, SR_CommonVertType::BITANGENT_VERTEX);
        offset += sr_vertex_byte_size(SR_CommonVertType::BITANGENT_VERTEX);
    }

    if (vertTypes & SR_CommonVertType::COLOR_VERTEX)
    {
        bytesWritten += sr_calc_mesh_geometry_colors(pMesh, pVbo + offset, vertStride);
        offset += sr_vertex_byte_size(SR_CommonVertType::COLOR_VERTEX);
    }

    return bytesWritten;
}



/*-------------------------------------
 * Function to load a bounding box for a mesh
-------------------------------------*/
void sr_update_mesh_bounds(const aiMesh* const pMesh, SR_BoundingBox& box) noexcept
{
    for (unsigned i = 0; i < pMesh->mNumVertices; ++i)
    {
        box.compare_and_update(sr_convert_assimp_vector(pMesh->mVertices[i]));
    }
}



/*-------------------------------------
 * Count all scene nodes in an aiScene
-------------------------------------*/
unsigned sr_count_assimp_nodes(const aiNode* const pNode) noexcept
{
    if (!pNode)
    {
        return 0;
    }

    const unsigned numChildren = pNode->mNumChildren;
    unsigned numNodes = 1 + numChildren;

    for (unsigned i = 0; i < numChildren; ++i)
    {
        numNodes += sr_count_assimp_nodes(pNode->mChildren[i]);
    }

    return numNodes;
}



/*-------------------------------------
 * Setup an animation for importing
-------------------------------------*/
SR_Animation sr_setup_imported_animation(
    const char* const name,
    const SR_AnimPrecision duration,
    const SR_AnimPrecision ticksPerSec,
    const unsigned numChannels
) noexcept
{
    SR_Animation anim{};

    anim.set_duration(duration);
    anim.set_anim_name(std::string{name});
    anim.set_ticks_per_sec(ticksPerSec > 0.0 ? ticksPerSec : 23.976);
    anim.reserve_anim_channels(numChannels);

    return anim;
}
