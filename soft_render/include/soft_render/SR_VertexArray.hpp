
#ifndef SR_VERTEX_ARRAY_HPP
#define SR_VERTEX_ARRAY_HPP

#include <cstddef> // ptrdiff_t
#include <vector>

#include "soft_render/SR_Geometry.hpp"



class SR_VertexArray
{
    enum
    {
        SR_INVALID_BUFFER_ID = std::numeric_limits<uint32_t>::max()
    };

  private:
    uint64_t mVboId;

    uint64_t mIboId;

    std::vector<SR_Dimension> mDimens;

    std::vector<SR_DataType> mTypes;

    std::vector<ptrdiff_t> mOffsets;

    std::vector<ptrdiff_t> mStrides;

  public:
    ~SR_VertexArray() noexcept;

    SR_VertexArray() noexcept;

    SR_VertexArray(const SR_VertexArray& v) noexcept;

    SR_VertexArray(SR_VertexArray&& v) noexcept;

    SR_VertexArray& operator=(const SR_VertexArray& v) noexcept;

    SR_VertexArray& operator=(SR_VertexArray&& v) noexcept;

    int set_num_bindings(std::size_t numBindings) noexcept;

    std::size_t num_bindings() const noexcept;

    void set_binding(
        std::size_t bindId,
        ptrdiff_t offset,
        ptrdiff_t stride,
        SR_Dimension numDimens,
        SR_DataType vertType) noexcept;

    ptrdiff_t offset(std::size_t bindId) const noexcept;

    ptrdiff_t offset(std::size_t bindId, std::size_t vertId) const noexcept;

    ptrdiff_t stride(std::size_t bindId) const noexcept;

    SR_DataType type(std::size_t bindId) const noexcept;

    SR_Dimension dimensions(std::size_t bindId) const noexcept;

    void remove_binding(std::size_t bindId) noexcept;

    void set_vertex_buffer(std::size_t vboId) noexcept;

    void remove_vertex_buffer() noexcept;

    bool has_vertex_buffer() const noexcept;

    uint64_t get_vertex_buffer() const noexcept;

    void set_index_buffer(std::size_t iboId) noexcept;

    void remove_index_buffer() noexcept;

    bool has_index_buffer() const noexcept;

    uint64_t get_index_buffer() const noexcept;

    void terminate() noexcept;
};



/*--------------------------------------
 * Retrieve the number of bindings associated with a VAO
--------------------------------------*/
inline std::size_t SR_VertexArray::num_bindings() const noexcept
{
    return mDimens.size();
}



/*--------------------------------------
 * Get the byte offset to the first element in a bound VBO
--------------------------------------*/
inline ptrdiff_t SR_VertexArray::offset(std::size_t bindId) const noexcept
{
    return mOffsets[bindId];
}



/*--------------------------------------
 * Get the byte offset to an element in a bound VBO
--------------------------------------*/
inline ptrdiff_t SR_VertexArray::offset(std::size_t bindId, std::size_t vertId) const noexcept
{
    return mOffsets[bindId] + (mStrides[bindId] * vertId);
}



/*--------------------------------------
 * Get the number of bytes padded between an element in a VBO
--------------------------------------*/
inline ptrdiff_t SR_VertexArray::stride(std::size_t bindId) const noexcept
{
    return mStrides[bindId];
}



/*--------------------------------------
 * Retrieve the data type of a VBO element
--------------------------------------*/
inline SR_DataType SR_VertexArray::type(std::size_t bindId) const noexcept
{
    return mTypes[bindId];
}



/*--------------------------------------
 * Determine the number of element dimensions (to help identify scalars, vectors).
--------------------------------------*/
inline SR_Dimension SR_VertexArray::dimensions(std::size_t bindId) const noexcept
{
    return mDimens[bindId];
}



/*--------------------------------------
 * Assign a VBO to this VAO
--------------------------------------*/
inline void SR_VertexArray::set_vertex_buffer(size_t vboId) noexcept
{
    mVboId = vboId;
}



/*--------------------------------------
 * Remove a VBO from *this VAO's binding.
--------------------------------------*/
inline void SR_VertexArray::remove_vertex_buffer() noexcept
{
    mVboId = SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Determine if we have a VBO attached
--------------------------------------*/
inline bool SR_VertexArray::has_vertex_buffer() const noexcept
{
    return mVboId != SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Attach an index buffer
--------------------------------------*/
inline void SR_VertexArray::set_index_buffer(size_t iboId) noexcept
{
    mIboId = iboId;
}



/*--------------------------------------
 * Remove an index buffer binding
--------------------------------------*/
inline void SR_VertexArray::remove_index_buffer() noexcept
{
    mIboId = SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Check if we have an index buffer bound
--------------------------------------*/
inline bool SR_VertexArray::has_index_buffer() const noexcept
{
    return mIboId != SR_INVALID_BUFFER_ID;
}



/*--------------------------------------
 * Retrieve the ID of the VBO attached to *this.
--------------------------------------*/
inline uint64_t SR_VertexArray::get_vertex_buffer() const noexcept
{
    return mVboId;
}



/*--------------------------------------
 * Retrieve the ID of the VBO attached to *this.
--------------------------------------*/
inline uint64_t SR_VertexArray::get_index_buffer() const noexcept
{
    return mIboId;
}



#endif /* SR_VERTEX_ARRAY_HPP */

