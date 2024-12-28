# Logical device and queues

## Introduction

물리 장치를 선택한 후에는 이를 인터페이스할 논리 장치를 설정해야 합니다. 논리 장치 생성 과정은 인스턴스 생성 과정과 유사하며, 우리가 사용하려는 기능들을 설명합니다. 또한, 사용할 큐 패밀리를 쿼리했으므로 어떤 큐를 생성할지 지정해야 합니다. 하나의 물리 장치에서 여러 개의 논리 장치를 생성할 수도 있으며, 이는 다양한 요구 사항이 있을 때 유용합니다.

먼저, 논리 장치 핸들을 저장할 새로운 클래스 멤버를 추가합니다.

```
VkDevice device;
```

그 다음, `initVulkan`에서 호출될 `createLogicalDevice` 함수를 추가합니다.

```
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

void createLogicalDevice() {

}
```

## Specifying the queues to be created

논리 장치를 생성하는 과정은 다시 한 번 여러 세부 사항을 구조체에 지정하는 과정입니다. 그 중 첫 번째는 `VkDeviceQueueCreateInfo` 구조체입니다. 이 구조체는 단일 큐 패밀리에 대해 원하는 큐의 수를 설명합니다. 현재 우리는 그래픽 기능을 가진 큐에만 관심이 있습니다.

```
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

VkDeviceQueueCreateInfo queueCreateInfo{};
queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
queueCreateInfo.queueCount = 1;
```

현재 사용 가능한 드라이버는 각 큐 패밀리에 대해 소수의 큐만 생성할 수 있게 허용하며, 실제로 하나 이상의 큐가 필요하지 않습니다. 그 이유는 여러 스레드에서 모든 명령 버퍼를 생성하고, 이를 메인 스레드에서 한 번에 낮은 오버헤드 호출로 모두 제출할 수 있기 때문입니다.

Vulkan은 큐에 우선순위를 지정하여 명령 버퍼 실행의 스케줄링에 영향을 미칠 수 있게 합니다. 우선순위는 `0.0`과 `1.0` 사이의 부동 소수점 숫자를 사용하여 지정합니다. 이는 큐가 하나뿐일 때도 필수입니다.

```
float queuePriority = 1.0f;
queueCreateInfo.pQueuePriorities = &queuePriority;
```

## Specifying used device features

다음으로 지정할 정보는 우리가 사용할 장치 기능의 집합입니다. 이 기능들은 이전 장에서 `vkGetPhysicalDeviceFeatures`로 지원 여부를 쿼리했던 기능들로, 예를 들어 기하 셰이더 등이 있습니다. 현재는 특별한 기능이 필요하지 않으므로, 이를 간단히 정의하고 모든 항목을 `VK_FALSE`로 설정할 수 있습니다. Vulkan으로 더 흥미로운 작업을 시작할 때 이 구조체를 다시 살펴볼 것입니다.

```
VkPhysicalDeviceFeatures deviceFeatures{};
```

## Creating the logical device

이전 두 구조체가 준비되었으므로, 이제 주요 `VkDeviceCreateInfo` 구조체를 채울 수 있습니다.

```
VkDeviceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
```

먼저 큐 생성 정보와 장치 기능 구조체에 대한 포인터를 추가합니다:

```
createInfo.pQueueCreateInfos = &queueCreateInfo;
createInfo.queueCreateInfoCount = 1;

createInfo.pEnabledFeatures = &deviceFeatures;
```

나머지 정보는 `VkInstanceCreateInfo` 구조체와 유사하며, 확장과 검증 레이어를 지정해야 합니다. 차이점은 이번에는 이것들이 장치에 특화된 정보라는 점입니다.

장치 특화 확장의 예로는 `VK_KHR_swapchain`이 있으며, 이는 해당 장치에서 렌더링된 이미지를 윈도우에 표시할 수 있게 합니다. 시스템에 이 기능이 없는 Vulkan 장치가 있을 수 있으며, 예를 들어 계산 작업만 지원하는 장치가 있을 수 있습니다. 이 확장은 나중에 스왑 체인 장에서 다시 다룰 예정입니다.

이전 Vulkan 구현은 인스턴스와 장치 특화 검증 레이어를 구분했지만, 이제는 더 이상 그렇게 하지 않습니다. 즉, `VkDeviceCreateInfo`의 `enabledLayerCount`와 `ppEnabledLayerNames` 필드는 최신 구현에서는 무시됩니다. 그럼에도 불구하고, 구버전 구현과 호환되도록 이를 설정하는 것이 여전히 좋은 방법입니다.

```
createInfo.enabledExtensionCount = 0;

if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}
```

지금은 장치 특화 확장이 필요하지 않습니다.

그렇다면 이제 적절한 이름의 `vkCreateDevice` 함수를 호출하여 논리 장치를 인스턴스화할 준비가 되었습니다.

```
if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
}
```

이 함수의 파라미터는 인터페이스할 물리 장치, 우리가 방금 지정한 큐와 사용 정보, 선택적인 할당 콜백 포인터, 그리고 논리 장치 핸들을 저장할 변수에 대한 포인터입니다. 인스턴스 생성 함수와 유사하게, 이 호출은 존재하지 않는 확장을 활성화하거나 지원되지 않는 기능의 사용을 지정할 경우 오류를 반환할 수 있습니다.

장치는 `vkDestroyDevice` 함수로 정리 시 파괴되어야 합니다.

```
void cleanup() {
    vkDestroyDevice(device, nullptr);
    ...
}
```

논리 장치는 인스턴스와 직접 상호작용하지 않으므로 인스턴스는 파라미터에 포함되지 않습니다.

## Retrieving queue handles

큐는 논리 장치와 함께 자동으로 생성되지만, 아직 이를 인터페이스할 핸들은 없습니다. 먼저 그래픽 큐에 대한 핸들을 저장할 클래스 멤버를 추가합니다:

```
VkQueue graphicsQueue;
```

장치 큐는 장치가 파괴될 때 암시적으로 정리되므로, `cleanup`에서 따로 할 일이 없습니다.

우리는 `vkGetDeviceQueue` 함수를 사용하여 각 큐 패밀리의 큐 핸들을 가져올 수 있습니다. 파라미터는 논리 장치, 큐 패밀리, 큐 인덱스, 그리고 큐 핸들을 저장할 변수에 대한 포인터입니다. 여기서는 이 패밀리에서 하나의 큐만 생성하므로 인덱스 0을 사용합니다.

```
vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
```

이제 논리 장치와 큐 핸들을 사용하여 실제로 그래픽 카드를 사용해 작업을 수행할 수 있습니다! 다음 몇 개의 장에서는 결과를 윈도우 시스템에 표시할 리소스를 설정할 것입니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/04_logical_device.cpp)