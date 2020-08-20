
#ifndef SL_TEXT_MESH_LOADER_HPP
#define SL_TEXT_MESH_LOADER_HPP

#include <string>

#include "lightsky/utils/Pointer.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_SceneGraph.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_Atlas;

struct SL_AtlasGlyph;

class SL_IndexBuffer;

class SL_VertexBuffer;

class SL_VertexArray;



/**------------------------------------
 * @brief Mesh Properties
 *
 * These enumerations contains properties for different types of geometry
 * objects. These constants used within contain vital information that's used
 * within the geometry object's implementation.
-------------------------------------*/
enum SL_TextProperty
{
    TEXT_VERTS_PER_GLYPH = 4,

    TEXT_INDICES_PER_GLYPH = 6,

    // TODO: Make this number editable as a member variable.
    DEFAULT_TEXT_SPACES_PER_TAB = 4,

    // TODO: Make this number editable as a member variable.
    DEFAULT_TEXT_LINE_SPACING = 1,
};






/*-----------------------------------------------------------------------------
 * @brief SL_TextLoadOpts Structure
 *
 * This structure can be passed into the text mesh loader to adjust the output
 * data of a scene being loaded.
-----------------------------------------------------------------------------*/
struct SL_TextLoadOpts
{
    // UVs are usually stored in two 32-bit floats. Use this flag to compress
    // UV data into two 16-bit floats.
    bool packUvs;

    // Determine if normals should be generated
    bool genNormals;

    // Implies "genNormals." Vertex normals will be compressed from a
    // math::vec3_t<float> type into an int32_t type. They can be unpacked
    // using either "sl_unpack_vertex_vec3()" or "sl_unpack_vertex_vec4()."
    bool packNormals;

    // Implies "genNormals." This will generate tangents and bitangents
    // for normal mapping.
    bool genTangents;

    // Generate an index ID on every vertex.
    bool genIndexVertex;
};



/**
 * @brief Retrieve the default text loading options.
 *
 * @note The following options are set by default:
 *     packUvs:          FALSE
 *     genNormals:       FALSE
 *     packNormals:      FALSE
 *     genTangents:      FALSE
 *     genIndexVertex:   FALSE
 *
 * @return A SL_SceneLoadOpts structure, containing standard data-modification
 * options which will affect a scene being loaded.
 */
SL_TextLoadOpts sl_default_text_load_opts() noexcept;



/*-----------------------------------------------------------------------------
 * Text Loader Class
-----------------------------------------------------------------------------*/
class SL_TextMeshLoader
{
  public:
    /**
     * @brief Retrieve a count of the number of characters which can be rendered
     * from OpenGL.
     *
     * This function attempts to ignore all whitespace characters that cannot be
     * rendered through OpenGL.
     *
     * @param str
     * A constant reference to a string of text whose characters will be analyzed
     * for renderability.
     *
     * @return An unsigned integral type, containing the number of renderable
     * characters in the input string.
     */
    static unsigned get_num_drawable_chars(const std::string& str) noexcept;

  private:
    struct TextMetaData
    {
        SL_CommonVertType vertAttribs;
        unsigned numAttribs;
        unsigned vertStride;
        unsigned numVerts;

        SL_DataType indexType;
        unsigned numIndices;
        unsigned indexStride;
    };

    TextMetaData metaData;

    SL_SceneGraph sceneData;

    float lineSpacing;

    float horizTabSpacing;

    float vertTabSpacing;

    template <typename data_t>
    char* set_text_vertex_data(char* const pVert, const data_t& data) noexcept;

    char* set_text_index_data(char* pIndices, const unsigned indexOffset) noexcept;

    unsigned calc_text_geometry_pos(const SL_AtlasGlyph& rGlyph, char* pVert, const ls::math::vec2& posOffset, const unsigned charIndex) noexcept;

    unsigned calc_text_geometry_uvs(const SL_AtlasGlyph& rGlyph, char* pVert) noexcept;

    unsigned calc_text_geometry_packed_uvs(const SL_AtlasGlyph& rGlyph, char* pVert) noexcept;

    unsigned calc_text_geometry_norms(char* pVert, const ls::math::vec3& normDir) noexcept;

    unsigned calc_text_geometry_packed_norms(char* pVert, const ls::math::vec3& normDir) noexcept;

    unsigned calc_text_geometry_indices(char* pVert, const unsigned indexId) noexcept;

    char* gen_text_geometry_vert(const SL_AtlasGlyph& rGlyph, char* const pData, const ls::math::vec2& posOffset, const unsigned currChar) noexcept;

    template <typename data_t>
    char* fill_geometry_indices(void* const pIndices, const unsigned indexOffset) noexcept;

    bool gen_text_geometry(const std::string& str, const SL_Atlas& atlas) noexcept;

    unsigned allocate_cpu_data(const std::string& str, const SL_CommonVertType vertexTypes, const bool loadBounds) noexcept;

    int allocate_gpu_data(const SL_Atlas& atlas) noexcept;

  public:
    /**
     * Destructor
     *
     * Clears all CPU-side data from *this. A manual call to "unload()" is
     * required to free GPU-side data.
     */
    ~SL_TextMeshLoader() noexcept;

    /**
     * Constructor
     *
     * Initializes all internal members to their default states.
     */
    SL_TextMeshLoader() noexcept;

    /**
     * Copy Constructor
     *
     * Copies data from the input parameter into *this.
     *
     * @param t
     * An l-value reference to another SL_TextMeshLoader object.
     */
    SL_TextMeshLoader(const SL_TextMeshLoader& t) noexcept;

    /**
     * Move Constructor
     *
     * Moves data from the input parameter into *this.
     *
     * @param t
     * An r-value reference to another SL_TextMeshLoader object.
     */
    SL_TextMeshLoader(SL_TextMeshLoader&& t) noexcept;

    /**
     * Copy Operator
     *
     * Copies data from the input parameter into *this.
     *
     * @param t
     * An l-value reference to another SL_TextMeshLoader object.
     *
     * @return A reference to *this.
     */
    SL_TextMeshLoader& operator=(const SL_TextMeshLoader& t) noexcept;

    /**
     * Move Operator
     *
     * Moves data from the input parameter into *this.
     *
     * @param t
     * An r-value reference to another SL_TextMeshLoader object.
     *
     * @return A reference to *this.
     */
    SL_TextMeshLoader& operator=(SL_TextMeshLoader&& t) noexcept;

    /**
     * @brief Initialize, generate, and emplace a set of textual geometry into an
     * OpenGL VBO and IBO.
     *
     * The winding/index order for all text rendering follows this basic format:
     *
     * 0--------2,3
     * |     /  |
     * |   /    |
     * | /      |
     * 1,4------5
     *
     * @param str
     * A constant reference to a string of text, containing the characters which
     * will be represented by the geometry contained within the vbo and ibo
     * parameters.
     *
     * @param vertexTypes
     * A bitmask, containing a set of vertex types which should be generated by
     * this function.
     *
     * @param atlas
     * A constant reference to an atlas object which contains glyph size and text
     * bitmaps which will be represented by the resulting VBO+IBO objects.
     *
     * @param loadBounds
     * Load the bounding boxes of all glyphs into memory. This only loads
     * bounding boxes on the CPU, not as vertex data on the GPU.
     *
     * @return An unsigned integral type, containing the number of indices which
     * were used to generate the vertex data in the "vbo" parameter.
     */
    unsigned load(const std::string& str, const SL_Atlas& atlas, const SL_TextLoadOpts& opts = sl_default_text_load_opts(),  const bool loadBounds = false) noexcept;

    /**
     * Clear all CPU and GPU data from *this. Reset all internal members to
     * their defaults.
     */
    void unload();

    const SL_SceneGraph& data() const noexcept;

    SL_SceneGraph& data() noexcept;

    void set_spaces_per_horiz_tab(const unsigned numSpaces = SL_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB) noexcept;

    unsigned get_spaces_per_horiz_tab() const noexcept;

    void set_spaces_per_vert_tab(const unsigned numSpaces = SL_TextProperty::DEFAULT_TEXT_SPACES_PER_TAB) noexcept;

    unsigned get_spaces_per_vert_tab() const noexcept;

    void set_line_spacing(const float numSpaces = (float)SL_TextProperty::DEFAULT_TEXT_LINE_SPACING) noexcept;

    float get_line_spacing() const noexcept;
};

/*-------------------------------------
 * Set the index data required by geometry text (helper function).
-------------------------------------*/
template <typename data_t>
char* SL_TextMeshLoader::fill_geometry_indices(void* const pIndices, const unsigned indexOffset) noexcept
{
    data_t* pData = reinterpret_cast<data_t*> (pIndices);

    *(pData++) = indexOffset + 0;
    *(pData++) = indexOffset + 1;
    *(pData++) = indexOffset + 2;
    *(pData++) = indexOffset + 2;
    *(pData++) = indexOffset + 1;
    *(pData++) = indexOffset + 3;

    return reinterpret_cast<char*> (pData);
}

/*-------------------------------------
 * Retrieve the currently loaded mesh (const)
-------------------------------------*/
inline const SL_SceneGraph& SL_TextMeshLoader::data() const noexcept
{
    return sceneData;
}

/*-------------------------------------
 * Retrieve the currently loaded mesh
-------------------------------------*/
inline SL_SceneGraph& SL_TextMeshLoader::data() noexcept
{
    return sceneData;
}

/*-------------------------------------
-------------------------------------*/
inline void SL_TextMeshLoader::set_spaces_per_horiz_tab(const unsigned numSpaces) noexcept
{
    horizTabSpacing = (float)numSpaces;
}

/*-------------------------------------
-------------------------------------*/
inline unsigned SL_TextMeshLoader::get_spaces_per_horiz_tab() const noexcept
{
    return (unsigned)ls::math::floor(horizTabSpacing + 0.5f);
}

/*-------------------------------------
-------------------------------------*/
inline void SL_TextMeshLoader::set_spaces_per_vert_tab(const unsigned numSpaces) noexcept
{
    vertTabSpacing = (float)numSpaces;
}

/*-------------------------------------
-------------------------------------*/
inline unsigned SL_TextMeshLoader::get_spaces_per_vert_tab() const noexcept
{
    return (unsigned)ls::math::floor(vertTabSpacing + 0.5f);
}

/*-------------------------------------
-------------------------------------*/
inline void SL_TextMeshLoader::set_line_spacing(const float numSpaces) noexcept
{
    lineSpacing = numSpaces;
}

/*-------------------------------------
-------------------------------------*/
inline float SL_TextMeshLoader::get_line_spacing() const noexcept
{
    return lineSpacing;
}



#endif /* SL_TEXT_MESH_LOADER_HPP */
