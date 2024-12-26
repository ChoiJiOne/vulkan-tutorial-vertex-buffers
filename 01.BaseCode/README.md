# Base Code

## General structure

이전 장에서는 모든 적절한 설정을 통해 Vulkan 프로젝트를 생성하고 샘플 코드를 사용하여 테스트했습니다. 이번 장에서는 다음 코드를 사용하여 처음부터 시작하겠습니다:

```
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication {
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() {

    }

    void mainLoop() {

    }

    void cleanup() {

    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

우선 LunarG SDK에서 제공하는 Vulkan 헤더를 포함합니다. 이 헤더는 함수, 구조체, 열거형을 제공합니다. 오류를 보고하고 전달하기 위해 `stdexcept`와 `iostream` 헤더를 포함하며, `cstdlib` 헤더는 `EXIT_SUCCESS` 및 `EXIT_FAILURE` 매크로를 제공합니다.

프로그램 자체는 클래스로 감싸져 있으며, Vulkan 객체를 클래스의 private 멤버로 저장하고, 각 객체를 초기화하는 함수를 추가합니다. 이러한 함수는 `initVulkan` 함수에서 호출됩니다. 모든 준비가 완료되면 메인 루프에 진입하여 프레임 렌더링을 시작합니다. 곧 메인 루프에 창이 닫힐 때까지 반복하는 루프를 포함하도록 `mainLoop` 함수를 작성할 예정입니다. 창이 닫히고 `mainLoop`간간 반환된 후에는 사용한 리소스를 해제하기 위해 `cleanup` 함수에서 정리 작업을 수행합니다.

실행 중 치명적인 오류가 발생할 경우, 설명 메시지를 포함한 `std::runtime_error` 예외를 던질 것입니다. 이 예외는 메인 함수로 전달되어 명령 프롬프트에 출력됩니다. 또한, 다양한 표준 예외 유형을 처리하기 위해 더 일반적인 `std::exception`도 캐치합니다. 예로는 필수 확장이 지원되지 않는 상황을 다루게 될 것입니다.

이후의 각 장에서는 `initVulkan`에서 호출할 새로운 함수를 추가하고, 정리 단계에서 해제해야 할 새로운 Vulkan 객체를 클래스의 private 멤버에 추가하게 될 것입니다.

## Resource management

`malloc`으로 할당한 메모리를 `free`로 해제해야 하듯, 생성한 모든 Vulkan 객체는 더 이상 필요하지 않을 때 명시적으로 파괴해야 합니다. C++에서는 `<memory>` 헤더가 제공하는 [RAII(Resource Acquisition Is Initialization)](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization)나 스마트 포인터를 사용해 자동으로 리소스를 관리할 수 있습니다. 하지만 이 튜토리얼에서는 Vulkan 객체의 할당 및 해제를 명시적으로 처리하도록 선택했습니다. Vulkan의 강점은 모든 작업을 명시적으로 처리하여 실수를 방지하는 데 있으므로, 객체의 생명 주기를 명확히 다루는 것이 API 작동 방식을 배우는 데 유용합니다.

이 튜토리얼을 완료한 후, Vulkan 객체를 생성자에서 획득하고 소멸자에서 해제하는 C++ 클래스를 작성하거나, `std::unique_ptr` 또는 `std::shared_ptr`에 사용자 정의 삭제자를 제공하여 자동 리소스 관리를 구현할 수 있습니다. 소유권 요구 사항에 따라 적절한 방법을 선택하면 됩니다. RAII는 규모가 큰 Vulkan 프로그램에서 권장되는 모델이지만, 학습 목적에서는 내부적으로 어떻게 작동하는지 이해하는 것이 중요합니다.

Vulkan 객체는 `vkCreateXXX`와 같은 함수로 직접 생성되거나, `vkAllocateXXX`와 같은 함수를 통해 다른 객체로부터 할당됩니다. 객체가 더 이상 사용되지 않는 것을 확인한 후에는 반드시 대응하는 `vkDestroyXXX`와 `vkFreeXXX` 함수로 파괴해야 합니다. 이러한 함수의 매개변수는 객체 유형에 따라 다르지만, 모든 함수에 공통적으로 포함된 하나의 매개변수가 있습니다: `pAllocator`. 이 매개변수는 사용자 정의 메모리 할당기를 위한 콜백을 지정할 수 있는 선택적 매개변수입니다. 이 튜토리얼에서는 이 매개변수를 무시하고 항상 `nullptr`을 인수로 전달할 것입니다.

## Integrating GLFW

Vulkan은 오프스크린 렌더링을 위해 창을 생성하지 않고도 완벽히 동작하지만, 화면에 실제로 무언가를 보여주는 것이 훨씬 흥미롭습니다! 먼저, #include `<vulkan/vulkan.h>` 줄을 다음과 같이 교체하세요:

```
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

GLFW는 자체 정의를 포함하고 Vulkan 헤더를 자동으로 로드합니다. 이제 `initWindow` 함수를 추가하고, `run` 함수에서 다른 호출 이전에 `initWindow`를 호출하도록 설정하세요. 이 함수에서 GLFW를 초기화하고 창을 생성할 것입니다.

```
void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

private:
    void initWindow() {

    }
```

`initWindow`의 첫 번째 호출은 `glfwInit()`이어야 하며, 이는 GLFW 라이브러리를 초기화합니다. GLFW는 원래 OpenGL 컨텍스트 생성을 위해 설계되었기 때문에, 다음과 같은 호출로 OpenGL 컨텍스트 생성을 비활성화해야 합니다:

```
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
```

창 크기 조정 처리는 특별한 주의가 필요하며, 이에 대해서는 나중에 다룰 예정입니다. 현재는 다른 창 힌트 호출로 이를 비활성화하세요:

```
glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
```

이제 실제 창을 생성하는 것만 남았습니다. `GLFWwindow* window;`를 private 클래스 멤버로 추가하여 참조를 저장하고, 창을 다음과 같이 초기화합니다:

```
window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
```

첫 번째, 두 번째, 세 번째 매개변수는 창의 너비, 높이 및 제목을 지정합니다. 네 번째 매개변수는 선택적으로 창을 열 모니터를 지정할 수 있으며, 마지막 매개변수는 OpenGL에만 관련이 있습니다.

너비와 높이를 하드코딩된 숫자 대신 상수로 사용하는 것이 좋습니다. 이후 여러 번 이 값을 참조할 것이기 때문입니다. HelloTriangleApplication 클래스 정의 위에 다음 줄을 추가했습니다:

```
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
```

그리고 창 생성 호출을 다음과 같이 교체했습니다.

```
window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
```

이제 `initWindow` 함수는 다음과 같아야 합니다:

```
void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}
```

애플리케이션이 오류가 발생하거나 창이 닫힐 때까지 실행되도록 하려면, `mainLoop` 함수에 이벤트 루프를 추가해야 합니다.

```
void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}
```

이 코드는 꽤 자명할 것입니다. 이 코드는 창이 사용자가 닫을 때까지 X 버튼을 누르는 것과 같은 이벤트를 확인하며 반복합니다. 또한, 나중에 한 프레임을 렌더링하는 함수를 호출할 루프이기도 합니다.

창이 닫히면 리소스를 정리해야 하므로 창을 파괴하고 GLFW 자체를 종료해야 합니다. 이것이 첫 번째 `cleanup` 코드가 될 것입니다:

```
void cleanup() {
    glfwDestroyWindow(window);

    glfwTerminate();
}
```

이제 프로그램을 실행하면 "Vulkan"이라는 제목의 창이 나타나고 창을 닫을 때까지 애플리케이션이 종료되지 않습니다. 이제 Vulkan 애플리케이션의 뼈대를 만들었으므로, 첫 번째 Vulkan 객체를 생성해봅시다!

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/00_base_code.cpp)