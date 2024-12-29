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
add_subdirectory(01.BaseCode)
add_subdirectory(02.Instance)
add_subdirectory(03.ValidationLayers)
add_subdirectory(04.PhysicalDevicesAndQueueFamilies)
add_subdirectory(05.LogicalDeviceAndQueues)
add_subdirectory(06.WindowSurface)
add_subdirectory(07.SwapChain)
add_subdirectory(08.ImageViews)