
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_AnimationChannel.hpp"
#include "soft_render/SR_AnimationPlayer.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_WindowBuffer.hpp"
#include "soft_render/SR_WindowEvent.hpp"

#include "test_common.hpp"



/*-------------------------------------
 *
-------------------------------------*/
void setup_animations(SR_SceneGraph& graph, SR_AnimationPlayer& animPlayer)
{
    std::vector<SR_Animation>& sceneAnims = graph.mAnimations;

    for (std::vector<SR_AnimationChannel>& animList : graph.mNodeAnims)
    {
        for (SR_AnimationChannel& track : animList)
        {
            track.mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_INTERPOLATE;
        }
    }

    std::cout << "Running " << sceneAnims.size() << " animations." << std::endl;

    animPlayer.set_play_state(SR_AnimationState::SR_ANIM_STATE_PLAYING);
    animPlayer.set_num_plays(SR_AnimationPlayer::PLAY_ONCE);
    animPlayer.set_time_dilation(1.f);
}



/*-------------------------------------
 * Animation updating
-------------------------------------*/
void update_animations(SR_SceneGraph& graph, SR_AnimationPlayer& animPlayer, unsigned& currentAnimId, float tickTime)
{
    if (graph.mAnimations.empty())
    {
        return;
    }

    // Play the current animation until it stops. Then move onto the next animation.
    if (animPlayer.is_stopped())
    {
        std::cout << "Completed animation " << currentAnimId << ". ";
        std::vector<SR_Animation>& animations = graph.mAnimations;
        currentAnimId = (currentAnimId + 1) % animations.size();

        // reset the transformations in graph graph to those at the beginning of an animation
        SR_Animation& initialState = graph.mAnimations[currentAnimId];
        initialState.init(graph);

        animPlayer.set_play_state(SR_AnimationState::SR_ANIM_STATE_PLAYING);
        animPlayer.set_num_plays(SR_AnimationPlayer::PLAY_ONCE);

        std::cout << "Now playing animation " << currentAnimId << '.' << std::endl;
    }

    animPlayer.tick(graph, currentAnimId, 1000u*tickTime);
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



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    ls::utils::Pointer<SR_RenderWindow> pWindow{std::move(SR_RenderWindow::create())};
    ls::utils::Pointer<SR_WindowBuffer> pRenderBuf{SR_WindowBuffer::create()};
    ls::utils::Pointer<SR_SceneGraph>   pGraph{std::move(create_context())};
    ls::utils::Pointer<bool[]>          pKeySyms{new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    SR_Context& context = pGraph->mContext;
    SR_AnimationPlayer animPlayer;
    unsigned currentAnimId = 0;

    setup_animations(*pGraph, animPlayer);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
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
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

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
            ++totalFrames;
            currSeconds += tickTime;
            totalSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "MS/F: " << 1000.f*(currSeconds/(float)currFrames) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            if (totalFrames >= 600)
            {
                shouldQuit = true;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                MeshUniforms* pUniforms = static_cast<MeshUniforms*>(context.shader(1).uniforms().get());
                const math::vec3&& camTransPos = -camTrans.get_position();
                pUniforms->light.pos = {camTransPos[0], camTransPos[1], camTransPos[2], 1.f};

                const math::mat4& v = camTrans.get_transform();
                pUniforms->spot.direction = math::normalize(math::vec4{v[0][2], v[1][2], v[2][2], 0.f});
            }
            const math::mat4&& vpMatrix = projMatrix * camTrans.get_transform();

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
            }

            update_animations(*pGraph, animPlayer, currentAnimId, tickTime);
            pGraph->update();

            //context.framebuffer(0).clear_color_buffer(0, SR_ColorRGB{128, 128, 168});
            context.framebuffer(0).clear_color_buffers();
            context.framebuffer(0).clear_depth_buffer();
            render_scene(pGraph.get(), vpMatrix);
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
