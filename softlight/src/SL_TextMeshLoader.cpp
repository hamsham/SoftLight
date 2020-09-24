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

#include "softlight/SL_Atlas.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_TextMeshLoader.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"



namespace math = ls::math;
namespace utils = ls::utils;



/*-------------------------------------
 * Default options for loading a text mesh
-------------------------------------*/
SL_TextLoadOpts sl_default_text_load_opts() noexcept
{
    SL_TextLoadOpts opts;
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
SL_TextMeshLoader::~SL_TextMeshLoader() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_TextMeshLoader::SL_TextMeshLoader() noexcept :
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
SL_TextMeshLoader::SL_TextMeshLoader(const SL_TextMeshLoader& t) noexcept
{
    *this = t;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_TextMeshLoader::SL_TextMeshLoader(SL_TextMeshLoader&& t) noexcept
{
    *this = std::move(t);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_TextMeshLoader& SL_TextMeshLoader::operator=(const SL_TextMeshLoader& t) noexcept
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
SL_TextMeshLoader& SL_TextMeshLoader::operator=(SL_TextMeshLoader&& t) noexcept
{
    metaData = t.metaData;
    t.metaData.vertAttribs = (SL_CommonVertType)0;
    t.metaData.numAttribs = 0;
    t.metaData.vertStride = 0;
    t.metaData.numVerts = 0;
    t.metaData.indexType = SL_DataType::VERTEX_DATA_INVALID;
    t.metaData.numIndices = 0;
    t.metaData.indexStride = 0;

    sceneData = std::move(t.sceneData);

    lineSpacing = t.lineSpacing;
    t.lineSpacing = SL_TextProperty::DEFAULT_TEXT_LINE_SPACING;

    horizTabSpacing = t.horizTabSpacing;
    t.horizTabSpacing = SL_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB;

    vertTabSpacing = t.vertTabSpacing;
    t.vertTabSpacing = SL_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB;

    return *this;
}



/*-------------------------------------
 * Utility function to get all of the non-whitespace characters in a string
-------------------------------------*/
unsigned SL_TextMeshLoader::get_num_drawable_chars(const std::string& str) noexcept
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
unsigned SL_TextMeshLoader::calc_text_geometry_pos(
    const SL_AtlasGlyph& rGlyph,
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

    SL_AlignedVector<SL_BoundingBox>& boundsBuffer = sceneData.mMeshBounds;
    if (!boundsBuffer.empty())
    {
        SL_BoundingBox& bb = boundsBuffer[charIndex];
        bb.min_point(math::vec4{xOffset, yOffset - glyphSize[1], 0.f, 1.f});
        bb.max_point(math::vec4{xOffset + glyphSize[0], yOffset, 0.f, 1.f});
    }

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::POSITION_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SL_TextMeshLoader::calc_text_geometry_uvs(
    const SL_AtlasGlyph& rGlyph,
    char* pVert
) noexcept
{
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[0][0], rGlyph.uv[0][1]});
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[0][0], rGlyph.uv[1][1]});
    pVert = set_text_vertex_data(pVert, math::vec2{rGlyph.uv[1][0], rGlyph.uv[0][1]});
    set_text_vertex_data(pVert, math::vec2{rGlyph.uv[1][0], rGlyph.uv[1][1]});

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::TEXTURE_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SL_TextMeshLoader::calc_text_geometry_packed_uvs(
    const SL_AtlasGlyph& rGlyph,
    char* pVert
) noexcept
{
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[0][0], (math::half)rGlyph.uv[0][1]});
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[0][0], (math::half)rGlyph.uv[1][1]});
    pVert = set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[1][0], (math::half)rGlyph.uv[0][1]});
    set_text_vertex_data(pVert, math::vec2_t<math::half>{(math::half)rGlyph.uv[1][0], (math::half)rGlyph.uv[1][1]});

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::TEXTURE_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SL_TextMeshLoader::calc_text_geometry_norms(
    char* pVert,
    const math::vec3& normDir
) noexcept
{
    pVert = set_text_vertex_data(pVert, normDir);
    pVert = set_text_vertex_data(pVert, normDir);
    pVert = set_text_vertex_data(pVert, normDir);
    set_text_vertex_data(pVert, normDir);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::NORMAL_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the UVs which should represent a glyph.
-------------------------------------*/
unsigned SL_TextMeshLoader::calc_text_geometry_packed_norms(
    char* pVert,
    const math::vec3& normDir
) noexcept
{
    const int32_t packedNorm = sl_pack_vertex_2_10_10_10(normDir);
    pVert = set_text_vertex_data(pVert, packedNorm);
    pVert = set_text_vertex_data(pVert, packedNorm);
    pVert = set_text_vertex_data(pVert, packedNorm);
    set_text_vertex_data(pVert, packedNorm);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::NORMAL_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to calculate the ID of a single character
-------------------------------------*/
unsigned SL_TextMeshLoader::calc_text_geometry_indices(char* pVert, const unsigned indexId) noexcept
{
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    pVert = set_text_vertex_data<unsigned>(pVert, indexId);
    set_text_vertex_data<unsigned>(pVert, indexId);

    //return byte-stride to the next vertex attrib
    static const unsigned vertOffset = sl_vertex_byte_size(SL_CommonVertType::INDEX_VERTEX);
    return vertOffset;
}



/*-------------------------------------
 * Function to dispatch all text-loading responsibilities to their respective loaders.
-------------------------------------*/
char* SL_TextMeshLoader::gen_text_geometry_vert(
    const SL_AtlasGlyph& rGlyph,
    char* const pData,
    const math::vec2& posOffset,
    const unsigned currChar
) noexcept
{
    const SL_CommonVertType vertTypes = metaData.vertAttribs;
    char* pVert = pData;

    if (vertTypes & SL_CommonVertType::POSITION_VERTEX)
    {
        pVert += calc_text_geometry_pos(rGlyph, pVert, posOffset, currChar);
    }

    if (vertTypes & SL_CommonVertType::TEXTURE_VERTEX)
    {
        pVert += calc_text_geometry_uvs(rGlyph, pVert);
    }

    if (vertTypes & SL_CommonVertType::PACKED_TEXTURE_VERTEX)
    {
        pVert += calc_text_geometry_packed_uvs(rGlyph, pVert);
    }

    if (vertTypes & SL_CommonVertType::NORMAL_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{0.f, 0.f, 1.f});
    }

    if (vertTypes & SL_CommonVertType::PACKED_NORMAL_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{0.f, 0.f, 1.f});
    }

    if (vertTypes & SL_CommonVertType::TANGENT_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{1.f, 0.f, 0.f});
    }

    if (vertTypes & SL_CommonVertType::PACKED_TANGENT_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{1.f, 0.f, 0.f});
    }

    if (vertTypes & SL_CommonVertType::BITANGENT_VERTEX)
    {
        pVert += calc_text_geometry_norms(pVert, math::vec3{0.f, 1.f, 0.f});
    }

    if (vertTypes & SL_CommonVertType::PACKED_BITANGENT_VERTEX)
    {
        pVert += calc_text_geometry_packed_norms(pVert, math::vec3{0.f, 1.f, 0.f});
    }

    if (vertTypes & SL_CommonVertType::INDEX_VERTEX)
    {
        pVert += calc_text_geometry_indices(pVert, currChar);
    }

    return pData + (metaData.vertStride * TEXT_VERTS_PER_GLYPH);
}



/*-------------------------------------
 * Calculate a portion of vertex data that a glyph should contain.
-------------------------------------*/
template <typename data_t>
char* SL_TextMeshLoader::set_text_vertex_data(char* const pVert, const data_t& data) noexcept
{
    *reinterpret_cast<data_t* const> (pVert) = data;
    return pVert + metaData.vertStride;
}



/*-------------------------------------
 * Set the index data required by geometry text.
-------------------------------------*/
char* SL_TextMeshLoader::set_text_index_data(
    char* pIndices,
    const unsigned indexOffset
) noexcept
{
    if (metaData.indexType == SL_DataType::VERTEX_DATA_BYTE)
    {
        return fill_geometry_indices<unsigned char>(pIndices, indexOffset);
    }
    else if (metaData.indexType == SL_DataType::VERTEX_DATA_SHORT)
    {
        return fill_geometry_indices<unsigned short>(pIndices, indexOffset);
    }

    return fill_geometry_indices<unsigned int>(pIndices, indexOffset);
}



/*-------------------------------------
 * Text/String Generation
-------------------------------------*/
bool SL_TextMeshLoader::gen_text_geometry(const std::string& str, const SL_Atlas& atlas) noexcept
{
    SL_VertexBuffer& vbo = sceneData.mContext.vbo(0);
    SL_IndexBuffer& ibo = sceneData.mContext.ibo(0);

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
    const ls::utils::Pointer<SL_AtlasGlyph[]>& pGlyphs = atlas.glyphs();

    // The y-origin (starting 'yPos') was found using a lot of testing. This
    // was for resolution independence
    const SL_AtlasGlyph& newline = pGlyphs[u'\n'];
    float yPos = newline.bearing[1] - (newline.bearing[1] - newline.size[1]);
    float xPos = 0.f;
    unsigned charId = 0;
    unsigned indexOffset = 0;

    for (unsigned i = 0; i < str.size(); ++i)
    {
        const unsigned currChar = (unsigned)str[i];
        const SL_AtlasGlyph& rGlyph = pGlyphs[currChar];

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
unsigned SL_TextMeshLoader::allocate_cpu_data(
    const std::string& str,
    const SL_CommonVertType vertexTypes,
    const bool loadBounds
) noexcept
{
    unsigned numSubmeshes = get_num_drawable_chars(str);

    // ignore vertex types which are have no support within the text loader
    metaData.vertAttribs = vertexTypes;
    metaData.vertStride = sl_vertex_stride(metaData.vertAttribs);
    metaData.numVerts = numSubmeshes * SL_TextProperty::TEXT_VERTS_PER_GLYPH;

    metaData.indexType = sl_required_index_type(metaData.numVerts);
    metaData.numIndices = numSubmeshes * SL_TextProperty::TEXT_INDICES_PER_GLYPH;
    metaData.indexStride = sl_bytes_per_type(metaData.indexType);

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
    SL_AlignedVector<SL_Material>& materials = sceneData.mMaterials;
    materials.resize(1);
    sl_reset(materials[0]);

    // Setup the initial text scene graph with some default draw params
    SL_AlignedVector<SL_Mesh>& meshes = sceneData.mMeshes;
    meshes.resize(numSubmeshes);

    for (unsigned m = 0, offset = 0; m < numSubmeshes; ++m, offset += TEXT_INDICES_PER_GLYPH)
    {
        SL_Mesh& mesh = meshes[m];
        sl_reset(mesh);

        mesh.vaoId = 0;
        mesh.elementBegin = offset;
        mesh.elementEnd = offset + SL_TextProperty::TEXT_INDICES_PER_GLYPH;
        mesh.mode = SL_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
        mesh.materialId = 0;
    }

    {
        sceneData.mNodes.push_back(SL_SceneNode{SL_SceneNodeType::NODE_TYPE_MESH, 0, 0, SCENE_NODE_ROOT_ID});
        sceneData.mBaseTransforms.emplace_back(math::mat4{1.f});
        sceneData.mCurrentTransforms.emplace_back(SL_Transform{math::mat4{1.f}, SL_TRANSFORM_TYPE_MODEL});
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

    numBytes += (unsigned)(sizeof(SL_Mesh) * meshes.size());

    if (loadBounds)
    {
        SL_AlignedVector<SL_BoundingBox>& boundsBuffer = sceneData.mMeshBounds;
        boundsBuffer.resize(numSubmeshes);
    }

    return numBytes;
}



/*-------------------------------------
 * GPU Memory Initialization
-------------------------------------*/
int SL_TextMeshLoader::allocate_gpu_data(const SL_Atlas& atlas) noexcept
{
    SL_AlignedVector<SL_Material>& materials = sceneData.mMaterials;
    SL_Material& material = materials.front();
    material.ambient = SL_ColorRGBAf{1.f, 1.f, 1.f, 1.f};
    material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT] = atlas.texture();

    SL_Context& renderData = sceneData.mContext;

    size_t vboId = renderData.create_vbo();
    SL_VertexBuffer& vbo = renderData.vbo(vboId);

    size_t iboId = renderData.create_ibo();
    SL_IndexBuffer& ibo = renderData.ibo(iboId);

    size_t vaoId = renderData.create_vao();
    SL_VertexArray& vao = renderData.vao(vaoId);

    // VBO Allocation
    unsigned vertStride = sl_vertex_byte_size(metaData.vertAttribs);
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
    unsigned numAttribs = sl_count_vertex_attribs(metaData.vertAttribs);
    vao.set_vertex_buffer(0);
    vao.set_index_buffer(0);
    if (vao.set_num_bindings((size_t)numAttribs) != (int)numAttribs)
    {
        return -4;
    }

    // VAO Setup
    for (unsigned i = 0; i < numAttribs; ++i)
    {
        SL_CommonVertType vertType = sl_get_vertex_attrib(metaData.vertAttribs, i);
        unsigned offset = sl_vertex_attrib_offset(metaData.vertAttribs, vertType);
        SL_Dimension dimens = sl_dimens_of_vertex(vertType);
        SL_DataType dataType = sl_type_of_vertex(vertType);
        vao.set_binding(i, offset, vertStride, dimens, dataType);
    }

    return 0;
}



/*-------------------------------------
 * Text/String loading
-------------------------------------*/
unsigned SL_TextMeshLoader::load(
    const std::string& str,
    const SL_Atlas& atlas,
    const SL_TextLoadOpts& opts,
    const bool loadBounds
) noexcept
{
    LS_LOG_MSG("Attempting to load text geometry.");
    unload(); // clear any prior data.

    SL_CommonVertType vertexTypes = SL_CommonVertType::POSITION_VERTEX;
    if (opts.packUvs)
    {
        vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::PACKED_TEXTURE_VERTEX);
    }
    else
    {
        vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::TEXTURE_VERTEX);
    }

    if (opts.genNormals || opts.packNormals || opts.genTangents)
    {
        if (opts.packNormals)
        {
            vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::PACKED_NORMAL_VERTEX);
        }
        else
        {
            vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::NORMAL_VERTEX);
        }

        if (opts.genTangents)
        {
            if (opts.packNormals)
            {
                vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::PACKED_TANGENT_VERTEX);
                vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::PACKED_BITANGENT_VERTEX);
            }
            else
            {
                vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::TANGENT_VERTEX);
                vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::BITANGENT_VERTEX);
            }
        }
    }

    if (opts.genIndexVertex)
    {
        vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::INDEX_VERTEX);
    }

    if (loadBounds)
    {
        vertexTypes = (SL_CommonVertType)((unsigned)vertexTypes | SL_CommonVertType::POSITION_VERTEX);
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
        "\n\t\tCharacters:  ", metaData.numIndices / SL_TextProperty::TEXT_INDICES_PER_GLYPH,
        "\n\t\tVertices:    ", metaData.numVerts,
        "\n\t\tVert Size:   ", metaData.numVerts * sl_vertex_byte_size(metaData.vertAttribs), " bytes",
        "\n\t\tIndices:     ", metaData.numIndices,
        "\n\t\tIndex Size:  ", sl_bytes_per_type(metaData.indexType), " bytes",
        "\n\t\tTotal Size:  ", metaData.numIndices * sl_bytes_per_type(metaData.indexType), " bytes",
        '\n'
    );

    return 0;
}

/*-------------------------------------
 * Text/String loading
-------------------------------------*/
void SL_TextMeshLoader::unload()
{
    sceneData.terminate();
    lineSpacing = (float)DEFAULT_TEXT_LINE_SPACING;
    horizTabSpacing = (float)DEFAULT_TEXT_SPACES_PER_TAB;
    vertTabSpacing = (float)DEFAULT_TEXT_SPACES_PER_TAB;
}
