# Window surface

Vulkan은 플랫폼에 구애받지 않는 API이기 때문에, 자체적으로 윈도우 시스템과 직접 상호작용할 수 없습니다. Vulkan과 윈도우 시스템 간 연결을 설정하여 화면에 결과를 표시하려면 WSI(Window System Integration) 확장을 사용해야 합니다. 이 장에서는 첫 번째 WSI 확장인 `VK_KHR_surface`에 대해 다룰 것입니다. 이 확장은 렌더링된 이미지를 표시할 추상적인 표면을 나타내는 `VkSurfaceKHR` 객체를 노출합니다. 프로그램에서 이 표면은 이미 GLFW를 사용해 열어 둔 창을 기반으로 합니다.

`VK_KHR_surface` 확장은 인스턴스 수준 확장으로, 사실 이미 활성화해 두었습니다. 이는 `glfwGetRequiredInstanceExtensions` 함수가 반환한 목록에 포함되어 있기 때문입니다. 이 목록에는 이후 몇 개의 장에서 사용할 다른 WSI 확장도 포함되어 있습니다.

윈도우 표면은 인스턴스를 생성한 직후에 만들어야 하는데, 이는 물리적 디바이스 선택에 실제로 영향을 줄 수 있기 때문입니다. 앞에서 이 작업을 미뤘던 이유는 윈도우 표면이 렌더 타겟 및 프레젠테이션이라는 더 큰 주제의 일부이기 때문입니다. 이러한 설명을 초기 설정 과정에 포함하면 내용이 산만해질 수 있었습니다. 또한, 윈도우 표면은 Vulkan에서 완전히 선택적인 구성 요소라는 점도 주목해야 합니다. 단순히 오프스크린 렌더링만 필요하다면, OpenGL처럼 보이지 않는 창을 생성하는 등의 꼼수를 쓰지 않고도 Vulkan에서 이를 수행할 수 있습니다.

## Window surface creation

디버그 콜백 바로 아래에 표면(surface) 클래스 멤버를 추가하는 것으로 시작하겠습니다.

```C++
VkSurfaceKHR surface;
```

`VkSurfaceKHR` 객체와 그 사용 방식은 플랫폼에 독립적이지만, 생성 과정은 플랫폼의 윈도우 시스템 세부사항에 의존하기 때문에 그렇지 않습니다. 예를 들어, Windows에서는 `HWND`와 `HMODULE` 핸들이 필요합니다. 따라서 확장에는 플랫폼별 추가 사항이 있으며, Windows에서는 이를 `VK_KHR_win32_surface`라고 합니다. 이 확장은 `glfwGetRequiredInstanceExtensions` 함수가 반환하는 목록에도 자동으로 포함됩니다.

이 플랫폼별 확장이 Windows에서 표면을 생성하는 데 어떻게 사용될 수 있는지 보여드리겠습니다. 하지만 이 튜토리얼에서는 실제로 사용하지는 않을 것입니다. GLFW 같은 라이브러리를 사용하면서 플랫폼별 코드를 사용하는 것은 큰 의미가 없기 때문입니다. GLFW에는 플랫폼 차이를 처리해 주는 `glfwCreateWindowSurface` 함수가 있기 때문에 이를 활용할 예정입니다. 그럼에도 불구하고, 본격적으로 GLFW를 의존하기 전에 이 함수가 내부적으로 어떤 작업을 수행하는지 확인하는 것이 좋습니다.

플랫폼 고유 함수에 접근하려면, 상단의 include를 다음과 같이 업데이트해야 합니다:

```C++
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
```

창 표면은 Vulkan 객체이므로, VkWin32SurfaceCreateInfoKHR 구조체를 채워야 합니다. 이 구조체에는 두 가지 중요한 매개변수인 hwnd와 hinstance가 있습니다. 이들은 창과 프로세스의 핸들을 나타냅니다.

```C++
VkWin32SurfaceCreateInfoKHR createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
createInfo.hwnd = glfwGetWin32Window(window);
createInfo.hinstance = GetModuleHandle(nullptr);
```

`glfwGetWin32Window` 함수는 GLFW 창 객체에서 원시 HWND를 가져오는 데 사용됩니다. `GetModuleHandle` 호출은 현재 프로세스의 `HINSTANCE` 핸들을 반환합니다.

그 후, 표면은 `vkCreateWin32SurfaceKHR`을 사용하여 생성할 수 있습니다. 이 함수는 인스턴스, 표면 생성 세부정보, 사용자 정의 할당자, 그리고 표면 핸들을 저장할 변수를 매개변수로 포함합니다. 기술적으로 이는 WSI 확장 함수이지만, 너무 자주 사용되기 때문에 표준 Vulkan 로더에 포함되어 있습니다. 따라서 다른 확장과 달리 명시적으로 로드할 필요가 없습니다.

```C++
if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
}
```

리눅스와 같은 다른 플랫폼에서는 과정이 비슷합니다. 예를 들어, `vkCreateXcbSurfaceKHR`은 XCB 연결과 창을 생성 세부정보로 사용하여 X11에서 작동합니다.

`glfwCreateWindowSurface` 함수는 각 플랫폼에 대해 다른 구현을 사용하여 정확히 이 작업을 수행합니다. 이제 이를 프로그램에 통합할 것입니다. 인스턴스 생성과 `setupDebugMessenger` 이후에 `initVulkan`에서 호출되도록 `createSurface`라는 함수를 추가하세요.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

void createSurface() {

}
```

GLFW 호출은 구조체 대신 간단한 매개변수를 사용하기 때문에 함수 구현이 매우 간단합니다:

```C++
void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
```

매개변수는 VkInstance, GLFW 창 포인터, 사용자 정의 할당자, 그리고 `VkSurfaceKHR` 변수에 대한 포인터입니다. 이 함수는 관련 플랫폼 호출에서 반환된 `VkResult`를 그대로 전달합니다. GLFW는 표면을 제거하는 특별한 함수를 제공하지 않지만, 이는 원래의 API를 통해 쉽게 수행할 수 있습니다:

```C++
void cleanup() {
        ...
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        ...
    }
```

표면을 인스턴스보다 먼저 제거해야 한다는 점을 반드시 유의하세요.

## Querying for presentation support

Vulkan 구현이 창 시스템 통합을 지원할 수 있지만, 시스템 내의 모든 디바이스가 이를 지원하는 것은 아닙니다. 따라서, `isDeviceSuitable` 함수를 확장하여 디바이스가 우리가 생성한 표면에 이미지를 표시할 수 있는지 확인해야 합니다. 표시 기능은 큐별로 특정된 기능이므로, 실제 문제는 우리가 생성한 표면에 표시할 수 있는 큐 패밀리를 찾는 것입니다.

그리기 명령을 지원하는 큐 패밀리와 표시를 지원하는 큐 패밀리가 겹치지 않을 가능성도 있습니다. 따라서 `QueueFamilyIndices` 구조체를 수정하여 별도의 표시 큐가 있을 수 있다는 점을 고려해야 합니다:

```C++
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
```

다음으로, findQueueFamilies 함수를 수정하여 창 표면에 표시할 수 있는 큐 패밀리를 찾도록 할 것입니다. 이를 확인하는 함수는 `vkGetPhysicalDeviceSurfaceSupportKHR`로, 물리 디바이스, 큐 패밀리 인덱스, 그리고 표면을 매개변수로 받습니다. 이를 `VK_QUEUE_GRAPHICS_BIT`와 동일한 루프에서 호출하도록 추가합니다:

```C++
VkBool32 presentSupport = false;
vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
```

그런 다음, 불리언 값의 결과를 확인하고 표시 큐 패밀리 인덱스를 저장합니다:

```C++
if (presentSupport) {
    indices.presentFamily = i;
}
```

이 두 큐 패밀리가 결국 동일할 가능성이 높지만, 프로그램 전반에서 이를 별개의 큐로 취급하여 일관된 접근 방식을 유지합니다. 그러나 성능 향상을 위해 그리기와 표시를 동일 큐에서 지원하는 물리 디바이스를 명시적으로 선호하도록 로직을 추가할 수도 있습니다.

## Creating the presentation queue

남은 작업은 논리적 디바이스 생성 절차를 수정하여 표시 큐를 생성하고 `VkQueue` 핸들을 가져오는 것입니다. 이를 위해 핸들을 저장할 멤버 변수를 추가하세요:

```C++
VkQueue presentQueue;
```

그다음, 여러 `VkDeviceQueueCreateInfo` 구조체를 생성하여 두 큐 패밀리 모두에서 큐를 생성해야 합니다. 이를 우아하게 처리하는 방법은 필요한 큐에 필요한 모든 고유 큐 패밀리의 집합을 만드는 것입니다:

```C++
#include <set>

...

QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

float queuePriority = 1.0f;
for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
}
```

그리고 `VkDeviceCreateInfo`를 수정하여 벡터를 가리키도록 설정합니다:

```C++
createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
createInfo.pQueueCreateInfos = queueCreateInfos.data();
```

큐 패밀리가 동일한 경우, 해당 인덱스만 한 번 전달하면 됩니다. 마지막으로 큐 핸들을 가져오는 호출을 추가하세요:

```C++
vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
```

큐 패밀리가 동일한 경우, 두 핸들이 같은 값을 가지게 될 가능성이 높습니다. 다음 장에서는 스왑 체인(swap chains)을 살펴보고, 이를 통해 이미지를 표면에 표시할 수 있는 방법을 다룰 것입니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/05_window_surface.cpp)