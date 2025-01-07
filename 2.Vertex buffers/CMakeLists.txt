cmake_minimum_required(VERSION 3.27)

set(VERTEX_INPUT_DESCRIPTION_PATH "2.1.Vertex input description")

set(VERTEX_INPUT_DESCRIPTION "2.1.VertexInputDescription")

add_subdirectory(${VERTEX_INPUT_DESCRIPTION_PATH})

set_target_properties(
    ${VERTEX_INPUT_DESCRIPTION}
    PROPERTIES FOLDER "2.Vertex buffers"
)