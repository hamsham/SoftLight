#!/bin/bash

set -ex



# FreeImage needs a few environment variables to build properly
export CC=@CMAKE_C_COMPILER@
export CXX=@CMAKE_CXX_COMPILER@
export CFLAGS=-std=c99\ -Wno-narrowing\ -O3\ -fPIC\ -fexceptions\ -fvisibility=hidden
export CXXFLAGS=-std=c++03\ -Wno-narrowing\ -O3\ -fPIC\ -fexceptions\ -fvisibility=hidden\ -Wno-ctor-dtor-privacy
export LDFLAGS=-fPIC



function freeimage_configure()
{
    pushd @EXTERNAL_PROJECT_PREFIX@/src/FreeImage

    # Freeimage attempts to give root permissions to the installed files. No
    sed -i -E 's/([ \t]+install)[ \t]+\-m[ \t]+.+root[ \t]+(\$.+)/\1 \2/g' Makefile.gnu

    # Rename functions which conflict with GLibC
    set +e
    grep 'internal_powf64' "./Source/LibRawLite/internal/dcraw_common.cpp"
    local haveInternalPowf=$?
    set -e

    if [[ $haveInternalPowf -ne 0 ]]; then
        sed -i -r "s/([^_])powf64(\b)/\1internal_powf64\2/g" "./Source/LibRawLite/internal/dcraw_common.cpp"
    fi

    popd
}



function freeimage_configure_mingw()
{
    pushd @EXTERNAL_PROJECT_PREFIX@/src/FreeImage

    # Fix hard-coded build configurations
    sed -i -E 's/CC = gcc/CC ?=/g' "./Makefile.mingw"
    sed -i -E 's/LD = g\+\+/LD ?=/g' "./Makefile.mingw"
    sed -i -E 's/RC = windres/RC ?=/g' "./Makefile.mingw"
    sed -i -E 's/DLLTOOL = dlltool/DLLTOOL ?=/g' "./Makefile.mingw"
    sed -i -E 's/LIBRARIES = -lws2_32/LIBRARIES ?=/g' "./Makefile.mingw"
    sed -i -E 's/-shared//g' "./Makefile.mingw"

    # Rename functions which conflict with GLibC
    sed -i -E 's/define snprintf _snprintf//g' "./Source/LibTIFF4/tif_config.h"

    # Ensure OpenEXR's optimization code uses the correct int size to contain a pointer
    sed -i -E 's/unsigned long( trailingBits = \(\()unsigned long/unsigned long long\1unsigned long long/g' "./Source/OpenEXR/IlmImf/ImfOptimizedPixelReading.h"

    # Remove posix_memalign from OpenEXR
    sed -i -E 's/posix_memalign.+/_aligned_malloc(size, alignment);/g' "./Source/OpenEXR/IlmImf/ImfSystemSpecific.h"
    sed -i -E 's/([ \t]+)free\(ptr\);/\1_aligned_free(ptr);/g' "./Source/OpenEXR/IlmImf/ImfSystemSpecific.h"

    # LibJXR defines conflicting pointer sizes and functions
    sed -i -E 's/#define PLATFORM_ANSI//g' ".//Source/LibJXR/image/sys/strcodec.h"
    sed -i -E 's/#include "ansi.h"//g' ".//Source/LibJXR/image/sys/strcodec.h"
    sed -i -E 's/_byteswap_ulong/_byteswap_uint/g' ".//Source/LibJXR/image/sys/strcodec.c"

    find Source/LibOpenJPEG -type f -exec sed -i 's/__stdcall//g' {} +
    find Source/LibJXR -type f -exec sed -i 's/__stdcall//g' {} +

    set -e
    popd
}



function freeimage_make()
{
    pushd @EXTERNAL_PROJECT_PREFIX@/src/FreeImage

    local dllTool=`echo "@CMAKE_RC_COMPILER@" | sed -E 's/windres/dlltool/'`
    local libs=`echo "@CMAKE_RC_COMPILER@"    | sed -E 's/(.+)\/bin(.+)-windres/\1\2/'`
    local linker=`echo "@CMAKE_RC_COMPILER@"  | sed -E 's/windres/g++/'`

    if [[ @CMAKE_SYSTEM_NAME@ = "Windows" ]]; then
        export CFLAGS="${CFLAGS}"\ -D\ WIN32\ -D\ CINTERFACE\ -DFREEIMAGE_LIB\ -DLIBRAW_NODLL\ -DOPJ_STATIC\ -DDISABLE_PERF_MEASUREMENT\ -DLIBRAW_LIBRARY_BUILD
        export CXXFLAGS="${CXXFLAGS}"\ -D\ WIN32\ -D\ CINTERFACE\ -DFREEIMAGE_LIB\ -DLIBRAW_NODLL\ -DOPJ_STATIC\ -DDISABLE_PERF_MEASUREMENT\ -DLIBRAW_LIBRARY_BUILD

        make --file="Makefile.mingw" \
            CC="@CMAKE_C_COMPILER@" \
            CXX="@CMAKE_CXX_COMPILER@" \
            INCDIR=@EXTERNAL_PROJECT_PREFIX@/include \
            INSTALLDIR=@EXTERNAL_PROJECT_PREFIX@/lib \
            DESTDIR=@EXTERNAL_PROJECT_PREFIX@ \
            FREEIMAGE_LIBRARY_TYPE=STATIC \
            RC="@CMAKE_RC_COMPILER@" \
            LD="$linker" \
            DLLTOOL="$dllTool" \
            LIBRARIES="-L${libs}/lib\ -lws_32"
    else
        make \
            CC="@CMAKE_C_COMPILER@" \
            CXX="@CMAKE_CXX_COMPILER@" \
            INCDIR=@EXTERNAL_PROJECT_PREFIX@/include \
            INSTALLDIR=@EXTERNAL_PROJECT_PREFIX@/lib \
            DESTDIR=@EXTERNAL_PROJECT_PREFIX@ \
            FREEIMAGE_LIBRARY_TYPE=SHARED
    fi

    popd
}



function freeimage_install()
{
    pushd @EXTERNAL_PROJECT_PREFIX@/src/FreeImage

    if [[ @CMAKE_SYSTEM_NAME@ = "Windows" ]]; then
        cp --update Dist/libFreeImage.a @EXTERNAL_PROJECT_PREFIX@/lib
        cp --update Dist/FreeImage.h @EXTERNAL_PROJECT_PREFIX@/include
    else
        make \
            CC="@CMAKE_C_COMPILER@" \
            CXX="@CMAKE_CXX_COMPILER@" \
            INCDIR=@EXTERNAL_PROJECT_PREFIX@/include \
            INSTALLDIR=@EXTERNAL_PROJECT_PREFIX@/lib \
            DESTDIR=@EXTERNAL_PROJECT_PREFIX@ \
            FREEIMAGE_LIBRARY_TYPE=SHARED \
            install
    fi

    popd
}



function build_freeimage()
{
    local doConfig=0
    local doMake=0
    local doInstall=0

    set +u

    while [ "$1" != "" ]; do
        case $1 in
            -c | --configure )
                doConfig=1
                ;;

            -m | --make )
                doMake=1
                ;;

            -i | --install )
                doInstall=1
                ;;
        esac
        shift
    done

    set -u

    if [[ $doConfig -ne 0 ]]; then
        echo "Configuring FreeImage"
        if [[ @CMAKE_SYSTEM_NAME@ = "Windows" ]]; then
            freeimage_configure_mingw
        else
            freeimage_configure
        fi
    fi

    if [[ $doMake -ne 0 ]]; then
        echo "Building FreeImage"
        freeimage_make
    fi

    if [[ $doInstall -ne 0 ]]; then
        echo "Installing FreeImage"
        freeimage_install
    fi
}



build_freeimage "$@"
