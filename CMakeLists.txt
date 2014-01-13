CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( OculusGLFWSkeleton )

INCLUDE(cmake_modules/HardcodeShaders.cmake)

SET( USE_ANTTWEAKBAR TRUE CACHE BOOL "Use AntTweakBar" )

#
# Platform-dependent section
#
IF( WIN32 )
    #
    # Custom Windows include and link dirs for my machine:
    #
    SET( LIBS_HOME "C:/lib" CACHE STRING "Location of support libraries Glut, Glew and others"  )

    SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.0.1.bin.WIN32" )
    INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLFW3_ROOT}/lib-msvc100" )

    SET( GLEW_ROOT "${LIBS_HOME}/glew" )
    INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib" )
    #ADD_DEFINITIONS( -DGLEW_STATIC )
    
    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK-0.2.2" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/Win32" )

    IF( USE_ANTTWEAKBAR )
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar_116/AntTweakBar" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS AntTweakBar.lib )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    SET( PLATFORM_LIBS
        glfw3.lib
        opengl32.lib glu32.lib glew32.lib 
        libovrd.lib
        winmm.lib
        ${ANT_LIBS}
        )

ELSEIF( UNIX )
    #
    # Custom Windows include and link dirs for my machine:
    #
    ADD_DEFINITIONS( -D_LINUX )
    SET( LIBS_HOME "~/lib" )

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/Linux/Release/x86_64/" )

    IF( USE_ANTTWEAKBAR )
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS libAntTweakBar.a )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    SET( PLATFORM_LIBS
        -lovr
        -lGL -lGLEW
        -lGLU -lglfw
        -pthread
        -ludev
        -lrt
        -lXinerama
        -lXrandr
        -lXxf86vm
        -lXi
        -lX11
        ${ANT_LIBS}
        )
ENDIF()


#
# Platform-independent section
#
FILE( GLOB_RECURSE UTIL_SOURCE_FILES
    src/utils/*.cpp
    src/utils/*.h
    )

FILE( GLOB_RECURSE VECTORMATH_SOURCE_FILES
    src/vectormath/*.cpp
    src/vectormath/*.h
    )

FILE( GLOB_RECURSE OVRKILL_SOURCE_FILES
    src/OVRkill/*.cpp
    src/OVRkill/*.h
    )

FILE( GLOB_RECURSE APPSKELETON_SOURCE_FILES
    src/appskeleton/*.cpp
    src/appskeleton/*.h
    )

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("src/utils")
INCLUDE_DIRECTORIES("src/utils/GL")
INCLUDE_DIRECTORIES("src/vectormath")
INCLUDE_DIRECTORIES("src/OVRkill")
INCLUDE_DIRECTORIES("src/appskeleton")

ADD_LIBRARY( Util        ${UTIL_SOURCE_FILES} )
ADD_LIBRARY( VectorMath  ${VECTORMATH_SOURCE_FILES} )
ADD_LIBRARY( OVRkill     ${OVRKILL_SOURCE_FILES} )
ADD_LIBRARY( AppSkeleton ${APPSKELETON_SOURCE_FILES} )

ADD_EXECUTABLE( ${PROJECT_NAME}
    src/main/simple_glfw_main.cpp
    )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    AppSkeleton
    OVRkill
    Util
    VectorMath
    ${PLATFORM_LIBS}
    )
