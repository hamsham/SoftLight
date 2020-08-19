
#ifndef SR_ATLAS_HPP
#define SR_ATLAS_HPP

#include "lightsky/utils/Pointer.h"

#include "lightsky/math/vec2.h"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_FontLoader;
class SR_Context;
class SR_Texture;



/**----------------------------------------------------------------------------
 * @brief SR_AtlasGlyph
 *
 * An SR_AtlasGlyph contains the coordinates of individual glyphs within a texture
 * atlas.
-----------------------------------------------------------------------------*/
struct SR_AtlasGlyph
{
    /**
     * Size corresponds to the vertex width and height of a glyph.
     */
    ls::math::vec2 size;

    /**
     * Advance is mostly used for font kerning
     */
    ls::math::vec2 advance;

    /**
     * Bearing is mostly used for font kerning.
     */
    ls::math::vec2 bearing;

    /**
     * UV is an array representing the top-left and bottom-right portions of a
     * glyph (an element in a texture atlas).
     */
    ls::math::vec2 uv[2];
};



/**----------------------------------------------------------------------------
 * @brief Texture SR_Atlas Class
 *
 * A texture atlas contains a collection of textures which may be loaded into
 * GPU memory as a single texture.
-----------------------------------------------------------------------------*/
class SR_Atlas
{
  private:

    /**
     * @brief mPixelRatio contains the recommended uniform scaling ratio
     * which should be applied to each generated glyph when rendering.
     */
    float mPixelRatio;

    /**
     * The number of entries in a texture atlas.
     */
    uint32_t mNumEntries;

    /**
     * An array of atlas entries. The UVs for these entries is calculated
     * when loading the image data from a texture file.
     */
    ls::utils::Pointer<SR_AtlasGlyph[]> mEntries;

    /**
     * Texture Rectangle used to store the texture atlas.
     */
    SR_Texture* mAtlasTex;

    /**
     * Initialize the internal texture object using data from a SR_FontLoader.
     *
     * @param context
     * A reference to the context which will contain the texture for *this.
     *
     * @param fr
     * A constant reference to a SR_FontLoader object which contains the data
     * required to initialize a texture on the GPU.
     *
     * @return TRUE if the internal texture was successfully initialized, FALSE
     * if not.
     */
    bool create_texture(SR_Context& context, const SR_FontLoader& fr) noexcept;

    /**
     * Determine the maximum number of glyphs which can be placed along a row
     * of an atlas texture.
     *
     * @param fr
     * A constant reference to a valid SR_FontLoader object containing font
     * glyphs.
     *
     * @return The maximum number of glyphs which can be placed along a row of
     * an atlas texture.
     */
    static int calc_glyph_dimensions(const SR_FontLoader& fr) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Releases all CPU-side data used by *this.
     */
    ~SR_Atlas() noexcept;

    /**
     * @brief Constructor
     */
    SR_Atlas() noexcept;

    /**
     * @brief Copy constructor
     *
     * Copies data from the input parameter into *this.
     *
     * @param a
     * A constant reference to an SR_Atlas object who's data will be copied
     * into *this (both GPU & CPU data).
     */
    SR_Atlas(const SR_Atlas& a) noexcept;

    /**
     * @brief Move Constructor
     *
     * @param a
     * An r-value reference to a temporary atlas object.
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed.
     */
    SR_Atlas(SR_Atlas&& a) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies data from the input parameter into *this.
     *
     * @param a
     * A constant reference to an SR_Atlas object who's data will be copied
     * into *this (both GPU & CPU data).
     *
     * @return A reference to *this.
     */
    SR_Atlas& operator=(const SR_Atlas&) noexcept;

    /**
     * @brief Move operator
     *
     * Moves all data from the input parameter into *this.
     *
     * @param a
     * An r-value reference to a temporary atlas object.
     *
     * @return A reference to *this
     */
    SR_Atlas& operator=(SR_Atlas&& a) noexcept;

    /**
     * @brief Store bitmap data from font file into a texture atlas.
     *
     * @param context
     * A reference to the context which will contain the texture for *this.
     *
     * @param fr
     * A fully loaded font resource object.
     *
     * @return True if the data was sent to OpenGL. False if an error
     * occurred.
     */
    bool init(SR_Context& context, const SR_FontLoader& fr) noexcept;

    /**
     * @brief Free all memory used by a texture atlas.
     *
     * @param a
     * An SR_Atlas object containing both CPU and GPU resources which need to be
     * released from system memory.
     */
    void terminate() noexcept;

    /**
     * Retrieve the texture used by *this.
     *
     * @return A constant reference to the texture object used by *this atlas.
     */
    const SR_Texture* texture() const noexcept;

    /**
     * Retrieve the number of glyphs used by *this atlas.
     *
     * @return The number of glyphs currently contained in *this.
     */
    unsigned num_glyphs() const noexcept;

    /**
     * Retrieve the pointer to the list of glyphs used by *this atlas.
     *
     * @return A constant reference to a pointer object which manages the
     * lifetime of *this atlas object's glyphs.
     */
    const ls::utils::Pointer<SR_AtlasGlyph[]>& glyphs() const noexcept;
};

/*-------------------------------------
 * Retrieve the texture used by *this.
-------------------------------------*/
inline const SR_Texture* SR_Atlas::texture() const noexcept
{
    return mAtlasTex;
}

/*-------------------------------------
 * Retrieve the number of glyphs used by *this atlas.
-------------------------------------*/
inline unsigned SR_Atlas::num_glyphs() const noexcept
{
    return mNumEntries;
}

/*-------------------------------------
 * Retrieve the pointer to the list of glyphs used by *this atlas.
-------------------------------------*/
inline const ls::utils::Pointer<SR_AtlasGlyph[]>& SR_Atlas::glyphs() const noexcept
{
    return mEntries;
}



#endif  /* SR_ATLAS_HPP */
