
#include "lightsky/math/vec_utils.h" // vector casting

#include "softlight/SL_Texture.hpp"

#include "sl_sdf_generator.hpp"

namespace math = ls::math;

typedef math::vec4_t<SignedValueType> PointType;

constexpr SignedValueType _SDF_EMPTY_VAL = 9999;
constexpr math::vec2_t<SignedValueType> _SDF_INSIDE_VEC2{ SignedValueType{0}, SignedValueType{0} };
constexpr math::vec2_t<SignedValueType> _SDF_EMPTY_VEC2{ _SDF_EMPTY_VAL, _SDF_EMPTY_VAL };



inline LS_INLINE PointType _sdf_get_texel(PointType* g, int w, int h, int x, int y)
{
    // OPTIMIZATION: you can skip the edge check code if you make your grid
    // have a 1-pixel gutter.
    if (x >= 0 && y >= 0 && x < w && y < h)
    {
        return g[x + w * y];
    }

    return math::vec4_t<SignedValueType>(_SDF_EMPTY_VAL);
}



inline LS_INLINE void _sdf_put_texel(PointType* g, int w, int x, int y, const PointType& p)
{
    g[x + w * y] = p;
}



inline LS_INLINE void _sdf_compare(PointType* g, PointType& p, int w, int h, int x, int y, SignedValueType xOffset, SignedValueType yOffset)
{
    const PointType&& other
        = _sdf_get_texel(g, w, h, x+(int)xOffset, y + (int)yOffset)
        + math::vec4_t<SignedValueType>{xOffset, yOffset, xOffset, yOffset};

    const auto&& o0 = math::vec2_t<SignedValueType>{other[0], other[1]};
    const auto&& p0 = math::vec2_t<SignedValueType>{p[0], p[1]};
    if (math::length_squared<SignedValueType>(o0) < math::length_squared<SignedValueType>(p0))
    {
        p[0] = other[0];
        p[1] = other[1];
    }

    const auto&& o1 = math::vec2_t<SignedValueType>{other[2], other[3]};
    const auto&& p1 = math::vec2_t<SignedValueType>{p[2], p[3]};
    if (math::length_squared<SignedValueType>(o1) < math::length_squared<SignedValueType>(p1))
    {
        p[2] = other[2];
        p[3] = other[3];
    }
}



void _sdf_generate(PointType* g, int w, int h)
{
    // Pass 0
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            PointType&& p = _sdf_get_texel(g, w, h, x, y);
            _sdf_compare(g, p, w, h, x, y, SignedValueType{-1}, SignedValueType{0});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{0},  SignedValueType{-1});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{-1}, SignedValueType{-1});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{1},  SignedValueType{-1});
            _sdf_put_texel(g, w, x, y, p);
        }

        for (int x = w; x--;)
        {
            PointType&& p = _sdf_get_texel(g, w, h, x, y);
            _sdf_compare(g, p, w, h, x, y, SignedValueType{1}, SignedValueType{0});
            _sdf_put_texel(g, w, x, y, p);
        }
    }

    // Pass 1
    for (int y = h; y--;)
    {
        for (int x = w; x--;)
        {
            PointType&& p = _sdf_get_texel(g, w, h, x, y);
            _sdf_compare(g, p, w, h, x, y, SignedValueType{1},  SignedValueType{0});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{0},  SignedValueType{1});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{-1}, SignedValueType{1});
            _sdf_compare(g, p, w, h, x, y, SignedValueType{1},  SignedValueType{1});
            _sdf_put_texel(g, w, x, y, p);
        }

        for (int x = 0; x < w; ++x)
        {
            PointType&& p = _sdf_get_texel(g, w, h, x, y);
            _sdf_compare(g, p, w, h, x, y, SignedValueType{-1}, SignedValueType{0});
            _sdf_put_texel(g, w, x, y, p);
        }
    }
}



int sl_create_sdf(const SL_Texture& inTex, SL_Texture& outTex, SignedValueType cutoff, SignedValueType amplitude)
{

    SL_Texture scratchTex;
    return sl_create_sdf(inTex, outTex, scratchTex, cutoff, amplitude);
}



int sl_create_sdf(
    const SL_Texture& inTex,
    SL_Texture& outTex,
    SL_Texture& scratchTex,
    SignedValueType cutoff,
    SignedValueType amplitude)
{
    if (inTex.type() != SL_COLOR_R_8U)
    {
        return -1;
    }

    const auto resize_buffer = [&inTex](SL_ColorDataType dataType, SL_Texture& tex)->bool
    {
        if (tex.type() != dataType
        || tex.width() != inTex.width()
        || tex.height() != inTex.height())
        {
            tex.terminate();
            if (0 != tex.init(dataType, inTex.width(), inTex.height(), 1))
            {
                return false;
            }
        }

        return true;
    };

    if (!resize_buffer(SDFDataType, outTex))
    {
        return -2;
    }

    if (!resize_buffer(SDFScratchDataType, scratchTex))
    {
        return -3;
    }

    // Initialize the grid
    for (uint16_t y = 0; y < inTex.height(); ++y)
    {
        for (uint16_t x = 0; x < inTex.width(); ++x)
        {
            // Points inside get marked with a dx/dy of zero.
            // Points outside get marked with an infinitely large distance.
            if (inTex.texel<uint8_t>(x, y) < cutoff)
            {
                scratchTex.texel<PointType>(x, y) = math::vec4_cast<SignedValueType>(_SDF_INSIDE_VEC2, _SDF_EMPTY_VEC2);
            }
            else
            {
                scratchTex.texel<PointType>(x, y) = math::vec4_cast<SignedValueType>(_SDF_EMPTY_VEC2, _SDF_INSIDE_VEC2);
            }
        }
    }

    // Generate the SDF.
    _sdf_generate((PointType*)scratchTex.data(), (int)scratchTex.width(), (int)scratchTex.height());

    // Render out the results.
    for (uint16_t y = 0; y < outTex.height(); ++y)
    {
        for (uint16_t x = 0; x < outTex.width(); ++x)
        {
            // Calculate the actual distance from the dx/dy
            const PointType&& p = _sdf_get_texel((PointType*)scratchTex.data(), (int)scratchTex.width(), (int)scratchTex.height(), (int)x, (int)y);
            const math::vec2_t<SignedValueType>&& p1 = {p[0], p[1]};
            const math::vec2_t<SignedValueType>&& p2 = {p[2], p[3]};

            int dist1 = (int)math::length<SignedValueType>(p1);
            int dist2 = (int)math::length<SignedValueType>(p2);
            int dist = dist1 - dist2;

            // Clamp and scale it, just for display purposes.
            int c = math::clamp<int>(dist*amplitude + cutoff, 0, 255);

            outTex.texel<uint8_t>(x, y) = (uint8_t)c;
        }
    }

    return 0;
}
