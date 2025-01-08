cmake_minimum_required(VERSION 3.27)

set(DESCRIPTOR_LAYOUT_AND_BUFFER_PATH "3.1.Descriptor layout and buffer")

set(DESCRIPTOR_LAYOUT_AND_BUFFER "3.1.DescriptorLayoutAndBuffer")

add_subdirectory(${DESCRIPTOR_LAYOUT_AND_BUFFER_PATH})

set_target_properties(
    ${DESCRIPTOR_LAYOUT_AND_BUFFER}
    PROPERTIES FOLDER "3.Uniform buffers"
)