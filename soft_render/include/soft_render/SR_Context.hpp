
#ifndef SR_CONTEXT_HPP
#define SR_CONTEXT_HPP

#include <cstdint> // uint16_t, uint32_t
#include <memory> // std::shared_ptr
#include <vector>

#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_ShaderProcessor.hpp" // SR_ProcessorPool



class SR_Framebuffer;
struct SR_FragmentShader;
class SR_IndexBuffer;
struct SR_Mesh;
class SR_Shader;
class SR_Texture;
struct SR_UniformBuffer;
class SR_VertexArray;
class SR_VertexBuffer;
struct SR_VertexShader;
class SR_WindowBuffer;



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
    const std::vector<SR_Shader>& shaders() const;

    const SR_Shader& shader(std::size_t index) const;

    SR_Shader& shader(std::size_t index);

    std::size_t create_shader(
        const SR_VertexShader& vertShader,
        const SR_FragmentShader& fragShader,
        const std::shared_ptr<SR_UniformBuffer>& pUniforms = nullptr);

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
    void draw(const SR_Mesh& m, uint32_t shaderId, uint32_t fboId) noexcept;

    /*
     *
     */
    void blit(SR_WindowBuffer& buffer, uint32_t textureId) noexcept;

    /*
     *
     */
    unsigned num_threads() const noexcept;

    /*
     *
     */
    unsigned num_threads(unsigned inNumThreads) noexcept;
};




#endif /* SR_CONTEXT_HPP */
