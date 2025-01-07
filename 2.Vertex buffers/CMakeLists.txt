cmake_minimum_required(VERSION 3.27)

set(VERTEX_INPUT_DESCRIPTION_PATH "2.1.Vertex input description")
set(VERTEX_BUFFER_CREATION_PATH "2.2.Vertex buffer creation")
set(STAGING_BUFFER_PATH "2.3.Staging buffer")

set(VERTEX_INPUT_DESCRIPTION "2.1.VertexInputDescription")
set(VERTEX_BUFFER_CREATION "2.2.VertexBufferCreation")
set(STAGING_BUFFER "2.3.StagingBuffer")

add_subdirectory(${VERTEX_INPUT_DESCRIPTION_PATH})
add_subdirectory(${VERTEX_BUFFER_CREATION_PATH})
add_subdirectory(${STAGING_BUFFER_PATH})

set_target_properties(
    ${VERTEX_INPUT_DESCRIPTION}
    ${VERTEX_BUFFER_CREATION}
    ${STAGING_BUFFER}
    PROPERTIES FOLDER "2.Vertex buffers"
)