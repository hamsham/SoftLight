
#ifndef SL_CONTEXT_HPP
#define SL_CONTEXT_HPP

#include <array>
#include <cstdint> // uint16_t, uint32_t
#include <memory> // std::shared_ptr
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "softlight/SL_PipelineState.hpp"
#include "softlight/SL_ProcessorPool.hpp"
#include "softlight/SL_Setup.hpp"
#include "softlight/SL_ViewportState.hpp"



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
    friend class SL_ProcessorPool;

  private:
    SL_AlignedVector<SL_VertexArray> mVaos;

    SL_AlignedVector<SL_Texture*> mTextures;

    SL_AlignedVector<SL_Framebuffer> mFbos;

    SL_AlignedVector<SL_VertexBuffer> mVbos;

    SL_AlignedVector<SL_IndexBuffer> mIbos;

    SL_AlignedVector<SL_UniformBuffer> mUniforms;

    SL_AlignedVector<SL_Shader> mShaders;

    SL_ViewportState mViewState;

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
    const SL_ViewportState& viewport_state() const noexcept;

    SL_ViewportState& viewport_state() noexcept;

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
inline const SL_ViewportState& SL_Context::viewport_state() const noexcept
{
    return mViewState;
}



/*-------------------------------------
-------------------------------------*/
inline SL_ViewportState& SL_Context::viewport_state() noexcept
{
    return mViewState;
}



#endif /* SL_CONTEXT_HPP */
