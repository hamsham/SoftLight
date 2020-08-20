
#include "softlight/SL_Mesh.hpp"



void sl_reset(SL_Mesh& m) noexcept
{
    m.vaoId = 0;
    m.elementBegin = 0;
    m.elementEnd = 0;
    m.mode = RENDER_MODE_TRIANGLES;
    m.materialId = 0;
}
