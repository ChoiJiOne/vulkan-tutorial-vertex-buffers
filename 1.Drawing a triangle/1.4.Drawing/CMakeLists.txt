cmake_minimum_required(VERSION 3.27)

add_subdirectory(1.4.1.Framebuffers)
add_subdirectory(1.4.2.CommandBuffers)
add_subdirectory(1.4.3.RenderingAndPresentation)

set_target_properties(
    1.4.1.Framebuffers
    1.4.2.CommandBuffers
    1.4.3.RenderingAndPresentation
    PROPERTIES FOLDER "1.Drawing a triangle/1.4.Drawing"
)