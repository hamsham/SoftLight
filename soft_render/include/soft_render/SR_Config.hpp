
#ifndef SR_CONFIG_HPP
#define SR_CONFIG_HPP



/*-----------------------------------------------------------------------------
 * Vertex Processing Configuration
-----------------------------------------------------------------------------*/
#ifndef SR_VERTEX_CACHING_ENABLED
    #define SR_VERTEX_CACHING_ENABLED 0
#endif /* SR_VERTEX_CACHING_ENABLED */

#if SR_VERTEX_CACHING_ENABLED
    #ifndef SR_VERTEX_CACHE_SIZE
        #define SR_VERTEX_CACHE_SIZE 64
    #endif /* SR_VERTEX_CACHE_SIZE */
#endif /* SR_VERTEX_CACHING_ENABLED */


/*-----------------------------------------------------------------------------
 * Render Pipeline Configuration
-----------------------------------------------------------------------------*/
#ifndef SR_Z_CLIPPING_ENABLED
    #define SR_Z_CLIPPING_ENABLED 1
#endif /* SR_VERTEX_CLIPPING_ENABLED */

#ifndef SR_REVERSED_Z_RENDERING
    #define SR_REVERSED_Z_RENDERING 1
#endif /* SR_REVERSED_Z_RENDERING */



#endif /* SR_CONFIG_HPP */
