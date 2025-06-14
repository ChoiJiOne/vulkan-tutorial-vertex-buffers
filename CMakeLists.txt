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

set(VERTEX_INPUT_DESCRIPTION_PATH "2.1.Vertex input description")
set(VERTEX_BUFFER_CREATION_PATH "2.2.Vertex buffer creation")
set(STAGING_BUFFER_PATH "2.3.Staging buffer")
set(INDEX_BUFFER_PATH "2.4.Index buffer")

set(VERTEX_INPUT_DESCRIPTION "2.1.VertexInputDescription")
set(VERTEX_BUFFER_CREATION "2.2.VertexBufferCreation")
set(STAGING_BUFFER "2.3.StagingBuffer")
set(INDEX_BUFFER "2.4.IndexBuffer")

add_subdirectory(${VERTEX_INPUT_DESCRIPTION_PATH})
add_subdirectory(${VERTEX_BUFFER_CREATION_PATH})
add_subdirectory(${STAGING_BUFFER_PATH})
add_subdirectory(${INDEX_BUFFER_PATH})

set_target_properties(
    ${VERTEX_INPUT_DESCRIPTION}
    ${VERTEX_BUFFER_CREATION}
    ${STAGING_BUFFER}
    ${INDEX_BUFFER}
    PROPERTIES FOLDER "2. Vertex buffers"
)
