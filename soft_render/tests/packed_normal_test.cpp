
#include "lightsky/utils/Log.h"

#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_PackedVertex.hpp"



int main()
{
    ls::math::vec3&& n = ls::math::normalize(ls::math::vec3{0.5f, 0.25f, 0.25f});
    int32_t i = sr_pack_vertex_2_10_10_10(n);
    ls::math::vec3&& p = sr_unpack_vertex_vec3(i);

    LS_LOG_MSG("Unpacked normal: ", n[0], ", ", n[1], ", ", n[2]);
    LS_LOG_MSG("Integral normal: ", i);
    LS_LOG_MSG("Unpacked normal: ", p[0], ", ", p[1], ", ", p[2]);

    return 0;
}
