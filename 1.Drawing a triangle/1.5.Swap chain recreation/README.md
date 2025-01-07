# Swap chain recreation

## Introduction

현재 애플리케이션은 삼각형을 성공적으로 그리지만, 아직 제대로 처리하지 못하는 상황들이 있습니다. 예를 들어, 윈도우 표면이 변경되어 스왑 체인이 더 이상 호환되지 않을 수 있습니다. 이는 윈도우 크기가 변경되는 경우 등에서 발생할 수 있습니다. 이러한 이벤트를 감지하고 스왑 체인을 다시 생성해야 합니다.

## Recreating the swap chain

새로운 `recreateSwapChain` 함수를 만들어야 합니다. 이 함수는 `createSwapChain`을 호출하고, 스왑 체인이나 윈도우 크기에 의존하는 객체들을 생성하는 모든 함수를 호출합니다.

```C++
void recreateSwapChain() {
    vkDeviceWaitIdle(device);

    createSwapChain();
    createImageViews();
    createFramebuffers();
}
```

먼저 `vkDeviceWaitIdle`을 호출합니다. 이는 이전 챕터에서와 마찬가지로 여전히 사용 중일 수 있는 리소스를 건드리지 않기 위함입니다. 물론, 스왑 체인 자체를 다시 생성해야 합니다. 이미지 뷰는 스왑 체인 이미지에 직접적으로 기반을 두고 있으므로 재생성이 필요합니다. 마지막으로, 프레임버퍼는 스왑 체인 이미지에 직접적으로 의존하기 때문에 역시 다시 생성해야 합니다.

기존 객체의 정리를 보장하기 위해, 일부 정리 코드를 별도의 함수로 옮겨야 합니다. 이 함수는 `recreateSwapChain` 함수에서 호출할 수 있어야 합니다. 이 함수의 이름을 `cleanupSwapChain`으로 정합시다.

```C++
void cleanupSwapChain() {

}

void recreateSwapChain() {
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();
}
```

여기서는 간소화를 위해 렌더 패스를 재생성하지 않는다는 점에 유의하세요. 이론적으로, 애플리케이션 실행 중에 스왑 체인의 이미지 포맷이 변경될 수 있습니다. 예를 들어, 윈도우를 표준 범위 모니터에서 HDR(High Dynamic Range) 모니터로 이동하는 경우와 같은 상황입니다. 이러한 경우, 동적 범위의 변경이 제대로 반영되도록 렌더 패스를 다시 생성해야 할 수도 있습니다.

스왑 체인 새로 고침의 일부로 다시 생성되는 모든 객체의 정리 코드를 `cleanup` 함수에서 `cleanupSwapChain` 함수로 이동합시다.

```C++
void cleanupSwapChain() {
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void cleanup() {
    cleanupSwapChain();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
```

`chooseSwapExtent` 함수에서는 이미 새로운 윈도우 해상도를 쿼리하여 스왑 체인 이미지가 올바른 크기를 가지도록 하고 있으므로, 이 함수는 수정할 필요가 없습니다. (스왑 체인을 생성할 때 이미 `glfwGetFramebufferSize`를 사용하여 표면의 픽셀 단위 해상도를 가져와야 했던 것을 기억하세요.)

이로써 스왑 체인을 재생성하는 데 필요한 모든 작업이 완료되었습니다! 그러나 이 접근 방식에는 단점이 있습니다. 새로운 스왑 체인을 생성하기 전에 모든 렌더링을 중단해야 한다는 점입니다.

스왑 체인의 오래된 이미지를 사용하는 드로잉 커맨드가 여전히 진행 중인 상태에서도 새로운 스왑 체인을 생성할 수 있습니다. 이를 위해 `VkSwapchainCreateInfoKHR` 구조체의 `oldSwapchain` 필드에 이전 스왑 체인을 전달하고, 이전 스왑 체인을 사용 완료된 직후에 파괴하면 됩니다.

## Suboptimal or out-of-date swap chain

이제 스왑 체인 재생성이 필요한 시점을 파악하고, 새로 작성한 `recreateSwapChain` 함수를 호출하면 됩니다. 다행히도 Vulkan은 대개 프레젠테이션 중에 스왑 체인이 더 이상 적합하지 않음을 알려줍니다. `vkAcquireNextImageKHR`와 `vkQueuePresentKHR` 함수는 아래와 같은 특별한 값을 반환하여 이를 나타낼 수 있습니다:

- `VK_ERROR_OUT_OF_DATE_KHR`: 스왑 체인이 표면과 호환되지 않게 되어 렌더링에 더 이상 사용할 수 없음을 나타냅니다. 보통 창 크기 조정 후 발생합니다.
- `VK_SUBOPTIMAL_KHR`: 스왑 체인을 사용하여 표면에 성공적으로 프레젠테이션을 할 수는 있지만, 표면 속성이 더 이상 정확히 일치하지 않음을 나타냅니다.

```C++
VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
}
```

스왑 체인을 통해 이미지를 얻으려 할 때 스왑 체인이 만료(out of date)된 경우, 해당 스왑 체인에 프레젠테이션을 할 수 없습니다. 따라서 즉시 스왑 체인을 재생성하고, 다음 `drawFrame` 호출에서 다시 시도해야 합니다.

스왑 체인이 `suboptimal` 상태인 경우에도 동일하게 처리할 수 있지만, 이미 이미지를 획득한 경우에는 그대로 진행하는 방식을 선택할 수 있습니다. 참고로 `VK_SUCCESS`와 `VK_SUBOPTIMAL_KHR`는 모두 "성공" 코드로 간주됩니다.

```C++
result = vkQueuePresentKHR(presentQueue, &presentInfo);

if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreateSwapChain();
} else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
}

currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

`vkQueuePresentKHR` 함수도 동일한 값과 의미를 반환합니다. 이 경우 최상의 결과를 위해 스왑 체인이 suboptimal 상태인 경우에도 스왑 체인을 재생성하도록 설정할 수 있습니다.

## Fixing a deadlock

코드를 지금 실행하면 데드락이 발생할 수 있습니다. 코드를 디버깅해보면 애플리케이션이 `vkWaitForFences`에 도달한 후 계속 진행되지 않는 것을 알 수 있습니다. 이는 `vkAcquireNextImageKHR`가 `VK_ERROR_OUT_OF_DATE_KHR`을 반환하면 스왑 체인을 재생성하고 `drawFrame`에서 반환하지만, 그 전에 현재 프레임의 펜스가 기다려지고 리셋되었기 때문입니다. 반환하기 전에 작업이 제출되지 않아서 펜스가 시그널되지 않으며, `vkWaitForFences`가 영원히 멈추게 됩니다.

다행히도 간단한 수정이 가능합니다. 펜스를 리셋하는 것을 우리가 확실히 작업을 제출할 것이라고 알게 된 후로 미룹니다. 따라서 일찍 반환하더라도 펜스는 여전히 시그널되고 `vkWaitForFences`가 같은 펜스 객체를 사용할 때 데드락이 발생하지 않도록 합니다.

`drawFrame`의 시작 부분은 이제 다음과 같아야 합니다:

```C++
vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

uint32_t imageIndex;
VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
}

// Only reset the fence if we are submitting work
vkResetFences(device, 1, &inFlightFences[currentFrame]);
```

## Handling resizes explicitly

많은 드라이버와 플랫폼은 창 크기 조정 후 자동으로 `VK_ERROR_OUT_OF_DATE_KHR`를 트리거하지만, 이는 보장되지 않습니다. 그래서 우리는 크기 조정을 명시적으로 처리하는 추가 코드를 추가할 것입니다. 먼저 크기 조정이 발생했음을 나타내는 새로운 멤버 변수를 추가합니다:

```C++
std::vector<VkFence> inFlightFences;

bool framebufferResized = false;
```

그 다음 `drawFrame` 함수는 이 플래그도 확인하도록 수정되어야 합니다:

```C++
if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
    framebufferResized = false;
    recreateSwapChain();
} else if (result != VK_SUCCESS) {
    ...
}
```

이것을 `vkQueuePresentKHR` 이후에 하는 것이 중요합니다. 그래야 세마포어가 일관된 상태로 유지되며, 그렇지 않으면 시그널된 세마포어가 제대로 기다려지지 않을 수 있습니다. 이제 실제로 크기 조정을 감지하려면 GLFW 프레임워크의 `glfwSetFramebufferSizeCallback` 함수를 사용하여 콜백을 설정할 수 있습니다:

```C++
void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {

}
```

우리가 콜백으로 정적 함수를 만드는 이유는 GLFW가 올바른 this 포인터를 사용하여 멤버 함수를 제대로 호출하는 방법을 알지 못하기 때문입니다.

하지만 콜백에서 `GLFWwindow`에 대한 참조는 얻을 수 있고, 이를 통해 임의의 포인터를 저장할 수 있는 다른 GLFW 함수인 `glfwSetWindowUserPointer`를 사용할 수 있습니다:

```C++
window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
glfwSetWindowUserPointer(window, this);
glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
```

이 값은 이제 콜백 내에서 `glfwGetWindowUserPointer`를 통해 가져와 플래그를 올바르게 설정할 수 있습니다.

```C++
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}
```

이제 프로그램을 실행하고 창 크기를 조정하여 프레임버프가 창과 함께 제대로 크기 조정되는지 확인해 보세요.

## Handling minimization

스왑 체인이 만료될 수 있는 또 다른 경우는 특별한 유형의 창 크기 조정인 창 최소화입니다. 이 경우는 특별한데, 그 이유는 프레임버프 크기가 0이 되기 때문입니다. 이 경우는 `recreateSwapChain` 함수를 확장하여 창이 다시 전면에 올 때까지 일시 정지하는 방식으로 처리할 것입니다:

```C++
void recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    ...
}
```

`glfwGetFramebufferSize`의 초기 호출은 크기가 이미 올바른 경우를 처리하며, 이 경우 `glfwWaitEvents`는 기다릴 항목이 없습니다.

축하합니다, 이제 당신은 첫 번째 잘 동작하는 Vulkan 프로그램을 완성했습니다! 다음 장에서는 버텍스 셰이더에 하드코딩된 버텍스를 제거하고 실제로 버텍스 버퍼를 사용하는 방법을 다룰 것입니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/17_swap_chain_recreation.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)