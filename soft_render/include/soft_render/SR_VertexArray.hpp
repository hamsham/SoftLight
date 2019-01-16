
#ifndef SR_VERTEX_ARRAY_HPP
#define SR_VERTEX_ARRAY_HPP

#include <cstddef> // ptrdiff_t
#include <vector>

#include "soft_render/SR_Geometry.hpp"



class SR_VertexArray
{
  private:
    uint64_t mVboId;

    uint64_t mIboId;

    std::vector<SR_Dimension> mDimens;

    std::vector<SR_DataType> mTypes;

    std::vector<ptrdiff_t> mOffsets;

    std::vector<ptrdiff_t> mStrides;

  public:
    ~SR_VertexArray();

    SR_VertexArray();

    SR_VertexArray(const SR_VertexArray& v);

    SR_VertexArray(SR_VertexArray&& v);

    SR_VertexArray& operator=(const SR_VertexArray& v);

    SR_VertexArray& operator=(SR_VertexArray&& v);

    int set_num_bindings(std::size_t numBindings);

    std::size_t num_bindings() const;

    void set_binding(
        std::size_t bindId,
        ptrdiff_t offset,
        ptrdiff_t stride,
        SR_Dimension numDimens,
        SR_DataType vertType);

    ptrdiff_t offset(std::size_t bindId) const;

    ptrdiff_t offset(std::size_t bindId, std::size_t vertId) const;

    ptrdiff_t stride(std::size_t bindId) const;

    SR_DataType type(std::size_t bindId) const;

    SR_Dimension dimensions(std::size_t bindId) const;

    void remove_binding(std::size_t bindId);

    void set_vertex_buffer(std::size_t vboId);

    void remove_vertex_buffer();

    bool has_vertex_buffer() const;

    uint64_t get_vertex_buffer() const;

    void set_index_buffer(std::size_t iboId);

    void remove_index_buffer();

    bool has_index_buffer() const;

    uint64_t get_index_buffer() const;

    void terminate();
};



/*--------------------------------------
 *
--------------------------------------*/
inline uint64_t SR_VertexArray::get_vertex_buffer() const
{
    return mVboId;
}



/*--------------------------------------
 *
--------------------------------------*/
inline uint64_t SR_VertexArray::get_index_buffer() const
{
    return mIboId;
}



#endif /* SR_VERTEX_ARRAY_HPP */

