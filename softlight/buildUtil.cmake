
# -------------------------------------
# CMake Library Finder
# -------------------------------------
# Library Locator Functions for all operating systems
function(sl_find_library outVariable libName isRequired)
    unset(temp CACHE)

    if (isRequired)
        find_library(temp ${libName} REQUIRED)
    else()
        find_library(temp ${libName})
    endif()

    if (temp MATCHES temp-NOTFOUND)
        message(FATAL_ERROR "-- Unable to find ${libName} for linking with SoftLight.")
    else()
        message("-- Found ${libName}: ${temp}")
        set(${outVariable} "${temp}" PARENT_SCOPE)
    endif()

    unset(temp CACHE)
endfunction()
