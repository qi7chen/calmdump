cmake_minimum_required(VERSION 3.0)

project(calmdump)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    src
)

add_definitions(
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -D_WIN32_WINNT=0x0502
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
)

file(GLOB_RECURSE LIB_HEADER_FILES src/*.h)
file(GLOB_RECURSE LIB_SOURCE_FILES src/*.cpp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(SOURCE_GROUP_BY_DIR source_files)
    foreach(FILE ${SRCS}) 
        # Get the directory of the source file
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

        # Remove common directory prefix to make the group
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

        # Make sure we are using windows slashes
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        # Group into "Source Files" and "Header Files"
        if ("${FILE}" MATCHES ".*\\.cpp")
           set(GROUP "Source Files${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.h")
           set(GROUP "Header Files${GROUP}")
        endif()

        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endmacro(SOURCE_GROUP_BY_DIR)

SOURCE_GROUP_BY_DIR(LIB_HEADER_FILES)
SOURCE_GROUP_BY_DIR(LIB_SOURCE_FILES)

add_library(calmdump ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})

add_subdirectory(example)
