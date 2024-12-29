# Setup: Validation layers

## What are validation layers?

Vulkan API는 최소한의 드라이버 오버헤드를 목표로 설계되었으며, 이를 실현하기 위한 한 가지 방식은 기본적으로 API에서 오류 검사를 매우 제한적으로 수행하는 것입니다. 열거형 값을 잘못 설정하거나 필수 매개변수에 null 포인터를 전달하는 것과 같은 간단한 실수조차도 일반적으로 명시적으로 처리되지 않으며, 충돌이나 정의되지 않은 동작으로 이어질 수 있습니다. Vulkan은 사용자가 수행하는 모든 작업에 대해 매우 명시적이기를 요구하기 때문에, 새로운 GPU 기능을 사용하면서 논리적 디바이스 생성 시 이를 요청하는 것을 잊는 것과 같은 작은 실수를 저지르기 쉽습니다.

그러나, 그렇다고 해서 이러한 검사 기능을 API에 추가할 수 없다는 것은 아닙니다. Vulkan은 이를 위한 우아한 시스템을 도입했는데, 이를 **검증 레이어(Validation Layers)**라고 합니다. 검증 레이어는 Vulkan 함수 호출에 연결되어 추가 작업을 수행하는 선택적 구성 요소입니다. 검증 레이어에서 일반적으로 수행되는 작업은 다음과 같습니다:

- 사양에 따라 매개변수 값을 확인하여 잘못된 사용 감지
- 객체의 생성 및 소멸을 추적하여 리소스 누수 탐지
- 호출이 발생한 스레드를 추적하여 스레드 안전성 확인
- 모든 호출 및 해당 매개변수를 표준 출력에 기록
- Vulkan 호출을 추적하여 프로파일링 및 재생

다음은 진단 검증 레이어에서 함수 구현이 어떻게 보일 수 있는지에 대한 예시입니다:

```C++
VkResult vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* instance) {

    if (pCreateInfo == nullptr || instance == nullptr) {
        log("Null pointer passed to required parameter!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return real_vkCreateInstance(pCreateInfo, pAllocator, instance);
}
```

이러한 검증 레이어는 원하는 디버깅 기능을 포함하도록 자유롭게 쌓을 수 있습니다. 디버그 빌드에서는 검증 레이어를 활성화하고, 릴리스 빌드에서는 완전히 비활성화하여 두 세계의 장점을 모두 누릴 수 있습니다!

Vulkan에는 기본적으로 검증 레이어가 포함되어 있지 않지만, LunarG Vulkan SDK는 일반적인 오류를 확인할 수 있는 훌륭한 레이어 세트를 제공합니다. 이들은 완전히 오픈 소스이므로, 어떤 종류의 실수를 점검하는지 확인하고 기여할 수도 있습니다. 검증 레이어를 사용하는 것은 정의되지 않은 동작에 의존하여 애플리케이션이 다른 드라이버에서 깨지는 것을 방지하는 가장 좋은 방법입니다.

검증 레이어는 시스템에 설치된 경우에만 사용할 수 있습니다. 예를 들어, LunarG 검증 레이어는 Vulkan SDK가 설치된 PC에서만 사용할 수 있습니다.

과거에는 Vulkan에서 인스턴스 검증 레이어와 디바이스 전용 검증 레이어라는 두 가지 유형의 검증 레이어가 있었습니다. 원래의 개념은 인스턴스 레이어는 인스턴스와 같은 글로벌 Vulkan 객체와 관련된 호출만 점검하고, 디바이스 전용 레이어는 특정 GPU와 관련된 호출만 점검한다는 것이었습니다. 그러나 디바이스 전용 레이어는 이제 더 이상 사용되지 않으며, 따라서 인스턴스 검증 레이어가 모든 Vulkan 호출에 적용됩니다.

사양 문서는 여전히 호환성을 위해 디바이스 수준에서도 검증 레이어를 활성화할 것을 권장하며, 이는 일부 구현에서 요구됩니다. 논리적 디바이스 수준에서 인스턴스와 동일한 레이어를 지정하는 방식으로 이를 처리할 것이며, 이에 대한 내용은 이후에 다룰 예정입니다.

## Using validation layers

이 섹션에서는 Vulkan SDK에서 제공하는 표준 진단 레이어를 활성화하는 방법을 알아보겠습니다. 확장 기능과 마찬가지로, 검증 레이어는 이름을 지정하여 활성화해야 합니다. 유용한 표준 검증 기능은 모두 SDK에 포함된 **VK_LAYER_KHRONOS_validation**이라는 레이어에 번들로 제공됩니다.

먼저, 프로그램에 두 개의 구성 변수를 추가하여 활성화할 레이어와 활성화 여부를 지정해 보겠습니다. 여기서는 프로그램이 디버그 모드에서 컴파일되고 있는지 여부에 따라 값을 설정하도록 하겠습니다. NDEBUG 매크로는 C++ 표준의 일부로, "디버그 아님"을 의미합니다.

```C++
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
```

새로운 함수 `checkValidationLayerSupport`를 추가하여 요청한 모든 레이어가 사용 가능한지 확인하겠습니다. 먼저 `vkEnumerateInstanceLayerProperties` 함수를 사용하여 모든 사용 가능한 레이어를 나열합니다. 이 함수의 사용법은 `vkEnumerateInstanceExtensionProperties`와 동일하며, 이는 인스턴스 생성 챕터에서 다루었습니다.

```C++
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return false;
}
```

다음으로, `validationLayers`에 있는 모든 레이어가 `availableLayers` 목록에 존재하는지 확인합니다. 이때 `strcmp`를 사용하기 위해 `<cstring>` 헤더를 포함해야 할 수도 있습니다.

```C++
for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
        }
    }

    if (!layerFound) {
        return false;
    }
}

return true;
```

이제 이 함수를 `createInstance` 함수에서 사용할 수 있습니다:

```C++
void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    ...
}
```

디버그 모드에서 프로그램을 실행하여 오류가 발생하지 않는지 확인합니다. 만약 오류가 발생한다면 FAQ를 참고하세요.

마지막으로, `VkInstanceCreateInfo` 구조체의 초기화를 수정하여 검증 레이어가 활성화된 경우 해당 레이어 이름을 포함시키도록 설정합니다:

```C++
if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}
```

검증 레이어 확인이 성공적으로 이루어졌다면, `vkCreateInstance`는 절대로 `VK_ERROR_LAYER_NOT_PRESENT` 오류를 반환하지 않아야 합니다. 그러나, 프로그램을 실행하여 이를 확실히 확인하는 것이 중요합니다.

## Message callback

검증 레이어는 기본적으로 디버그 메시지를 표준 출력으로 출력하지만, 프로그램에 명시적인 콜백을 제공함으로써 직접 처리할 수도 있습니다. 이렇게 하면 치명적인 오류가 아닌 메시지도 선택적으로 확인할 수 있습니다. 지금 당장 이 작업을 원하지 않는다면, 이 장의 마지막 섹션으로 건너뛰어도 괜찮습니다.

프로그램에서 메시지를 처리하기 위한 콜백을 설정하려면, `VK_EXT_debug_utils` 확장을 사용하여 디버그 메시저와 콜백을 설정해야 합니다.

먼저, `getRequiredExtensions` 함수를 작성하여 검증 레이어가 활성화되었는지 여부에 따라 필요한 확장 목록을 반환하겠습니다:

```C++
std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}
```

GLFW에서 지정한 확장은 항상 필요하며, 디버그 메시저 확장은 조건부로 추가됩니다. 여기서 `VK_EXT_DEBUG_UTILS_EXTENSION_NAME` 매크로를 사용했는데, 이 매크로는 문자열 `VK_EXT_debug_utils`와 동일합니다. 이 매크로를 사용하면 오타를 방지할 수 있습니다.

이제 `createInstance` 함수에서 이 getRequiredExtensions 함수를 사용할 수 있습니다:

```C++
auto extensions = getRequiredExtensions();
createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
createInfo.ppEnabledExtensionNames = extensions.data();
```

프로그램을 실행하여 `VK_ERROR_EXTENSION_NOT_PRESENT` 오류가 발생하지 않는지 확인하세요. 이 확장의 존재 여부를 확인할 필요는 없습니다. 왜냐하면 검증 레이어가 제공되면 이 확장이 자동으로 포함되어야 하기 때문입니다.

이제 디버그 콜백 함수가 어떻게 생겼는지 살펴보겠습니다. `debugCallback`이라는 새로운 정적 멤버 함수를 추가하고, 이 함수는 `PFN_vkDebugUtilsMessengerCallbackEXT` 프로토타입을 가집니다. `VKAPI_ATTR`와 `VKAPI_CALL`은 이 함수가 Vulkan이 호출할 수 있는 올바른 서명을 갖도록 보장합니다.

```C++
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
```

첫 번째 매개변수는 메시지의 심각도를 지정하며, 이는 다음 플래그 중 하나입니다:

- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT`: 진단 메시지
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT`: 리소스 생성과 같은 정보 메시지
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT`: 오류는 아니지만 애플리케이션에서 버그가 발생할 가능성이 높은 동작에 대한 메시지
- `VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT`: 잘못된 동작에 대한 메시지로, 크래시를 일으킬 수 있음

이 열거형 값들은 비교 연산을 통해 메시지가 특정 심각도 수준과 같거나 더 심각한지 확인할 수 있도록 설정되어 있습니다. 예를 들어:

```
if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
}
```

`messageType` 매개변수는 다음과 같은 값들을 가질 수 있습니다:

- `VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT`: 사양이나 성능과 관련 없는 이벤트가 발생했을 때
- `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT`: 사양을 위반했거나 잠재적인 실수를 나타내는 일이 발생했을 때
- `VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT`: Vulkan의 비최적 사용 가능성에 대한 메시지

`pCallbackData` 매개변수는 메시지 자체에 대한 세부 정보를 담고 있는 `VkDebugUtilsMessengerCallbackDataEXT` 구조체를 참조합니다. 이 구조체에서 가장 중요한 멤버는 다음과 같습니다:

- `pMessage`: 널 종료 문자열로 된 디버그 메시지
- `pObjects`: 메시지와 관련된 Vulkan 객체 핸들의 배열
- `objectCount`: 배열의 객체 수

마지막으로, `pUserData` 매개변수는 콜백 설정 중에 지정된 포인터를 포함하며, 이를 통해 사용자 정의 데이터를 콜백에 전달할 수 있습니다.

콜백은 불리언 값을 반환하며, 이 값은 해당 검증 레이어 메시지를 트리거한 Vulkan 호출이 중단되어야 하는지를 나타냅니다. 콜백이 true를 반환하면, 호출은 `VK_ERROR_VALIDATION_FAILED_EXT` 오류와 함께 중단됩니다. 이는 보통 검증 레이어 자체를 테스트할 때만 사용되므로, 일반적으로 `VK_FALSE`를 반환해야 합니다.

이제 남은 작업은 Vulkan에 콜백 함수에 대해 알리는 것입니다. 다소 놀랍게도, Vulkan에서 디버그 콜백도 명시적으로 생성하고 소멸시켜야 하는 핸들로 관리됩니다. 이러한 콜백은 디버그 메시저의 일부이며, 원하는 만큼 여러 개를 가질 수 있습니다. 이 핸들을 `instance` 바로 아래에 클래스 멤버로 추가하세요.

```C++
VkDebugUtilsMessengerEXT debugMessenger;
```

이제 `setupDebugMessenger` 함수를 추가하여 `createInstance` 직후 `initVulkan`에서 호출되도록 설정하세요.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
}

void setupDebugMessenger() {
    if (!enableValidationLayers) return;

}
```

디버그 메시저와 그 콜백에 대한 세부 정보를 채우기 위해 구조체를 작성해야 합니다.

```C++
VkDebugUtilsMessengerCreateInfoEXT createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
createInfo.pfnUserCallback = debugCallback;
createInfo.pUserData = nullptr; // Optional
```

`messageSeverity` 필드는 콜백이 호출되기를 원하는 심각도 유형을 지정할 수 있게 해줍니다. 여기서는 `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT`를 제외한 모든 유형을 지정하여 일반적인 디버그 정보는 제외하고, 가능한 문제에 대한 알림을 받도록 설정했습니다.

유사하게, `messageType` 필드는 콜백이 알림을 받을 메시지 유형을 필터링할 수 있게 해줍니다. 여기서는 모든 유형을 활성화했습니다. 필요하지 않은 유형은 언제든지 비활성화할 수 있습니다.

마지막으로, `pfnUserCallback` 필드는 콜백 함수의 포인터를 지정합니다. 선택적으로 `pUserData` 필드에 포인터를 전달할 수 있으며, 이는 콜백 함수로 전달됩니다. 예를 들어, `VkApp` 클래스에 대한 포인터를 전달하는 데 사용할 수 있습니다.

검증 레이어 메시지와 디버그 콜백을 구성하는 방법에는 더 많은 옵션이 있지만, 이 설정은 이 튜토리얼을 시작하기에 적합한 설정입니다. 추가적인 가능성에 대한 정보는 [확장 사양](https://docs.vulkan.org/spec/latest/chapters/debugging.html#VK_EXT_debug_utils)에서 확인할 수 있습니다.

이 구조체는 `vkCreateDebugUtilsMessengerEXT` 함수에 전달되어 `VkDebugUtilsMessengerEXT` 객체를 생성해야 합니다. 불행히도 이 함수는 확장 함수이기 때문에 자동으로 로드되지 않습니다. 우리는 `vkGetInstanceProcAddr`를 사용하여 직접 함수 주소를 찾아야 합니다. 이를 처리하는 자체 프록시 함수를 클래스 정의 바로 위에 추가했습니다.

```C++
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
```

`vkGetInstanceProcAddr` 함수는 함수가 로드되지 않으면 `nullptr`를 반환합니다. 이제 이 함수를 호출하여 확장 객체를 생성할 수 있습니다, 만약 사용 가능하다면:

```C++
if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
}
```

두 번째에서 마지막 매개변수는 선택적인 할당자 콜백으로, 이를 `nullptr`로 설정하며, 나머지 매개변수는 비교적 직관적입니다. 디버그 메시저는 특정 Vulkan 인스턴스 및 해당 레이어와 관련이 있기 때문에 첫 번째 매개변수로 명시적으로 지정해야 합니다. 나중에 다른 자식 객체들에서도 이 패턴을 볼 수 있습니다.

`VkDebugUtilsMessengerEXT` 객체는 `vkDestroyDebugUtilsMessengerEXT`를 호출하여 정리해야 합니다. `vkCreateDebugUtilsMessengerEXT`와 마찬가지로 이 함수는 명시적으로 로드해야 합니다.

`CreateDebugUtilsMessengerEXT` 아래에 또 다른 프록시 함수를 추가하세요.

```C++
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
```

이 함수가 static 클래스 함수이거나 클래스 외부에 있는 함수인지 확인하세요. 그런 다음, 이 함수를 정리(`cleanup`) 함수에서 호출할 수 있습니다.

```C++
void cleanup() {
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
```

## Debugging instance creation and destruction

현재까지는 프로그램에 디버깅을 위해 validation layer를 추가했지만, 모든 것을 다룬 것은 아닙니다. `vkCreateDebugUtilsMessengerEXT` 호출은 유효한 인스턴스가 생성되어야 하고, `vkDestroyDebugUtilsMessengerEXT`는 인스턴스가 파괴되기 전에 호출되어야 합니다. 이로 인해 현재 `vkCreateInstance`와 `vkDestroyInstance` 호출에서 발생할 수 있는 문제를 디버깅할 수 없는 상황입니다.

그러나 [확장 문서](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/appendices/VK_EXT_debug_utils.adoc#examples)를 자세히 읽어보면, 이 두 함수 호출을 위한 별도의 debug utils messenger를 생성하는 방법이 있다는 것을 알 수 있습니다. 이를 위해서는 `VkInstanceCreateInfo`의 `pNext` 확장 필드에 `VkDebugUtilsMessengerCreateInfoEXT` 구조체의 포인터를 전달하면 됩니다. 먼저, messenger 생성 정보를 별도의 함수로 추출합니다:

```C++
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

...

void setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}
```

이제 `createInstance` 함수에서 이를 재사용할 수 있습니다:

```C++
void createInstance() {
    ...

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    ...

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}
```

`debugCreateInfo` 변수는 if 문 외부에 배치하여 `vkCreateInstance` 호출 이전에 파괴되지 않도록 합니다. 이렇게 추가적인 debug messenger를 생성하면 `vkCreateInstance`와 `vkDestroyInstance` 동안 자동으로 사용되며, 그 후에 정리됩니다.

## Testing

이제 validation layers가 작동하는지 확인하기 위해 의도적으로 실수를 만들어 봅시다. `cleanup` 함수에서 `DestroyDebugUtilsMessengerEXT` 호출을 임시로 제거하고 프로그램을 실행해 보세요. 프로그램이 종료되면 다음과 같은 메시지를 볼 수 있어야 합니다:

![image00](../Image/image00.png)

> 만약 메시지가 보이지 않는다면, [설치](https://vulkan.lunarg.com/doc/view/1.2.131.1/windows/getting_started.html#user-content-verify-the-installation)를 확인해 보세요.

메시지를 트리거한 호출을 확인하고 싶다면, 메시지 콜백에 중단점을 추가하고 스택 추적을 확인할 수 있습니다.

## Configuration


검증 레이어의 동작을 설정하는 방법에는 `VkDebugUtilsMessengerCreateInfoEXT` 구조체에 지정된 플래그 외에도 많은 설정이 있습니다. Vulkan SDK로 이동하여 Config 디렉토리를 찾아보세요. 그곳에서 `vk_layer_settings.txt` 파일을 확인할 수 있으며, 이 파일은 레이어 설정을 구성하는 방법을 설명합니다.

자신의 애플리케이션에 대한 레이어 설정을 구성하려면 해당 파일을 프로젝트의 `Debug` 및 `Release` 디렉토리에 복사하고, 원하는 동작을 설정하는 방법에 따라 구성하면 됩니다. 하지만 이 튜토리얼의 나머지 부분에서는 기본 설정을 사용하는 것으로 가정하겠습니다.

이 튜토리얼을 진행하면서 몇 가지 의도적인 실수를 만들어서 검증 레이어가 이를 어떻게 잡아내는지 보여주고, Vulkan을 사용할 때 정확히 무엇을 해야 하는지 아는 것이 얼마나 중요한지 가르쳐 줄 것입니다. 이제 시스템에서 Vulkan 장치를 살펴보겠습니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/02_validation_layers.cpp)