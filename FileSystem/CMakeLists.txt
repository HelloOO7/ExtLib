cmake_minimum_required(VERSION 3.0.0)
project(ExtLib.FileSystem VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(..)

file(GLOB SOURCES
    *.h
    *.cpp
    ../*.h
)

add_library(ExtLib.FileSystem OBJECT ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
