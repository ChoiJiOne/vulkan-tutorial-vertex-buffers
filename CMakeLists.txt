cmake_minimum_required(VERSION 3.27)

project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (MSVC)
    add_compile_options("/wd4819")
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(ThirdParty)

set(DRAWING_A_TRIANGLE "1.Drawing a triangle")
set(VERTEX_BUFFERS "2.Vertex buffers")
set(UNIFORM_BUFFERS "3.Uniform buffers")

add_subdirectory(${DRAWING_A_TRIANGLE})
add_subdirectory(${VERTEX_BUFFERS})
add_subdirectory(${UNIFORM_BUFFERS})
