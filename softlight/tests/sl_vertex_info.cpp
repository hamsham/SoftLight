
#include "lightsky/utils/Log.h"

#include "softlight/SL_Geometry.hpp"

namespace utils = ls::utils;




void print_data_type_info()
{
    utils::log_msg(
        "Data Byte Sizes:",
        "\n\tVERTEX_DATA_BYTE:    ", sl_bytes_per_type(VERTEX_DATA_BYTE),
        "\n\tVERTEX_DATA_SHORT:   ", sl_bytes_per_type(VERTEX_DATA_SHORT),
        "\n\tVERTEX_DATA_INT:     ", sl_bytes_per_type(VERTEX_DATA_INT),
        "\n\tVERTEX_DATA_LONG:    ", sl_bytes_per_type(VERTEX_DATA_LONG),
        "\n\tVERTEX_DATA_FLOAT:   ", sl_bytes_per_type(VERTEX_DATA_FLOAT),
        "\n\tVERTEX_DATA_DOUBLE:  ", sl_bytes_per_type(VERTEX_DATA_DOUBLE),
        "\n\tVERTEX_DATA_INVALID: ", sl_bytes_per_type(VERTEX_DATA_INVALID),
        '\n');
}



void print_vertex_info()
{
    utils::log_msg(
        "Vertex Byte Sizes:",
        "\n\tPOSITION_VERTEX:           ", sl_vertex_byte_size(POSITION_VERTEX),
        "\n\tTEXTURE_VERTEX:            ", sl_vertex_byte_size(TEXTURE_VERTEX),
        "\n\tPACKED_TEXTURE_VERTEX:     ", sl_vertex_byte_size(PACKED_TEXTURE_VERTEX),
        "\n\tCOLOR_VERTEX:              ", sl_vertex_byte_size(COLOR_VERTEX),
        "\n\tNORMAL_VERTEX:             ", sl_vertex_byte_size(NORMAL_VERTEX),
        "\n\tTANGENT_VERTEX:            ", sl_vertex_byte_size(TANGENT_VERTEX),
        "\n\tBITANGENT_VERTEX:          ", sl_vertex_byte_size(BITANGENT_VERTEX),
        "\n\tPACKED_NORMAL_VERTEX:      ", sl_vertex_byte_size(PACKED_NORMAL_VERTEX),
        "\n\tPACKED_TANGENT_VERTEX:     ", sl_vertex_byte_size(PACKED_TANGENT_VERTEX),
        "\n\tPACKED_BITANGENT_VERTEX:   ", sl_vertex_byte_size(PACKED_BITANGENT_VERTEX),
        "\n\tMODEL_MAT_VERTEX:          ", sl_vertex_byte_size(MODEL_MAT_VERTEX),
        "\n\tBONE_ID_VERTEX:            ", sl_vertex_byte_size(BONE_ID_VERTEX),
        "\n\tPACKED_BONE_ID_VERTEX:     ", sl_vertex_byte_size(PACKED_BONE_ID_VERTEX),
        "\n\tBONE_WEIGHT_VERTEX:        ", sl_vertex_byte_size(BONE_WEIGHT_VERTEX),
        "\n\tPACKED_BONE_WEIGHT_VERTEX: ", sl_vertex_byte_size(PACKED_BONE_WEIGHT_VERTEX),
        "\n\tINDEX_VERTEX:              ", sl_vertex_byte_size(INDEX_VERTEX),
        "\n\tBBOX_TRR_VERTEX:           ", sl_vertex_byte_size(BBOX_TRR_VERTEX),
        "\n\tBBOX_BFL_VERTEX:           ", sl_vertex_byte_size(BBOX_BFL_VERTEX),
        "\n\tSTANDARD_VERTEX:           ", sl_vertex_byte_size(STANDARD_VERTEX),
        "\n\tBONE_VERTEX:               ", sl_vertex_byte_size(BONE_VERTEX),
        "\n\tOCCLUSION_VERTEX:          ", sl_vertex_byte_size(OCCLUSION_VERTEX),
        '\n');
}



void print_stride_info()
{
    utils::log_msg(
        "Vertex Strides:",
        "\n\tPOSITION_VERTEX:           ", sl_vertex_stride(POSITION_VERTEX),
        "\n\tTEXTURE_VERTEX:            ", sl_vertex_stride(TEXTURE_VERTEX),
        "\n\tPACKED_TEXTURE_VERTEX:     ", sl_vertex_stride(PACKED_TEXTURE_VERTEX),
        "\n\tCOLOR_VERTEX:              ", sl_vertex_stride(COLOR_VERTEX),
        "\n\tNORMAL_VERTEX:             ", sl_vertex_stride(NORMAL_VERTEX),
        "\n\tTANGENT_VERTEX:            ", sl_vertex_stride(TANGENT_VERTEX),
        "\n\tBITANGENT_VERTEX:          ", sl_vertex_stride(BITANGENT_VERTEX),
        "\n\tPACKED_NORMAL_VERTEX:      ", sl_vertex_stride(PACKED_NORMAL_VERTEX),
        "\n\tPACKED_TANGENT_VERTEX:     ", sl_vertex_stride(PACKED_TANGENT_VERTEX),
        "\n\tPACKED_BITANGENT_VERTEX:   ", sl_vertex_stride(PACKED_BITANGENT_VERTEX),
        "\n\tMODEL_MAT_VERTEX:          ", sl_vertex_stride(MODEL_MAT_VERTEX),
        "\n\tBONE_ID_VERTEX:            ", sl_vertex_stride(BONE_ID_VERTEX),
        "\n\tPACKED_BONE_ID_VERTEX:     ", sl_vertex_stride(PACKED_BONE_ID_VERTEX),
        "\n\tBONE_WEIGHT_VERTEX:        ", sl_vertex_stride(BONE_WEIGHT_VERTEX),
        "\n\tPACKED_BONE_WEIGHT_VERTEX: ", sl_vertex_stride(PACKED_BONE_WEIGHT_VERTEX),
        "\n\tINDEX_VERTEX:              ", sl_vertex_stride(INDEX_VERTEX),
        "\n\tBBOX_TRR_VERTEX:           ", sl_vertex_stride(BBOX_TRR_VERTEX),
        "\n\tBBOX_BFL_VERTEX:           ", sl_vertex_stride(BBOX_BFL_VERTEX),
        "\n\tSTANDARD_VERTEX:           ", sl_vertex_stride(STANDARD_VERTEX),
        "\n\tBONE_VERTEX:               ", sl_vertex_stride(BONE_VERTEX),
        "\n\tOCCLUSION_VERTEX:          ", sl_vertex_stride(OCCLUSION_VERTEX),
        '\n');
}



void print_index_info()
{
    utils::log_msg(
        "Index Info:",
        "\n\tRequired index type  (254):    ", sl_required_index_type(254),
        "\n\tRequired index bytes (254):    ", sl_index_byte_size(sl_required_index_type(254)),
        "\n\tRequired index type  (255):    ", sl_required_index_type(255),
        "\n\tRequired index bytes (255):    ", sl_index_byte_size(sl_required_index_type(255)),
        "\n\tRequired index type  (256):    ", sl_required_index_type(256),
        "\n\tRequired index bytes (256):    ", sl_index_byte_size(sl_required_index_type(256)),
        "\n\tRequired index type  (32768):  ", sl_required_index_type(32768),
        "\n\tRequired index bytes (32768):  ", sl_index_byte_size(sl_required_index_type(32768)),
        "\n\tRequired index type  (65536):  ", sl_required_index_type(65536),
        "\n\tRequired index bytes (65536):  ", sl_index_byte_size(sl_required_index_type(65536)),
        '\n');
}



int main()
{
    print_data_type_info();
    print_vertex_info();
    print_stride_info();
    print_index_info();

    return 0;
}
