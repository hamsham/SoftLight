
#ifndef SL_ATLAS_HPP
#define SL_ATLAS_HPP

#include "lightsky/math/vec2.h"

#include "lightsky/utils/Pointer.h"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_FontLoader;
class SL_Context;
class SL_Texture;



/**----------------------------------------------------------------------------
 * @brief SL_AtlasGlyph
 *
 * An SL_AtlasGlyph contains the coordinates of individual glyphs within a texture
 * atlas.
-----------------------------------------------------------------------------*/
struct SL_AtlasGlyph
{
    ls::math::vec2 baseline;

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
 * @brief Texture SL_Atlas Class
 *
 * A texture atlas contains a collection of textures which may be loaded into
 * GPU memory as a single texture.
-----------------------------------------------------------------------------*/
class SL_Atlas
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
    ls::utils::Pointer<SL_AtlasGlyph[]> mEntries;

    /**
     * Texture Rectangle used to store the texture atlas.
     */
    SL_Texture* mAtlasTex;

    /**
     * Initialize the internal texture object using data from a SL_FontLoader.
     *
     * @param context
     * A reference to the context which will contain the texture for *this.
     *
     * @param fr
     * A constant reference to a SL_FontLoader object which contains the data
     * required to initialize a texture on the GPU.
     *
     * @return TRUE if the internal texture was successfully initialized, FALSE
     * if not.
     */
    bool create_texture(SL_Context& context, const SL_FontLoader& fr) noexcept;

    /**
     * Determine the maximum number of glyphs which can be placed along a row
     * of an atlas texture.
     *
     * @param fr
     * A constant reference to a valid SL_FontLoader object containing font
     * glyphs.
     *
     * @return The maximum number of glyphs which can be placed along a row of
     * an atlas texture.
     */
    static int calc_glyph_dimensions(const SL_FontLoader& fr) noexcept;

  public:
    /**
     * @brief Destructor
     *
     * Releases all CPU-side data used by *this.
     */
    ~SL_Atlas() noexcept;

    /**
     * @brief Constructor
     */
    SL_Atlas() noexcept;

    /**
     * @brief Copy constructor
     *
     * Copies data from the input parameter into *this.
     *
     * @param a
     * A constant reference to an SL_Atlas object who's data will be copied
     * into *this (both GPU & CPU data).
     */
    SL_Atlas(const SL_Atlas& a) noexcept;

    /**
     * @brief Move Constructor
     *
     * @param a
     * An r-value reference to a temporary atlas object.
     *
     * Moves all data from the input parameter into *this. No copies are
     * performed.
     */
    SL_Atlas(SL_Atlas&& a) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies data from the input parameter into *this.
     *
     * @param a
     * A constant reference to an SL_Atlas object who's data will be copied
     * into *this (both GPU & CPU data).
     *
     * @return A reference to *this.
     */
    SL_Atlas& operator=(const SL_Atlas&) noexcept;

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
    SL_Atlas& operator=(SL_Atlas&& a) noexcept;

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
    bool init(SL_Context& context, const SL_FontLoader& fr) noexcept;

    /**
     * @brief Free all memory used by a texture atlas.
     *
     * @param a
     * An SL_Atlas object containing both CPU and GPU resources which need to be
     * released from system memory.
     */
    void terminate() noexcept;

    /**
     * Retrieve the texture used by *this.
     *
     * @return A constant reference to the texture object used by *this atlas.
     */
    const SL_Texture* texture() const noexcept;

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
    const ls::utils::Pointer<SL_AtlasGlyph[]>& glyphs() const noexcept;
};

/*-------------------------------------
 * Retrieve the texture used by *this.
-------------------------------------*/
inline const SL_Texture* SL_Atlas::texture() const noexcept
{
    return mAtlasTex;
}

/*-------------------------------------
 * Retrieve the number of glyphs used by *this atlas.
-------------------------------------*/
inline unsigned SL_Atlas::num_glyphs() const noexcept
{
    return mNumEntries;
}

/*-------------------------------------
 * Retrieve the pointer to the list of glyphs used by *this atlas.
-------------------------------------*/
inline const ls::utils::Pointer<SL_AtlasGlyph[]>& SL_Atlas::glyphs() const noexcept
{
    return mEntries;
}



#endif  /* SL_ATLAS_HPP */
