cmake_minimum_required(VERSION 3.5)
project(MingleC)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(ann_1.1.2)
#add_subdirectory(libpng-1.6.23)

set(SOURCE_FILES main.cpp main.h EdgeBundler.cpp EdgeBundler.h EdgeBundleTree.cpp EdgeBundleTree.h Util.cpp Util.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-register")
add_executable(MingleC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ann ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})