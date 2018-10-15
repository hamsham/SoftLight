
#ifndef PROJECT_TEST_COMMON_HPP
#define PROJECT_TEST_COMMON_HPP

#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_UniformBuffer.hpp"



#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */



namespace utils = ls::utils;
namespace math = ls::math;



struct Light
{
    math::vec4 pos;
    math::vec4 ambient;
    math::vec4 diffuse;
    math::vec4 specular;
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
};



struct MeshUniforms : SR_UniformBuffer
{
    const SR_Texture* pTexture;
    Light light;
    PointLight point;
    SpotLight spot;

    math::mat4 modelMatrix;
    math::mat4 mvpMatrix;
};




/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
SR_VertexShader normal_vert_shader();



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
SR_FragmentShader normal_frag_shader();



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
SR_VertexShader texture_vert_shader();



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
SR_FragmentShader texture_frag_shader();



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> create_context();


/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix);



#endif /* PROJECT_TEST_COMMON_HPP */
