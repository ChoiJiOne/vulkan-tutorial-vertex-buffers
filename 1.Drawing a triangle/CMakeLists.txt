cmake_minimum_required(VERSION 3.27)

set(SETUP "1.1.Setup")
set(PRESENTATION "1.2.Presentation")
set(GRAPHICS_PIPELINE_BASICS "1.3.Graphics pipelinee basics")
set(DRAWING "1.4.Drawing")

add_subdirectory(${SETUP})
add_subdirectory(${PRESENTATION})
add_subdirectory(${GRAPHICS_PIPELINE_BASICS})
add_subdirectory(${DRAWING})