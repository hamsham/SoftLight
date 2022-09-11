
#ifndef SL_MESH_HPP
#define SL_MESH_HPP

#include <cstdlib> // size_t
#include <cstdint> // uint32_t



/*--------------------------------------
 * Primitive types for rendering a mesh.
--------------------------------------*/
enum SL_RenderMode : uint32_t
{
    RENDER_MODE_POINTS            = 0x00000001,
    RENDER_MODE_INDEXED_POINTS    = 0x00000003,
    RENDER_MODE_LINES             = 0x00000004,
    RENDER_MODE_INDEXED_LINES     = 0x0000000C,
    RENDER_MODE_TRIANGLES         = 0x00000010,
    RENDER_MODE_INDEXED_TRIANGLES = 0x00000030,
    RENDER_MODE_TRI_WIRE          = 0x00000050,
    RENDER_MODE_INDEXED_TRI_WIRE  = 0x00000070,
};



/*--------------------------------------
 * Compact data structure for mesh information on the GPU.
--------------------------------------*/
struct SL_Mesh
{
    // 32 bytes
    size_t vaoId;
    size_t elementBegin;
    size_t elementEnd;

    SL_RenderMode mode;
    uint32_t materialId;
};



void sl_reset(SL_Mesh& m) noexcept;



#endif /* SL_MESH_HPP */
