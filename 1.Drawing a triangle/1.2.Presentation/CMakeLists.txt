cmake_minimum_required(VERSION 3.27)

set(WINDOW_SURFACE_PATH "1.2.1.Window surface")
set(SWAP_CHAIN_PATH  "1.2.2.Swap chain")
set(IMAGE_VIEWS_PATH  "1.2.3.Image views")

set(WINDOW_SURFACE "1.2.1.WindowSurface")
set(SWAP_CHAIN "1.2.2.SwapChain")
set(IMAGE_VIEWS "1.2.3.ImageViews")

add_subdirectory(${WINDOW_SURFACE_PATH})
add_subdirectory(${SWAP_CHAIN_PATH})
add_subdirectory(${IMAGE_VIEWS_PATH})

set_target_properties(
    ${WINDOW_SURFACE}
    ${SWAP_CHAIN}
    ${IMAGE_VIEWS}
    PROPERTIES FOLDER "1.Drawing a triangle/1.2.Presentation"
)