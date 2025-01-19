
#include <algorithm> // std::copy
#include <utility> // std::move

#include "lightsky/utils/Log.h"

#include "lightsky/math/scalar_utils.h"

#include "softlight/SL_Atlas.hpp"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_FontLoader.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_VertexBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Namespace Setup
-----------------------------------------------------------------------------*/
namespace utils = ls::utils;
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_Atlas classs
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_Atlas::~SL_Atlas() noexcept
{
    terminate();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_Atlas::SL_Atlas() noexcept :
    mPixelRatio{1.f},
    mNumEntries{0},
    mEntries{nullptr},
    mAtlasTex{nullptr}
{
}



/*-------------------------------------
 * Font SL_Atlas Copy Constructor
-------------------------------------*/
SL_Atlas::SL_Atlas(const SL_Atlas& a) noexcept :
    mPixelRatio{a.mPixelRatio},
    mNumEntries{a.mNumEntries},
    mEntries{a.mEntries ? new(std::nothrow) SL_AtlasGlyph[a.mNumEntries] : nullptr},
    mAtlasTex{a.mAtlasTex}
{
    if (!mEntries)
    {
        terminate();
        return;
    }

    for (unsigned i = 0; i < a.mNumEntries; ++i)
    {
        mEntries[i] = a.mEntries[i];
    }
}



/*-------------------------------------
 * Font SL_Atlas Move constructor
-------------------------------------*/
SL_Atlas::SL_Atlas(SL_Atlas&& a) noexcept :
    mPixelRatio{a.mPixelRatio},
    mNumEntries{a.mNumEntries},
    mEntries{std::move(a.mEntries)},
    mAtlasTex{a.mAtlasTex}
{
    a.mNumEntries = 0;
    a.mPixelRatio = 1.f;
    a.mAtlasTex = nullptr;
}



/*-------------------------------------
 * Font SL_Atlas Copy Operator
-------------------------------------*/
SL_Atlas& SL_Atlas::operator=(const SL_Atlas& a) noexcept
{
    if (mEntries)
    {
        terminate();
    }

    if (!a.mEntries || !a.mNumEntries)
    {
        return *this;
    }

    mPixelRatio = a.mPixelRatio;
    mNumEntries = a.mNumEntries;
    mEntries.reset(new(std::nothrow) SL_AtlasGlyph[mNumEntries]);
    mAtlasTex = a.mAtlasTex;

    if (!mEntries)
    {
        terminate();
        return *this;
    }

    for (unsigned i = 0; i < a.mNumEntries; ++i)
    {
        mEntries[i] = a.mEntries[i];
    }

    return *this;
}



/*-------------------------------------
 * Font SL_Atlas move operator
-------------------------------------*/
SL_Atlas& SL_Atlas::operator=(SL_Atlas&& a) noexcept
{
    mPixelRatio = a.mPixelRatio;
    a.mPixelRatio = 1.f;

    mNumEntries = a.mNumEntries;
    a.mNumEntries = 0;

    mEntries = std::move(a.mEntries);

    mAtlasTex = a.mAtlasTex;
    a.mAtlasTex = nullptr;

    return *this;
}



/*-------------------------------------
 * Calculate the dimensions each glyph's X & Y size
-------------------------------------*/
int SL_Atlas::calc_glyph_dimensions(const SL_FontLoader& fr) noexcept
{
    return (int)math::fast_sqrt((float)fr.get_num_glyphs());
}



/*-------------------------------------
 * Create the texture object which will contain the atlas
-------------------------------------*/
bool SL_Atlas::create_texture(SL_Context& context, const SL_FontLoader& fr) noexcept
{
    // Calculate the size of the atlas.
    const int dimensions = calc_glyph_dimensions(fr);
    const math::vec2i&& maxGlyphSize = fr.get_max_glyph_size() * dimensions;

    // 8 bits per pixel.
    size_t texId = context.create_texture();
    mAtlasTex = &(context.texture(texId));
    return mAtlasTex && (0 == mAtlasTex->init(SL_COLOR_R_8U, (uint16_t)(maxGlyphSize[0]), (uint16_t)(maxGlyphSize[1])));
}



/*-------------------------------------
 * Store a fontFile's texture data on OpenGL server memory
-------------------------------------*/
bool SL_Atlas::init(SL_Context& context, const SL_FontLoader& fr) noexcept
{
    terminate();

    LS_LOG_MSG("Attempting to load a font atlas.");

    // prepare the array of atlas entries
    mEntries.reset(new(std::nothrow) SL_AtlasGlyph[fr.get_num_glyphs()]);
    if (mEntries == nullptr)
    {
        LS_LOG_ERR("\tUnable to generate an array of font atlas entries.\n");
        return false;
    }

    if (!create_texture(context, fr))
    {
        LS_LOG_ERR("\tAn error occurred while allocating space for a font atlas.\n");
        mEntries.reset();
        return false;
    }

    // Calculate the size of the atlas.
    mPixelRatio = 1.f / (float)fr.get_font_size(); // DPI scaling
    mNumEntries = fr.get_num_glyphs();

    const math::vec2i&& maxGlyphSize  = fr.get_max_glyph_size();
    const SL_FontGlyph* pGlyphs       = fr.get_glyphs();
    const int           dimensions    = calc_glyph_dimensions(fr);
    const float         fDimensionInv = 1.f / (float)dimensions;
    const math::vec2i&& atlasSize     = math::vec2i{(int)mAtlasTex->width(), (int)mAtlasTex->height()};
    const math::vec2&&  texResolution = (math::vec2)atlasSize;

    for (int x = 0, glyphIndex = 0; x < dimensions; ++x)
    {
        for (int y = 0; y < dimensions; ++y)
        {
            const SL_FontGlyph& pGlyph = pGlyphs[glyphIndex];
            SL_AtlasGlyph& pEntry = mEntries[glyphIndex];

            // Add normalized position/sizing data for each glyph.
            pEntry.baseline = fDimensionInv * (math::vec2)pGlyph.baseline;
            pEntry.size = fDimensionInv * (math::vec2)pGlyph.size;
            pEntry.advance = fDimensionInv * (math::vec2)pGlyph.advance;
            pEntry.bearing = fDimensionInv * (math::vec2)pGlyph.bearing;

            // Upload glyph data
            const int texPosX = x * maxGlyphSize[0];
            const int texPosY = y * maxGlyphSize[1];
            mAtlasTex->set_texels((uint16_t)texPosX, (uint16_t)texPosY, 0, (uint16_t)pGlyph.size[0], (uint16_t)pGlyph.size[1], 1, pGlyph.pData);

            // top-left & bottom right glyph corner
            pEntry.uv[0] = (math::vec2)math::vec2i{maxGlyphSize[0] * x, maxGlyphSize[1] * y};
            pEntry.uv[1] = pEntry.uv[0] + (math::vec2)pGlyph.size;

            // normalize UV coordinates to within (0, 1)
            pEntry.uv[0] /= texResolution;
            pEntry.uv[1] /= texResolution;

            // next glyph
            ++glyphIndex;
        }
    }

    LS_LOG_MSG(
        "\tSuccessfully loaded a font atlas.",
        "\n\t\tTotal Resolution:   ", texResolution[0], 'x', texResolution[1],
        "\n\t\tGlyphs Per Row/Col: ", dimensions, " x ", dimensions,
        "\n\t\tTotal Glyph Count:  ", fr.get_num_glyphs(),
        "\n\t\tWidth Per Glyph:    ", maxGlyphSize[0],
        "\n\t\tHeight Per Glyph:   ", maxGlyphSize[1],
        '\n'
    );

    return true;
}

/*-------------------------------------
 * Frees all memory used by *this.
-------------------------------------*/
void SL_Atlas::terminate() noexcept
{
    mPixelRatio = 1.f;
    mNumEntries = 0;
    mEntries.reset();

    if (mAtlasTex)
    {
        mAtlasTex = nullptr;
    }
}
