# Staging buffer

## Introduction

현재의 정점 버퍼는 제대로 작동하지만, CPU에서 접근할 수 있는 메모리 타입이 그래픽 카드가 읽기에 가장 최적의 메모리 타입은 아닐 수 있습니다. 가장 최적의 메모리는 `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT` 플래그를 가지고 있으며, 보통 전용 그래픽 카드에서는 CPU에서 접근할 수 없습니다. 이번 챕터에서는 두 개의 정점 버퍼를 만들 것입니다. 하나는 CPU에서 접근 가능한 메모리인 스테이징 버퍼를 사용해 정점 배열에서 데이터를 업로드하고, 다른 하나는 장치 로컬 메모리에 있는 최종 정점 버퍼입니다. 그 후, 버퍼 복사 명령을 사용해 스테이징 버퍼에서 실제 정점 버퍼로 데이터를 이동시킬 것입니다.

## Transfer queue

버퍼 복사 명령은 전송 작업을 지원하는 큐 패밀리가 필요하며, 이는 `VK_QUEUE_TRANSFER_BIT`로 표시됩니다. 좋은 점은 `VK_QUEUE_GRAPHICS_BIT` 또는 `VK_QUEUE_COMPUTE_BIT` 기능을 가진 모든 큐 패밀리가 이미 암묵적으로 `VK_QUEUE_TRANSFER_BIT` 작업을 지원한다는 것입니다. 구현에서는 이러한 경우 `queueFlags`에 이를 명시적으로 나열할 필요가 없습니다.

도전 과제를 원하신다면, 전송 작업을 위해 별도의 큐 패밀리를 사용하는 방법도 시도할 수 있습니다. 이를 위해 프로그램을 다음과 같이 수정해야 합니다:

- `QueueFamilyIndices` 및 `findQueueFamilies`를 수정하여 `VK_QUEUE_TRANSFER_BIT` 비트가 있지만 `VK_QUEUE_GRAPHICS_BIT` 비트는 없는 큐 패밀리를 명시적으로 찾습니다.
- `createLogicalDevice`를 수정하여 전송 큐에 대한 핸들을 요청합니다.
- 전송 큐 패밀리에 제출되는 명령 버퍼를 위한 두 번째 명령 풀을 생성합니다.
- 리소스의 `sharingMode`를 `VK_SHARING_MODE_CONCURRENT`로 변경하고, 그래픽 및 전송 큐 패밀리 모두를 지정합니다.
- 이 챕터에서 사용할 `vkCmdCopyBuffer`와 같은 전송 명령을 그래픽 큐 대신 전송 큐에 제출합니다.

조금 번거롭지만, 큐 패밀리 간 리소스 공유 방식에 대해 많은 것을 배울 수 있는 기회가 될 것입니다.

## Abstracting buffer creation

이 장에서는 여러 개의 버퍼를 생성할 예정이므로, 버퍼 생성을 헬퍼 함수로 분리하는 것이 좋습니다. 새로운 함수 `createBuffer`를 생성하고, `createVertexBuffer`에서 코드(매핑 제외)를 그 함수로 옮깁니다.

```C++
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
```

버퍼 크기, 메모리 속성 및 사용 용도를 위한 매개변수를 추가하여 이 함수를 사용해 다양한 종류의 버퍼를 생성할 수 있도록 해야 합니다. 마지막 두 매개변수는 핸들을 저장할 출력 변수입니다.

이제 `createVertexBuffer`에서 버퍼 생성 및 메모리 할당 코드를 제거하고 대신 `createBuffer`를 호출하면 됩니다.

```C++
void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, vertexBufferMemory);
}
```

프로그램을 실행하여 버퍼가 여전히 제대로 작동하는지 확인하세요.

## Using a staging buffer

이제 `createVertexBuffer`를 수정하여 호스트 가시적 버퍼를 임시 버퍼로만 사용하고 실제 정점 버퍼는 장치 로컬 버퍼로 사용합니다.

```C++
void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
}
```

우리는 이제 새로운 `stagingBuffer`와 `stagingBufferMemory`를 사용하여 매핑하고 정점 데이터를 복사합니다. 이 장에서는 두 가지 새로운 버퍼 사용 플래그를 사용할 것입니다:

- `VK_BUFFER_USAGE_TRANSFER_SRC_BIT`: 메모리 전송 작업에서 소스로 사용할 수 있는 버퍼.
- `VK_BUFFER_USAGE_TRANSFER_DST_BIT`: 메모리 전송 작업에서 대상으로 사용할 수 있는 버퍼.

이제 `vertexBuffer`는 장치 로컬 메모리 유형에서 할당됩니다. 이는 일반적으로 `vkMapMemory`를 사용할 수 없음을 의미합니다. 그러나 우리는 `stagingBuffer`에서 `vertexBuffer`로 데이터를 복사할 수 있습니다. 이 작업을 하겠다고 명시해야 하므로 `stagingBuffer`에는 전송 소스 플래그를, `vertexBuffer`에는 전송 대상 플래그와 정점 버퍼 사용 플래그를 지정합니다.

이제 두 버퍼 간에 내용을 복사하는 `copyBuffer` 함수를 작성할 것입니다.

```C++
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

}
```

메모리 전송 작업은 그리기 명령과 마찬가지로 커맨드 버퍼를 사용하여 실행됩니다. 따라서 먼저 임시 커맨드 버퍼를 할당해야 합니다. 이러한 종류의 단기 버퍼를 위한 별도의 커맨드 풀을 만드는 것이 좋습니다. 왜냐하면 구현에서 메모리 할당 최적화를 적용할 수 있기 때문입니다. 이 경우 커맨드 풀 생성 시 `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT` 플래그를 사용해야 합니다.

```C++
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
}
```

그리고 바로 커맨드 버퍼 기록을 시작합니다:

```C++
VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

vkBeginCommandBuffer(commandBuffer, &beginInfo);
```

우리는 커맨드 버퍼를 한 번만 사용할 예정이며, 복사 작업이 완료될 때까지 함수에서 반환하지 않도록 합니다. 드라이버에게 의도를 알리는 것은 좋은 습관이므로 `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT` 플래그를 사용하는 것이 좋습니다.

```C++
VkBufferCopy copyRegion{};
copyRegion.srcOffset = 0; // Optional
copyRegion.dstOffset = 0; // Optional
copyRegion.size = size;
vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
```

버퍼의 내용은 `vkCmdCopyBuffer` 명령을 사용하여 전송됩니다. 이 명령은 소스 버퍼와 대상 버퍼를 인자로 받으며, 복사할 영역들의 배열을 받습니다. 영역은 `VkBufferCopy` 구조체로 정의되며, 소스 버퍼 오프셋, 대상 버퍼 오프셋, 크기로 구성됩니다. `vkMapMemory` 명령과 달리 여기서는 `VK_WHOLE_SIZE`를 지정할 수 없습니다.

```C++
vkEndCommandBuffer(commandBuffer);
```

이 커맨드 버퍼에는 복사 명령만 포함되므로, 복사 명령 후 바로 커맨드 기록을 중지할 수 있습니다. 이제 커맨드 버퍼를 실행하여 전송을 완료합니다:

```C++
VkSubmitInfo submitInfo{};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffer;

vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
vkQueueWaitIdle(graphicsQueue);
```

그리기 명령과 달리 이번에는 대기해야 할 이벤트가 없습니다. 우리는 단지 버퍼 전송을 즉시 실행하고 싶습니다. 전송이 완료될 때까지 기다리는 두 가지 방법이 있습니다. 하나는 펜스를 사용하고 `vkWaitForFences`로 기다리는 방법, 또 다른 하나는 전송 큐가 유휴 상태가 될 때까지 기다리는 `vkQueueWaitIdle`을 사용하는 방법입니다. 펜스를 사용하면 여러 전송 작업을 동시에 예약하고 모두 완료될 때까지 기다릴 수 있기 때문에 한 번에 하나씩 실행하는 것보다 더 최적화할 수 있는 기회를 드라이버에게 제공할 수 있습니다.

```C++
vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
```

전송 작업에 사용된 커맨드 버퍼를 정리하는 것을 잊지 마세요.

이제 `createVertexBuffer` 함수에서 `copyBuffer`를 호출하여 버텍스 데이터를 디바이스 로컬 버퍼로 이동시킬 수 있습니다:

```C++
createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
```

스테이징 버퍼에서 디바이스 버퍼로 데이터를 복사한 후에는 이를 정리해야 합니다:

```C++
    ...

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
```

프로그램을 실행하여 익숙한 삼각형이 다시 보이는지 확인하세요. 지금 당장 개선된 성능을 눈으로 확인할 수는 없지만, 버텍스 데이터는 이제 고성능 메모리에서 로드되고 있습니다. 이는 더 복잡한 기하학적 형태를 렌더링할 때 중요한 차이를 만들 것입니다.

## Conclusion

실제 애플리케이션에서는 각 버퍼마다 `vkAllocateMemory`를 호출하는 것은 권장되지 않습니다. 동시에 할 수 있는 메모리 할당의 최대 수는 `maxMemoryAllocationCount` 물리적 장치 한계에 의해 제한되며, 고성능 하드웨어인 NVIDIA GTX 1080에서도 이 값이 4096에 불과할 수 있습니다. 많은 수의 객체에 대해 메모리를 할당하는 올바른 방법은 사용자 정의 할당기를 만들어, 앞서 본 여러 함수에서의 오프셋 매개변수를 사용하여 단일 할당을 여러 객체에 분할하는 것입니다.

이런 할당기를 직접 구현하거나, GPUOpen 이니셔티브에서 제공하는 [VulkanMemoryAllocator 라이브러리](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)를 사용할 수 있습니다. 하지만 이번 튜토리얼에서는 이러한 한계에 도달하지 않으므로 각 리소스마다 별도로 할당을 사용하는 방식이 괜찮습니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/20_staging_buffer.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.frag)