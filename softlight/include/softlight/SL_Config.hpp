
#ifndef SL_CONFIG_HPP
#define SL_CONFIG_HPP



/*-----------------------------------------------------------------------------
 * Vertex Processing Configuration
-----------------------------------------------------------------------------*/
#ifndef SL_VERTEX_CACHING_ENABLED
    #define SL_VERTEX_CACHING_ENABLED 0
#endif /* SL_VERTEX_CACHING_ENABLED */

#ifndef SL_VERTEX_CACHE_SIZE
    #define SL_VERTEX_CACHE_SIZE 16
#endif /* SL_VERTEX_CACHE_SIZE */



/*-----------------------------------------------------------------------------
 * Render Pipeline Configuration
-----------------------------------------------------------------------------*/
#ifndef SL_Z_CLIPPING_ENABLED
    #define SL_Z_CLIPPING_ENABLED 1
#endif /* SL_VERTEX_CLIPPING_ENABLED */



#endif /* SL_CONFIG_HPP */
