
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>

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
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_Plane.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"

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

    math::vec4 camPos;
    Light light;
    PointLight point;

    math::mat4 modelMatrix;
    math::mat4 mvpMatrix;
};



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
SL_FragmentShader normal_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = [](SL_FragmentParam& fragParams)->bool
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
    };

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
SL_FragmentShader texture_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = [](SL_FragmentParam& fragParams)->bool
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
    };

    return shader;
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 100.f;

    if (pKeys[SDL_SCANCODE_W])
    {
        camTrans.move(math::vec3{0.f, 0.f, camSpeed * tickTime}, false);
    }

    if (pKeys[SDL_SCANCODE_S])
    {
        camTrans.move(math::vec3{0.f, 0.f, -camSpeed * tickTime}, false);
    }

    if (pKeys[SDL_SCANCODE_E])
    {
        camTrans.move(math::vec3{0.f, camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SDL_SCANCODE_Q])
    {
        camTrans.move(math::vec3{0.f, -camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SDL_SCANCODE_A])
    {
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SDL_SCANCODE_D])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
    }
}



/*-------------------------------------
 * Render the Scene
-------------------------------------*/
void render_scene(SL_SceneGraph* pGraph, unsigned w, unsigned h, const math::mat4& projection, const SL_Transform& camTrans)
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
    LS_ASSERT(retCode == (int)SL_TEST_MAX_THREADS);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_color_buffer(0, tex);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_depth_buffer(depth);
    LS_ASSERT(retCode == 0);

    fbo.clear_color_buffers();
    fbo.clear_depth_buffer();

    retCode = fbo.valid();
    LS_ASSERT(retCode == 0);

    opts.packNormals = true;
    retCode = meshLoader.load("testdata/sibenik/sibenik.obj", opts);
    //retCode = meshLoader.load("testdata/sponza/sponza.obj", opts);
    LS_ASSERT(retCode != 0);

    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 0);

    pGraph->mCurrentTransforms[0].scale( math::vec3{20.f});
    //pGraph->mCurrentTransforms[0].scale(math::vec3{0.25f});

    pGraph->update();

    const SL_VertexShader&&   normVertShader    = normal_vert_shader();
    const SL_VertexShader&&   texVertShader     = texture_vert_shader();
    const SL_FragmentShader&& normFragShader    = normal_frag_shader();
    const SL_FragmentShader&& texFragShader     = texture_frag_shader();

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

    LS_ASSERT(texShaderId == 0);
    LS_ASSERT(normShaderId == 1);
    (void)texShaderId;
    (void)normShaderId;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * SDL Texture Handling
-----------------------------------------------------------------------------*/
inline SDL_PixelFormatEnum sl_pixel_fmt_to_sdl(const SL_ColorDataType slFmt) noexcept
{
    switch (slFmt)
    {
        case SL_COLOR_RGB_8U: return SDL_PIXELFORMAT_BGR888;
        case SL_COLOR_RGBA_8U: return SDL_PIXELFORMAT_ARGB8888;

        default:
            break;
    }

    return SDL_PIXELFORMAT_UNKNOWN;
}



inline int sl_get_texture_pitch(const SL_Texture& pTex) noexcept
{
    int w = (int)pTex.width();
    int bpp = (int)sl_bytes_per_color(pTex.type());
    return w * bpp;
}



int select_sdl_render_driver() noexcept
{
    int numDrivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < numDrivers; ++i)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        if (info.flags & (SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE))
        {
            for (unsigned fmtId = 0; fmtId < info.num_texture_formats; ++fmtId)
            {
                if (info.texture_formats[fmtId] == SDL_PIXELFORMAT_ARGB8888)
                {
                    return i;
                }
            }
        }
    }

    return -1;
}



inline void update_sdl_backbuffer(SL_Texture& tex, SDL_Texture* pBackbuffer) noexcept
{
    #if 0
        void* pTexData = nullptr;
        int pitch = 0;

        SDL_LockTexture(pBackbuffer, nullptr, &pTexData, &pitch);

        if (!pTexData || pitch != sl_get_texture_pitch(tex))
        {
            LS_ASSERT(false);
        }

        utils::fast_memcpy(pTexData, tex.data(), (uint_fast64_t)pitch*(uint_fast64_t)tex.height());
        SDL_UnlockTexture(pBackbuffer);

    #else
        SDL_UpdateTexture(pBackbuffer, nullptr, tex.data(), sl_get_texture_pitch(tex));
    #endif
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
extern "C" SDLMAIN_DECLSPEC int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        LS_LOG_ERR(
            "Unable to initialize SDL due to an internal library error: \"",
            SDL_GetError(), "\"\n",
            "Complain to your local programmer.\n"
        );
        return -1;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    LS_LOG_MSG("Successfully initialized SDL.");

    SDL_Window* pWindow = SDL_CreateWindow("SoftLight", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, IMAGE_WIDTH, IMAGE_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!pWindow)
    {
        LS_LOG_ERR("Unable to create a display.\n", SDL_GetError());
        SDL_Quit();
        return -2;
    }
    else
    {
        LS_LOG_MSG("Successfully created a window through SDL.");
    }

    int renderDriverId = select_sdl_render_driver();
    SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, renderDriverId, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
    if (!pRenderer)
    {
        LS_LOG_ERR("Unable to instantiate an accelerated render backend.");
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return -3;
    }
    else
    {
        LS_LOG_MSG("Successfully instantiated an accelerated render backend (", renderDriverId, ").");
        SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_NONE);
    }

    utils::Pointer<bool[]> pKeySyms{new bool[SDL_NUM_SCANCODES]};
    std::fill_n(pKeySyms.get(), SDL_NUM_SCANCODES, false);

    utils::Pointer<SL_SceneGraph> pGraph{std::move(create_context())};
    SL_Context& context = pGraph->mContext;

    SDL_Texture* pBackBuffer = SDL_CreateTexture(pRenderer, sl_pixel_fmt_to_sdl(context.texture(0).type()), SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
    if (!pBackBuffer)
    {
        LS_LOG_ERR("Unable to instantiate a backbuffer texture.");
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return -4;
    }
    else
    {
        uint32_t fmt;
        int access, w, h;
        SDL_QueryTexture(pBackBuffer, &fmt, &access, &w, &h);
        LS_LOG_MSG("Successfully instantiated a (backbuffer ", w, 'x', h, ").");
    }

    int shouldQuit = 0;
    bool mouseCapture = false;
    bool amPaused = false;

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    camTrans.look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    //camTrans.look_at(math::vec3{200.f, 150.f, 0.f}, math::vec3{0.f, 100.f, 0.f}, math::vec3{0.f, 1.f, 0.f});

    math::mat4 projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    timer.start();

    while (!shouldQuit)
    {
        SDL_Event evt;
        bool haveEvent = false;

        if (amPaused)
        {
            if (SDL_WaitEvent(&evt) == 1)
            {
                haveEvent = true;
            }
        }
        else
        {
            haveEvent = 0 != SDL_PollEvent(&evt);
        }

        if (haveEvent)
        {
            if (evt.type == SDL_WINDOWEVENT)
            {
                if (evt.window.windowID != SDL_GetWindowID(pWindow))
                {
                    //continue;
                }

                if (evt.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    LS_LOG_MSG("Window close event caught. Exiting.");
                    shouldQuit = true;
                }
                else if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    LS_LOG_MSG("Window resized: ", evt.window.data1, 'x', evt.window.data2);
                    context.texture(0).init(context.texture(0).type(), (uint16_t)evt.window.data1, (uint16_t)evt.window.data2);
                    context.texture(1).init(context.texture(1).type(), (uint16_t)evt.window.data1, (uint16_t)evt.window.data2);
                    projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)evt.window.data1/(float)evt.window.data2, 0.01f);

                    SDL_DestroyTexture(pBackBuffer);
                    pBackBuffer = SDL_CreateTexture(pRenderer, sl_pixel_fmt_to_sdl(context.texture(0).type()), SDL_TEXTUREACCESS_STREAMING, evt.window.data1, evt.window.data2);
                    if (!pBackBuffer)
                    {
                        LS_LOG_ERR("Unable to resize the backbuffer.");
                        shouldQuit = true;
                    }
                    else
                    {
                        LS_LOG_MSG("Successfully resized the backbuffer (", evt.window.data1, 'x', evt.window.data2, ").");
                    }
                }
            }
            else if (evt.type == SDL_KEYDOWN)
            {
                pKeySyms[evt.key.keysym.scancode] = true;
            }
            else if (evt.type == SDL_KEYUP)
            {
                pKeySyms[evt.key.keysym.scancode] = false;

                switch (evt.key.keysym.scancode)
                {
                    case SDL_SCANCODE_SPACE:
                        if (!amPaused)
                        {
                            LS_LOG_MSG("Space button pressed. Pausing.");
                            timer.stop();
                        }
                        else
                        {
                            LS_LOG_MSG("Space button pressed. Resuming.");
                            timer.start();
                        }

                        amPaused = !amPaused;
                        break;

                    case SDL_SCANCODE_LEFT:
                        SDL_SetWindowSize(pWindow, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
                        break;

                    case SDL_SCANCODE_RIGHT:
                        SDL_SetWindowSize(pWindow, IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;

                    case SDL_SCANCODE_UP:
                        numThreads = math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SDL_SCANCODE_DOWN:
                        numThreads = math::max(numThreads - 1u, 1u);
                        context.num_threads(numThreads);
                        break;

                    case SDL_SCANCODE_F1:
                        mouseCapture = !mouseCapture;
                        SDL_SetRelativeMouseMode(mouseCapture ? SDL_TRUE : SDL_FALSE);
                        SDL_CaptureMouse(mouseCapture ? SDL_TRUE : SDL_FALSE);
                        LS_LOG_MSG("Mouse Capture: ", (int)mouseCapture);
                        break;

                    case SDL_SCANCODE_ESCAPE:
                        LS_LOG_MSG("Escape button pressed. Exiting.");
                        shouldQuit = true;
                        break;

                    default:
                        break;
                }
            }
            else if (evt.type == SDL_QUIT)
            {
                LS_LOG_MSG("User quit event caught. Exiting.");
                shouldQuit = true;
            }
            else if (evt.type == SDL_MOUSEMOTION)
            {
                if (!amPaused && mouseCapture)
                {
                    float dpi, hdpi, vdpi;
                    if (SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(pWindow), &dpi, &hdpi, &vdpi))
                    {
                        LS_LOG_ERR(SDL_GetError());
                        SDL_ClearError();
                    }
                    else
                    {
                        dx = (float)evt.motion.xrel / (float)hdpi * 0.05f;
                        dy = (float)evt.motion.yrel / (float)vdpi * 0.05f;
                        camTrans.rotate(math::vec3{dx, dy, 0.f});
                    }
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
                //LS_LOG_MSG("MS/F: ", 1000.f*(currSeconds/(float)currFrames));
                LS_LOG_MSG("FPS: ", utils::to_str((float)currFrames/currSeconds));
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 5000)
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

            SL_Texture& frontBuffer = context.texture(0);
            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);
            render_scene(pGraph.get(), (unsigned)frontBuffer.width(), (unsigned)frontBuffer.height(), projMatrix, camTrans);
            update_sdl_backbuffer(frontBuffer, pBackBuffer);

            SDL_RenderCopyEx(pRenderer, pBackBuffer, nullptr, nullptr, 0.0, nullptr, SDL_FLIP_VERTICAL);
            SDL_RenderPresent(pRenderer);
        }
    }

    LS_LOG_MSG(
        "Rendered ", totalFrames, " frames in ", totalSeconds, " seconds (",
        ((double)totalFrames/(double)totalSeconds), " average fps).");

    SDL_DestroyTexture(pBackBuffer);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;
}
