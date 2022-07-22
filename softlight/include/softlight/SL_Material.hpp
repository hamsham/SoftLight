
#ifndef SL_MATERIAL_HPP
#define SL_MATERIAL_HPP

#include <cstdint>

#include "softlight/SL_Color.hpp"



/**----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_Texture;



/**----------------------------------------------------------------------------
 * Material Properties
-----------------------------------------------------------------------------*/
enum SL_MaterialProperty : uint32_t
{
    SL_MATERIAL_TEXTURE_AMBIENT  = 0,
    SL_MATERIAL_TEXTURE_DIFFUSE  = 1,
    SL_MATERIAL_TEXTURE_NORMAL   = 2,
    SL_MATERIAL_TEXTURE_HEIGHT   = 3,
    SL_MATERIAL_TEXTURE_SPECULAR = 4,
    SL_MATERIAL_TEXTURE_OPACITY  = 5,
    SL_MATERIAL_TEXTURE_MISC0    = 6,
    SL_MATERIAL_TEXTURE_MISC1    = 7,

    SL_MATERIAL_MAX_TEXTURES     = 8,
    SL_MATERIAL_INVALID_TEXTURE  = (uint32_t)-1,
};



/**----------------------------------------------------------------------------
 * Material Validation
-----------------------------------------------------------------------------*/
enum SL_MaterialStatus
{
    SL_MATERIAL_STATUS_VALID,
    SL_MATERIAL_STATUS_INVALID_TEXTURE,
    SL_MATERIAL_STATUS_DUPLICATE_TEXTURES,
    SL_MATERIAL_STATUS_VALUE_UNDERFLOW,
    SL_MATERIAL_STATUS_VALUE_OVERFLOW
};



/**----------------------------------------------------------------------------
 * Material Type
-----------------------------------------------------------------------------*/
struct alignas(alignof(float)*4) SL_Material
{
    const SL_Texture* pTextures[SL_MaterialProperty::SL_MATERIAL_MAX_TEXTURES];

    SL_ColorRGBAf ambient;
    SL_ColorRGBAf diffuse;
    SL_ColorRGBAf specular;
    float shininess;
};



/**------------------------------------
 * Reset all material parameters
-------------------------------------*/
void sl_reset(SL_Material& m) noexcept;



/**------------------------------------
 * Validate and retrieve any errors currently found with *this material.
 *
 * This method will look for duplicate textures, samplers, and texture bind
 * slots.
 *
 * @return material_status_t::MATERIAL_STATUS_VALID if *this represents a valid
 * material, otherwise an error from the material_status_t will be
 * returned.
-------------------------------------*/
SL_MaterialStatus validate(const SL_Material& m) noexcept;



#endif /* SL_MATERIAL_HPP */
