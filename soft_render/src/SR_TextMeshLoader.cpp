/*
 * File:   draw/GeometryLoader.cpp
 * Author: miles
 *
 * Created on August 3, 2015, 9:50 PM
 */

#include <algorithm> // std::copy, std::fill
#include <cctype> // isgraph(...)
#include <new> // std::nothrow

#include "lightsky/setup/Macros.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"
#include "lightsky/utils/Hash.h"
#include "lightsky/utils/StringUtils.h"

#include "lightsky/math/bits.h"
#include "lightsky/math/half.h"
#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"

#include "soft_render/SR_Atlas.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_PackedVertex.hpp"
#include "soft_render/SR_TextMeshLoader.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"



namespace math = ls::math;
namespace utils = ls::utils;



/*-------------------------------------
 * Default options for loading a text mesh
-------------------------------------*/
SR_TextLoadOpts sr_default_text_load_opts() noexcept
{
    SR_TextLoadOpts opts;
    opts.packUvs = false;
    opts.genNormals = false;
    opts.packNormals = false;
    opts.genTangents = false;
    opts.genIndexVertex = false;

    return opts;
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_TextMeshLoader::~SR_TextMeshLoader() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_TextMeshLoader::SR_TextMeshLoader() noexcept :
    metaData(),
    sceneData{},
    lineSpacing{(float)DEFAULT_TEXT_LINE_SPACING},
    horizTabSpacing{(float)DEFAULT_TEXT_SPACES_PER_TAB},
    vertTabSpacing{(float)DEFAULT_TEXT_SPACES_PER_TAB}
{
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_TextMeshLoader::SR_TextMeshLoader(const SR_TextMeshLoader& t) noexcept
{
    *this = t;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_TextMeshLoader::SR_TextMeshLoader(SR_TextMeshLoader&& t) noexcept
{
    *this = std::move(t);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_TextMeshLoader& SR_TextMeshLoader::operator=(const SR_TextMeshLoader& t) noexcept
{
    metaData = t.metaData;
    sceneData = t.sceneData;
    lineSpacing = t.lineSpacing;
    horizTabSpacing = t.horizTabSpacing;
    vertTabSpacing = t.vertTabSpacing;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_TextMeshLoader& SR_TextMeshLoader::operator=(SR_TextMeshLoader&& t) noexcept
{
    metaData = t.metaData;
    t.metaData.vertAttribs = (SR_CommonVertType)0;
    t.metaData.numAttribs = 0;
    t.metaData.vertStride = 0;
    t.metaData.numVerts = 0;
    t.metaData.indexType = SR_DataType::VERTEX_DATA_INVALID;
    t.metaData.numIndices = 0;
    t.metaData.indexStride = 0;

    sceneData = std::move(t.sceneData);

    lineSpacing = t.lineSpacing;
    t.lineSpacing = SR_TextProperty::DEFAULT_TEXT_LINE_SPACING;

    horizTabSpacing = t.horizTabSpacing;
    t.horizTabSpacing = SR_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB;

    vertTabSpacing = t.vertTabSpacing;
    t.vertTabSpacing = SR_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB;

    return *this;
}



/*-------------------------------------
 * Utility function to get all of the non-whitespace characters in a string
-------------------------------------*/
unsigned SR_TextMeshLoader::get_num_drawable_chars(const std::string& str) noexcept
{
    unsigned charCount = 0;

    for (unsigned i = 0; i < str.size(); ++i)
    {
        if (isgraph(str[i]))
        {
            ++charCount;
        }
    }

    return charCount;
}



/*-------------------------------------
 * Calculate the vertex positions that a glyph should represent.
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_pos(
    const SR_AtlasGlyph& rGlyph,
    char* pVert,
    const math::vec2& posOffset,
    const unsigned charIndex
) noexcept
{
    const float xOffset = posOffset[0];
    const float yOffset = posOffset[1];
    const math::vec2& glyphSize = rGlyph.size;

    const math::vec3 posData[4] = {
        math::vec3{xOffset, yOffset - glyphSize[1], 0.f},
        math::vec3{xOffset, yOffset, 0.f},
        math::vec3{xOffset + glyphSize[0], yOffset - glyphSize[1], 0.f},
        math::vec3{xOffset + glyphSize[0], yOffset, 0.f}
    };

    pVert = set_text_vertex_data(pVert, posData[0]);
    pVert = set_text_vertex_data(pVert, posData[1]);
    pVert = set_text_vertex_data(pVert, posData[2]);
    set_text_vertex_data(pVert, posData[3]);

    SR_AlignedVector<SR_BoundingBox>& boundsBuffer = sceneData.mMeshBounds;
    if (!boundsBuffer.empty())
    {
        SR_BoundingBox& bb = boundsBuffer[charIndex];
        bb.min_point(math::vec4{xOffset, yOffset - glyphSize[1], 0.f, 1.f});
        bb.max_point(math::vec4{xOffset + glyphSize[0], yOffset, 0.f, 1.f});
    }

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::POSITION_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_uvs(
    const SR_AtlasGlyph& rGlyph,
    char* pVert
) noexcept
{
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[0][0], rGlyph.uv[0][1]});
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[0][0], rGlyph.uv[1][1]});
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[1][0], rGlyph.uv[0][1]});
    set_text_vertex_data(pVert, math::vec2{rGlyph.uv[1][0], rGlyph.uv[1][1]});

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::TEXTURE_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_packed_uvs(
    const SR_AtlasGlyph& rGlyph,
    char* pVert
) noexcept
{
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[0][0], (math::half)rGlyph.uv[0][1]});
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[0][0], (math::half)rGlyph.uv[1][1]});
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[1][0], (math::half)rGlyph.uv[0][1]});
    set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[1][0], (math::half)rGlyph.uv[1][1]});

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::TEXTURE_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_norms(
    char* pVert,
    const math::vec3& normDir
) noexcept
{
    pVert = set_text_vertex_data(pVert, normDir);
    pVert = set_text_vertex_data(pVert, normDir);
    pVert = set_text_vertex_data(pVert, normDir);
    set_text_vertex_data(pVert, normDir);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::NORMAL_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_packed_norms(
    char* pVert,
    const math::vec3& normDir
) noexcept
{
    const int32_t packedNorm = sr_pack_vertex_2_10_10_10(normDir);
    pVert = set_text_vertex_data(pVert, packedNorm);
    pVert = set_text_vertex_data(pVert, packedNorm);
    pVert = set_text_vertex_data(pVert, packedNorm);
    set_text_vertex_data(pVert, packedNorm);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::NORMAL_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the ID of a single character
-------------------------------------*/
unsigned SR_TextMeshLoader::calc_text_geometry_indices(char* pVert, const unsigned indexId) noexcept
{
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    set_text_vertex_data<unsigned>(pVert, indexId);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sr_vertex_byte_size(SR_CommonVertType::INDEX_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to dispatch all text-loading responsibilities to their respective loaders.
-------------------------------------*/
char* SR_TextMeshLoader::gen_text_geometry_vert(
    const SR_AtlasGlyph& rGlyph,
    char* const pData,
    const math::vec2& posOffset,
    const unsigned currChar
) noexcept
{
    const SR_CommonVertType vertTypes = metaData.vertAttribs;
    char* pVert = pData;

    if (vertTypes & SR_CommonVertType::POSITION_VERTEX)
    {
        pVert += calc_text_geometry_pos(rGlyph, pVert, posOffset, currChar);
    }

    if (vertTypes & SR_CommonVertType::TEXTURE_VERTEX)
    {
        pVert += calc_text_geometry_uvs(rGlyph, pVert);
    }

    if (vertTypes & SR_CommonVertType::PACKED_TEXTURE_VERTEX)
    {
        pVert += calc_text_geometry_packed_uvs(rGlyph, pVert);
    }

    if (vertTypes & SR_CommonVertType::NORMAL_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{0.f, 0.f, 1.f});
    }

    if (vertTypes & SR_CommonVertType::PACKED_NORMAL_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{0.f, 0.f, 1.f});
    }

    if (vertTypes & SR_CommonVertType::TANGENT_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{1.f, 0.f, 0.f});
    }

    if (vertTypes & SR_CommonVertType::PACKED_TANGENT_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{1.f, 0.f, 0.f});
    }

    if (vertTypes & SR_CommonVertType::BITANGENT_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{0.f, 1.f, 0.f});
    }

    if (vertTypes & SR_CommonVertType::PACKED_BITANGENT_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{0.f, 1.f, 0.f});
    }

    if (vertTypes & SR_CommonVertType::INDEX_VERTEX)
    {
        pVert += calc_text_geometry_indices(pVert, currChar);
    }

    return pData + (metaData.vertStride * TEXT_VERTS_PER_GLYPH);
}



/*-------------------------------------
 * Calculate a portion of vertex data that a glyph should contain.
-------------------------------------*/
template <typename data_t>
char* SR_TextMeshLoader::set_text_vertex_data(char* const pVert, const data_t& data) noexcept
{
    *reinterpret_cast<data_t* const> (pVert) = data;
    return pVert + metaData.vertStride;
}



/*-------------------------------------
 * Set the index data required by geometry text.
-------------------------------------*/
char* SR_TextMeshLoader::set_text_index_data(
    char* pIndices,
    const unsigned indexOffset
) noexcept
{
    if (metaData.indexType == SR_DataType::VERTEX_DATA_BYTE)
    {
        return fill_geometry_indices<unsigned char>(pIndices, indexOffset);
    }
    else if (metaData.indexType == SR_DataType::VERTEX_DATA_SHORT)
    {
        return fill_geometry_indices<unsigned short>(pIndices, indexOffset);
    }

    return fill_geometry_indices<unsigned int>(pIndices, indexOffset);
}



/*-------------------------------------
 * Text/String Generation
-------------------------------------*/
bool SR_TextMeshLoader::gen_text_geometry(const std::string& str, const SR_Atlas& atlas) noexcept
{
    SR_VertexBuffer& vbo = sceneData.mContext.vbo(0);
    SR_IndexBuffer& ibo = sceneData.mContext.ibo(0);

    // VBO Mapping
    char* pVerts = (char*)vbo.data();
    if (!pVerts)
    {
        LS_LOG_ERR("\t\tAn error occurred while attempting to map a VBO for text geometry.");
        return false;
    }

    // IBO Mapping
    char* pIndices = (char*)ibo.data();
    if (!pIndices)
    {
        LS_LOG_ERR("\t\tAn error occurred while attempting to map an IBO for text geometry.");
        return false;
    }

    // Get pointers to the buffer data that will be filled with quads
    const ls::utils::Pointer<SR_AtlasGlyph[]>& pGlyphs = atlas.glyphs();

    // The y-origin (starting 'yPos') was found using a lot of testing. This
    // was for resolution independence
    const SR_AtlasGlyph& newline = pGlyphs[u'\n'];
    float yPos = newline.bearing[1] - (newline.bearing[1] - newline.size[1]);
    float xPos = 0.f;
    unsigned charId = 0;
    unsigned indexOffset = 0;

    for (unsigned i = 0; i < str.size(); ++i)
    {
        const unsigned currChar = (unsigned)str[i];
        const SR_AtlasGlyph& rGlyph = pGlyphs[currChar];

        // Amount the each glyph "hangs" below its Y-origin
        const float vertHang = rGlyph.bearing[1] - rGlyph.size[1];

        if (currChar == u' ')
        {
            xPos += rGlyph.advance[0];
        }
        else if (currChar == u'\t')
        {
            xPos += rGlyph.advance[0] * horizTabSpacing;
        }
        else if (currChar == u'\n')
        {
            // formula found through trial and error.
            yPos += (rGlyph.bearing[1] + lineSpacing) + vertHang;
            xPos = 0.f;
        }
        else if (currChar == u'\r')
        {
            xPos = 0.f;
        }
        else if (currChar == u'\v')
        {
            yPos += ((rGlyph.bearing[1] + lineSpacing) + vertHang) * vertTabSpacing;
        }
        else
        {
            const math::vec2 posOffset{
                xPos + rGlyph.bearing[0],
                yPos - vertHang
            };
            xPos = xPos + rGlyph.advance[0];
            pVerts = gen_text_geometry_vert(rGlyph, pVerts, posOffset, charId++);
            pIndices = set_text_index_data(pIndices, indexOffset);
            indexOffset = indexOffset + TEXT_VERTS_PER_GLYPH;
        }
    }

    return true;
}



/*-------------------------------------
 * CPU Memory Initialization
-------------------------------------*/
unsigned SR_TextMeshLoader::allocate_cpu_data(
    const std::string& str,
    const SR_CommonVertType vertexTypes,
    const bool loadBounds
) noexcept
{
    unsigned numSubmeshes = get_num_drawable_chars(str);

    // ignore vertex types which are have no support within the text loader
    metaData.vertAttribs = vertexTypes;
    metaData.vertStride = sr_vertex_stride(metaData.vertAttribs);
    metaData.numVerts = numSubmeshes * SR_TextProperty::TEXT_VERTS_PER_GLYPH;

    metaData.indexType = sr_required_index_type(metaData.numVerts);
    metaData.numIndices = numSubmeshes * SR_TextProperty::TEXT_INDICES_PER_GLYPH;
    metaData.indexStride = sr_bytes_per_type(metaData.indexType);

    LS_LOG_MSG(
        "Text Geometry Meta Data:",
        "\n\tVertex Count:      ", metaData.numVerts,
        "\n\tIndex Count:       ", metaData.numIndices,
        "\n\tBytes Per Vertex:  ", metaData.vertStride,
        "\n\tBytes Per Index:   ", metaData.indexStride,
        "\n\tVertex Bytes:      ", metaData.numVerts * metaData.vertStride,
        "\n\tIndex Bytes:       ", metaData.numIndices * metaData.indexStride
    );

    unsigned numBytes = 0;

    // Initial setup for atlas texture data
    SR_AlignedVector<SR_Material>& materials = sceneData.mMaterials;
    materials.resize(1);
    sr_reset(materials[0]);

    // Setup the initial text scene graph with some default draw params
    SR_AlignedVector<SR_Mesh>& meshes = sceneData.mMeshes;
    meshes.resize(numSubmeshes);

    for (unsigned m = 0, offset = 0; m < numSubmeshes; ++m, offset += TEXT_INDICES_PER_GLYPH)
    {
        SR_Mesh& mesh = meshes[m];
        sr_reset(mesh);

        mesh.vaoId = 0;
        mesh.elementBegin = offset;
        mesh.elementEnd = offset + SR_TextProperty::TEXT_INDICES_PER_GLYPH;
        mesh.mode = SR_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
        mesh.materialId = 0;
    }

    {
        sceneData.mNodes.push_back(SR_SceneNode{SR_SceneNodeType::NODE_TYPE_MESH, 0, 0, SCENE_NODE_ROOT_ID});
        sceneData.mBaseTransforms.emplace_back(math::mat4{1.f});
        sceneData.mCurrentTransforms.emplace_back(SR_Transform{math::mat4{1.f}, SR_TRANSFORM_TYPE_MODEL});
        sceneData.mModelMatrices.emplace_back(math::mat4{1.f});
        sceneData.mNodeNames.emplace_back(std::string{"textMesh-"} + std::to_string(utils::hash_fnv1(str.c_str())));

        sceneData.mNodeMeshes.emplace_back(new size_t[numSubmeshes]);
        ls::utils::Pointer<size_t[]>& meshIds = sceneData.mNodeMeshes[0];
        for (unsigned sm = 0; sm < numSubmeshes; ++sm)
        {
            meshIds[sm] = sm;
        }

        sceneData.mNumNodeMeshes.push_back(numSubmeshes);
    }

    numBytes += sizeof(SR_Mesh) * meshes.size();

    if (loadBounds)
    {
        SR_AlignedVector<SR_BoundingBox>& boundsBuffer = sceneData.mMeshBounds;
        boundsBuffer.resize(numSubmeshes);
    }

    return numBytes;
}



/*-------------------------------------
 * GPU Memory Initialization
-------------------------------------*/
int SR_TextMeshLoader::allocate_gpu_data(const SR_Atlas& atlas) noexcept
{
    SR_AlignedVector<SR_Material>& materials = sceneData.mMaterials;
    SR_Material& material = materials.front();
    material.ambient = SR_ColorRGBAf{1.f, 1.f, 1.f, 1.f};
    material.pTextures[SR_MATERIAL_TEXTURE_AMBIENT] = atlas.texture();

    SR_Context& renderData = sceneData.mContext;

    size_t vboId = renderData.create_vbo();
    SR_VertexBuffer& vbo = renderData.vbo(vboId);

    size_t iboId = renderData.create_ibo();
    SR_IndexBuffer& ibo = renderData.ibo(iboId);

    size_t vaoId = renderData.create_vao();
    SR_VertexArray& vao = renderData.vao(vaoId);

    // VBO Allocation
    unsigned vertStride = sr_vertex_byte_size(metaData.vertAttribs);
    if (vbo.init(metaData.numVerts * vertStride, nullptr) != 0)
    {
        return -2;
    }

    // IBO Allocation
    if (ibo.init(metaData.numIndices, metaData.indexType, nullptr) != 0)
    {
        return -3;
    }

    // VAO Allocation
    vao.set_vertex_buffer(vboId);
    unsigned numAttribs = sr_count_vertex_attribs(metaData.vertAttribs);
    vao.set_vertex_buffer(0);
    vao.set_index_buffer(0);
    if (vao.set_num_bindings((size_t)numAttribs) != (int)numAttribs)
    {
        return -4;
    }

    // VAO Setup
    for (unsigned i = 0; i < numAttribs; ++i)
    {
        SR_CommonVertType vertType = sr_get_vertex_attrib(metaData.vertAttribs, i);
        unsigned offset = sr_vertex_attrib_offset(metaData.vertAttribs, vertType);
        SR_Dimension dimens = sr_dimens_of_vertex(vertType);
        SR_DataType dataType = sr_type_of_vertex(vertType);
        vao.set_binding(i, offset, vertStride, dimens, dataType);
    }

    return 0;
}



/*-------------------------------------
 * Text/String loading
-------------------------------------*/
unsigned SR_TextMeshLoader::load(
    const std::string& str,
    const SR_Atlas& atlas,
    const SR_TextLoadOpts& opts,
    const bool loadBounds
) noexcept
{
    LS_LOG_MSG("Attempting to load text geometry.");
    unload(); // clear any prior data.

    SR_CommonVertType vertexTypes = SR_CommonVertType::POSITION_VERTEX;
    if (opts.packUvs)
    {
        vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::PACKED_TEXTURE_VERTEX);
    }
    else
    {
        vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::TEXTURE_VERTEX);
    }

    if (opts.genNormals || opts.packNormals || opts.genTangents)
    {
        if (opts.packNormals)
        {
            vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::PACKED_NORMAL_VERTEX);
        }
        else
        {
            vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::NORMAL_VERTEX);
        }

        if (opts.genTangents)
        {
            if (opts.packNormals)
            {
                vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::PACKED_TANGENT_VERTEX);
                vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::PACKED_BITANGENT_VERTEX);
            }
            else
            {
                vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::TANGENT_VERTEX);
                vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::BITANGENT_VERTEX);
            }
        }
    }

    if (opts.genIndexVertex)
    {
        vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::INDEX_VERTEX);
    }

    if (loadBounds)
    {
        vertexTypes = (SR_CommonVertType)((unsigned)vertexTypes | SR_CommonVertType::POSITION_VERTEX);
    }

    LS_LOG_MSG("\tAllocating RAM for text mesh data.");
    const unsigned numBytes = allocate_cpu_data(str, vertexTypes, loadBounds);

    if (!numBytes)
    {
        LS_LOG_ERR("\t\tFailed to allocate ", numBytes, " bytes of memory for text mesh data.\n");
        sceneData.terminate();
        return -1;
    }
    LS_LOG_MSG("\t\tDone. Successfully allocated ", numBytes, " of memory in RAM.");

    LS_LOG_MSG("\tAllocating GPU memory for text mesh data.");
    if (allocate_gpu_data(atlas) != 0)
    {
        sceneData.terminate();
        LS_LOG_ERR("\t\tUnable to initialize text mesh data on the GPU.\n");
        return -2;
    }
    LS_LOG_MSG("\t\tDone.");

    // Generate the text geometry
    LS_LOG_MSG("\tGenerating a text mesh on the GPU.");
    if (!gen_text_geometry(str, atlas))
    {
        LS_LOG_ERR("\t\tUnable to send text geometry data to the GPU.\n");
        sceneData.terminate();
        return -3;
    }
    LS_LOG_MSG("\t\tDone.");

    LS_LOG_MSG(
        "\tSuccessfully sent a string to the GPU.",
        "\n\t\tCharacters:  ", metaData.numIndices / SR_TextProperty::TEXT_INDICES_PER_GLYPH,
        "\n\t\tVertices:    ", metaData.numVerts,
        "\n\t\tVert Size:   ", metaData.numVerts * sr_vertex_byte_size(metaData.vertAttribs), " bytes",
        "\n\t\tIndices:     ", metaData.numIndices,
        "\n\t\tIndex Size:  ", sr_bytes_per_type(metaData.indexType), " bytes",
        "\n\t\tTotal Size:  ", metaData.numIndices * sr_bytes_per_type(metaData.indexType), " bytes",
        '\n'
    );

    return 0;
}

/*-------------------------------------
 * Text/String loading
-------------------------------------*/
void SR_TextMeshLoader::unload()
{
    sceneData.terminate();
    lineSpacing = (float)DEFAULT_TEXT_LINE_SPACING;
    horizTabSpacing = (float)DEFAULT_TEXT_SPACES_PER_TAB;
    vertTabSpacing = (float)DEFAULT_TEXT_SPACES_PER_TAB;
}
