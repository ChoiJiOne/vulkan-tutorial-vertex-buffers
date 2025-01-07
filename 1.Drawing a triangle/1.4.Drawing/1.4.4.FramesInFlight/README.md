# Frames in flight

현재 우리의 렌더 루프에는 한 가지 명백한 결함이 있습니다. 다음 프레임을 렌더링하기 전에 이전 프레임이 완료되기를 기다려야 하며, 이는 호스트의 불필요한 대기를 초래합니다.

이 문제를 해결하려면 여러 프레임을 동시에 처리할 수 있게 해야 합니다. 즉, 한 프레임을 렌더링하는 동안 다음 프레임을 기록하는 작업이 서로 간섭하지 않도록 해야 합니다. 이를 어떻게 구현할 수 있을까요? 렌더링 중에 액세스되고 수정되는 모든 리소스를 복제해야 합니다. 따라서 여러 개의 커맨드 버퍼, 세마포어, 그리고 펜스가 필요합니다. 이후 챕터에서는 다른 리소스의 여러 인스턴스를 추가하며 이 개념이 다시 등장할 것입니다.

프로그램 상단에 처리할 동시 프레임 수를 정의하는 상수를 추가하는 것으로 시작하세요:

```C++
const int MAX_FRAMES_IN_FLIGHT = 2;
```

우리가 프레임 동시 처리 개수를 2로 선택한 이유는 CPU가 GPU보다 너무 앞서가지 않도록 하기 위함입니다. 2개의 프레임을 동시에 처리할 경우, CPU와 GPU는 각자 자신의 작업을 동시에 진행할 수 있습니다. 만약 CPU가 먼저 작업을 끝내면, GPU가 렌더링을 완료할 때까지 기다렸다가 추가 작업을 제출합니다. 반면, 3개 이상의 프레임을 동시에 처리하면 CPU가 GPU보다 앞서게 되어 프레임 지연(latency)이 추가될 수 있습니다. 일반적으로 이러한 추가 지연은 바람직하지 않습니다. 그러나 처리할 프레임 수를 애플리케이션에서 제어할 수 있도록 하는 것도 Vulkan의 명시적인 특성을 보여주는 또 다른 예입니다.

각 프레임은 자체 커맨드 버퍼, 세마포어 세트, 그리고 펜스를 가져야 합니다. 이를 반영하여 관련 객체를 `std::vector`로 변경하고 이름을 수정하세요:

```C++
std::vector<VkCommandBuffer> commandBuffers;

...

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
```

다음으로, 여러 개의 커맨드 버퍼를 생성해야 합니다. `createCommandBuffer`의 이름을 `createCommandBuffers`로 변경하세요. 그런 다음, 커맨드 버퍼 벡터의 크기를 MAX_FRAMES_IN_FLIGHT 값에 맞게 조정하고, `VkCommandBufferAllocateInfo`를 수정하여 해당 개수만큼의 커맨드 버퍼를 포함하도록 변경해야 합니다. 또한, 생성된 커맨드 버퍼의 저장 위치를 커맨드 버퍼 벡터로 변경하세요.

```C++
void createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    ...
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
```

`createSyncObjects` 함수는 모든 동기화 객체를 생성하도록 수정되어야 합니다.

```C++
void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}
```

마찬가지로, 모든 동기화 객체를 정리(clean-up)하도록 변경해야 합니다.

```C++
void cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    ...
}
```

기억하세요, 커맨드 버퍼는 커맨드 풀을 해제할 때 자동으로 해제되므로 커맨드 버퍼 정리에 대해 추가로 작업할 필요가 없습니다.

매 프레임에서 올바른 객체를 사용하려면 현재 프레임을 추적해야 합니다. 이를 위해 프레임 인덱스를 사용할 것입니다:

```C++
uint32_t currentFrame = 0;
```

`drawFrame` 함수는 이제 올바른 객체를 사용하도록 수정할 수 있습니다.

```C++
void drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    ...

    vkResetCommandBuffer(commandBuffers[currentFrame],  0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    ...

    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    ...

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};

    ...

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    ...

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
}
```

물론, 매번 다음 프레임으로 넘어가는 것도 잊지 말아야 합니다:

```C++
void drawFrame() {
    ...

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
```

모듈로 연산자(%)를 사용함으로써, 프레임 인덱스가 `MAX_FRAMES_IN_FLIGHT` 프레임이 큐에 쌓인 후 다시 처음으로 순환되도록 보장합니다.

이제 필요한 모든 동기화를 구현하여, `MAX_FRAMES_IN_FLIGHT` 이상의 작업이 큐에 쌓이지 않으며 각 프레임이 서로 간섭하지 않도록 했습니다. 하지만 코드의 다른 부분, 예를 들어 최종 정리 단계와 같은 경우에는 `vkDeviceWaitIdle`과 같은 비교적 단순한 동기화에 의존해도 괜찮습니다. 어떤 접근 방식을 사용할지는 성능 요구사항에 따라 결정해야 합니다.

동기화에 대한 예제를 통해 더 배우고 싶다면 [Khronos의 광범위한 개요서](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present)를 참조하세요.

다음 챕터에서는 Vulkan 프로그램이 제대로 작동하도록 하는 데 필요한 또 다른 작은 부분을 다룰 것입니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/16_frames_in_flight.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)