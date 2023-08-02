cmake_minimum_required(VERSION 3.0.0)
project(ExtLib.Tests VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(..)

file(GLOB SOURCES_HEAPTEST
    exl_TestHeapArea.cpp
    ../*.h
)
file(GLOB SOURCES_ARCTEST
    exl_TestFSArchive.cpp
    ../*.h
)
file(GLOB SOURCES_MPEGTEST
    exl_TestMPEGDecoder.cpp
    ../*.h
)
file(GLOB SOURCES_CINEPAKTEST
    exl_TestCinepakDecoder.cpp
    ../*.h
)

add_executable(exl_TestHeapArea ${SOURCES_HEAPTEST})
target_link_libraries(exl_TestHeapArea ExtLib)

add_executable(exl_TestFSArchive ${SOURCES_ARCTEST})
target_link_libraries(exl_TestFSArchive ExtLib)

add_executable(exl_TestMPEGDecoder ${SOURCES_MPEGTEST})
target_link_libraries(exl_TestMPEGDecoder ExtLib)

add_executable(exl_TestCinepakDecoder ${SOURCES_CINEPAKTEST})
target_link_libraries(exl_TestCinepakDecoder ExtLib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
