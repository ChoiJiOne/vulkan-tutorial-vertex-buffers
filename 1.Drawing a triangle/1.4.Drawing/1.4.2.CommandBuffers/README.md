# Command buffers

Vulkan에서 드로잉 작업과 메모리 전송과 같은 명령은 함수 호출을 통해 직접 실행되지 않습니다. 수행하려는 모든 작업을 명령 버퍼 객체에 기록해야 합니다. 이러한 방식의 장점은 Vulkan에 수행할 작업을 알릴 준비가 되었을 때, 모든 명령이 함께 제출되므로 Vulkan이 모든 명령을 한꺼번에 처리하여 더 효율적으로 작업을 수행할 수 있다는 점입니다. 또한, 필요에 따라 명령 기록을 여러 스레드에서 병렬로 수행할 수도 있습니다.

## Command pools

명령 버퍼를 생성하기 전에 명령 풀(command pool)을 만들어야 합니다. 명령 풀은 버퍼를 저장하는 데 사용되는 메모리를 관리하며, 명령 버퍼는 이 명령 풀에서 할당됩니다. `VkCommandPool`을 저장할 새 클래스 멤버를 추가하세요:

```C++
VkCommandPool commandPool;
```

그런 다음, 새로운 함수 `createCommandPool`을 작성하고, 프레임버퍼 생성 후 `initVulkan`에서 호출하세요.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
}

...

void createCommandPool() {

}
```

명령 풀 생성에는 두 가지 매개변수만 필요합니다:

```C++
QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

VkCommandPoolCreateInfo poolInfo{};
poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
```

명령 풀에는 다음 두 가지 플래그를 설정할 수 있습니다:

- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`: 명령 버퍼가 매우 자주 새 명령으로 다시 기록될 경우(메모리 할당 동작이 변경될 수 있음) 이 플래그를 설정합니다.
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`: 명령 버퍼를 개별적으로 다시 기록할 수 있도록 허용합니다. 이 플래그가 없으면 모든 명령 버퍼를 한꺼번에 재설정해야 합니다.

우리는 매 프레임마다 명령 버퍼를 기록해야 하므로, 이를 재설정하고 다시 기록할 수 있어야 합니다. 따라서 `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT` 플래그 비트를 명령 풀에 설정해야 합니다.

명령 버퍼는 그래픽 및 표시 큐와 같은 디바이스 큐 중 하나에 제출되어 실행됩니다. 각 명령 풀은 단일 유형의 큐에 제출되는 명령 버퍼만 할당할 수 있습니다. 우리는 드로잉 작업을 기록할 예정이므로 그래픽 큐 패밀리를 선택합니다.

```C++
if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
}
```

`vkCreateCommandPool` 함수를 사용하여 명령 풀을 생성하세요. 이 함수에는 특별한 매개변수가 필요하지 않습니다. 명령은 프로그램 전반에서 화면에 그래픽을 그리는 데 사용되므로, 명령 풀은 프로그램 종료 시점에만 삭제해야 합니다.

```C++
void cleanup() {
    vkDestroyCommandPool(device, commandPool, nullptr);

    ...
}
```

## Command buffer allocation

이제 명령 버퍼를 할당하기 시작할 수 있습니다.

`VkCommandBuffer` 객체를 클래스 멤버로 생성합니다. 명령 버퍼는 명령 풀이 삭제될 때 자동으로 해제되므로 명시적으로 정리할 필요가 없습니다.

```C++
VkCommandBuffer commandBuffer;
```

이제 명령 풀에서 단일 명령 버퍼를 할당하는 `createCommandBuffer` 함수를 작성하기 시작하겠습니다.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffer();
}

...

void createCommandBuffer() {

}
```

명령 버퍼는 `vkAllocateCommandBuffers` 함수로 할당되며, 이 함수는 명령 풀과 할당할 버퍼 수를 지정하는 `VkCommandBufferAllocateInfo` 구조체를 매개변수로 받습니다:

```C++
VkCommandBufferAllocateInfo allocInfo{};
allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
allocInfo.commandPool = commandPool;
allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
allocInfo.commandBufferCount = 1;

if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
}
```

`level` 매개변수는 할당된 명령 버퍼가 기본 명령 버퍼인지 보조 명령 버퍼인지를 지정합니다.

- `VK_COMMAND_BUFFER_LEVEL_PRIMARY`: 큐에 제출되어 실행될 수 있지만 다른 명령 버퍼에서 호출될 수는 없습니다.
- `VK_COMMAND_BUFFER_LEVEL_SECONDARY`: 직접 제출할 수는 없지만 기본 명령 버퍼에서 호출될 수 있습니다.

여기서는 보조 명령 버퍼 기능을 사용하지 않지만, 기본 명령 버퍼에서 공통 작업을 재사용할 때 유용할 수 있습니다.

단일 명령 버퍼만 할당하므로 `commandBufferCount` 매개변수는 1로 설정합니다.

## Command buffer recording

이제 명령 버퍼에 실행할 명령을 기록하는 `recordCommandBuffer` 함수 작성을 시작하겠습니다. 사용할 `VkCommandBuffer`는 매개변수로 전달되며, 기록할 현재 스왑체인 이미지의 인덱스도 매개변수로 전달됩니다.

```C++
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

}
```

항상 `vkBeginCommandBuffer`를 호출하여 명령 버퍼 기록을 시작하며, 이때 이 특정 명령 버퍼의 사용에 대한 세부 정보를 지정하는 작은 `VkCommandBufferBeginInfo` 구조체를 인수로 전달합니다.

```C++
VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = 0; // Optional
beginInfo.pInheritanceInfo = nullptr; // Optional

if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
}
```

`flags` 매개변수는 명령 버퍼를 어떻게 사용할지 지정하며, 다음 값들이 사용 가능합니다:

- `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`: 명령 버퍼는 한 번 실행된 후 즉시 다시 기록됩니다.
- `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`: 이는 단일 렌더 패스 내에서만 사용되는 보조 명령 버퍼입니다.
- `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`: 명령 버퍼는 실행 대기 중일 때도 다시 제출될 수 있습니다.

현재 우리에게는 이러한 플래그가 모두 적용되지 않습니다.

`pInheritanceInfo` 매개변수는 보조 명령 버퍼에만 해당합니다. 이는 호출된 기본 명령 버퍼에서 어떤 상태를 상속받을지를 지정합니다.

명령 버퍼가 이미 한 번 기록되었으면, `vkBeginCommandBuffer` 호출은 자동으로 이를 리셋합니다. 명령 버퍼에 나중에 명령을 추가하는 것은 불가능합니다.

## Starting a render pass

그리기는 `vkCmdBeginRenderPass`를 사용하여 렌더 패스를 시작하는 것으로 시작됩니다. 렌더 패스는 `VkRenderPassBeginInfo` 구조체의 몇 가지 매개변수를 사용하여 구성됩니다.

```C++
VkRenderPassBeginInfo renderPassInfo{};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
renderPassInfo.renderPass = renderPass;
renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
```

첫 번째 매개변수는 렌더 패스 자체와 바인딩할 첨부 파일입니다. 우리는 각 스왑체인 이미지에 대해 색상 첨부 파일로 지정된 프레임버퍼를 생성했습니다. 따라서 우리는 그리기를 원하는 스왑체인 이미지를 위한 프레임버퍼를 바인딩해야 합니다. 전달된 imageIndex 매개변수를 사용하여 현재 스왑체인 이미지에 맞는 프레임버퍼를 선택할 수 있습니다.

```C++
renderPassInfo.renderArea.offset = {0, 0};
renderPassInfo.renderArea.extent = swapChainExtent;
```

다음 두 매개변수는 렌더 영역의 크기를 정의합니다. 렌더 영역은 셰이더의 로드 및 저장이 발생할 위치를 정의합니다. 이 영역 밖의 픽셀은 정의되지 않은 값을 가질 것입니다. 최상의 성능을 위해 첨부 파일의 크기와 일치해야 합니다.

```C++
VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
renderPassInfo.clearValueCount = 1;
renderPassInfo.pClearValues = &clearColor;
```

마지막 두 매개변수는 `VK_ATTACHMENT_LOAD_OP_CLEAR`에 대해 사용할 클리어 값을 정의합니다. 우리는 색상 첨부 파일에 대한 로드 작업으로 `VK_ATTACHMENT_LOAD_OP_CLEAR`를 사용했습니다. 클리어 색상은 단순히 검정색에 100% 불투명도로 정의했습니다.

```C++
vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
```

이제 렌더 패스를 시작할 수 있습니다. 명령을 기록하는 모든 함수는 `vkCmd` 접두사를 가집니다. 이 함수들은 모두 void를 반환하므로, 기록이 완료될 때까지 오류 처리는 없습니다.

모든 명령의 첫 번째 매개변수는 항상 명령을 기록할 명령 버퍼입니다. 두 번째 매개변수는 방금 제공한 렌더 패스의 세부 사항을 지정합니다. 마지막 매개변수는 렌더 패스 내에서 그리기 명령이 어떻게 제공될지를 제어합니다. 이 값은 두 가지 값 중 하나를 가질 수 있습니다:

- `VK_SUBPASS_CONTENTS_INLINE`: 렌더 패스 명령은 기본 명령 버퍼에 직접 포함되며, 보조 명령 버퍼는 실행되지 않습니다.
- `VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS`: 렌더 패스 명령은 보조 명령 버퍼에서 실행됩니다.

우리는 보조 명령 버퍼를 사용하지 않으므로 첫 번째 옵션을 선택합니다.

## Basic drawing commands

이제 그래픽 파이프라인을 바인딩할 수 있습니다:

```C++
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
```

두 번째 매개변수는 파이프라인 객체가 그래픽 파이프라인인지 컴퓨팅 파이프라인인지를 지정합니다. 이제 우리는 Vulkan에 그래픽 파이프라인에서 실행할 작업과 프래그먼트 셰이더에서 사용할 첨부 파일을 지정했습니다.

[고정 함수 장](https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions#dynamic-state)에서 언급했듯이, 우리는 이 파이프라인에 대한 뷰포트와 시저 상태를 동적으로 지정했습니다. 따라서 그리기 명령을 발행하기 전에 명령 버퍼에서 이를 설정해야 합니다.

```C++
VkViewport viewport{};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = static_cast<float>(swapChainExtent.width);
viewport.height = static_cast<float>(swapChainExtent.height);
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

VkRect2D scissor{};
scissor.offset = {0, 0};
scissor.extent = swapChainExtent;
vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
```

이제 삼각형에 대한 그리기 명령을 발행할 준비가 되었습니다.

```C++
vkCmdDraw(commandBuffer, 3, 1, 0, 0);
```

실제 `vkCmdDraw` 함수는 다소 예상보다 간단하지만, 우리가 미리 지정한 모든 정보 덕분에 매우 단순합니다. 이 함수는 명령 버퍼 외에도 다음과 같은 매개변수를 가집니다:

- `vertexCount`: 우리는 정점 버퍼가 없지만, 기술적으로 여전히 그릴 3개의 정점이 있습니다.
- `instanceCount`: 인스턴스 렌더링에 사용되며, 이를 사용하지 않는 경우 `1`을 사용합니다.
- `firstVertex`: 정점 버퍼에서의 오프셋으로 사용되며, `gl_VertexIndex`의 최소값을 정의합니다.
- `firstInstance`: 인스턴스 렌더링을 위한 오프셋으로 사용되며, `gl_InstanceIndex`의 최소값을 정의합니다.

## Finishing up

이제 렌더 패스를 끝낼 수 있습니다:

```C++
vkCmdEndRenderPass(commandBuffer);
```

그리고 우리는 명령 버퍼의 기록을 마쳤습니다:

```C++
if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
}
```

다음 장에서는 메인 루프에 대한 코드를 작성할 것입니다. 이 코드는 스왑체인에서 이미지를 획득하고, 명령 버퍼를 기록하고 실행한 다음, 완료된 이미지를 스왑체인에 반환하는 작업을 수행합니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/14_command_buffers.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)