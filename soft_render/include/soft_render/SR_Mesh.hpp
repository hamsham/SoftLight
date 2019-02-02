
#ifndef SR_MESH_HPP
#define SR_MESH_HPP

#include <cstdlib> // size_t
#include <cstdint> // uint32_t



/*--------------------------------------
 *
--------------------------------------*/
enum SR_RenderMode : uint32_t
{
    RENDER_MODE_POINTS            = 0x00000001,
    RENDER_MODE_INDEXED_POINTS    = 0x00000003,
    RENDER_MODE_LINES             = 0x00000004,
    RENDER_MODE_INDEXED_LINES     = 0x0000000C,
    RENDER_MODE_TRI_WIRE          = 0x00000044,
    RENDER_MODE_INDEXED_TRI_WIRE  = 0x0000004C,
    RENDER_MODE_TRIANGLES         = 0x00000010,
    RENDER_MODE_INDEXED_TRIANGLES = 0x00000030
};



/*--------------------------------------
 *
--------------------------------------*/
struct SR_Mesh
{
    // 32 bytes
    size_t vaoId;
    size_t elementBegin;
    size_t elementEnd;

    SR_RenderMode mode;
    uint32_t materialId;
};



void sr_reset(SR_Mesh& m) noexcept;



#endif /* SR_MESH_HPP */
