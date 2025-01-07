cmake_minimum_required(VERSION 3.27)

set(FRAME_BUFFERS_PATH "1.4.1.Framebuffers")
set(COMMAND_BUFFERS_PATH "1.4.2.Command buffers")
set(RENDERING_AND_PRESENTATION_PATH "1.4.3.Rendering and presentation")
set(FRAMES_IN_FLIGHT_PATH "1.4.4.Frames in flight")

set(FRAME_BUFFERS "1.4.1.Framebuffers")
set(COMMAND_BUFFERS "1.4.2.CommandBuffers")
set(RENDERING_AND_PRESENTATION "1.4.3.RenderingAndPresentation")
set(FRAMES_IN_FLIGHT "1.4.4.FramesInFlight")

add_subdirectory(${FRAME_BUFFERS_PATH})
add_subdirectory(${COMMAND_BUFFERS_PATH})
add_subdirectory(${RENDERING_AND_PRESENTATION_PATH})
add_subdirectory(${FRAMES_IN_FLIGHT_PATH})

set_target_properties(
    ${FRAME_BUFFERS}
    ${COMMAND_BUFFERS}
    ${RENDERING_AND_PRESENTATION}
    ${FRAMES_IN_FLIGHT}
    PROPERTIES FOLDER "1.Drawing a triangle/1.4.Drawing"
)