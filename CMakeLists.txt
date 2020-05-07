cmake_minimum_required(VERSION 3.11)

project(amaze)

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY(src SRCFILES)
FILE(GLOB_RECURSE SRCFILES  src/*.cpp)

add_executable(amaze ${SRCFILES})
target_link_libraries(amaze ${OPENGL_LIBRARY} ${SDL2_LIBRARIES} ${GLEW_LIBRARIES})

add_custom_command(TARGET amaze POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:amaze>/shaders/)
