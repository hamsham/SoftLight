
#ifndef SL_VERTEX_ARRAY_HPP
#define SL_VERTEX_ARRAY_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Pointer.h"

#include "softlight/SL_Geometry.hpp" // SL_Dimension, SL_DataType



class SL_VertexArray
{
    enum
    {
        MAX_BINDINGS = 8,
        INVALID_BUFFER_ID = std::numeric_limits<std::size_t>::max(),
    };

  private:
    struct BindInfo
    {
        SL_Dimension dimens;
        SL_DataType type;
        ptrdiff_t offset;
        ptrdiff_t stride;
    };

    std::size_t mVboId;

    std::size_t mIboId;

    std::size_t mNumBindings;

    BindInfo mBindings[SL_VertexArray::MAX_BINDINGS];

  public:
    ~SL_VertexArray() noexcept;

    SL_VertexArray() noexcept;

    SL_VertexArray(const SL_VertexArray& v) noexcept;

    SL_VertexArray(SL_VertexArray&& v) noexcept;

    SL_VertexArray& operator=(const SL_VertexArray& v) noexcept;

    SL_VertexArray& operator=(SL_VertexArray&& v) noexcept;

    int set_num_bindings(std::size_t numBindings) noexcept;

    std::size_t num_bindings() const noexcept;

    void set_binding(
        std::size_t bindId,
        ptrdiff_t offset,
        ptrdiff_t stride,
        SL_Dimension numDimens,
        SL_DataType vertType) noexcept;

    ptrdiff_t offset(std::size_t bindId) const noexcept;

    ptrdiff_t offset(std::size_t bindId, std::size_t vertId) const noexcept;

    ptrdiff_t stride(std::size_t bindId) const noexcept;

    SL_DataType type(std::size_t bindId) const noexcept;

    SL_Dimension dimensions(std::size_t bindId) const noexcept;

    void remove_binding(std::size_t bindId) noexcept;

    std::size_t get_vertex_buffer() const noexcept;

    void set_vertex_buffer(std::size_t vboId) noexcept;

    void remove_vertex_buffer() noexcept;

    bool has_vertex_buffer() const noexcept;

    std::size_t get_index_buffer() const noexcept;

    void set_index_buffer(std::size_t iboId) noexcept;

    void remove_index_buffer() noexcept;

    bool has_index_buffer() const noexcept;

    void terminate() noexcept;
};



/*--------------------------------------
 * Retrieve the number of bindings associated with a VAO
--------------------------------------*/
inline std::size_t SL_VertexArray::num_bindings() const noexcept
{
    return mNumBindings;
}



/*--------------------------------------
 * Get the byte offset to the first element in a bound VBO
--------------------------------------*/
inline ptrdiff_t SL_VertexArray::offset(std::size_t bindId) const noexcept
{
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);
    return mBindings[bindId].offset;
}



/*--------------------------------------
 * Get the byte offset to an element in a bound VBO
--------------------------------------*/
inline ptrdiff_t SL_VertexArray::offset(std::size_t bindId, std::size_t vertId) const noexcept
{
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);

    const SL_VertexArray::BindInfo& binding = mBindings[bindId];
    return binding.offset + (binding.stride * vertId);
}



/*--------------------------------------
 * Get the number of bytes padded between an element in a VBO
--------------------------------------*/
inline ptrdiff_t SL_VertexArray::stride(std::size_t bindId) const noexcept
{
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);
    return mBindings[bindId].stride;
}



/*--------------------------------------
 * Retrieve the data type of a VBO element
--------------------------------------*/
inline SL_DataType SL_VertexArray::type(std::size_t bindId) const noexcept
{
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);
    return mBindings[bindId].type;
}



/*--------------------------------------
 * Determine the number of element dimensions (to help identify scalars, vectors).
--------------------------------------*/
inline SL_Dimension SL_VertexArray::dimensions(std::size_t bindId) const noexcept
{
    // LS_DEBUG_ASSERT(bindId < SL_VertexArray::MAX_BINDINGS);
    return mBindings[bindId].dimens;
}



/*--------------------------------------
 * Retrieve the ID of the VBO attached to *this.
--------------------------------------*/
inline std::size_t SL_VertexArray::get_vertex_buffer() const noexcept
{
    return mVboId;
}



/*--------------------------------------
 * Assign a VBO to this VAO
--------------------------------------*/
inline void SL_VertexArray::set_vertex_buffer(size_t vboId) noexcept
{
    mVboId = vboId;
}



/*--------------------------------------
 * Remove a VBO from *this VAO's binding.
--------------------------------------*/
inline void SL_VertexArray::remove_vertex_buffer() noexcept
{
    mVboId = SL_VertexArray::INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Determine if we have a VBO attached
--------------------------------------*/
inline bool SL_VertexArray::has_vertex_buffer() const noexcept
{
    return mVboId != SL_VertexArray::INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Retrieve the ID of the VBO attached to *this.
--------------------------------------*/
inline std::size_t SL_VertexArray::get_index_buffer() const noexcept
{
    return mIboId;
}



/*--------------------------------------
 * Attach an index buffer
--------------------------------------*/
inline void SL_VertexArray::set_index_buffer(size_t iboId) noexcept
{
    mIboId = iboId;
}



/*--------------------------------------
 * Remove an index buffer binding
--------------------------------------*/
inline void SL_VertexArray::remove_index_buffer() noexcept
{
    mIboId = SL_VertexArray::INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Check if we have an index buffer bound
--------------------------------------*/
inline bool SL_VertexArray::has_index_buffer() const noexcept
{
    return mIboId != SL_VertexArray::INVALID_BUFFER_ID;
}



#endif /* SL_VERTEX_ARRAY_HPP */

