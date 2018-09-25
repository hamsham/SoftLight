
#include <iostream>

#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"

#include "test_common.hpp"



#ifndef SR_TEST_MAX_THREADS
    #define SR_TEST_MAX_THREADS 14
#endif /* SR_TEST_MAX_THREADS */



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _normal_vert_shader_impl(const uint32_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);

    const math::vec3& vert = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec3& norm = *vbo.element<const math::vec3>(vao.offset(1, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 0.f};
    varyings[1] = pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f};

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader normal_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 2;
    shader.shader = _normal_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _normal_frag_shader_impl(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, SR_ColorRGBAf* outputs)
{
    const MeshUniforms* pUniforms     = static_cast<const MeshUniforms*>(uniforms);
    const Light         l             = pUniforms->light;
    const math::vec4    pos           = varyings[0];
    const math::vec4    norm          = math::normalize(varyings[1]);
    SR_ColorRGBAf&      output        = outputs[0];

    math::vec4          lightDir      = l.pos - pos;
    const float         lightDist     = math::length(lightDir);
    lightDir = math::normalize(lightDir);

    const float         lightAngle    = math::max(math::dot(lightDir, norm), 0.f);
    const float         constant      = pUniforms->point.constant;
    const float         linear        = pUniforms->point.linear;
    const float         quadratic     = pUniforms->point.quadratic;
    const float         attenuation   = math::rcp(constant + (linear*lightDist) + (quadratic*lightDist*lightDist));
    const math::vec4&&  diffuse       = l.diffuse * (lightAngle * attenuation);

    const SpotLight     s             = pUniforms->spot;
    const float         theta         = math::dot(lightDir, s.direction);
    const float         spotIntensity = math::clamp((theta - s.outerCutoff) * s.epsilon, 0.f, 1.f);
    const math::vec4&&  specular      = diffuse + (l.specular * (spotIntensity * attenuation));

    output.r = specular[0];
    output.g = specular[1];
    output.b = specular[2];

    return true;
}



SR_FragmentShader normal_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs = 1;
    shader.shader = _normal_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(const uint32_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec3&   vert      = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec2&   uv        = *vbo.element<const math::vec2>(vao.offset(1, vertId));
    const math::vec3&   norm      = *vbo.element<const math::vec3>(vao.offset(2, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
    varyings[1] = math::vec4{uv.v[0], uv.v[1], 0.f, 0.f};
    varyings[2] = math::normalize(pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f});

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader texture_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader_spot(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, SR_ColorRGBAf* outputs)
{
    const math::vec4     pos       = varyings[0];
    const math::vec4     uv        = varyings[1];
    const math::vec4     norm      = varyings[2];
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const SR_Texture*    albedo    = pUniforms->pTexture;

    math::vec4           pixel;
    float attenuation;
    math::vec4 diffuse, specular{0.f};

    // normalize the texture colors to within (0.f, 1.f)
    {
        // vectors are faster than colors
        SR_ColorRGB8&&       pixel8 = albedo->nearest<SR_ColorRGB8>(uv[0], uv[1]);
        //const SR_ColorRGBf&& pixelF = color_cast<float, uint8_t>(pixel8);
        math::vec4_t<uint8_t> pixelF{255};
        pixelF[0] = pixel8.r;
        pixelF[1] = pixel8.g;
        pixelF[2] = pixel8.b;
        //pixel = math::vec4{pixelF.r, pixelF.g, pixelF.b, 1.f};
        pixel = (math::vec4)pixelF * math::vec4{0.00392156862745f};
    }

    // Light direction calculation
    const Light l         = pUniforms->light;
    math::vec4  lightDir  = l.pos - pos;
    const float lightDist = math::length(lightDir);

    lightDir = math::normalize(lightDir);

    // Diffuse light calculation
    {
        const PointLight p     = pUniforms->point;
        const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);
        const float constant   = p.constant;
        const float linear     = p.linear;
        const float quadratic  = p.quadratic;

        attenuation = math::rcp(constant + (linear * lightDist) + (quadratic * lightDist * lightDist));
        diffuse     = l.diffuse * (lightAngle * attenuation);
    }

    // Specular light calculation
    {
        const SpotLight s         = pUniforms->spot;
        const float theta         = math::dot(lightDir, s.direction);
        const float spotIntensity = math::clamp((theta - s.outerCutoff) * s.epsilon, 0.f, 1.f);

        specular = l.specular * (spotIntensity * attenuation);
    }

    // output composition
    {
        pixel = pixel * (diffuse + specular);
        pixel = math::min(pixel, math::vec4{1.f});

        outputs[0].r = pixel[0];
        outputs[0].g = pixel[1];
        outputs[0].b = pixel[2];
        outputs[0].a = pixel[3];
    }

    return true;
}



// Calculate the metallic component of a surface
template <class vec_type = math::vec4>
constexpr inline vec_type fresnel_schlick(float cosTheta, const vec_type& surfaceReflection)
{
    return surfaceReflection + (vec_type{1.f} - surfaceReflection) * math::pow(1.f - cosTheta, 5.f);
}



// normal distribution function within a hemisphere
template <class vec_type = math::vec4>
inline float distribution_ggx(const vec_type& norm, const vec_type& hemisphere, float roughness)
{
    float roughSquared = roughness * roughness;
    float roughQuad = roughSquared * roughSquared;
    float nDotH = math::max(math::dot(norm, hemisphere), 0.f);
    float nDotH2 = nDotH * nDotH;

    float distribution = nDotH2 * (roughQuad - 1.f) + 1.f;

    return nDotH2 / (LS_PI * distribution  * distribution );
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



bool _texture_frag_shader_pbr(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, SR_ColorRGBAf* outputs)
{
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec4     pos       = varyings[0];
    const math::vec4     uv        = varyings[1];
    const math::vec4     norm      = math::normalize(varyings[2]);
    SR_ColorRGBAf&       output    = outputs[0];
    const SR_Texture*    pTexture  = pUniforms->pTexture;
    math::vec4           pixel;

    {
        // vectors are faster than colors
        SR_ColorRGB8&&       pixel8 = pTexture->nearest<SR_ColorRGB8>(uv[0], uv[1]);
        //const SR_ColorRGBf&& pixelF = color_cast<float, uint8_t>(pixel8);
        const math::vec4_t<uint8_t> pixelF{pixel8.r, pixel8.g, pixel8.b, 255};
        //pixel = math::vec4{pixelF.r, pixelF.g, pixelF.b, 1.f};

        pixel = (math::vec4)pixelF * math::vec4{0.00392156862745f};
        pixel[0] = math::pow(pixel[0], 2.2f);
        pixel[1] = math::pow(pixel[1], 2.2f);
        pixel[2] = math::pow(pixel[2], 2.2f);
    }

    // surface model
    const math::vec4     camPos           = pUniforms->light.pos;
    const math::vec4&&   viewDir          = math::normalize(camPos - pos);
    constexpr math::vec4 lightPos         = {30.f, 45.f, 45.f, 0.f};
    const math::vec4     albedo           = {1.f};
    constexpr float      metallic         = 0.4f;
    constexpr float      roughness        = 0.15f;
    constexpr float      ambientIntensity = 0.25f;

    // Metallic reflectance at a normal incidence
    // 0.04f should be close to plastic.
    constexpr math::vec4 surfaceConstant   = {0.04f, 0.04f, 0.04f, 1.f};
    const math::vec4&& surfaceReflection = math::mix(surfaceConstant, albedo, metallic);

    math::vec4         lightDir0         = {0.f};
    math::vec4&&       lightDirN         = math::normalize(lightPos - pos);
    math::vec4&&       hemisphere        = math::normalize(viewDir + lightDirN);

    const float        distance          = math::length(lightPos - pos);
    const float        attenuation       = math::rcp(distance * distance);
    math::vec4&&       radianceObj       = pUniforms->light.diffuse * attenuation;

    const float        ndf               = distribution_ggx(norm, hemisphere, roughness);
    const float        geom              = geometry_smith(norm, viewDir, lightPos, roughness);
    const math::vec4&& fresnel           = fresnel_schlick(math::clamp(math::dot(hemisphere, viewDir), 0.f, 1.f), surfaceReflection);

    const math::vec4&& brdf              = fresnel * ndf * geom;
    const float        cookTorrance      = math::rcp(4.f * math::max(math::dot(norm, viewDir), 0.f) * math::max(math::dot(norm, lightDirN), 0.f));
    const math::vec4&& specular          = brdf * math::max(cookTorrance, LS_EPSILON); // avoid divide-by-0

    const math::vec4&  specContrib       = surfaceReflection;
    const math::vec4&& refractRatio      = (math::vec4{1.f} - specContrib) * (math::vec4{1.f} - metallic);

    const float normDotLight             = math::max(math::dot(norm, lightDirN), 0.f);
    lightDir0                            += (refractRatio * albedo / LS_PI + specular) * radianceObj * normDotLight;

    const math::vec4&& ambient           = pUniforms->light.ambient * albedo * ambientIntensity * pixel;

    // Color normalization and light contribution
    float outR = ambient[0] + lightDir0[0];
    float outG = ambient[1] + lightDir0[1];
    float outB = ambient[2] + lightDir0[2];

    // Tone mapping
    output.r /= outR + 1.f;
    output.g /= outG + 1.f;
    output.b /= outB + 1.f;

    // HDR Tone mapping
    //const float exposure = 5.f;
    //outR = 1.f - math::exp(-outR * exposure);
    //outG = 1.f - math::exp(-outG * exposure);
    //outB = 1.f - math::exp(-outB * exposure);

    // Gamma correction
    constexpr float gamma = 1.f / 2.2f;
    output.r = math::clamp(math::pow(outR, gamma), 0.f, 1.f);
    output.g = math::clamp(math::pow(outG, gamma), 0.f, 1.f);
    output.b = math::clamp(math::pow(outB, gamma), 0.f, 1.f);

    return true;
}



SR_FragmentShader texture_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs = 1;
    shader.shader = _texture_frag_shader_spot;
    //shader.shader = _texture_frag_shader_pbr;

    return shader;
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
    uint32_t fboId   = context.create_framebuffer();
    uint32_t texId   = context.create_texture();
    uint32_t depthId = context.create_texture();

    retCode = context.num_threads(SR_TEST_MAX_THREADS);
    assert(retCode == SR_TEST_MAX_THREADS);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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

    //retCode = meshLoader.load("testdata/african_head/african_head.obj");
    //retCode = meshLoader.load("testdata/bob/Bob.md5mesh");
    //retCode = meshLoader.load("testdata/rover/testmesh.dae");
    //retCode = meshLoader.load("testdata/base_model.dae");
    retCode = meshLoader.load("testdata/sibenik/sibenik.obj");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    pGraph->mCurrentTransforms[0].scale(math::vec3{20.f});
    pGraph->update();

    const SR_VertexShader&&   normVertShader = normal_vert_shader();
    const SR_FragmentShader&& normFragShader = normal_frag_shader();
    const SR_VertexShader&&   texVertShader  = texture_vert_shader();
    const SR_FragmentShader&& texFragShader  = texture_frag_shader();

    // I keep getting this weird error about alignment so I'm using malloc
    #ifndef LS_ARCH_X86
    std::shared_ptr<MeshUniforms>  pUniforms{(MeshUniforms*)malloc(sizeof(MeshUniforms)), [](MeshUniforms* p)->void {free(p);}};
    #else
    std::shared_ptr<MeshUniforms>  pUniforms{(MeshUniforms*)_mm_malloc(sizeof(MeshUniforms), sizeof(__m128)), [](MeshUniforms* p)->void {_mm_free(p);}};
    #endif

    pUniforms->light.pos        = math::vec4{30.f, 45.f, 45.f, 1.f};
    pUniforms->light.ambient    = math::vec4{1.f};
    pUniforms->light.diffuse    = math::vec4{1.f};
    pUniforms->light.specular   = math::vec4{1.f};
    pUniforms->point.constant   = 1.f;
    pUniforms->point.linear     = 0.009f;
    pUniforms->point.quadratic  = 0.00018f;
    pUniforms->spot.innerCutoff = std::cos(LS_DEG2RAD(6.5f));
    pUniforms->spot.outerCutoff = std::cos(LS_DEG2RAD(13.f));
    pUniforms->spot.epsilon     = math::rcp(pUniforms->spot.innerCutoff - pUniforms->spot.outerCutoff);

    uint32_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  pUniforms);
    uint32_t normShaderId = context.create_shader(normVertShader, normFragShader, pUniforms);

    assert(texShaderId == 0);
    assert(normShaderId == 1);
    (void)texShaderId;
    (void)normShaderId;

    //const math::mat4&& viewMatrix = math::look_at(math::vec3{75.f}, math::vec3{0.f, 10.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& viewMatrix = math::look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    render_scene(pGraph.get(), projMatrix*viewMatrix);

    sr_img_save_ppm(IMAGE_WIDTH, IMAGE_HEIGHT, reinterpret_cast<const SR_ColorRGB8*>(tex.data()), "window_buffer_test.ppm");

    SR_Texture& baseTex = context.texture(2);
    sr_img_save_ppm(baseTex.width(), baseTex.height(), reinterpret_cast<const SR_ColorRGB8*>(baseTex.data()), "window_buffer_texture.ppm");

    if (retCode != 0)
    {
        abort();
    }

    std::cout << "First frame rendered." << std::endl;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context& context = pGraph->mContext;
    MeshUniforms* pUniforms = static_cast<MeshUniforms*>(context.shader(0).uniforms().get());

    for (SR_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const unsigned numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<unsigned[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vpMatrix * modelMat;

        for (unsigned meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const unsigned        nodeMeshId = meshIds[meshId];
            const SR_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            //const SR_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];
            pUniforms->pTexture = material.pTextures[0];

            // Use the textureless shader if needed
            /*
            if (!material.pTextures[0])
            {
                continue;
            }
            */
            const uint32_t shaderId   = (uint32_t)(material.pTextures[0] == nullptr);
            //const uint32_t shaderId = 0;

            /*
            if (!sr_is_visible(box, pUniforms->mvpMatrix))
            {
                continue;
            }
            */

            context.draw(m, shaderId, 0);
        }
    }
}
