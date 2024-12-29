cmake_minimum_required(VERSION 3.27)

add_subdirectory(1.1.1.BaseCode)
add_subdirectory(1.1.2.Instance)
add_subdirectory(1.1.3.ValidationLayers)
add_subdirectory(1.1.4.PhysicalDevicesAndQueueFamilies)
add_subdirectory(1.1.5.LogicalDeviceAndQueues)

set_target_properties(
    1.1.1.BaseCode 
    1.1.2.Instance 
    1.1.3.ValidationLayers 
    1.1.4.PhysicalDevicesAndQueueFamilies
    1.1.5.LogicalDeviceAndQueues
    PROPERTIES FOLDER "1.DrawingATriangle/1.1.Setup"
)