
#ifndef SR_CONTEXT_HPP
#define SR_CONTEXT_HPP

#include <cstdint> // uint16_t, uint32_t
#include <memory> // std::shared_ptr
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_ProcessorPool.hpp"



class SR_Framebuffer;
struct SR_FragmentShader;
class SR_IndexBuffer;
struct SR_Mesh;
class SR_Shader;
class SR_Texture;
class SR_UniformBuffer;
class SR_VertexArray;
class SR_VertexBuffer;
struct SR_VertexShader;
class SR_WindowBuffer;



template <typename T>
struct SR_ColorRType;

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



class SR_Context
{
    friend class SR_SceneFilePreload;
    friend class SR_SceneFileLoader;

  private:
    std::vector<SR_VertexArray> mVaos;

    std::vector<SR_Texture*> mTextures;

    std::vector<SR_Framebuffer> mFbos;

    std::vector<SR_VertexBuffer> mVbos;

    std::vector<SR_IndexBuffer> mIbos;

    std::vector<SR_UniformBuffer> mUniforms;

    std::vector<SR_Shader> mShaders;

    SR_ProcessorPool mProcessors;

  public:
    ~SR_Context() noexcept;

    SR_Context() noexcept;

    SR_Context(const SR_Context& c) noexcept;

    SR_Context(SR_Context&& c) noexcept;

    SR_Context& operator=(const SR_Context& c) noexcept;

    SR_Context& operator=(SR_Context&& c) noexcept;

    /*
     *
     */
    const std::vector<SR_VertexArray>& vaos() const;

    const SR_VertexArray& vao(std::size_t index) const;

    SR_VertexArray& vao(std::size_t index);

    std::size_t create_vao();

    void destroy_vao(std::size_t index);

    /*
     *
     */
    const std::vector<SR_Texture*>& textures() const;

    const SR_Texture& texture(std::size_t index) const;

    SR_Texture& texture(std::size_t index);

    std::size_t create_texture();

    void destroy_texture(std::size_t index);

    /*
     *
     */
    const std::vector<SR_Framebuffer>& framebuffers() const;

    const SR_Framebuffer& framebuffer(std::size_t index) const;

    SR_Framebuffer& framebuffer(std::size_t index);

    std::size_t create_framebuffer();

    void destroy_framebuffer(std::size_t index);

    /*
     *
     */
    const std::vector<SR_VertexBuffer>& vbos() const;

    const SR_VertexBuffer& vbo(std::size_t index) const;

    SR_VertexBuffer& vbo(std::size_t index);

    std::size_t create_vbo();

    void destroy_vbo(std::size_t index);

    /*
     *
     */
    const std::vector<SR_IndexBuffer>& ibos() const;

    const SR_IndexBuffer& ibo(std::size_t index) const;

    SR_IndexBuffer& ibo(std::size_t index);

    std::size_t create_ibo();

    void destroy_ibo(std::size_t index);

    /*
     *
     */
    const std::vector<SR_UniformBuffer>& ubos() const;

    const SR_UniformBuffer& ubo(std::size_t index) const;

    SR_UniformBuffer& ubo(std::size_t index);

    std::size_t create_ubo();

    void destroy_ubo(std::size_t index);

    /*
     *
     */
    const std::vector<SR_Shader>& shaders() const;

    const SR_Shader& shader(std::size_t index) const;

    SR_Shader& shader(std::size_t index);

    std::size_t create_shader(
        const SR_VertexShader& vertShader,
        const SR_FragmentShader& fragShader);

    std::size_t create_shader(
        const SR_VertexShader& vertShader,
        const SR_FragmentShader& fragShader,
        std::size_t uniformIndex);

    void destroy_shader(std::size_t index);

    /*
     *
     */
    void terminate();

    /*
     *
     */
    void import(SR_Context&& inContext) noexcept;

    /*
     *
     */
    void draw(const SR_Mesh& m, size_t shaderId, size_t fboId) noexcept;

    /*
     *
     */
    void draw_multiple(const SR_Mesh* meshes, size_t numMeshes, size_t shaderId, size_t fboId) noexcept;

    /*
     *
     */
    void draw_instanced(const SR_Mesh& meshes, size_t numInstances, size_t shaderId, size_t fboId) noexcept;

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
    void blit(SR_WindowBuffer& buffer, size_t textureId) noexcept;

    /*
     *
     */
    void blit(
        SR_WindowBuffer& buffer,
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
    template <typename color_type>
    void clear(size_t outTextureId, const color_type& color) noexcept;

    /*
     *
     */
    unsigned num_threads() const noexcept;

    /*
     *
     */
    unsigned num_threads(unsigned inNumThreads) noexcept;
};



extern template void SR_Context::clear<SR_ColorRType<uint8_t>>( size_t, const SR_ColorRType<uint8_t>&) noexcept;
extern template void SR_Context::clear<SR_ColorRType<uint16_t>>(size_t, const SR_ColorRType<uint16_t>&) noexcept;
extern template void SR_Context::clear<SR_ColorRType<uint32_t>>(size_t, const SR_ColorRType<uint32_t>&) noexcept;
extern template void SR_Context::clear<SR_ColorRType<uint64_t>>(size_t, const SR_ColorRType<uint64_t>&) noexcept;
extern template void SR_Context::clear<SR_ColorRType<float>>(   size_t, const SR_ColorRType<float>&) noexcept;
extern template void SR_Context::clear<SR_ColorRType<double>>(  size_t, const SR_ColorRType<double>&) noexcept;

extern template void SR_Context::clear<ls::math::vec2_t<uint8_t>>( size_t, const ls::math::vec2_t<uint8_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec2_t<uint16_t>>(size_t, const ls::math::vec2_t<uint16_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec2_t<uint32_t>>(size_t, const ls::math::vec2_t<uint32_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec2_t<uint64_t>>(size_t, const ls::math::vec2_t<uint64_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec2_t<float>>(   size_t, const ls::math::vec2_t<float>&) noexcept;
extern template void SR_Context::clear<ls::math::vec2_t<double>>(  size_t, const ls::math::vec2_t<double>&) noexcept;

extern template void SR_Context::clear<ls::math::vec3_t<uint8_t>>( size_t, const ls::math::vec3_t<uint8_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec3_t<uint16_t>>(size_t, const ls::math::vec3_t<uint16_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec3_t<uint32_t>>(size_t, const ls::math::vec3_t<uint32_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec3_t<uint64_t>>(size_t, const ls::math::vec3_t<uint64_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec3_t<float>>(   size_t, const ls::math::vec3_t<float>&) noexcept;
extern template void SR_Context::clear<ls::math::vec3_t<double>>(  size_t, const ls::math::vec3_t<double>&) noexcept;

extern template void SR_Context::clear<ls::math::vec4_t<uint8_t>>( size_t, const ls::math::vec4_t<uint8_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec4_t<uint16_t>>(size_t, const ls::math::vec4_t<uint16_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec4_t<uint32_t>>(size_t, const ls::math::vec4_t<uint32_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec4_t<uint64_t>>(size_t, const ls::math::vec4_t<uint64_t>&) noexcept;
extern template void SR_Context::clear<ls::math::vec4_t<float>>(   size_t, const ls::math::vec4_t<float>&) noexcept;
extern template void SR_Context::clear<ls::math::vec4_t<double>>(  size_t, const ls::math::vec4_t<double>&) noexcept;



#endif /* SR_CONTEXT_HPP */
