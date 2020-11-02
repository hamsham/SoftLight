
#ifndef SL_SDF_GENERATOR_HPP
#define SL_SDF_GENERATOR_HPP

#include "softlight/SL_Color.hpp"

typedef int SignedValueType;

#ifndef SDFDataType
    #define SDFDataType SL_COLOR_R_8U
#endif

#ifndef SDFScratchDataType
    #define SDFScratchDataType SL_COLOR_RGBA_32U
#endif



int sl_create_sdf(const SL_Texture& inTex, SL_Texture& outTex, SignedValueType cutoff = 128, SignedValueType amplitude = 3);

int sl_create_sdf(
    const SL_Texture& inTex,
    SL_Texture& outTex,
    SL_Texture& scratchTex,
    SignedValueType cutoff = 128,
    SignedValueType amplitude = 3);



#endif /* SL_SDF_GENERATOR_HPP */
