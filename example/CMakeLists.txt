project(example)

file(GLOB PROJECT_HEADER_FILES *.h)
file(GLOB PROJECT_SOURCE_FILES *.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
add_executable(example WIN32 ${PROJECT_HEADER_FILES} ${PROJECT_SOURCE_FILES})

target_link_libraries(example calmdump)
