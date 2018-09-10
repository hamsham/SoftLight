
#include <iostream>

#include "soft_render/SR_Geometry.hpp"



void print_data_type_info()
{
    std::cout
        << "Data Byte Sizes:"
        << "\n\tVERTEX_DATA_BYTE:    " << sr_bytes_per_type(VERTEX_DATA_BYTE)
        << "\n\tVERTEX_DATA_SHORT:   " << sr_bytes_per_type(VERTEX_DATA_SHORT)
        << "\n\tVERTEX_DATA_INT:     " << sr_bytes_per_type(VERTEX_DATA_INT)
        << "\n\tVERTEX_DATA_LONG:    " << sr_bytes_per_type(VERTEX_DATA_LONG)
        << "\n\tVERTEX_DATA_FLOAT:   " << sr_bytes_per_type(VERTEX_DATA_FLOAT)
        << "\n\tVERTEX_DATA_DOUBLE:  " << sr_bytes_per_type(VERTEX_DATA_DOUBLE)
        << "\n\tVERTEX_DATA_INVALID: " << sr_bytes_per_type(VERTEX_DATA_INVALID)
        << '\n' << std::endl;
}



void print_vertex_info()
{
    std::cout
        << "Vertex Byte Sizes:"
        << "\n\tPOSITION_VERTEX:    " << sr_vertex_byte_size(POSITION_VERTEX)
        << "\n\tTEXTURE_VERTEX:     " << sr_vertex_byte_size(TEXTURE_VERTEX)
        << "\n\tCOLOR_VERTEX:       " << sr_vertex_byte_size(COLOR_VERTEX)

        << "\n\tNORMAL_VERTEX:      " << sr_vertex_byte_size(NORMAL_VERTEX)
        << "\n\tTANGENT_VERTEX:     " << sr_vertex_byte_size(TANGENT_VERTEX)
        << "\n\tBITANGENT_VERTEX:   " << sr_vertex_byte_size(BITANGENT_VERTEX)

        << "\n\tMODEL_MAT_VERTEX:   " << sr_vertex_byte_size(MODEL_MAT_VERTEX)

        << "\n\tBONE_ID_VERTEX:     " << sr_vertex_byte_size(BONE_ID_VERTEX)
        << "\n\tBONE_WEIGHT_VERTEX: " << sr_vertex_byte_size(BONE_WEIGHT_VERTEX)

        << "\n\tAMBIENT_VERTEX:     " << sr_vertex_byte_size(AMBIENT_VERTEX)
        << "\n\tDIFFUSE_VERTEX:     " << sr_vertex_byte_size(DIFFUSE_VERTEX)
        << "\n\tSPECULAR_VERTEX:    " << sr_vertex_byte_size(SPECULAR_VERTEX)
        << "\n\tROUGHNESS_VERTEX:   " << sr_vertex_byte_size(ROUGHNESS_VERTEX)
        << "\n\tMETALLIC_VERTEX:    " << sr_vertex_byte_size(METALLIC_VERTEX)

        << "\n\tINDEX_VERTEX:       " << sr_vertex_byte_size(INDEX_VERTEX)
        << "\n\tBBOX_TRR_VERTEX:    " << sr_vertex_byte_size(BBOX_TRR_VERTEX)
        << "\n\tBBOX_BFL_VERTEX:    " << sr_vertex_byte_size(BBOX_BFL_VERTEX)

        << "\n\tSTANDARD_VERTEX:    " << sr_vertex_byte_size(STANDARD_VERTEX)
        << "\n\tBONE_VERTEX:        " << sr_vertex_byte_size(BONE_VERTEX)
        << "\n\tOCCLUSION_VERTEX:   " << sr_vertex_byte_size(OCCLUSION_VERTEX)

        << '\n' << std::endl;
}



void print_stride_info()
{
    std::cout
        << "Vertex Strides:"
        << "\n\tPOSITION_VERTEX:    " << sr_vertex_stride(POSITION_VERTEX)
        << "\n\tTEXTURE_VERTEX:     " << sr_vertex_stride(TEXTURE_VERTEX)
        << "\n\tCOLOR_VERTEX:       " << sr_vertex_stride(COLOR_VERTEX)

        << "\n\tNORMAL_VERTEX:      " << sr_vertex_stride(NORMAL_VERTEX)
        << "\n\tTANGENT_VERTEX:     " << sr_vertex_stride(TANGENT_VERTEX)
        << "\n\tBITANGENT_VERTEX:   " << sr_vertex_stride(BITANGENT_VERTEX)

        << "\n\tMODEL_MAT_VERTEX:   " << sr_vertex_stride(MODEL_MAT_VERTEX)

        << "\n\tBONE_ID_VERTEX:     " << sr_vertex_stride(BONE_ID_VERTEX)
        << "\n\tBONE_WEIGHT_VERTEX: " << sr_vertex_stride(BONE_WEIGHT_VERTEX)

        << "\n\tAMBIENT_VERTEX:     " << sr_vertex_stride(AMBIENT_VERTEX)
        << "\n\tDIFFUSE_VERTEX:     " << sr_vertex_stride(DIFFUSE_VERTEX)
        << "\n\tSPECULAR_VERTEX:    " << sr_vertex_stride(SPECULAR_VERTEX)
        << "\n\tROUGHNESS_VERTEX:   " << sr_vertex_stride(ROUGHNESS_VERTEX)
        << "\n\tMETALLIC_VERTEX:    " << sr_vertex_stride(METALLIC_VERTEX)

        << "\n\tINDEX_VERTEX:       " << sr_vertex_stride(INDEX_VERTEX)
        << "\n\tBBOX_TRR_VERTEX:    " << sr_vertex_stride(BBOX_TRR_VERTEX)
        << "\n\tBBOX_BFL_VERTEX:    " << sr_vertex_stride(BBOX_BFL_VERTEX)

        << "\n\tSTANDARD_VERTEX:    " << sr_vertex_stride(STANDARD_VERTEX)
        << "\n\tBONE_VERTEX:        " << sr_vertex_stride(BONE_VERTEX)
        << "\n\tOCCLUSION_VERTEX:   " << sr_vertex_stride(OCCLUSION_VERTEX)

        << '\n' << std::endl;
}



void print_index_info()
{
    std::cout
        << "Index Info:"
        << "\n\tRequired index type  (254):    " << sr_required_index_type(254)
        << "\n\tRequired index bytes (254):    " << sr_index_byte_size(sr_required_index_type(254))

        << "\n\tRequired index type  (255):    " << sr_required_index_type(255)
        << "\n\tRequired index bytes (255):    " << sr_index_byte_size(sr_required_index_type(255))

        << "\n\tRequired index type  (256):    " << sr_required_index_type(256)
        << "\n\tRequired index bytes (256):    " << sr_index_byte_size(sr_required_index_type(256))

        << "\n\tRequired index type  (32768):  " << sr_required_index_type(32768)
        << "\n\tRequired index bytes (32768):  " << sr_index_byte_size(sr_required_index_type(32768))

        << "\n\tRequired index type  (65536):  " << sr_required_index_type(65536)
        << "\n\tRequired index bytes (65536):  " << sr_index_byte_size(sr_required_index_type(65536))

        << '\n' << std::endl;
}



int main()
{
    print_data_type_info();
    print_vertex_info();
    print_index_info();

    return 0;
}
