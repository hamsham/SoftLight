
#ifndef SR_CONFIG_HPP
#define SR_CONFIG_HPP



/*-----------------------------------------------------------------------------
 * Vertex Processing Configuration
-----------------------------------------------------------------------------*/
#ifndef SR_TUNE_COMPLEX_GEOM
    #define SR_TUNE_COMPLEX_GEOM 0
#endif /* SR_TUNE_COMPLEX_GEOM */

#ifndef SR_VERTEX_CACHING_ENABLED
    #define SR_VERTEX_CACHING_ENABLED 0

    #if SR_VERTEX_CACHING_ENABLED
        #ifndef SR_VERTEX_CACHE_TYPE_LRU
            #define SR_VERTEX_CACHE_TYPE_LRU 0
        #endif /* SR_VERTEX_CACHE_TYPE_LRU */

        #ifndef SR_VERTEX_CACHE_SIZE
            #define SR_VERTEX_CACHE_SIZE 32
        #endif /* SR_VERTEX_CACHE_SIZE */

    #endif /* SR_VERTEX_CACHING_ENABLED */
#endif /* SR_VERTEX_CACHING_ENABLED */



/*-----------------------------------------------------------------------------
 * Render Pipeline Configuration
-----------------------------------------------------------------------------*/
#ifndef SR_PRIMITIVE_CLIPPING_ENABLED
    #define SR_PRIMITIVE_CLIPPING_ENABLED 1
#endif /* SR_VERTEX_CLIPPING_ENABLED */

#ifndef SR_REVERSED_Z_RENDERING
    #define SR_REVERSED_Z_RENDERING 1
#endif /* SR_REVERSED_Z_RENDERING */



#endif /* SR_CONFIG_HPP */
