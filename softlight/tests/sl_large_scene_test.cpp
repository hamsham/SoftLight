
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Log.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Config.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_Plane.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_WindowBuffer.hpp"
#include "softlight/SL_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */

#ifndef SL_TEST_MAX_THREADS
    #define SL_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SL_TEST_MAX_THREADS */

#ifndef SL_BENCHMARK_SCENE
    #define SL_BENCHMARK_SCENE 0
#endif /* SL_BENCHMARK_SCENE */

#ifndef SL_TEST_BUMP_MAPS
    #define SL_TEST_BUMP_MAPS 0
#endif /* SL_TEST_BUMP_MAPS */

#ifndef TEST_REVERSED_DEPTH
    #define TEST_REVERSED_DEPTH 1
#endif

namespace math = ls::math;
namespace utils = ls::utils;

template <typename... data_t>
using Tuple = utils::Tuple<data_t...>;



/*-----------------------------------------------------------------------------
 * Structures to create uniform variables shared across all shader stages.
-----------------------------------------------------------------------------*/
struct Light
{
    math::vec4 pos;
    math::vec4 ambient;
    math::vec4 diffuse;
};



struct PointLight
{
    float constant;
    float linear;
    float quadratic;
    float padding;
};



struct MeshUniforms
{
    const SL_Texture* pTexture;

#if SL_TEST_BUMP_MAPS
    const SL_Texture* pBump;
#endif

    math::vec4 camPos;
    Light light;
    PointLight point;

    math::mat4 modelMatrix;
    math::mat4 mvpMatrix;
};



/*-----------------------------------------------------------------------------
 * PBR Helper functions
-----------------------------------------------------------------------------*/
// Calculate the metallic component of a surface
template <class vec_type = math::vec4>
inline vec_type fresnel_schlick(float cosTheta, const vec_type& surfaceReflection)
{
    return math::fmadd(vec_type{1.f} - surfaceReflection, vec_type{math::pow(1.f - cosTheta, 5.f)}, surfaceReflection);
}



// normal distribution function within a hemisphere
template <class vec_type = math::vec4>
inline float distribution_ggx(const vec_type& norm, const vec_type& hemisphere, float roughness)
{
    float roughSquared = roughness * roughness;
    float roughQuad    = roughSquared * roughSquared;
    float nDotH        = math::max(math::dot(norm, hemisphere), 0.f);
    float nDotH2       = nDotH * nDotH;
    float distribution = nDotH2 * (roughQuad - 1.f) + 1.f;

    return nDotH2 / (LS_PI * distribution  * distribution);
}



// Determine how a surface's roughness affects how it reflects light
inline float geometry_schlick_ggx(float normDotView, float roughness)
{
    roughness += 1.f;
    roughness = (roughness*roughness) * 0.125f; // 1/8

    float geometry = normDotView * (1.f - roughness) + roughness;
    return normDotView / geometry;
}



// PBR Geometry function for determining how light bounces off a surface
template <class vec_type = math::vec4>
inline float geometry_smith(const vec_type& norm, const vec_type& viewDir, const vec_type& radiance, float roughness)
{
    float normDotView = math::max(math::dot(norm, viewDir), 0.f);
    float normDotRad = math::max(math::dot(norm, radiance), 0.f);

    return geometry_schlick_ggx(normDotView, roughness) * geometry_schlick_ggx(normDotRad, roughness);
}



/*-----------------------------------------------------------------------------
 * Bump Mapping Helper functions
-----------------------------------------------------------------------------*/
#if SL_TEST_BUMP_MAPS
inline LS_INLINE math::vec4 bumped_normal(const SL_Texture* bumpMap, const math::vec4& uv) noexcept
{
    const float stepX = 1.f / (float)bumpMap->width();
    const float stepY = 1.f / (float)bumpMap->height();

    math::vec4_t<uint8_t> b;
    b[0] = sl_sample_nearest<SL_ColorRType<uint8_t>, SL_WrapMode::REPEAT>(*bumpMap, uv[0],       uv[1]).r;
    b[1] = sl_sample_nearest<SL_ColorRType<uint8_t>, SL_WrapMode::REPEAT>(*bumpMap, uv[0]+stepX, uv[1]).r;
    b[2] = sl_sample_nearest<SL_ColorRType<uint8_t>, SL_WrapMode::REPEAT>(*bumpMap, uv[0],       uv[1]+stepY).r;
    b[3] = 0;

    return color_cast<float, uint8_t>(b) * 2.f - 1.f;
}
#endif



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _normal_vert_shader_impl(SL_VertexParam& param)
{
    // Used to retrieve packed verex data in a single call
    typedef Tuple<math::vec3, int32_t> Vertex;

    const MeshUniforms* pUniforms = param.pUniforms->as<MeshUniforms>();
    const Vertex*       v         = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert      = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&  norm      = sl_unpack_vertex_vec4(v->const_element<1>());

    param.pVaryings[0] = pUniforms->modelMatrix * vert;
    param.pVaryings[1] = pUniforms->modelMatrix * norm;

    return pUniforms->mvpMatrix * vert;
}



SL_VertexShader normal_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode    = SL_CULL_BACK_FACE;
    shader.shader      = _normal_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _normal_frag_shader_impl(SL_FragmentParam& fragParams)
{
    const MeshUniforms* pUniforms  = fragParams.pUniforms->as<MeshUniforms>();
    const math::vec4&    pos       = fragParams.pVaryings[0];
    math::vec4&&         norm      = math::normalize(fragParams.pVaryings[1]);
    float                attenuation;
    math::vec4           diffuse;
    float                specular;

    constexpr float diffuseMultiplier = 4.f;
    constexpr float specularity = 0.5f;
    constexpr float shininess = 50.f;

    // Light direction calculation
    const Light& l         = pUniforms->light;
    math::vec4&& lightDir  = l.pos - pos;
    const float  lightDist = math::length(lightDir);

    // normalize
    lightDir = lightDir * math::rcp(lightDist);

    const math::vec4 ambient = l.ambient;

    // Diffuse light calculation
    {
        const PointLight& p    = pUniforms->point;
        const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);
        const float constant   = p.constant;
        const float linear     = p.linear;
        const float quadratic  = p.quadratic;

        attenuation = math::rcp(constant + (linear * lightDist) + (quadratic * lightDist * lightDist));
        diffuse     = l.diffuse * (lightAngle * attenuation) * diffuseMultiplier;
    }

    // specular reflection calculation
    {
        const math::vec4&  eyeVec     = math::normalize(pUniforms->camPos - pos);
        const math::vec4&& halfVec    = math::normalize(lightDir + eyeVec);
        const float        reflectDir = math::max(math::dot(norm, halfVec), 0.f);

        specular = specularity * math::pow(reflectDir, shininess);
    }

    // output composition
    {
        const math::vec4&& accumulation = math::min(diffuse+specular+ambient, math::vec4{1.f});

        fragParams.pOutputs[0] = accumulation;
    }

    return true;
}



bool _normal_frag_shader_pbr(SL_FragmentParam& fragParams)
{
    const MeshUniforms* pUniforms  = fragParams.pUniforms->as<MeshUniforms>();
    const math::vec4     pos       = fragParams.pVaryings[0];
    const math::vec4     norm      = math::normalize(fragParams.pVaryings[1]);
    math::vec4&          output    = fragParams.pOutputs[0];

    // surface model
    const math::vec4     camPos           = pUniforms->camPos;
    const math::vec4     viewDir          = math::normalize(camPos - pos);
    const math::vec4     lightPos         = pUniforms->light.pos;
    const math::vec4     albedo           = math::vec4{1.f};
    constexpr float      metallic         = 0.8f;
    constexpr float      roughness        = 0.025f;
    constexpr float      ambientIntensity = 0.5f;
    constexpr float      diffuseIntensity = 50.f;

    // Metallic reflectance at a normal incidence
    // 0.04f should be close to plastic.
    const math::vec4   surfaceConstant   = {0.875f, 0.875f, 0.875f, 1.f};
    const math::vec4&& surfaceReflection = math::mix(surfaceConstant, albedo, metallic);

    math::vec4         lightDir0         = {0.f};

    math::vec4         lightDirN         = lightPos - pos;
    const float        distance          = math::length(lightDirN);
    lightDirN                            = lightDirN * math::rcp(distance); // normalize
    math::vec4         hemisphere        = math::normalize(viewDir + lightDirN);
    const float        attenuation       = math::rcp(distance);
    math::vec4         radianceObj       = pUniforms->light.diffuse * attenuation * diffuseIntensity;

    const float        ndf               = distribution_ggx(norm, hemisphere, roughness);
    const float        geom              = geometry_smith(norm, viewDir, lightDirN, roughness);
    const math::vec4   fresnel           = fresnel_schlick(math::max(math::dot(hemisphere, viewDir), 0.f), surfaceReflection);

    const math::vec4   brdf              = fresnel * ndf * geom;
    const float        cookTorrance      = 4.f * math::max(math::dot(norm, viewDir), 0.f) * math::max(math::dot(norm, lightDirN), 0.f) + LS_EPSILON;  // avoid divide-by-0
    const math::vec4   specular          = brdf * math::rcp(cookTorrance);

    const math::vec4&  specContrib       = fresnel;
    const math::vec4   refractRatio      = (math::vec4{1.f} - specContrib) * (math::vec4{1.f} - metallic);

    const float normDotLight             = math::max(math::dot(lightDirN, norm), 0.f);
    lightDir0                            += (refractRatio * albedo * LS_PI_INVERSE + specular) * radianceObj * normDotLight;

    const math::vec4   ambient           = pUniforms->light.ambient * ambientIntensity;

    // Color normalization and light contribution
    math::vec4 outRGB = albedo * (ambient + lightDir0);

    // Tone mapping
    //outRGB *= math::rcp(outRGB + math::vec4{1.f, 1.f, 1.f, 0.f});

    // HDR Tone mapping
    const float exposure = 4.f;
    outRGB = math::vec4{1.f} - math::exp(-outRGB * exposure);
    outRGB[3] = 1.f;

    // Gamma correction
    //const math::vec4 gamma = {1.f / 2.2f};
    //outRGB = math::clamp(math::pow(outRGB, gamma), math::vec4{0.f}, math::vec4{1.f});
    //outRGB[3] = 1.f;

    output = outRGB;

    return true;
}



SL_FragmentShader normal_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;

    #ifdef TEST_REVERSED_DEPTH
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    #else
    shader.depthTest = SL_DEPTH_TEST_LESS_EQUAL;
    #endif

    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _normal_frag_shader_impl;

    return shader;
}



SL_FragmentShader normal_frag_shader_pbr()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;

    #ifdef TEST_REVERSED_DEPTH
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    #else
    shader.depthTest = SL_DEPTH_TEST_LESS_EQUAL;
    #endif

    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _normal_frag_shader_pbr;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(SL_VertexParam& param)
{
    // Used to retrieve packed verex data in a single call
    typedef Tuple<math::vec3, math::vec2, int32_t> Vertex;

    const MeshUniforms* pUniforms = param.pUniforms->as<MeshUniforms>();
    const Vertex*       v         = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert      = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&  uv        = math::vec4_cast(v->const_element<1>(), 0.f, 0.f);
    const math::vec4&&  norm      = sl_unpack_vertex_vec4(v->const_element<2>());

    param.pVaryings[0] = pUniforms->modelMatrix * vert;
    param.pVaryings[1] = uv;
    param.pVaryings[2] = pUniforms->modelMatrix * norm;

    return pUniforms->mvpMatrix * vert;
}



SL_VertexShader texture_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader_spot(SL_FragmentParam& fragParams)
{
    const MeshUniforms* pUniforms  = fragParams.pUniforms->as<MeshUniforms>();
    const math::vec4&    pos       = fragParams.pVaryings[0];
    const math::vec4&    uv        = fragParams.pVaryings[1];
    math::vec4&&         norm      = math::normalize(fragParams.pVaryings[2]);
    const SL_Texture*    albedo    = pUniforms->pTexture;
    float                attenuation;
    math::vec4           pixel;
    math::vec4           diffuse;
    float                specular;

    constexpr float diffuseMultiplier = 4.f;
    constexpr float specularity = 0.5f;
    constexpr float shininess = 50.f;

    // normalize the texture colors to within (0.f, 1.f)
    if (albedo->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = sl_sample_nearest<math::vec3_t<uint8_t>, SL_WrapMode::REPEAT>(*albedo, uv[0], uv[1]);
        pixel = color_cast<float, uint8_t>(math::vec4_cast<uint8_t>(pixel8, 255));
    }
    else
    {
        pixel = color_cast<float, uint8_t>(sl_sample_nearest<math::vec4_t<uint8_t>, SL_WrapMode::REPEAT>(*albedo, uv[0], uv[1]));
    }

    #if SL_TEST_BUMP_MAPS
        const SL_Texture* bumpMap = pUniforms->pBump;
        if (bumpMap)
        {
            const math::vec4&& bumpedNorm = bumped_normal(bumpMap, uv);
            norm = math::normalize(norm * bumpedNorm);
        }
    #endif

    // Light direction calculation
    const Light& l         = pUniforms->light;
    math::vec4&& lightDir  = l.pos - pos;
    const float  lightDist = math::length(lightDir);

    // normalize
    lightDir = lightDir * math::rcp(lightDist);

    const math::vec4 ambient = l.ambient;

    // Diffuse light calculation
    {
        const PointLight& p    = pUniforms->point;
        const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);
        const float constant   = p.constant;
        const float linear     = p.linear;
        const float quadratic  = p.quadratic;

        attenuation = math::rcp(constant + (linear * lightDist) + (quadratic * lightDist * lightDist));
        diffuse     = l.diffuse * (lightAngle * attenuation) * diffuseMultiplier;
    }

    // gamma corection
    pixel = math::pow(pixel, math::vec4{2.2f});

    // specular reflection calculation
    {
        const math::vec4&  eyeVec     = math::normalize(pUniforms->camPos - pos);
        const math::vec4&& halfVec    = math::normalize(lightDir + eyeVec);
        const float        reflectDir = math::max(math::dot(norm, halfVec), 0.f);

        specular = specularity * math::pow(reflectDir, shininess);
    }

    // output composition
    {
        const math::vec4&& accumulation = math::min(diffuse+specular+ambient, math::vec4{1.f});

        fragParams.pOutputs[0] = pixel * accumulation;
    }

    return true;
}



bool _texture_frag_shader_pbr(SL_FragmentParam& fragParams)
{
    const MeshUniforms* pUniforms  = fragParams.pUniforms->as<MeshUniforms>();
    const math::vec4     pos       = fragParams.pVaryings[0];
    const math::vec4     uv        = fragParams.pVaryings[1];
    math::vec4&&         norm      = math::normalize(fragParams.pVaryings[2]);
    math::vec4&          output    = fragParams.pOutputs[0];
    const SL_Texture*    pTexture  = pUniforms->pTexture;
    math::vec4           pixel;

    // normalize the texture colors to within (0.f, 1.f)
    if (pTexture->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = sl_sample_nearest<math::vec3_t<uint8_t>, SL_WrapMode::REPEAT>(*pTexture, uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        pixel = color_cast<float, uint8_t>(pixelF);
    }
    else
    {
        const math::vec4_t<uint8_t>&& pixelF = sl_sample_nearest<math::vec4_t<uint8_t>, SL_WrapMode::REPEAT>(*pTexture, uv[0], uv[1]);
        pixel = color_cast<float, uint8_t>(pixelF);
    }

    #if SL_TEST_BUMP_MAPS
        const SL_Texture* bumpMap = pUniforms->pBump;
        if (bumpMap)
        {
            const math::vec4&& bumpedNorm = bumped_normal(bumpMap, uv);
            norm = math::normalize(norm * bumpedNorm);
        }
    #endif

    // gamma correction
    pixel = math::pow(pixel, math::vec4{2.2f});

    // surface model
    const math::vec4     camPos           = pUniforms->camPos;
    const math::vec4     viewDir          = math::normalize(camPos - pos);
    const math::vec4     lightPos         = pUniforms->light.pos;
    const math::vec4     albedo           = pixel;
    constexpr float      metallic         = 0.4f;
    constexpr float      roughness        = 0.35f;
    constexpr float      ambientIntensity = 0.5f;
    constexpr float      diffuseIntensity = 50.f;

    // Metallic reflectance at a normal incidence
    // 0.04f should be close to plastic.
    const math::vec4   surfaceConstant   = {0.4f, 0.4f, 0.4f, 1.f};
    const math::vec4&& surfaceReflection = math::mix(surfaceConstant, albedo, metallic);

    math::vec4         lightDir0         = {0.f};

    math::vec4         lightDirN         = lightPos - pos;
    const float        distance          = math::length(lightDirN);
    lightDirN                            = lightDirN * math::rcp(distance); // normalize
    math::vec4         hemisphere        = math::normalize(viewDir + lightDirN);
    const float        attenuation       = math::rcp(distance);
    math::vec4         radianceObj       = pUniforms->light.diffuse * attenuation * diffuseIntensity;

    const float        ndf               = distribution_ggx(norm, hemisphere, roughness);
    const float        geom              = geometry_smith(norm, viewDir, lightDirN, roughness);
    const math::vec4   fresnel           = fresnel_schlick(math::max(math::dot(hemisphere, viewDir), 0.f), surfaceReflection);

    const math::vec4   brdf              = fresnel * ndf * geom;
    const float        cookTorrance      = 4.f * math::max(math::dot(norm, viewDir), 0.f) * math::max(math::dot(norm, lightDirN), 0.f) + LS_EPSILON;  // avoid divide-by-0
    const math::vec4   specular          = brdf * math::rcp(cookTorrance);

    const math::vec4&  specContrib       = fresnel;
    const math::vec4   refractRatio      = (math::vec4{1.f} - specContrib) * (math::vec4{1.f} - metallic);

    const float normDotLight             = math::max(math::dot(lightDirN, norm), 0.f);
    lightDir0                            += (refractRatio * albedo * LS_PI_INVERSE + specular) * radianceObj * normDotLight;

    const math::vec4   ambient           = pUniforms->light.ambient * ambientIntensity;

    // Color normalization and light contribution
    math::vec4 outRGB = albedo * (ambient + lightDir0);

    // Tone mapping
    //outRGB *= math::rcp(outRGB + math::vec4{1.f, 1.f, 1.f, 0.f});

    // HDR Tone mapping
    const float exposure = 4.f;
    outRGB = math::vec4{1.f} - math::exp(-outRGB * exposure);
    outRGB[3] = 1.f;

    // Gamma correction
    //const math::vec4 gamma = {1.f / 2.2f};
    //outRGB = math::clamp(math::pow(outRGB, gamma), math::vec4{0.f}, math::vec4{1.f});
    //outRGB[3] = 1.f;

    output = outRGB;

    return true;
}



SL_FragmentShader texture_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;

    #ifdef TEST_REVERSED_DEPTH
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    #else
    shader.depthTest = SL_DEPTH_TEST_LESS_EQUAL;
    #endif

    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader_spot;

    return shader;
}



SL_FragmentShader texture_frag_shader_pbr()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;

    #ifdef TEST_REVERSED_DEPTH
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    #else
    shader.depthTest = SL_DEPTH_TEST_LESS_EQUAL;
    #endif

    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader_pbr;

    return shader;
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 100.f;

    if (pKeys[SL_KeySymbol::KEY_SYM_w] || pKeys[SL_KeySymbol::KEY_SYM_W])
    {
        camTrans.move(math::vec3{0.f, 0.f, camSpeed * tickTime}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_s] || pKeys[SL_KeySymbol::KEY_SYM_S])
    {
        camTrans.move(math::vec3{0.f, 0.f, -camSpeed * tickTime}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_e] || pKeys[SL_KeySymbol::KEY_SYM_E])
    {
        camTrans.move(math::vec3{0.f, camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_q] || pKeys[SL_KeySymbol::KEY_SYM_Q])
    {
        camTrans.move(math::vec3{0.f, -camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_a] || pKeys[SL_KeySymbol::KEY_SYM_A])
    {
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_d] || pKeys[SL_KeySymbol::KEY_SYM_D])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
    }
}



/*-------------------------------------
 * Render the Scene
-------------------------------------*/
void render_scene(SL_SceneGraph* pGraph, unsigned w, unsigned h, const math::mat4& projection, const SL_Transform& camTrans, bool usePbr)
{
    SL_Context&    context   = pGraph->mContext;
    MeshUniforms*  pUniforms = context.ubo(0).as<MeshUniforms>();
    SL_Plane       planes[6];

    const math::mat4&& p  = math::perspective(math::radians(60.f), (float)w/(float)h, 0.1f, 100.f);
    const math::mat4&& vp = projection * camTrans.transform();

    sl_extract_frustum_planes(p, planes);

    for (SL_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4&  modelMat      = pGraph->mModelMatrices[n.nodeId];
        const math::mat4&& mv            = camTrans.transform() * modelMat;
        const size_t       numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vp * modelMat;

        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];
        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SL_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SL_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SL_Material&    material   = pGraph->mMaterials[m.materialId];

            if (!sl_is_visible(box, mv, planes))
            {
                continue;
            }

            if (!(m.mode & SL_RenderMode::RENDER_MODE_TRIANGLES))
            {
                continue;
            }

            pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT];

            #if SL_TEST_BUMP_MAPS
                pUniforms->pBump = material.pTextures[SL_MATERIAL_TEXTURE_HEIGHT];
            #endif

            // Use the textureless shader if needed
            size_t shaderId = (size_t)(material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT] == nullptr);

            pUniforms->light.ambient = material.ambient;
            pUniforms->light.diffuse = material.diffuse;

            if (usePbr)
            {
                shaderId += 2;
            }

            context.draw(m, shaderId, 0);
        }
    }
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> create_context()
{
    int retCode = 0;

    SL_SceneFileLoader meshLoader;
    SL_SceneLoadOpts opts = sl_default_scene_load_opts();
    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};
    SL_Context& context = pGraph->mContext;
    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SL_TEST_MAX_THREADS);
    assert(retCode == (int)SL_TEST_MAX_THREADS);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    assert(retCode == 0);

    retCode = fbo.attach_color_buffer(0, tex);
    assert(retCode == 0);

    retCode = fbo.attach_depth_buffer(depth);
    assert(retCode == 0);

    fbo.clear_color_buffers();
    fbo.clear_depth_buffer();

    retCode = fbo.valid();
    assert(retCode == 0);

    opts.packNormals = true;
    retCode = meshLoader.load("testdata/sibenik/sibenik.obj", opts);
    //retCode = meshLoader.load("testdata/sponza/sponza.obj", opts);
    assert(retCode != 0);

    retCode = (int)pGraph->import(meshLoader.data());
    assert(retCode == 0);

    pGraph->mCurrentTransforms[0].scale( math::vec3{20.f});
    //pGraph->mCurrentTransforms[0].scale(math::vec3{0.25f});

    pGraph->update();

    const SL_VertexShader&&   normVertShader    = normal_vert_shader();
    const SL_VertexShader&&   texVertShader     = texture_vert_shader();
    const SL_FragmentShader&& normFragShader    = normal_frag_shader();
    const SL_FragmentShader&& texFragShader     = texture_frag_shader();
    const SL_FragmentShader&& normFragShaderPbr = normal_frag_shader_pbr();
    const SL_FragmentShader&& texFragShaderPbr  = texture_frag_shader_pbr();

    size_t uboId = context.create_ubo();
    SL_UniformBuffer& ubo = context.ubo(uboId);
    MeshUniforms* pUniforms = ubo.as<MeshUniforms>();

    pUniforms->light.pos        = math::vec4{30.f, 45.f, 45.f, 1.f};
    pUniforms->light.ambient    = math::vec4{0.f, 0.f, 0.f, 1.f};
    pUniforms->light.diffuse    = math::vec4{0.5f, 0.5f, 0.5f, 1.f};
    pUniforms->point.constant   = 1.f;
    pUniforms->point.linear     = 0.009f;
    pUniforms->point.quadratic  = 0.00018f;

    size_t texShaderId     = context.create_shader(texVertShader,  texFragShader,     uboId);
    size_t normShaderId    = context.create_shader(normVertShader, normFragShader,    uboId);
    size_t texShaderPbrId  = context.create_shader(texVertShader,  texFragShaderPbr,  uboId);
    size_t normShaderPbrId = context.create_shader(normVertShader, normFragShaderPbr, uboId);

    assert(texShaderId == 0);
    assert(normShaderId == 1);
    assert(texShaderPbrId == 2);
    assert(normShaderPbrId == 3);
    (void)texShaderId;
    (void)normShaderId;
    (void)texShaderPbrId;
    (void)normShaderPbrId;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    utils::Pointer<SL_RenderWindow> pWindow{std::move(SL_RenderWindow::create())};
    utils::Pointer<SL_WindowBuffer> pRenderBuf{SL_WindowBuffer::create()};
    utils::Pointer<SL_SceneGraph>   pGraph{std::move(create_context())};
    utils::Pointer<bool[]>          pKeySyms{new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    SL_Context& context = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    bool usePbr = false;
    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    camTrans.look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    //camTrans.look_at(math::vec3{200.f, 150.f, 0.f}, math::vec3{0.f, 100.f, 0.f}, math::vec3{0.f, 1.f, 0.f});

    #if TEST_REVERSED_DEPTH
        math::mat4 projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);
    #else
        math::mat4 projMatrix = math::perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 10.f, 500.f);
    #endif

    if (shouldQuit)
    {
        return shouldQuit;
    }

    if (!pWindow->run())
    {
        std::cerr << "Unable to run the test window!" << std::endl;
        pWindow->destroy();
        return -1;
    }

    if (pRenderBuf->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Mesh Test") != 0)
    {
        return -2;
    }

    pWindow->set_keys_repeat(false); // text mode
    timer.start();

    while (!shouldQuit)
    {
        pWindow->update();
        SL_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SL_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
                context.texture(0).init(context.texture(0).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                context.texture(1).init(context.texture(1).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());

                #if TEST_REVERSED_DEPTH
                    projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
                #else
                    projMatrix = math::perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.1f, 500.f);
                #endif
            }

            if (evt.type == SL_WinEventType::WIN_EVENT_KEY_DOWN)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = false;

                switch (keySym)
                {
                    case SL_KeySymbol::KEY_SYM_SPACE:
                        if (pWindow->state() == WindowStateInfo::WINDOW_RUNNING)
                        {
                            std::cout << "Space button pressed. Pausing." << std::endl;
                            pWindow->pause();
                        }
                        else
                        {
                            std::cout << "Space button pressed. Resuming." << std::endl;
                            pWindow->run();
                            timer.start();
                        }
                        break;

                    case SL_KeySymbol::KEY_SYM_LEFT:
                        pWindow->set_size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
                        break;

                    case SL_KeySymbol::KEY_SYM_RIGHT:
                        pWindow->set_size(IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;

                    case SL_KeySymbol::KEY_SYM_UP:
                        numThreads = math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_DOWN:
                        numThreads = math::max(numThreads - 1u, 1u);
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_F1:
                        pWindow->set_mouse_capture(!pWindow->is_mouse_captured());
                        pWindow->set_keys_repeat(!pWindow->keys_repeat()); // no text mode
                        std::cout << "Mouse Capture: " << pWindow->is_mouse_captured() << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_F2:
                        usePbr = !usePbr;
                        std::cout << "PBR Rendering: " << usePbr << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_ESCAPE:
                        std::cout << "Escape button pressed. Exiting." << std::endl;
                        shouldQuit = true;
                        break;

                    default:
                        break;
                }
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_CLOSING)
            {
                std::cout << "Window close event caught. Exiting." << std::endl;
                shouldQuit = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_MOVED)
            {
                if (pWindow->is_mouse_captured())
                {
                    SL_MousePosEvent& mouse = evt.mousePos;
                    dx = (float)mouse.dx / (float)pWindow->dpi() * -0.05f;
                    dy = (float)mouse.dy / (float)pWindow->dpi() * -0.05f;
                    camTrans.rotate(math::vec3{dx, dy, 0.f});
                }
            }
        }
        else
        {
            timer.tick();
            const float tickTime = timer.tick_time().count();

            ++currFrames;
            ++totalFrames;
            currSeconds += tickTime;
            totalSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                //std::cout << "MS/F: " << 1000.f*(currSeconds/(float)currFrames) << std::endl;
                std::cout << "FPS: " << utils::to_str((float)currFrames/currSeconds) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 1200)
                {
                    shouldQuit = true;
                }
            #endif

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                MeshUniforms* pUniforms = context.ubo(0).as<MeshUniforms>();
                pUniforms->camPos = math::vec4_cast(camTrans.absolute_position(), 1.f);
            }

            pGraph->update();

            #if TEST_REVERSED_DEPTH
                context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);
            #else
                context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 1.0);
            #endif

            render_scene(pGraph.get(), pWindow->width(), pWindow->height(), projMatrix, camTrans, usePbr);

            context.blit(*pRenderBuf, 0);
            pWindow->render(*pRenderBuf);
        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            std::cout << "Window close state encountered. Exiting." << std::endl;
            shouldQuit = true;
        }
    }

    pRenderBuf->terminate();

    std::cout
        << "Rendered " << totalFrames << " frames in " << totalSeconds << " seconds ("
        << ((double)totalFrames/(double)totalSeconds) << " average fps)." << std::endl;

    return pWindow->destroy();
}
