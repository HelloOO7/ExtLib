cmake_minimum_required(VERSION 3.0.0)

message("ExtLib build begin.")

message("Target platform: " ${EXL_PLATFORM})

IF (EXL_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")
add_compile_definitions(EXL_PLATFORM_GFL)

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "-r -march=armv5t -mthumb" CACHE INTERNAL "")

add_compile_options(-r -march=armv5t -mthumb)

ELSEIF (EXL_PLATFORM STREQUAL "Win32")
message("Building for Win32")
add_compile_definitions(EXL_PLATFORM_WIN32)

ELSE ()
message( FATAL_ERROR "Invalid target platform!")

ENDIF ()

message("Initial compiler configuration done.")

project(ExtLib VERSION 0.1.0)
project(ExtLib LANGUAGES C CXX ASM)

include(CTest)
enable_testing()

add_compile_options(-fno-rtti -fno-exceptions -fvisibility=hidden)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_compile_definitions(DEBUG)
endif()

#add_compile_definitions(DEBUG EXL_DESMUME)

add_subdirectory(Heap)
add_subdirectory(Compression)
add_subdirectory(IO)
add_subdirectory(FileSystem)
add_subdirectory(Media)

file(GLOB SOURCES
    Util/*.h
    Util/*.cpp
    *.h
)

file(GLOB SOURCES_ABI
    ABI/exl_CxxAbi.h
    ABI/exl_CxxAbi.cpp
)

add_library(ExtLib.ABI OBJECT
    ${SOURCES_ABI}
)
target_include_directories(ExtLib.ABI PRIVATE .)

IF (EXL_PLATFORM STREQUAL "ARMv5T")
target_sources(ExtLib.ABI PRIVATE ABI/exl_LibGCCHelper.s)
ENDIF()

IF (WIN32)

    add_library(ExtLib
        ${SOURCES}
        ${SOURCES_ABI}
        $<TARGET_OBJECTS:ExtLib.Heap> 
        $<TARGET_OBJECTS:ExtLib.Compression> 
        $<TARGET_OBJECTS:ExtLib.IO> 
        $<TARGET_OBJECTS:ExtLib.FileSystem>
        $<TARGET_OBJECTS:ExtLib.Media.MPEG>
        $<TARGET_OBJECTS:ExtLib.Media.Cinepak>
    )
    target_include_directories(ExtLib PRIVATE .)

    add_library(ExtLib.DLL SHARED exl_DllExport.h)
    target_link_libraries(ExtLib.DLL ExtLib)

ELSEIF (EXL_PLATFORM STREQUAL "ARMv5T")

    add_library(ExtLib.Base OBJECT ${SOURCES})
    target_include_directories(ExtLib.Base PUBLIC .)

    function(subLib subLibName)
        add_executable(${subLibName}.elf $<TARGET_OBJECTS:${subLibName}>)
        target_link_libraries(${subLibName}.elf ${subLibName} ExtLib.ABI)
        target_include_directories(${subLibName} PUBLIC .)    
    endfunction()

    subLib(ExtLib.Heap)
    subLib(ExtLib.IO)
    subLib(ExtLib.FileSystem)
    subLib(ExtLib.Media.MPEG)
    subLib(ExtLib.Media.Cinepak)

ENDIF ()

IF (WIN32)
add_subdirectory(Tests)
ENDIF ()

add_library(ExtLib.Include OBJECT exl_DllExport.h)
target_include_directories(ExtLib.Include PUBLIC .)

IF (EXL_PLATFORM STREQUAL "ARMv5T")
    target_compile_definitions(ExtLib.Include PUBLIC EXL_PLATFORM_GFL)
ELSE()
    target_compile_definitions(ExtLib.Include PUBLIC EXL_PLATFORM_WIN32)
ENDIF()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
