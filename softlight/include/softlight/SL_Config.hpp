
#ifndef SL_CONFIG_HPP
#define SL_CONFIG_HPP



/*-----------------------------------------------------------------------------
 * Vertex Processing Configuration
-----------------------------------------------------------------------------*/
#ifndef SL_VERTEX_CACHING_ENABLED
    #define SL_VERTEX_CACHING_ENABLED 1
#endif /* SL_VERTEX_CACHING_ENABLED */

#ifndef SL_VERTEX_CACHE_SIZE
    #define SL_VERTEX_CACHE_SIZE 8
#endif /* SL_VERTEX_CACHE_SIZE */



/*-----------------------------------------------------------------------------
 * Render Pipeline Configuration
-----------------------------------------------------------------------------*/
#ifndef SL_Z_CLIPPING_ENABLED
    #define SL_Z_CLIPPING_ENABLED 1
#endif /* SL_Z_CLIPPING_ENABLED */

#ifndef SL_CONSERVE_MEMORY
    #define SL_CONSERVE_MEMORY 0
#endif /* SL_CONSERVE_MEMORY */



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SL_ShaderLimits
{
    SL_SHADER_MAX_WORLD_COORDS    = 3,
    SL_SHADER_MAX_SCREEN_COORDS   = 3,
    SL_SHADER_MAX_VARYING_VECTORS = 4,
    SL_SHADER_MAX_FRAG_OUTPUTS    = 4,

    // Maximum number of fragments that get queued before being placed on a
    // framebuffer.
    #if !SL_CONSERVE_MEMORY
    SL_SHADER_MAX_QUEUED_FRAGS    = 600,
    #else
    SL_SHADER_MAX_QUEUED_FRAGS    = 16,
    #endif /* !SL_CONSERVE_MEMORY */

    // Maximum number of vertex groups which get binned before being sent to a
    // fragment processor.
    SL_SHADER_MAX_BINNED_PRIMS    = 1024,

    // Maximum possible amount of fragment operations running while
    // simultaneously allowing vertex processing.
    SL_VERT_PROCESSOR_MAX_BUFFERS = 8
};



#endif /* SL_CONFIG_HPP */
