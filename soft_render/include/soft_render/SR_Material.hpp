
#ifndef SR_MATERIAL_HPP
#define SR_MATERIAL_HPP

#include <cstdint>

#include "soft_render/SR_Color.hpp"



/**----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_Texture;



/**----------------------------------------------------------------------------
 * Material Properties
-----------------------------------------------------------------------------*/
enum SR_MaterialProperty : uint32_t
{
    SR_MATERIAL_INVALID_TEXTURE = (uint32_t)-1,
    SR_MATERIAL_MAX_TEXTURES = 8
};



/**----------------------------------------------------------------------------
 * Material Validation
-----------------------------------------------------------------------------*/
enum SR_MaterialStatus
{
    SR_MATERIAL_STATUS_VALID,
    SR_MATERIAL_STATUS_INVALID_TEXTURE,
    SR_MATERIAL_STATUS_DUPLICATE_TEXTURES,
    SR_MATERIAL_STATUS_VALUE_UNDERFLOW,
    SR_MATERIAL_STATUS_VALUE_OVERFLOW
};



/**----------------------------------------------------------------------------
 * Material Type
-----------------------------------------------------------------------------*/
struct SR_Material
{
    const SR_Texture* pTextures[SR_MaterialProperty::SR_MATERIAL_MAX_TEXTURES];

    SR_ColorRGBf ambient;
    SR_ColorRGBf diffuse;
    SR_ColorRGBf specular;
    float shininess;
};



/**------------------------------------
 * Reset all material parameters
-------------------------------------*/
void sr_reset(SR_Material& m) noexcept;



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
SR_MaterialStatus validate(const SR_Material& m) noexcept;



#endif /* SR_MATERIAL_HPP */
