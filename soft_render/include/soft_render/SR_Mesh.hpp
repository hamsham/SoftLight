
#ifndef SR_MESH_HPP
#define SR_MESH_HPP

#include <cstdint> // uint16_t, uint32_t



/*--------------------------------------
 *
--------------------------------------*/
enum SR_RenderMode : uint16_t
{
    RENDER_MODE_POINTS            = 0x0001,
    RENDER_MODE_INDEXED_POINTS    = 0x0003,
    RENDER_MODE_LINES             = 0x0004,
    RENDER_MODE_INDEXED_LINES     = 0x000C,
    RENDER_MODE_TRIANGLES         = 0x0010,
    RENDER_MODE_INDEXED_TRIANGLES = 0x1030
};



/*--------------------------------------
 *
--------------------------------------*/
struct SR_Mesh
{
    // 128 bytes
    uint32_t vaoId;
    uint32_t elementBegin;
    uint32_t elementEnd;

    SR_RenderMode mode;
    uint16_t materialId;
};



void sr_reset(SR_Mesh& m) noexcept;



#endif /* SR_MESH_HPP */
