
#include "lightsky/utils/Log.h"

#include "lightsky/math/vec_utils.h"

#include "softlight/SL_PackedVertex.hpp"



int main()
{
    ls::math::vec4&& n = ls::math::normalize(ls::math::vec4{-0.5f, 0.35f, -0.25f, 0.f});
    int32_t i = sl_pack_vec4_10_10_10_2(n);
    ls::math::vec4&& p = sl_unpack_vec4_10_10_10_2(i);

    LS_LOG_MSG("Unpacked normal: ", n[0], ", ", n[1], ", ", n[2], ", ", n[3]);
    LS_LOG_MSG("Integral normal: ", i);
    LS_LOG_MSG("Unpacked normal: ", p[0], ", ", p[1], ", ", p[2], ", ", p[3]);

    return 0;
}
