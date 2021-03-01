
#ifndef SL_CONTEXT_HPP
#define SL_CONTEXT_HPP

#include <array>
#include <cstdint> // uint16_t, uint32_t
#include <memory> // std::shared_ptr
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "softlight/SL_ProcessorPool.hpp"
#include "softlight/SL_RasterState.hpp"
#include "softlight/SL_Setup.hpp"



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
class SL_Framebuffer;
struct SL_FragmentShader;
class SL_IndexBuffer;
struct SL_Mesh;
class SL_Shader;
class SL_Texture;
class SL_UniformBuffer;
class SL_VertexArray;
class SL_VertexBuffer;
struct SL_VertexShader;
class SL_WindowBuffer;



template <typename T>
struct SL_ColorRType;

namespace ls
{
namespace math
{
    template <typename T>
    struct vec2_t;

    template <typename T>
    struct vec3_t;

    template <typename T>
    union vec4_t;
}
}



/**----------------------------------------------------------------------------
 * @brief SL_Context object manage all "GPU" related data.
-----------------------------------------------------------------------------*/
class SL_Context
{
    friend class SL_SceneFilePreload;
    friend class SL_SceneFileLoader;

  private:
    SL_AlignedVector<SL_VertexArray> mVaos;

    SL_AlignedVector<SL_Texture*> mTextures;

    SL_AlignedVector<SL_Framebuffer> mFbos;

    SL_AlignedVector<SL_VertexBuffer> mVbos;

    SL_AlignedVector<SL_IndexBuffer> mIbos;

    SL_AlignedVector<SL_UniformBuffer> mUniforms;

    SL_AlignedVector<SL_Shader> mShaders;

    SL_RasterState mState;

    SL_ProcessorPool mProcessors;

  public:
    ~SL_Context() noexcept;

    SL_Context() noexcept;

    SL_Context(const SL_Context& c) noexcept;

    SL_Context(SL_Context&& c) noexcept;

    SL_Context& operator=(const SL_Context& c) noexcept;

    SL_Context& operator=(SL_Context&& c) noexcept;

    /*
     *
     */
    const SL_AlignedVector<SL_VertexArray>& vaos() const;

    const SL_VertexArray& vao(std::size_t index) const;

    SL_VertexArray& vao(std::size_t index);

    std::size_t create_vao();

    void destroy_vao(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_Texture*>& textures() const;

    const SL_Texture& texture(std::size_t index) const;

    SL_Texture& texture(std::size_t index);

    std::size_t create_texture();

    void destroy_texture(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_Framebuffer>& framebuffers() const;

    const SL_Framebuffer& framebuffer(std::size_t index) const;

    SL_Framebuffer& framebuffer(std::size_t index);

    std::size_t create_framebuffer();

    void destroy_framebuffer(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_VertexBuffer>& vbos() const;

    const SL_VertexBuffer& vbo(std::size_t index) const;

    SL_VertexBuffer& vbo(std::size_t index);

    std::size_t create_vbo();

    void destroy_vbo(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_IndexBuffer>& ibos() const;

    const SL_IndexBuffer& ibo(std::size_t index) const;

    SL_IndexBuffer& ibo(std::size_t index);

    std::size_t create_ibo();

    void destroy_ibo(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_UniformBuffer>& ubos() const;

    const SL_UniformBuffer& ubo(std::size_t index) const;

    SL_UniformBuffer& ubo(std::size_t index);

    std::size_t create_ubo();

    void destroy_ubo(std::size_t index);

    /*
     *
     */
    const SL_AlignedVector<SL_Shader>& shaders() const;

    const SL_Shader& shader(std::size_t index) const;

    SL_Shader& shader(std::size_t index);

    std::size_t create_shader(
        const SL_VertexShader& vertShader,
        const SL_FragmentShader& fragShader);

    std::size_t create_shader(
        const SL_VertexShader& vertShader,
        const SL_FragmentShader& fragShader,
        std::size_t uniformIndex);

    void destroy_shader(std::size_t index);

    /*
     *
     */
    void terminate();

    /*
     *
     */
    void import(SL_Context&& inContext) noexcept;

    /*
     *
     */
    void cull_mode(SL_CullMode cm) noexcept;

    SL_CullMode cull_mode() const noexcept;

    void depth_test(SL_DepthTest dt) noexcept;

    SL_DepthTest depth_test() const noexcept;

    void depth_mask(SL_DepthMask dm) noexcept;

    SL_DepthMask depth_mask() const noexcept;

    void blend_mode(SL_BlendMode bm) noexcept;

    SL_BlendMode blend_mode() const noexcept;

    void viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept;

    ls::math::vec4_t<uint16_t> viewport() const noexcept;

    void scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept;

    ls::math::vec4_t<uint16_t> scissor() const noexcept;

    /*
     *
     */
    void draw(const SL_Mesh& m, size_t shaderId, size_t fboId) noexcept;

    /*
     *
     */
    void draw_multiple(const SL_Mesh* meshes, size_t numMeshes, size_t shaderId, size_t fboId) noexcept;

    /*
     *
     */
    void draw_instanced(const SL_Mesh& meshes, size_t numInstances, size_t shaderId, size_t fboId) noexcept;

    /*
     *
     */
    void blit(size_t outTextureId, size_t inTextureId) noexcept;

    /*
     *
     */
    void blit(
        size_t outTextureId,
        size_t inTextureId,
        uint16_t srcX0,
        uint16_t srcY0,
        uint16_t srcX1,
        uint16_t srcY1,
        uint16_t dstX0,
        uint16_t dstY0,
        uint16_t dstX1,
        uint16_t dstY1) noexcept;

    /*
     *
     */
    void blit(SL_WindowBuffer& buffer, size_t textureId) noexcept;

    /*
     *
     */
    void blit(
        SL_WindowBuffer& buffer,
        size_t textureId,
        uint16_t srcX0,
        uint16_t srcY0,
        uint16_t srcX1,
        uint16_t srcY1,
        uint16_t dstX0,
        uint16_t dstY0,
        uint16_t dstX1,
        uint16_t dstY1) noexcept;

    /*
     *
     */
    void clear_color_buffer(size_t fboId, size_t attachmentId, const ls::math::vec4_t<double>& color) noexcept;

    /*
     *
     */
    void clear_depth_buffer(size_t fboId, double depth) noexcept;

    /*
     *
     */
    void clear_framebuffer(size_t fboId, size_t attachmentId, const ls::math::vec4_t<double>& color, double depth) noexcept;

    /*
     *
     */
    void clear_framebuffer(size_t fboId, const std::array<size_t, 2>& bufferIndices, const std::array<ls::math::vec4_t<double>, 2>& colors, double depth) noexcept;

    /*
     *
     */
    void clear_framebuffer(size_t fboId, const std::array<size_t, 3>& bufferIndices, const std::array<ls::math::vec4_t<double>, 3>& colors, double depth) noexcept;

    /*
     *
     */
    void clear_framebuffer(size_t fboId, const std::array<size_t, 4>& bufferIndices, const std::array<ls::math::vec4_t<double>, 4>& colors, double depth) noexcept;

    /*
     *
     */
    unsigned num_threads() const noexcept;

    /*
     *
     */
    unsigned num_threads(unsigned inNumThreads) noexcept;
};



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::cull_mode(SL_CullMode cm) noexcept
{
    mState.cull_mode(cm);
}



/*-------------------------------------
-------------------------------------*/
inline SL_CullMode SL_Context::cull_mode() const noexcept
{
    return mState.cull_mode();
}



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::depth_test(SL_DepthTest dt) noexcept
{
    mState.depth_test(dt);
}



/*-------------------------------------
-------------------------------------*/
inline SL_DepthTest SL_Context::depth_test() const noexcept
{
    return mState.depth_test();
}



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::depth_mask(SL_DepthMask dm) noexcept
{
    mState.depth_mask(dm);
}



/*-------------------------------------
-------------------------------------*/
inline SL_DepthMask SL_Context::depth_mask() const noexcept
{
    return mState.depth_mask();
}



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::blend_mode(SL_BlendMode bm) noexcept
{
    mState.blend_mode(bm);
}



/*-------------------------------------
-------------------------------------*/
inline SL_BlendMode SL_Context::blend_mode() const noexcept
{
    return mState.blend_mode();
}



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept
{
    mState.viewport(x, y, w, h);
}



/*-------------------------------------
-------------------------------------*/
inline ls::math::vec4_t<uint16_t> SL_Context::viewport() const noexcept
{
    return mState.viewport();
}



/*-------------------------------------
-------------------------------------*/
inline void SL_Context::scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept
{
    mState.scissor(x, y, w, h);
}



/*-------------------------------------
-------------------------------------*/
inline ls::math::vec4_t<uint16_t> SL_Context::scissor() const noexcept
{
    return mState.scissor();
}



#endif /* SL_CONTEXT_HPP */
