
#include "soft_render/SR_Material.hpp"



/*-------------------------------------
 * Reset all material parameters
-------------------------------------*/
void sr_reset(SR_Material& m) noexcept
{
    for (unsigned i = SR_MaterialProperty::SR_MATERIAL_MAX_TEXTURES; i --> 0;)
    {
        m.pTextures[i] = nullptr;
    }

    m.ambient = SR_ColorRGBf{1.f, 1.f, 1.f};
    m.diffuse = SR_ColorRGBf{1.f, 1.f, 1.f};
    m.specular = SR_ColorRGBf{1.f, 1.f, 1.f};
    m.shininess = 0.f;
}



/*-------------------------------------
 * Material Validation
-------------------------------------*/
SR_MaterialStatus validate(const SR_Material& m) noexcept
{
    for (const SR_Texture* texA : m.pTextures)
    {
        if (!texA)
        {
            return SR_MATERIAL_STATUS_INVALID_TEXTURE;
        }

        for (const SR_Texture* texB : m.pTextures)
        {
            if (texA == texB)
            {
                return SR_MATERIAL_STATUS_DUPLICATE_TEXTURES;
            }
        }
    }

    if (m.ambient.r < 0.f
    || m.ambient.g < 0.f
    || m.ambient.b < 0.f
    || m.diffuse.r < 0.f
    || m.diffuse.g < 0.f
    || m.diffuse.b < 0.f
    || m.specular.r < 0.f
    || m.specular.g < 0.f
    || m.specular.b < 0.f
    || m.shininess < 0.f)
    {
        return SR_MATERIAL_STATUS_VALUE_UNDERFLOW;
    }

    if (m.ambient.r > 1.f
    || m.ambient.g > 1.f
    || m.ambient.b > 1.f
    || m.diffuse.r > 1.f
    || m.diffuse.g > 1.f
    || m.diffuse.b > 1.f
    || m.specular.r > 1.f
    || m.specular.g > 1.f
    || m.specular.b > 1.f
    || m.shininess > 1.f)
    {
        return SR_MATERIAL_STATUS_VALUE_OVERFLOW;
    }

    return SR_MATERIAL_STATUS_VALID;
}
