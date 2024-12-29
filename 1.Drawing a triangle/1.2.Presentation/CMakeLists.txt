cmake_minimum_required(VERSION 3.27)

add_subdirectory(1.2.1.WindowSurface)
add_subdirectory(1.2.2.SwapChain)
add_subdirectory(1.2.3.ImageViews)

set_target_properties(
    1.2.1.WindowSurface
    1.2.2.SwapChain
    1.2.3.ImageViews
    PROPERTIES FOLDER "1.Drawing a triangle/1.2.Presentation"
)