cmake_minimum_required(VERSION 3.27)

set(BASE_CODE_PATH "1.1.1.Base Code")
set(INSTANCE_PATH "1.1.2.Instance")
set(VALIDATION_LAYERS_PATH "1.1.3.Validation layers")
set(PHYSICAL_DEVICE_AND_QUEUE_FAMILIIES_PATH "1.1.4.Physical devices and queue families")
set(LOGICAL_DEVICE_AND_QUEUES_PATH "1.1.5.Logical device and queues")

set(BASE_CODE "1.1.1.BaseCode")
set(INSTANCE "1.1.2.Instance")
set(VALIDATION_LAYERS "1.1.3.ValidationLayers")
set(PHYSICAL_DEVICE_AND_QUEUE_FAMILIIES "1.1.4.PhysicalDevicesAndQueueFamilies")
set(LOGICAL_DEVICE_AND_QUEUES "1.1.5.LogicalDeviceAndQueues")

add_subdirectory(${BASE_CODE_PATH})
add_subdirectory(${INSTANCE_PATH})
add_subdirectory(${VALIDATION_LAYERS_PATH})
add_subdirectory(${PHYSICAL_DEVICE_AND_QUEUE_FAMILIIES_PATH})
add_subdirectory(${LOGICAL_DEVICE_AND_QUEUES_PATH})

set_target_properties(
    ${BASE_CODE}
    ${INSTANCE}
    ${VALIDATION_LAYERS}
    ${PHYSICAL_DEVICE_AND_QUEUE_FAMILIIES}
    ${LOGICAL_DEVICE_AND_QUEUES}
    PROPERTIES FOLDER "1.Drawing a triangle/1.1.Setup"
)