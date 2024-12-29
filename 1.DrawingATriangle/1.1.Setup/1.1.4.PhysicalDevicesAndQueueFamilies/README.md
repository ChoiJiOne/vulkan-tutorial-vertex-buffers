# Physical devices and queue families

## Selecting a physical device

Vulkan 라이브러리를 `VkInstance`를 통해 초기화한 후에는, 필요한 기능을 지원하는 그래픽 카드를 시스템에서 찾아 선택해야 합니다. 사실, 여러 개의 그래픽 카드를 선택하고 동시에 사용할 수도 있지만, 이 튜토리얼에서는 우리의 요구를 충족하는 첫 번째 그래픽 카드만 사용하겠습니다.

`pickPhysicalDevice` 함수를 추가하고 이를 `initVulkan` 함수에서 호출하겠습니다.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
}

void pickPhysicalDevice() {

}
```

선택하게 될 그래픽 카드는 새로운 클래스 멤버로 추가된 `VkPhysicalDevice` 핸들에 저장됩니다. 이 객체는 `VkInstance`가 삭제될 때 암묵적으로 함께 삭제되므로, 정리(`cleanup`) 함수에서 별도로 처리할 필요가 없습니다.

```C++
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
```

그래픽 카드를 나열하는 과정은 확장을 나열하는 과정과 매우 유사하며, 먼저 개수만을 쿼리하는 것으로 시작합니다.

```C++
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
```

만약 Vulkan을 지원하는 장치가 0개라면, 더 이상 진행할 필요가 없습니다.

```C++
if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
}
```

그렇지 않다면, 이제 모든 `VkPhysicalDevice` 핸들을 저장할 배열을 할당할 수 있습니다.

```C++
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
```

이제 각 장치를 평가하고 우리가 수행하려는 작업에 적합한지 확인해야 합니다. 모든 그래픽 카드가 동일하지 않기 때문입니다. 이를 위해 새로운 함수를 도입할 것입니다.

```C++
bool isDeviceSuitable(VkPhysicalDevice device) {
    return true;
}
```

그리고 그 함수에 추가할 요구사항을 충족하는 물리 장치가 있는지 확인할 것입니다.

```C++
for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
        physicalDevice = device;
        break;
    }
}

if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
}
```

다음 섹션에서는 `isDeviceSuitable` 함수에서 확인할 첫 번째 요구사항을 소개합니다. 이후 챕터에서 더 많은 Vulkan 기능을 사용할수록 이 함수에 더 많은 확인 사항을 추가하게 될 것입니다.

## Base device suitability checks

장치의 적합성을 평가하려면 몇 가지 세부 정보를 쿼리하는 것부터 시작할 수 있습니다. 이름, 유형, 지원하는 Vulkan 버전과 같은 기본 장치 속성은 `vkGetPhysicalDeviceProperties`를 사용하여 쿼리할 수 있습니다.

```C++
VkPhysicalDeviceProperties deviceProperties;
vkGetPhysicalDeviceProperties(device, &deviceProperties);
```

텍스처 압축, 64비트 부동 소수점, 멀티 뷰포트 렌더링(VR에 유용)과 같은 선택적 기능 지원 여부는 `vkGetPhysicalDeviceFeatures`를 사용하여 쿼리할 수 있습니다.

```C++
VkPhysicalDeviceFeatures deviceFeatures;
vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
```

장치 메모리와 큐 패밀리와 관련하여 쿼리할 수 있는 추가 세부 정보가 있으며, 이는 다음 섹션에서 다룰 것입니다.

예를 들어, 애플리케이션이 지오메트리 셰이더를 지원하는 전용 그래픽 카드에서만 작동하도록 고려한다고 가정해 봅시다. 이 경우 `isDeviceSuitable` 함수는 다음과 같은 형태가 될 것입니다.

```C++
bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader;
}
```

단순히 장치가 적합한지 여부만 확인하고 첫 번째 장치를 선택하는 대신, 각 장치에 점수를 부여하고 최고 점수를 가진 장치를 선택할 수도 있습니다. 이렇게 하면 전용 그래픽 카드는 더 높은 점수를 부여하여 우선적으로 선택하고, 유일한 선택지가 통합 GPU인 경우에는 이를 대체 옵션으로 사용할 수 있습니다. 다음과 같이 이를 구현할 수 있습니다.

```C++
#include <map>

...

void pickPhysicalDevice() {
    ...

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

int rateDeviceSuitability(VkPhysicalDevice device) {
    ...

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}
```

이 튜토리얼에서는 모든 것을 구현할 필요는 없지만, 장치 선택 프로세스를 어떻게 설계할 수 있는지에 대한 아이디어를 제공하기 위한 것입니다. 물론, 선택지를 화면에 표시하고 사용자에게 선택하도록 할 수도 있습니다.

지금은 시작 단계이기 때문에 Vulkan 지원 여부만 확인하면 되며, 따라서 어떤 GPU든 사용 가능한 것으로 만족하겠습니다.

```C++
bool isDeviceSuitable(VkPhysicalDevice device) {
    return true;
}
```

다음 섹션에서는 확인해야 할 첫 번째 실제 필수 기능에 대해 논의하겠습니다.

## Queue families

이전에 간단히 언급했듯이, Vulkan에서는 그리기부터 텍스처 업로드에 이르기까지 거의 모든 작업이 명령을 큐에 제출하는 방식으로 이루어집니다. 이러한 큐는 서로 다른 큐 패밀리에서 파생되며, 각 큐 패밀리는 명령의 하위 집합만 허용합니다. 예를 들어, 계산 명령만 처리할 수 있는 큐 패밀리나 메모리 전송 관련 명령만 허용하는 큐 패밀리가 있을 수 있습니다.

우리는 장치가 지원하는 큐 패밀리를 확인하고, 그중에서 우리가 사용하려는 명령을 지원하는 큐를 찾아야 합니다. 이를 위해 필요한 모든 큐 패밀리를 찾는 `findQueueFamilies`라는 새로운 함수를 추가할 것입니다.

현재는 그래픽 명령을 지원하는 큐만 찾으면 되므로 함수는 다음과 같은 형태가 될 수 있습니다.

```C++
uint32_t findQueueFamilies(VkPhysicalDevice device) {
    // Logic to find graphics queue family
}
```

그러나 다음 챕터 중 하나에서는 추가적인 큐를 찾아야 하므로, 이를 대비해 인덱스를 구조체로 묶는 것이 좋습니다.

```C++
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with
    return indices;
}
```

하지만 큐 패밀리가 없으면 어떻게 해야 할까요? `findQueueFamilies` 함수에서 예외를 던질 수도 있지만, 이 함수는 장치 적합성에 대한 결정을 내리기에 적절한 위치가 아닙니다. 예를 들어, 전용 전송 큐 패밀리를 선호할 수는 있지만 반드시 필요하지는 않을 수 있습니다. 따라서 특정 큐 패밀리를 발견했는지 여부를 나타낼 방법이 필요합니다.

큐 패밀리가 존재하지 않음을 나타내기 위해 마법의 값을 사용하는 것은 적합하지 않습니다. 왜냐하면 `uint32_t`의 모든 값이 이론적으로는 유효한 큐 패밀리 인덱스일 수 있기 때문입니다. 다행히도 C++17에서는 값의 존재 여부를 구분할 수 있는 데이터 구조가 도입되었습니다.

```C++
#include <optional>

...

std::optional<uint32_t> graphicsFamily;

std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; // false

graphicsFamily = 0;

std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; // true
```

`std::optional`은 값을 할당하기 전까지는 아무 값도 포함하지 않는 래퍼입니다. 언제든지 `has_value()` 멤버 함수를 호출하여 값이 존재하는지 확인할 수 있습니다. 이를 통해 로직을 다음과 같이 변경할 수 있습니다.

```C++
#include <optional>

...

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Assign index to queue families that could be found
    return indices;
}
```

이제 `findQueueFamilies`를 실제로 구현하기 시작할 수 있습니다.

```C++
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    ...

    return indices;
}
```

큐 패밀리 목록을 검색하는 과정은 예상대로 진행되며, `vkGetPhysicalDeviceQueueFamilyProperties`를 사용합니다.

```C++
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
```

`VkQueueFamilyProperties` 구조체는 큐 패밀리에 대한 세부 정보를 포함하며, 지원되는 작업 유형과 해당 패밀리를 기반으로 생성할 수 있는 큐의 수를 제공합니다. 우리는 `VK_QUEUE_GRAPHICS_BIT`을 지원하는 큐 패밀리를 적어도 하나 찾아야 합니다.

```C++
int i = 0;
for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
    }

    i++;
}
```

이제 이 정교한 큐 패밀리 검색 함수를 사용하여 `isDeviceSuitable` 함수에서 장치가 우리가 사용하려는 명령을 처리할 수 있는지 확인할 수 있습니다.

```C++
bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.graphicsFamily.has_value();
}
```

이를 조금 더 편리하게 만들기 위해, 구조체 자체에 일반적인 확인 기능도 추가할 것입니다.

```C++
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

...

bool isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete();
}
```

이제 `findQueueFamilies`에서 조기 종료를 위해 이 기능을 사용할 수도 있습니다.

```C++
for (const auto& queueFamily : queueFamilies) {
    ...

    if (indices.isComplete()) {
        break;
    }

    i++;
}
```

좋습니다! 적절한 물리 장치를 찾기 위해 필요한 모든 준비가 완료되었습니다. 다음 단계는 이 장치와 인터페이스하기 위한 논리 장치를 생성하는 것입니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/03_physical_device_selection.cpp)