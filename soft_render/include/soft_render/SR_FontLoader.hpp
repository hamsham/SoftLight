
#ifndef SR_FONT_LOADER_HPP
#define SR_FONT_LOADER_HPP

#include <string>

#include "lightsky/utils/Resource.h"
#include "lightsky/utils/StringUtils.h"

#include "lightsky/math/vec2.h"


/*-----------------------------------------------------------------------------
    Forward Declarations
-----------------------------------------------------------------------------*/
struct FT_FaceRec_;



/**----------------------------------------------------------------------------
 * @brief Font Metrics
 *
 * This contains the basic data for each individual glyph in a font.
-----------------------------------------------------------------------------*/
struct SR_FontGlyph {
    ls::math::vec2i size = {0};
    ls::math::vec2i advance = {0};
    ls::math::vec2i bearing = {0};
    char* pData = nullptr;

    ~SR_FontGlyph() {
        delete [] pData;
    }
};



/**----------------------------------------------------------------------------
 * @brief Font File loader
 *
 * A font loader can load TTF and OTF fonts by proxy of the FreeType library.
 * When using the getData() method, the returned pointer should be type casted
 * to "const glyph*".
-----------------------------------------------------------------------------*/
class SR_FontLoader final : public ls::utils::Resource {
  public:

    /**
     * Default font size (as DPI) of fonts loaded from a file.
     */
    enum : unsigned {
        FONT_SIZE_LOW = 48,
        FONT_SIZE_MEDIUM = 72,
        FONT_SIZE_HIGH = 96,

        FONT_SIZE_DEFAULT = 72
    };

  private:
    /**
     * Number of loaded glyphs
     */
    unsigned numGlyphs;

    /**
     * Contains the currently loaded font's glyph size
     */
    unsigned glyphSize;

    /**
     * Size of the largest glyph
     */
    ls::math::vec2i maxGlyphSize;

    /**
     * @brief Internal loading function for all glyphs.
     *
     * @param ftFace
     * A pointer to a FreeType font face array. This is used internally to
     * convert True-Type font metrics into a bitmap which can be sent to
     * OpenGL.
     *
     * @return TRUE if a font's glyphs were successfully loaded from a file
     * by FreeType, FALSE if not.
     */
    bool load_glyphs(FT_FaceRec_* ftFace);

  public:
    /**
     * @brief Constructor
     *
     * Initialized all variables to 0 values.
     */
    SR_FontLoader();

    /**
     * Copy Constructor - DELETED
     */
    SR_FontLoader(const SR_FontLoader&) = delete;
    /**
     * @brief Move Constructor
     *
     * Attempts to move all resources from the input argument into *this.
     * No deletions are made.
     *
     * @param fr
     * An r-value reference to another SR_FontLoader object.
     */
    SR_FontLoader(SR_FontLoader&& fr);

    /**
     * @brief Destructor
     *
     * Calls "unload()" and releases all memory used by *this.
     */
    virtual ~SR_FontLoader() override;

    /**
     * @brief Copy Operator - DELETED
     */
    SR_FontLoader& operator=(const SR_FontLoader&) = delete;

    /**
     * @brief Move Operator
     *
     * Attempts to move all resources from the input argument into *this.
     * No copies are made.
     *
     * @param fr
     * An r-value reference to another SR_FontLoader object.
     *
     * @return a reference to *this.
     */
    SR_FontLoader& operator=(SR_FontLoader&& fr);

    /**
     * @brief Load a font file
     *
     * This method will load a font file using a specified pixel size.
     *
     * @param filename
     * A c-style string containing the relative path name
     * to a file that should be loaded into memory.
     *
     * @param pixelSize
     * The size, in pixels, that each glyph should be.
     *
     * @return true if the font was successfully loaded. False if not.
     */
    bool load_file(const std::string& filename, unsigned pixelSize);

    /**
     * @brief Load a font file
     *
     * This method will load a font file using a specified pixel size.
     *
     * @param filename
     * A c-style string containing the relative path name
     * to a file that should be loaded into memory.
     *
     * @param pixelSize
     * the size, in pixels, that each glyph should be.
     *
     * @return true if the font was successfully loaded. False if not.
     */
    bool load_file(const std::wstring& filename, unsigned pixelSize);

    /**
     * @brief Load a font file
     *
     * This method will load a font file using the default font size.
     *
     * @param filename
     * A c-style string containing the relative path name
     * to a file that should be loaded into memory.
     *
     * @return true if the font was successfully loaded. False if not.
     */
    virtual bool load_file(const std::string& filename) override;

    /**
     * @brief Load a font file
     *
     * This method will load a font file using the default font size.
     *
     * @param filename
     * A c-style string containing the relative path name
     * to a file that should be loaded into memory.
     *
     * @return true if the font was successfully loaded. False if not.
     */
    virtual bool load_file(const std::wstring& filename) override;

    /**
     * @brief Save a file
     *
     * This method will save a font to a file indicated by the specified
     * path name.
     *
     * @param filename
     * A string object containing the relative path name to a file that
     * should be saved to the computer.
     *
     * @return true if the file was successfully saved. False if not.
     */
    virtual bool save_file(const std::string& filename) const override;

    /**
     * @brief Unload
     *
     * Free all memory used by *this.
     */
    virtual void unload() override;

    /**
     * Get the size of the currently loaded fonts
     */
    unsigned get_font_size() const;

    /**
     * Get the array of glyphs used by *this
     */
    const SR_FontGlyph* get_glyphs() const;

    /**
     * Get the number of glyphs that have been loaded
     */
    unsigned get_num_glyphs() const;

    /**
     * Get the size of the widest glyph
     */
    ls::math::vec2i get_max_glyph_size() const;
};

/*-------------------------------------
 * Load a font file using the default font size.
-------------------------------------*/
inline bool SR_FontLoader::load_file(const std::wstring& filename, unsigned pixelSize) {
    return load_file(ls::utils::to_str(filename), pixelSize);
}

/*-------------------------------------
 * Load a font file using the default font size.
-------------------------------------*/
inline bool SR_FontLoader::load_file(const std::wstring& filename) {
    return load_file(ls::utils::to_str(filename));
}

/*-------------------------------------
 * Load a font file using the default font size.
-------------------------------------*/
inline bool SR_FontLoader::load_file(const std::string& filename) {
    return load_file(filename, FONT_SIZE_DEFAULT);
}

/*-------------------------------------
 * Get the size of the currently loaded fonts
-------------------------------------*/
inline unsigned SR_FontLoader::get_font_size() const {
    return glyphSize;
}

/*-------------------------------------
 * Get the array of glyphs used by *this
-------------------------------------*/
inline const SR_FontGlyph* SR_FontLoader::get_glyphs() const {
    return reinterpret_cast<SR_FontGlyph*> (pData);
}

/*-------------------------------------
 * Get the number of glyphs that have been loaded
-------------------------------------*/
inline unsigned SR_FontLoader::get_num_glyphs() const {
    return numGlyphs;
}

/*-------------------------------------
 * Get the size of the widest glyph
-------------------------------------*/
inline ls::math::vec2i SR_FontLoader::get_max_glyph_size() const {
    return maxGlyphSize;
}



#endif  /* SR_FONT_LOADER_HPP */
