cmake_minimum_required(VERSION 3.0.0)
project(ExtLib.Media VERSION 0.1.0)

include(CTest)
enable_testing()

file(GLOB SOURCES_MPEG
    exl_MPEGDecoder.h
    exl_MPEGDecoder.cpp
    detail/pl_mpeg.h
)
file(GLOB SOURCES_CINEPAK
    exl_CinepakDecoder.h
    exl_CinepakDecoder.cpp
    detail/avi_player.h
    detail/avi_player.cpp
)

add_library(ExtLib.Media.MPEG OBJECT ${SOURCES_MPEG})
target_include_directories(ExtLib.Media.MPEG PRIVATE ..)
add_library(ExtLib.Media.Cinepak OBJECT ${SOURCES_CINEPAK})
target_include_directories(ExtLib.Media.Cinepak PRIVATE ..)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
