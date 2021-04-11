cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)

project (LearnOpenGL)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif(WIN32)

link_directories(${CMAKE_SOURCE_DIR}/lib)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

# find the required packages
find_package(glm REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(assimp REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")
# find_package(SOIL REQUIRED)
# message(STATUS "Found SOIL in ${SOIL_INCLUDE_DIR}")
find_package(GLEW REQUIRED)
message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIR}")

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

if(WIN32)
    set(LIBS glfw3 opengl32 assimp freetype irrKlang)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -lGLU")
    find_package(OpenGL REQUIRED)
    add_definitions(${OPENGL_DEFINITIONS})
    find_package(X11 REQUIRED)
    # note that the order is important for setting the libs
    # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
    set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl -lglut -lGL -lGLEW -lglfw")
elseif(APPLE)
    INCLUDE_DIRECTORIES(/System/Library/Frameworks)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKit_LIBRARY IOKit)
    FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
    MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
    SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
    set(LIBS ${LIBS} ${APPLE_LIBS})
endif(WIN32)

add_executable(task1 main.cpp window.cpp window.h)