
#include "soft_render/SR_Mesh.hpp"



void sr_reset(SR_Mesh& m) noexcept
{
    m.vaoId = 0;
    m.elementBegin = 0;
    m.elementEnd = 0;
    m.mode = RENDER_MODE_TRIANGLES;
    m.materialId = 0;
}
