
#include <fstream>
#include <iostream>
#include <memory> // std::move()
#include "lightsky/utils/Copy.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"
#include "soft_render/SR_Camera.hpp"

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_WindowBuffer.hpp"
#include "soft_render/SR_WindowEvent.hpp"

#include "test_common.hpp"



/*-----------------------------------------------------------------------------
 * Shader data to render volumes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Uniforms to share across shader stages
--------------------------------------*/
struct VolumeUniforms : SR_UniformBuffer
{
    const SR_Texture* pTexture;
    math::mat4 modelMatrix;
    math::mat4 mvpMatrix;
    math::mat4 viewMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _volume_vert_shader(const uint32_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const VolumeUniforms* pUniforms = static_cast<const VolumeUniforms*>(uniforms);

    const math::vec3& vert = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec3& uv   = *vbo.element<const math::vec3>(vao.offset(1, vertId));
    const math::vec3& norm = *vbo.element<const math::vec3>(vao.offset(2, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
    varyings[1] = math::vec4{uv[0], uv[1], uv[2], 0.f};
    varyings[2] = math::normalize(pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f});

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader volume_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.shader = _volume_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _volume_frag_shader(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    math::vec4 pos = varyings[0];
    math::vec4 uv  = varyings[1];

    const VolumeUniforms* pUniforms = static_cast<const VolumeUniforms*>(uniforms);
    const SR_Texture*     volumeTex = pUniforms->pTexture;
    const math::mat4      camTrans  = pUniforms->viewMatrix;
    const math::vec4      camPos    = math::vec4{camTrans[0][2], camTrans[1][2], camTrans[2][2], 0.f};

    const float step = 1.f / 64.f;
    const math::vec4 rayDir = math::normalize(camPos - pos);

    unsigned texel = 0;

    for (unsigned i = 0; i < 32; ++i)
    {
        const uint8_t threshold = volumeTex->nearest<uint8_t>(uv[0], uv[1], uv[2]);
        texel += -(threshold > 24) & (threshold/4);

        if (texel >= 255 || uv >= 1.f || uv <= 0.f)
        {
            break;
        }

        uv += rayDir * step;
    }

    texel = math::min(texel, 255);

    const math::vec4&& pixel = color_cast<float, uint8_t>(math::vec4_t<uint8_t>{(uint8_t)texel});

    // output composition
    outputs[0] = math::min(pixel, math::vec4{1.f});

    return texel > 0;
}



SR_FragmentShader volume_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs = 1;
    shader.shader = _volume_frag_shader;

    return shader;
}



/*-------------------------------------
 * Read a volume file
-------------------------------------*/
int read_volume_file(SR_SceneGraph& graph)
{
    const std::string volFile = "testdata/head256x256x109";
    std::ifstream fin{volFile, std::ios::in | std::ios::binary};
    if (!fin.good())
    {
        return -1;
    }

    const size_t texId = graph.mContext.create_texture();
    SR_Texture&  pTex  = graph.mContext.texture(texId);

    if (0 != pTex.init(SR_COLOR_R_8U, 256, 256, 109))
    {
        return -2;
    }

    constexpr size_t numTexels = 256*256*109;
    constexpr size_t numBytes = sizeof(char) * numTexels;

    fin.read(reinterpret_cast<char*>(pTex.data()), numBytes);
    fin.close();

    pTex.set_wrap_mode(SR_TexWrapMode::SR_TEXTURE_WRAP_CLAMP);

    return 0;
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SR_SceneGraph& graph)
{
    int retCode = 0;
    SR_Context& context = graph.mContext;
    constexpr unsigned numVerts = 36;
    constexpr size_t stride = sizeof(math::vec3);
    uint32_t numVboBytes = 0;

    uint32_t vboId = context.create_vbo();
    SR_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numVerts*stride*3);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    uint32_t vaoId = context.create_vao();
    SR_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    retCode = vao.set_num_bindings(3);
    if (retCode != 3)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    math::vec3 verts[numVerts];
    verts[0]  = math::vec3{-1.f, -1.f, 1.f};
    verts[1]  = math::vec3{1.f, -1.f, 1.f};
    verts[2]  = math::vec3{1.f, 1.f, 1.f};
    verts[3]  = math::vec3{1.f, 1.f, 1.f};
    verts[4]  = math::vec3{-1.f, 1.f, 1.f};
    verts[5]  = math::vec3{-1.f, -1.f, 1.f};
    verts[6]  = math::vec3{1.f, -1.f, 1.f};
    verts[7]  = math::vec3{1.f, -1.f, -1.f};
    verts[8]  = math::vec3{1.f, 1.f, -1.f};
    verts[9]  = math::vec3{1.f, 1.f, -1.f};
    verts[10] = math::vec3{1.f, 1.f, 1.f};
    verts[11] = math::vec3{1.f, -1.f, 1.f};
    verts[12] = math::vec3{-1.f, 1.f, -1.f};
    verts[13] = math::vec3{1.f, 1.f, -1.f};
    verts[14] = math::vec3{1.f, -1.f, -1.f};
    verts[15] = math::vec3{1.f, -1.f, -1.f};
    verts[16] = math::vec3{-1.f, -1.f, -1.f};
    verts[17] = math::vec3{-1.f, 1.f, -1.f};
    verts[18] = math::vec3{-1.f, -1.f, -1.f};
    verts[19] = math::vec3{-1.f, -1.f, 1.f};
    verts[20] = math::vec3{-1.f, 1.f, 1.f};
    verts[21] = math::vec3{-1.f, 1.f, 1.f};
    verts[22] = math::vec3{-1.f, 1.f, -1.f};
    verts[23] = math::vec3{-1.f, -1.f, -1.f};
    verts[24] = math::vec3{-1.f, -1.f, -1.f};
    verts[25] = math::vec3{1.f, -1.f, -1.f};
    verts[26] = math::vec3{1.f, -1.f, 1.f};
    verts[27] = math::vec3{1.f, -1.f, 1.f};
    verts[28] = math::vec3{-1.f, -1.f, 1.f};
    verts[29] = math::vec3{-1.f, -1.f, -1.f};
    verts[30] = math::vec3{-1.f, 1.f, 1.f};
    verts[31] = math::vec3{1.f, 1.f, 1.f};
    verts[32] = math::vec3{1.f, 1.f, -1.f};
    verts[33] = math::vec3{1.f, 1.f, -1.f};
    verts[34] = math::vec3{-1.f, 1.f, -1.f};
    verts[35] = math::vec3{-1.f, 1.f, 1.f};
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(0, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    // Ensure UVs are only between 0-1.
    for (unsigned i = 0; i < numVerts; ++i)
    {
        verts[i] = 0.5f + verts[i] * 0.5f;
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(1, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    // Normalizing the vertex positions should allow for smooth shading.
    for (unsigned i = 0; i < numVerts; ++i)
    {
        verts[i] = math::normalize(verts[i] - 0.5f);
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(2, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    assert(numVboBytes == (numVerts*stride*3));

    graph.mMaterials.push_back(SR_Material());
    SR_Material& material = graph.mMaterials.back();
    material.pTextures[0] = context.textures().back();

    graph.mMeshes.push_back(SR_Mesh());
    SR_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numVerts;
    mesh.mode = SR_RenderMode::RENDER_MODE_TRIANGLES;
    mesh.materialId = graph.mMaterials.size()-1;

    return 0;
}




/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
void render_volume(SR_SceneGraph*, const math::mat4&);



utils::Pointer<SR_SceneGraph> init_volume_context()
{
    int retCode = 0;
    utils::Pointer<SR_SceneGraph> pGraph  {new SR_SceneGraph{}};
    SR_Context&                   context = pGraph->mContext;
    uint32_t                      fboId   = context.create_framebuffer();
    uint32_t                      texId   = context.create_texture();
    uint32_t                      depthId = context.create_texture();

    context.num_threads(4);

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

    retCode = read_volume_file(*pGraph);
    assert(retCode == 0);

    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);

    const SR_VertexShader&&   volVertShader = volume_vert_shader();
    const SR_FragmentShader&& volFragShader = volume_frag_shader();
    std::shared_ptr<VolumeUniforms> pUniforms{new VolumeUniforms};
    pUniforms->pTexture = context.textures().back();

    uint32_t volShaderId = context.create_shader(volVertShader, volFragShader, pUniforms);
    assert(volShaderId == 0);
    (void)volShaderId;

    pGraph->update();

    const math::mat4&& viewMatrix = math::look_at(math::vec3{3.f}, math::vec3{0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(45.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);
    pUniforms->viewMatrix = viewMatrix;

    render_volume(pGraph.get(), projMatrix*viewMatrix);
    sr_img_save_ppm(IMAGE_WIDTH, IMAGE_HEIGHT, reinterpret_cast<const math::vec3_t<uint8_t>*>(tex.data()), "volume_test.ppm");

    if (retCode != 0)
    {
        abort();
    }

    std::cout << "First frame rendered." << std::endl;

    return pGraph;
}



/*-------------------------------------
 * Render a scene
-------------------------------------*/
void render_volume(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context&      context   = pGraph->mContext;
    VolumeUniforms*  pUniforms = static_cast<VolumeUniforms*>(context.shader(0).uniforms().get());
    const math::mat4 modelMat  = math::mat4{1.f};
    pUniforms->modelMatrix     = modelMat;
    pUniforms->mvpMatrix       = vpMatrix * modelMat;

    context.draw(pGraph->mMeshes.back(), 0, 0);
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SR_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 10.f;

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



/*-----------------------------------------------------------------------------
 * main()
-----------------------------------------------------------------------------*/
int main()
{
    ls::utils::Pointer<SR_RenderWindow> pWindow{std::move(SR_RenderWindow::create())};
    ls::utils::Pointer<SR_WindowBuffer> pRenderBuf{SR_WindowBuffer::create()};
    ls::utils::Pointer<SR_SceneGraph>   pGraph{std::move(init_volume_context())};
    ls::utils::Pointer<bool[]>          pKeySyms{new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    SR_Context& context = pGraph->mContext;
    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    float currSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;

    unsigned numThreads = context.num_threads();

    SR_Transform camTrans;
    camTrans.set_type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    camTrans.extract_transforms(math::look_from(math::vec3{3.f}, math::vec3{0.f}, math::vec3{0.f, 1.f, 0.f}));
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(45.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

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
                        {
                            unsigned w, h;
                            pWindow->get_size(w, h);
                            std::cout << "Window size changed: " << w << ' ' << h << std::endl;
                        }
                        break;

                    case SR_KeySymbol::KEY_SYM_RIGHT:
                        pWindow->set_size(IMAGE_WIDTH, IMAGE_HEIGHT);
                        {
                            unsigned w, h;
                            pWindow->get_size(w, h);
                            std::cout << "Window size changed: " << w << ' ' << h << std::endl;
                        }
                        break;

                    case SR_KeySymbol::KEY_SYM_UP:
                        numThreads = ls::math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_DOWN:
                        numThreads = ls::math::max(numThreads - 1u, 1u);
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
                    dx = ((float)mouse.dx / (float)pWindow->width()) * 0.05f;
                    dy = ((float)mouse.dy / (float)pWindow->height()) * -0.05f;
                    camTrans.rotate(math::vec3{dx, dy, 0.f});
                }
            }
        }
        else
        {
            timer.tick();
            const float tickTime = timer.tick_time().count();

            ++currFrames;
            currSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "FPS: " << (float)currFrames/currSeconds << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                SR_Context&      context   = pGraph->mContext;
                VolumeUniforms*  pUniforms = static_cast<VolumeUniforms*>(context.shader(0).uniforms().get());
                pUniforms->viewMatrix = camTrans.get_transform();
            }
            const math::mat4&& vpMatrix = projMatrix * camTrans.get_transform();

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
            }

            pGraph->update();

            //context.framebuffer(0).clear_color_buffer(0, SR_ColorRGB{128, 128, 168});
            context.framebuffer(0).clear_color_buffers();
            context.framebuffer(0).clear_depth_buffer();

            render_volume(pGraph.get(), vpMatrix);

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

    return pWindow->destroy();
}

