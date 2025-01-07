cmake_minimum_required(VERSION 3.27)

set(VERTEX_INPUT_DESCRIPTION_PATH "2.1.Vertex input description")
set(VERTEX_BUFFER_CREATION_PATH "2.2.Vertex buffer creation")

set(VERTEX_INPUT_DESCRIPTION "2.1.VertexInputDescription")
set(VERTEX_BUFFER_CREATION "2.2.VertexBufferCreation")

add_subdirectory(${VERTEX_INPUT_DESCRIPTION_PATH})
add_subdirectory(${VERTEX_BUFFER_CREATION_PATH})

set_target_properties(
    ${VERTEX_INPUT_DESCRIPTION}
    ${VERTEX_BUFFER_CREATION}
    PROPERTIES FOLDER "2.Vertex buffers"
)