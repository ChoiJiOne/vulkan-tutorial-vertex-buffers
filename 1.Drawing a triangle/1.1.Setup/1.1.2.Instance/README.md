# Instance

## Creating an instance

가장 먼저 해야 할 일은 Vulkan 라이브러리를 초기화하는 것입니다. 이를 위해 인스턴스를 생성해야 합니다. 인스턴스는 애플리케이션과 Vulkan 라이브러리 간의 연결 역할을 하며, 이를 생성하는 과정에서 드라이버에 애플리케이션에 대한 일부 세부 정보를 지정해야 합니다.

먼저 `createInstance` 함수를 추가하고, `initVulkan` 함수에서 이를 호출하도록 하세요.

```C++
void initVulkan() {
    createInstance();
}
```

또한, 인스턴스 핸들을 저장할 데이터 멤버를 추가하세요:

```C++
private:
    VkInstance instance;
```

이제 인스턴스를 생성하기 위해, 먼저 애플리케이션에 대한 정보를 담은 구조체를 채워야 합니다. 이 데이터는 기술적으로 선택 사항이지만, 드라이버가 특정 애플리케이션을 최적화하는 데 유용한 정보를 제공할 수 있습니다(예: 특정 특수 동작을 가진 잘 알려진 그래픽 엔진을 사용할 경우). 이 구조체는 `VkApplicationInfo`라고 합니다.

```C++
void createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
}
```

앞서 언급했듯이, Vulkan의 많은 구조체는 `sType` 멤버에 타입을 명시적으로 지정해야 합니다. 이 구조체는 또한 미래에 확장 정보가 담길 수 있는 `pNext` 멤버를 가진 여러 구조체 중 하나입니다. 우리는 값 초기화를 사용하여 이를 `nullptr`로 두었습니다.

Vulkan에서는 많은 정보가 함수 매개변수 대신 구조체를 통해 전달되며, 인스턴스를 생성하기 위해 충분한 정보를 제공하려면 또 하나의 구조체를 채워야 합니다. 이 구조체는 선택 사항이 아니며, Vulkan 드라이버에 우리가 사용하려는 전역 확장과 검증 레이어를 알려줍니다. 여기서 전역(global)은 특정 디바이스가 아니라 전체 프로그램에 적용된다는 의미이며, 이는 다음 몇 개의 장에서 명확해질 것입니다.

```C++
VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;
```

첫 번째 두 매개변수는 간단합니다. 다음 두 레이어는 원하는 전역 확장을 지정합니다. 개요 장에서 언급했듯이, Vulkan은 플랫폼에 독립적인 API이기 때문에 창 시스템과 인터페이스를 하기 위해 확장이 필요합니다. GLFW는 이를 위해 필요한 확장 목록을 반환하는 유용한 내장 함수를 제공하며, 우리는 이를 구조체에 전달할 수 있습니다.

```C++
uint32_t glfwExtensionCount = 0;
const char** glfwExtensions;

glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

createInfo.enabledExtensionCount = glfwExtensionCount;
createInfo.ppEnabledExtensionNames = glfwExtensions;
```

구조체의 마지막 두 멤버는 활성화할 전역 검증 레이어를 결정합니다. 이 부분은 다음 장에서 더 자세히 다룰 예정이므로, 지금은 비워 두셔도 됩니다.

```C++
createInfo.enabledLayerCount = 0;
```

이제 Vulkan이 인스턴스를 생성하는 데 필요한 모든 정보를 지정했으며, 드디어 `vkCreateInstance` 호출을 할 수 있습니다.

```C++
VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
```

Vulkan에서 객체 생성 함수 매개변수의 일반적인 패턴은 다음과 같습니다:

- 생성 정보가 담긴 구조체에 대한 포인터
- 사용자 정의 할당자 콜백에 대한 포인터, 이 튜토리얼에서는 항상 nullptr
- 새로운 객체의 핸들을 저장할 변수에 대한 포인터

모든 것이 잘 진행되었다면, 인스턴스의 핸들은 `VkInstance` 클래스 멤버에 저장됩니다. 거의 모든 Vulkan 함수는 `VkResult` 타입을 반환하며, 이는 `VK_SUCCESS` 또는 오류 코드입니다. 인스턴스가 성공적으로 생성되었는지 확인하려면 결과를 저장할 필요 없이 성공 값을 체크하면 됩니다:

이제 프로그램을 실행하여 인스턴스가 성공적으로 생성되었는지 확인하세요.

## Encountered VK_ERROR_INCOMPATIBLE_DRIVER:

최신 MoltenVK SDK를 사용하는 MacOS에서는 `vkCreateInstance` 호출 시 `VK_ERROR_INCOMPATIBLE_DRIVER` 오류가 반환될 수 있습니다. 시작 노트에 따르면, Vulkan SDK 1.3.216부터 `VK_KHR_PORTABILITY_subset` 확장은 필수입니다.

이 오류를 해결하려면, 먼저 `VkInstanceCreateInfo` 구조체의 플래그에 `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR` 비트를 추가하고, 그 다음으로 인스턴스에서 활성화할 확장 목록에 `VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME`을 추가해야 합니다.

일반적으로 코드는 다음과 같이 작성될 수 있습니다:

```C++
...

std::vector<const char*> requiredExtensions;

for(uint32_t i = 0; i < glfwExtensionCount; i++) {
    requiredExtensions.emplace_back(glfwExtensions[i]);
}

requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
createInfo.ppEnabledExtensionNames = requiredExtensions.data();

if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
}
```

## Checking for extension support

`vkCreateInstance` 문서를 보면 가능한 오류 코드 중 하나가 `VK_ERROR_EXTENSION_NOT_PRESENT`임을 확인할 수 있습니다. 우리는 필요한 확장 기능을 지정하고, 해당 오류 코드가 반환되면 프로그램을 종료할 수 있습니다. 이는 창 시스템 인터페이스와 같은 필수 확장 기능에 대해 합리적이지만, 선택적인 기능을 확인하려면 어떻게 해야 할까요?

인스턴스를 생성하기 전에 지원되는 확장 목록을 가져오려면 `vkEnumerateInstanceExtensionProperties` 함수를 사용할 수 있습니다. 이 함수는 확장의 수를 저장할 변수와 확장의 세부 정보를 저장할 `VkExtensionProperties` 배열을 인자로 받습니다. 또한, 특정 검증 레이어를 기준으로 확장을 필터링할 수 있는 선택적 첫 번째 매개변수를 받지만, 우리는 이 부분은 지금은 무시할 것입니다.

확장 세부 정보를 담을 배열을 할당하려면 먼저 확장의 수를 알아야 합니다. 후자의 매개변수를 비워 두면 확장 수만 요청할 수 있습니다:

```C++
uint32_t extensionCount = 0;
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
```

이제 확장 세부 정보를 담을 배열을 할당합니다 (여기서 `<vector>` 헤더를 포함):

```C++
std::vector<VkExtensionProperties> extensions(extensionCount);
```

마침내 확장 세부 정보를 쿼리할 수 있습니다:

```C++
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
```

각 `VkExtensionProperties` 구조체는 확장의 이름과 기타 정보를 포함합니다.

```C++
std::cout << "available extensions:\n";

for (const auto& extension : extensions) {
    std::cout << '\t' << extension.extensionName << '\n';
}
```

이 코드를 `createInstance` 함수에 추가하면 Vulkan 지원에 대한 세부 정보를 제공할 수 있습니다. 도전 과제로, `glfwGetRequiredInstanceExtensions`가 반환한 모든 확장이 지원되는 확장 목록에 포함되어 있는지 확인하는 함수를 작성해 보세요.

## Cleaning up

`VkInstance`는 프로그램이 종료되기 직전에만 파괴되어야 합니다. `vkDestroyInstance` 함수를 사용하여 정리(cleanup) 시에 파괴할 수 있습니다:

```C++
void cleanup() {
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
```

`vkDestroyInstance` 함수의 매개변수는 간단합니다. 이전 장에서 언급했듯이, Vulkan에서의 할당 및 해제 함수는 선택적인 할당자 콜백을 제공하지만, 우리는 이를 무시하고 `nullptr`을 전달합니다. 이후 장에서 생성할 다른 모든 Vulkan 리소스는 인스턴스가 파괴되기 전에 정리되어야 합니다.

인스턴스 생성 후 더 복잡한 단계로 진행하기 전에, 디버깅 옵션을 평가하고 검증 레이어(validation layers)를 확인할 차례입니다.

## Source Code
- [C++ Code](https://vulkan-tutorial.com/code/01_instance_creation.cpp)