
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Log.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Plane.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_WindowBuffer.hpp"
#include "soft_render/SR_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */

#ifndef SR_TEST_MAX_THREADS
    #define SR_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency()/2, 1))
#endif /* SR_TEST_MAX_THREADS */

#ifndef SR_TEST_USE_PBR
    #define SR_TEST_USE_PBR 0
#endif /* SR_TEST_USE_PBR */

#ifndef SR_TEST_DEBUG_AABBS
    #define SR_TEST_DEBUG_AABBS 0
#endif

#ifndef SR_BENCHMARK_SCENE
    #define SR_BENCHMARK_SCENE 1
#endif /* SR_BENCHMARK_SCENE */

#ifndef SR_REVERSED_Z_BUFFER
    #define SR_REVERSED_Z_BUFFER 1
#endif /* SR_REVERSED_Z_BUFFER */

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
    math::vec4 spot;
};



struct PointLight
{
    float constant;
    float linear;
    float quadratic;
    float padding;
};



struct SpotLight
{
    math::vec4 direction;
    float outerCutoff;
    float innerCutoff;
    float epsilon;
    float padding;
};



struct MeshUniforms : SR_UniformBuffer
{
    const SR_Texture* pTexture;
    const SR_BoundingBox* aabb;

    math::vec4 camPos;
    Light light;
    PointLight point;
    SpotLight spot;

    math::mat4 modelMatrix;
    math::mat4 mvpMatrix;
};



/*-----------------------------------------------------------------------------
 * Shader to display bounding boxes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _box_vert_shader_impl(const size_t vertId, const SR_VertexArray&, const SR_VertexBuffer&, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec4&   trr       = pUniforms->aabb->max_point();
    const math::vec4&   bfl       = pUniforms->aabb->min_point();
    const math::vec4    points[]  = {
        {bfl[0], bfl[1], trr[2], 1.f},
        {trr[0], bfl[1], trr[2], 1.f},
        {trr[0], trr[1], trr[2], 1.f},
        {bfl[0], trr[1], trr[2], 1.f},
        {bfl[0], bfl[1], bfl[2], 1.f},
        {trr[0], bfl[1], bfl[2], 1.f},
        {trr[0], trr[1], bfl[2], 1.f},
        {bfl[0], trr[1], bfl[2], 1.f}
    };

    varyings[0] = math::vec4{
        (vertId % 3 == 0) ? 1.f : 0.f,
        (vertId % 3 == 1) ? 1.f : 0.f,
        (vertId % 3 == 2) ? 1.f : 0.f,
        1.f
    };

    return pUniforms->mvpMatrix * points[vertId % LS_ARRAY_SIZE(points)];
}



SR_VertexShader box_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode    = SR_CULL_OFF;
    shader.shader      = _box_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _box_frag_shader_impl(SR_FragmentParam& fragParams)
{
    fragParams.pOutputs[0] = fragParams.pVaryings[0];
    //outputs[0] = SR_ColorRGBAf{1.f, 0.f, 1.f, 1.f};
    return true;
}



SR_FragmentShader box_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_OFF;
    shader.depthMask   = SR_DEPTH_MASK_OFF;
    shader.shader      = _box_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _normal_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    // Used to retrieve packed verex data in a single call
    typedef Tuple<math::vec3, math::vec3> Vertex;

    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const Vertex&       v         = *vbo.element<const Vertex>(vao.offset(0, vertId));
    const math::vec3&   vert      = v.const_element<0>();
    const math::vec3&   norm      = v.const_element<1>();

    varyings[0] = pUniforms->modelMatrix * math::vec4_cast(vert, 0.f);
    varyings[1] = pUniforms->modelMatrix * math::vec4_cast(norm, 0.f);

    return pUniforms->mvpMatrix * math::vec4_cast(vert, 1.f);
}



SR_VertexShader normal_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode    = SR_CULL_BACK_FACE;
    shader.shader      = _normal_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _normal_frag_shader_impl(SR_FragmentParam& fragParams)
{
    const MeshUniforms* pUniforms     = static_cast<const MeshUniforms*>(fragParams.pUniforms);
    const math::vec4    pos           = fragParams.pVaryings[0];
    const math::vec4    norm          = math::normalize(fragParams.pVaryings[1]);
    math::vec4&         output        = fragParams.pOutputs[0];


    // Light direction calculation
    math::vec4  lightDir  = pUniforms->camPos - pos;
    const float lightDist = math::length(lightDir);

    // normalize
    lightDir = lightDir * math::rcp(lightDist);

    const Light         l             = pUniforms->light;
    const math::vec4    ambient       = l.ambient;
    const float         lightAngle    = math::max(math::dot(lightDir, norm), 0.f);
    const float         constant      = pUniforms->point.constant;
    const float         linear        = pUniforms->point.linear;
    const float         quadratic     = pUniforms->point.quadratic;
    const float         attenuation   = math::rcp(constant + (linear*lightDist) + (quadratic*lightDist*lightDist));
    const math::vec4&&  diffuse       = l.diffuse * (lightAngle * attenuation);

    const SpotLight     s             = pUniforms->spot;
    const float         theta         = math::dot(lightDir, s.direction);
    const float         spotIntensity = math::smoothstep(s.innerCutoff, s.outerCutoff, theta);
    const math::vec4&&  specular      = ambient + diffuse + (l.spot * (spotIntensity * attenuation));

    output = math::min(specular, math::vec4{1.f});

    return true;
}



SR_FragmentShader normal_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _normal_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    // Used to retrieve packed verex data in a single call
    typedef Tuple<math::vec3, math::vec2, math::vec3> Vertex;

    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const Vertex&       v         = *vbo.element<const Vertex>(vao.offset(0, vertId));
    const math::vec3&   vert      = v.const_element<0>();
    const math::vec2&   uv        = v.const_element<1>();
    const math::vec3&   norm      = v.const_element<2>();

    varyings[0] = pUniforms->modelMatrix * math::vec4_cast(vert, 0.f);
    varyings[1] = math::vec4_cast(uv, 0.f, 0.f);
    varyings[2] = math::normalize(pUniforms->modelMatrix * math::vec4_cast(norm, 0.f));

    return pUniforms->mvpMatrix * math::vec4_cast(vert, 1.f);
}



SR_VertexShader texture_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SR_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader_spot(SR_FragmentParam& fragParams)
{
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(fragParams.pUniforms);
    const math::vec4&    pos       = fragParams.pVaryings[0];
    const math::vec4&    uv        = fragParams.pVaryings[1];
    const math::vec4&    norm      = fragParams.pVaryings[2];
    const SR_Texture*    albedo    = pUniforms->pTexture;
    float                attenuation;
    math::vec4           pixel;
    math::vec4           diffuse;
    math::vec4           spot;
    float                specular;

    constexpr float diffuseMultiplier = 2.f;
    constexpr float specularity = 0.5f;
    constexpr float shininess = 50.f;

    // normalize the texture colors to within (0.f, 1.f)
    if (albedo->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = albedo->nearest<math::vec3_t<uint8_t>>(uv[0], uv[1]);
        pixel = color_cast<float, uint8_t>(math::vec4_cast<uint8_t>(pixel8, 255));
    }
    else
    {
        pixel = color_cast<float, uint8_t>(albedo->nearest<math::vec4_t<uint8_t>>(uv[0], uv[1]));
    }

    // Light direction calculation
    math::vec4&& lightDir  = pUniforms->camPos - pos;
    const float  lightDist = math::length(lightDir);

    // normalize
    lightDir = lightDir * math::rcp(lightDist);

    const Light&     l       = pUniforms->light;
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

    // Spot light calculation
    {
        const SpotLight& s        = pUniforms->spot;
        const float theta         = math::dot(lightDir, s.direction);
        const float spotIntensity = math::smoothstep(s.innerCutoff, s.outerCutoff, theta);

        spot = l.spot * (spotIntensity * attenuation) * s.epsilon;
    }

    // specular reflection calculation
    {
        const math::vec4&& eyeVec     = pUniforms->camPos - pos;
        const math::vec4&& halfVec    = math::normalize((-l.pos-pos) + eyeVec);
        const float        reflectDir = math::max(math::dot(halfVec, norm), 0.f);

        specular = specularity * math::pow(reflectDir, shininess);
    }

    // output composition
    {
        const math::vec4&& accumulation = math::min(diffuse+spot+specular+ambient, math::vec4{1.f});

        fragParams.pOutputs[0] = pixel * accumulation;
    }

    return true;
}



// Calculate the metallic component of a surface
template <class vec_type = math::vec4>
inline vec_type fresnel_schlick(float cosTheta, const vec_type& surfaceReflection)
{
    return math::fmadd(vec_type{1.f} - surfaceReflection, math::pow(1.f - cosTheta, 5.f), surfaceReflection);
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



bool _texture_frag_shader_pbr(SR_FragmentParam& fragParams)
{
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(fragParams.pUniforms);
    const math::vec4     pos       = fragParams.pVaryings[0];
    const math::vec4     uv        = fragParams.pVaryings[1];
    const math::vec4     norm      = math::normalize(fragParams.pVaryings[2]);
    math::vec4&          output    = fragParams.pOutputs[0];
    const SR_Texture*    pTexture  = pUniforms->pTexture;
    math::vec4           pixel;

    // normalize the texture colors to within (0.f, 1.f)
    if (pTexture->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = pTexture->nearest<math::vec3_t<uint8_t>>(uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        pixel = color_cast<float, uint8_t>(pixelF);
    }
    else
    {
        const math::vec4_t<uint8_t>&& pixelF = pTexture->nearest<math::vec4_t<uint8_t>>(uv[0], uv[1]);
        pixel = color_cast<float, uint8_t>(pixelF);
    }

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



SR_FragmentShader texture_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;

    #if SR_TEST_USE_PBR
    shader.shader = _texture_frag_shader_pbr;
    #else
    shader.shader = _texture_frag_shader_spot;
    #endif

    return shader;
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SR_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 100.f;

    if (pKeys[SR_KeySymbol::KEY_SYM_w] || pKeys[SR_KeySymbol::KEY_SYM_W])
    {
        camTrans.move(math::vec3{0.f, 0.f, camSpeed * tickTime}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_s] || pKeys[SR_KeySymbol::KEY_SYM_S])
    {
        camTrans.move(math::vec3{0.f, 0.f, -camSpeed * tickTime}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_e] || pKeys[SR_KeySymbol::KEY_SYM_E])
    {
        camTrans.move(math::vec3{0.f, camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_q] || pKeys[SR_KeySymbol::KEY_SYM_Q])
    {
        camTrans.move(math::vec3{0.f, -camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_d] || pKeys[SR_KeySymbol::KEY_SYM_D])
    {
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_a] || pKeys[SR_KeySymbol::KEY_SYM_A])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
    }
}



/*-------------------------------------
 * Render the Scene
-------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix, float aspect, float fov, const SR_Transform& camTrans)
{
    SR_Context&    context   = pGraph->mContext;
    MeshUniforms*  pUniforms = static_cast<MeshUniforms*>(context.shader(0).uniforms().get());
    unsigned       numHidden = 0;
    unsigned       numTotal  = 0;
    SR_Plane       planes[6];

    const math::mat4 projection = math::perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 1.f, 10.f);
    const math::mat4 vp2 = projection * camTrans.get_transform();

    sr_extract_frustum_planes(projection, planes);

    (void)aspect;
    (void)fov;
    (void)camTrans;

    for (SR_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SR_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SR_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];

            pUniforms->pTexture = material.pTextures[0];

            // Use the textureless shader if needed
            const size_t shaderId = (size_t)(material.pTextures[0] == nullptr);

            ++numTotal;

            if (!sr_is_visible(box, vp2 * modelMat, planes))
            {
                ++numHidden;
                continue;
            }

            context.draw(m, shaderId, 0);
        }
    }

    // debugging
#if SR_TEST_DEBUG_AABBS
    const SR_Mesh& boxMesh = pGraph->mMeshes[0];

    for (SR_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t nodeMeshId = meshIds[meshId];
            const SR_BoundingBox& box = pGraph->mMeshBounds[nodeMeshId];
            pUniforms->aabb = &box;

            if (!sr_is_visible(box, vp2*modelMat, planes))
            {
                continue;
            }

            context.draw(boxMesh, 2,  0);
        }
    }
#endif

    //LS_LOG_MSG("Meshes Hidden: ", numHidden, '/', numTotal, " (", 100.f*((float)numHidden/(float)numTotal), "%).");
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SR_SceneGraph& graph)
{
    int retCode = 0;
    SR_Context& context = graph.mContext;
    constexpr unsigned numVerts = 8;
    constexpr unsigned numIndices = 36;
    size_t numVboBytes = 0;

    size_t vboId = context.create_vbo();
    SR_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numVerts*sizeof(math::vec3));
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t iboId = context.create_ibo();
    SR_IndexBuffer& ibo = context.ibo(iboId);
    retCode = ibo.init(numIndices, SR_DataType::VERTEX_DATA_BYTE);
    if (retCode != 0)
    {
        std::cerr << "Error while creating an IBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SR_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    vao.set_index_buffer(iboId);
    retCode = vao.set_num_bindings(1);
    if (retCode != 1)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    constexpr math::vec3 verts[numVerts] = {
        {-1.f, -1.f,  1.f}, // 0
        { 1.f, -1.f,  1.f}, // 1
        { 1.f,  1.f,  1.f}, // 2
        {-1.f,  1.f,  1.f}, // 3
        {-1.f, -1.f, -1.f}, // 4
        { 1.f, -1.f, -1.f}, // 5
        { 1.f,  1.f, -1.f}, // 6
        {-1.f,  1.f, -1.f}  // 7
    };

    constexpr uint8_t indices[numIndices] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };

    // Create the vertex buffer
    vbo.assign(verts, numVboBytes, sizeof(verts));
    ibo.assign(indices, 0, numIndices);
    vao.set_binding(0, numVboBytes, sizeof(math::vec3), SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);

    ls::utils::Pointer<size_t[]> meshId{new size_t[1]};
    meshId[0] = 0;

    graph.mNodes.push_back({SR_SceneNodeType::NODE_TYPE_EMPTY, 0, 0, SCENE_NODE_ROOT_ID});
    graph.mBaseTransforms.emplace_back(math::mat4{1.f});
    graph.mCurrentTransforms.push_back(SR_Transform{math::mat4{1.f}, SR_TRANSFORM_TYPE_MODEL});
    graph.mNodeNames.emplace_back(std::string{"AABB"});
    graph.mModelMatrices.emplace_back(math::mat4{1.f});
    graph.mNodeMeshes.emplace_back(std::move(meshId));
    graph.mNumNodeMeshes.push_back(1);
    graph.mMeshes.emplace_back(SR_Mesh());

    SR_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numIndices;
    mesh.mode = SR_RenderMode::RENDER_MODE_INDEXED_TRI_WIRE;
    mesh.materialId = (uint32_t)-1;

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> create_context()
{
    int retCode = 0;

    #ifdef LS_ARCH_X86
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _mm_setcsr(_mm_getcsr() | 0x8040); // denormals-are-zero
    #endif

    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;
    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SR_TEST_MAX_THREADS);
    assert(retCode == SR_TEST_MAX_THREADS);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Framebuffer& fbo = context.framebuffer(fboId);
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

    // cube exists at index 0
#if SR_TEST_DEBUG_AABBS
    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);
#endif

    retCode = meshLoader.load("testdata/sibenik/sibenik.obj");
    //retCode = meshLoader.load("testdata/sponza/sponza.obj");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

#if SR_TEST_DEBUG_AABBS
    pGraph->mCurrentTransforms[1].scale(math::vec3{20.f});
    //pGraph->mCurrentTransforms[1].scale(math::vec3{0.125f});
#else
    pGraph->mCurrentTransforms[0].scale( math::vec3{20.f});
    //pGraph->mCurrentTransforms[0].scale(math::vec3{0.125f});
#endif
    pGraph->update();

    const SR_VertexShader&&   normVertShader = normal_vert_shader();
    const SR_FragmentShader&& normFragShader = normal_frag_shader();
    const SR_VertexShader&&   texVertShader  = texture_vert_shader();
    const SR_FragmentShader&& texFragShader  = texture_frag_shader();
    const SR_VertexShader&&   boxVertShader  = box_vert_shader();
    const SR_FragmentShader&& boxFragShader  = box_frag_shader();

    // Uniform variables don't always align properly because of the vtable
    std::shared_ptr<MeshUniforms>  pUniforms{(MeshUniforms*)ls::utils::aligned_malloc(sizeof(MeshUniforms)), [](MeshUniforms* p)->void {ls::utils::aligned_free(p);}};

    pUniforms->light.pos        = math::vec4{30.f, 45.f, 45.f, 1.f};
    pUniforms->light.ambient    = math::vec4{0.f, 0.f, 0.f, 1.f};//math::vec4{0.125f, 0.125f, 0.125f, 1.f};
    pUniforms->light.diffuse    = math::vec4{0.5f, 0.5f, 0.5f, 1.f};
    pUniforms->light.spot       = math::vec4{1.f};
    pUniforms->point.constant   = 1.f;
    pUniforms->point.linear     = 0.009f;
    pUniforms->point.quadratic  = 0.00018f;
    pUniforms->spot.innerCutoff = std::cos(LS_DEG2RAD(13.f));
    pUniforms->spot.outerCutoff = std::cos(LS_DEG2RAD(6.5f));
    pUniforms->spot.epsilon     = pUniforms->spot.outerCutoff / pUniforms->spot.innerCutoff;

    size_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  pUniforms);
    size_t normShaderId = context.create_shader(normVertShader, normFragShader, pUniforms);
    size_t boxShaderId  = context.create_shader(boxVertShader,  boxFragShader,  pUniforms);

    assert(texShaderId == 0);
    assert(normShaderId == 1);
    assert(boxShaderId == 2);
    (void)texShaderId;
    (void)normShaderId;
    (void)boxShaderId;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    utils::Pointer<SR_RenderWindow> pWindow{std::move(SR_RenderWindow::create())};
    utils::Pointer<SR_WindowBuffer> pRenderBuf{SR_WindowBuffer::create()};
    utils::Pointer<SR_SceneGraph>   pGraph{std::move(create_context())};
    utils::Pointer<bool[]>          pKeySyms{new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    SR_Context& context = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;

    unsigned numThreads = context.num_threads();

    SR_Transform camTrans;
    camTrans.set_type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    //camTrans.extract_transforms(math::look_at(math::vec3{75.f}, math::vec3{0.f, 10.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
    camTrans.extract_transforms(math::look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));

    #if SR_REVERSED_Z_BUFFER
        math::mat4 projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);
    #else
        math::mat4 projMatrix = math::perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.1f, 500.f);
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
        SR_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SR_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
                context.texture(0).init(context.texture(0).type(), pWindow->width(), pWindow->height());
                context.texture(1).init(context.texture(1).type(), pWindow->width(), pWindow->height());

                #if SR_REVERSED_Z_BUFFER
                    projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
                #else
                    projMatrix = math::perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.1f, 500.f);
                #endif
            }

            if (evt.type == SR_WinEventType::WIN_EVENT_KEY_DOWN)
            {
                const SR_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = true;
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SR_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = false;

                switch (keySym)
                {
                    case SR_KeySymbol::KEY_SYM_SPACE:
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

                    case SR_KeySymbol::KEY_SYM_LEFT:
                        pWindow->set_size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
                        break;

                    case SR_KeySymbol::KEY_SYM_RIGHT:
                        pWindow->set_size(IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;

                    case SR_KeySymbol::KEY_SYM_UP:
                        numThreads = math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_DOWN:
                        numThreads = math::max(numThreads - 1u, 1u);
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_F1:
                        pWindow->set_mouse_capture(!pWindow->is_mouse_captured());
                        pWindow->set_keys_repeat(!pWindow->keys_repeat()); // no text mode
                        std::cout << "Mouse Capture: " << pWindow->is_mouse_captured() << std::endl;
                        break;

                    case SR_KeySymbol::KEY_SYM_ESCAPE:
                        std::cout << "Escape button pressed. Exiting." << std::endl;
                        shouldQuit = true;
                        break;

                    default:
                        break;
                }
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_CLOSING)
            {
                std::cout << "Window close event caught. Exiting." << std::endl;
                shouldQuit = true;
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_MOUSE_MOVED)
            {
                if (pWindow->is_mouse_captured())
                {
                    SR_MousePosEvent& mouse = evt.mousePos;
                    dx = (float)mouse.dx / (float)pWindow->dpi() * 0.05f;
                    dy = (float)mouse.dy / -(float)pWindow->dpi() * 0.05f;
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
                std::cout << "FPS: " << ((float)currFrames/currSeconds) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SR_BENCHMARK_SCENE
            if (totalFrames >= 600)
            {
                shouldQuit = true;
            }
            #endif

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                MeshUniforms* pUniforms = static_cast<MeshUniforms*>(context.shader(1).uniforms().get());
                const math::vec3&& camTransPos = -camTrans.get_position();
                pUniforms->camPos = {camTransPos[0], camTransPos[1], camTransPos[2], 1.f};

                const math::mat4& v = camTrans.get_transform();
                pUniforms->spot.direction = math::normalize(math::vec4{v[0][2], v[1][2], v[2][2], 0.f});
            }
            const math::mat4&& vpMatrix = projMatrix * camTrans.get_transform();

            pGraph->update();

            context.framebuffer(0).clear_color_buffers();

            #if SR_REVERSED_Z_BUFFER
                context.framebuffer(0).clear_depth_buffer();
            #else
                context.framebuffer(0).clear_depth_buffer(1.f);
            #endif

            render_scene(pGraph.get(), vpMatrix, ((float)pRenderBuf->width() / (float)pWindow->height()), LS_DEG2RAD(60.f), camTrans);

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
