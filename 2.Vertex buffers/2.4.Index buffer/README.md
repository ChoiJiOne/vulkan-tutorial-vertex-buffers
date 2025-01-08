# Index buffer

## Introduction

실제 애플리케이션에서 렌더링할 3D 메시들은 종종 여러 삼각형에서 정점을 공유하게 됩니다. 이 문제는 간단한 직사각형을 그릴 때도 발생합니다:

![image11](../../Image/image11.png)

직사각형을 그리려면 두 개의 삼각형이 필요하므로, 6개의 정점이 있는 버텍스 버퍼가 필요합니다. 문제는 두 개의 정점 데이터가 중복되어야 한다는 점인데, 이로 인해 50%의 중복이 발생합니다. 이 문제는 더 복잡한 메시로 넘어갈수록 심해집니다. 예를 들어, 정점들이 평균적으로 3개의 삼각형에서 재사용되는 경우가 많습니다. 이 문제를 해결하는 방법은 바로 인덱스 버퍼를 사용하는 것입니다.

인덱스 버퍼는 본질적으로 버텍스 버퍼에 대한 포인터 배열입니다. 인덱스 버퍼를 사용하면 버텍스 데이터를 재정렬하고, 동일한 데이터를 여러 정점에서 재사용할 수 있습니다. 위의 그림은 각기 다른 네 개의 고유한 정점을 포함한 버텍스 버퍼가 있을 때 직사각형에 대한 인덱스 버퍼가 어떻게 보일지 보여줍니다. 첫 번째 세 개의 인덱스는 오른쪽 위 삼각형을 정의하고, 마지막 세 개의 인덱스는 왼쪽 아래 삼각형을 정의합니다.

## Index buffer creation

이번 장에서는 버텍스 데이터를 수정하고 인덱스 데이터를 추가하여 그림과 같은 사각형을 그릴 것입니다. 버텍스 데이터를 수정하여 4개의 모서리를 나타냅니다:

```C++
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
```

왼쪽 상단 모서리는 빨간색, 오른쪽 상단 모서리는 초록색, 오른쪽 하단 모서리는 파란색, 왼쪽 하단 모서리는 흰색입니다. 그런 다음, 인덱스 버퍼의 내용을 나타내는 새로운 배열 `indices`를 추가합니다. 이 배열은 상단 오른쪽 삼각형과 하단 왼쪽 삼각형을 그리기 위해 그림의 인덱스와 일치해야 합니다.

```C++
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};
```

인덱스 버퍼에는 `uint16_t` 또는 `uint32_t`를 사용할 수 있으며, 이는 `vertices`의 항목 수에 따라 달라집니다. 현재는 고유한 정점이 65535개 미만이므로 `uint16_t`를 사용할 수 있습니다.

버텍스 데이터처럼, 인덱스도 GPU가 접근할 수 있도록 `VkBuffer`에 업로드해야 합니다. 이를 위해 인덱스 버퍼의 자원을 보유할 새로운 클래스 멤버를 정의합니다.

```C++
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
```

이제 추가할 `createIndexBuffer` 함수는 `createVertexBuffer`와 거의 동일합니다.

```C++
void initVulkan() {
    ...
    createVertexBuffer();
    createIndexBuffer();
    ...
}

void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}
```

두 가지 중요한 차이점만 있습니다. `bufferSize`는 이제 인덱스 수에 인덱스 타입의 크기(uint16_t 또는 uint32_t)를 곱한 값입니다. 인덱스 버퍼의 사용 용도는 `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT` 대신 `VK_BUFFER_USAGE_INDEX_BUFFER_BIT`로 설정되어야 합니다. 그 외에는 과정이 동일합니다. 우리는 `indices`의 내용을 복사하기 위한 스테이징 버퍼를 생성한 후, 이를 최종 디바이스 로컬 인덱스 버퍼로 복사합니다.

인덱스 버퍼도 프로그램 끝에서 버텍스 버퍼처럼 정리해야 합니다.

```C++
void cleanup() {
    cleanupSwapChain();

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    ...
}
```

## Using an index buffer

인덱스 버퍼를 사용하여 그리기 작업을 할 때는 `recordCommandBuffer`에 두 가지 변경이 필요합니다. 먼저 인덱스 버퍼를 바인딩해야 합니다. 이는 버텍스 버퍼에서 했던 것과 같습니다. 차이점은 인덱스 버퍼는 하나만 사용할 수 있다는 것입니다. 불행히도 각 버텍스 속성에 대해 다른 인덱스를 사용할 수 없기 때문에, 속성 하나만 달라져도 여전히 버텍스 데이터를 완전히 중복해서 사용해야 합니다.

```C++
vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
```

인덱스 버퍼는 `vkCmdBindIndexBuffer`로 바인딩합니다. 이 함수는 인덱스 버퍼, 그 안의 바이트 오프셋, 그리고 인덱스 데이터 타입을 매개변수로 받습니다. 앞서 언급했듯이 가능한 타입은 `VK_INDEX_TYPE_UINT16`과 `VK_INDEX_TYPE_UINT32`입니다.

인덱스 버퍼를 바인딩한다고 해서 아무것도 바뀌지 않으며, 그리기 명령도 인덱스 버퍼를 사용하라고 Vulkan에 알려줘야 합니다. `vkCmdDraw`를 제거하고 대신 `vkCmdDrawIndexed`로 바꿔야 합니다.

```C++
vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
```

이 함수 호출은 `vkCmdDraw`와 매우 유사합니다. 첫 번째 두 매개변수는 인덱스 수와 인스턴스 수를 지정합니다. 우리는 인스턴싱을 사용하지 않으므로 인스턴스를 `1`로 지정합니다. 인덱스 수는 버텍스 셰이더에 전달될 버텍스의 수를 나타냅니다. 다음 매개변수는 인덱스 버퍼에서의 오프셋을 지정합니다. 값으로 `1`을 사용하면 그래픽 카드가 두 번째 인덱스에서 읽기 시작하게 됩니다. 두 번째로 마지막 매개변수는 인덱스 버퍼에서 인덱스에 추가할 오프셋을 지정합니다. 마지막 매개변수는 인스턴싱에 대한 오프셋을 지정하는데, 우리는 이를 사용하지 않습니다.

이제 프로그램을 실행하면 다음과 같은 결과를 볼 수 있습니다:

![image12](../../Image/image12.png)


이제 인덱스 버퍼를 사용하여 버텍스를 재사용함으로써 메모리를 절약하는 방법을 알게 되었습니다. 이는 향후 장에서 복잡한 3D 모델을 로드할 때 특히 중요해질 것입니다.

이전 장에서는 여러 리소스, 예를 들어 버퍼를 단일 메모리 할당에서 할당해야 한다고 언급했지만, 사실 한 걸음 더 나아가야 합니다. [드라이버 개발자들은](https://developer.nvidia.com/vulkan-memory-management) 버텍스 버퍼와 인덱스 버퍼와 같은 여러 버퍼를 단일 `VkBuffer`에 저장하고 `vkCmdBindVertexBuffers`와 같은 명령에서 오프셋을 사용하는 것을 권장합니다. 이점은 데이터가 더 가까이 위치하여 캐시 친화적이기 때문입니다. 또한 동일한 렌더링 작업에서 사용되지 않는 경우, 여러 리소스에 대해 동일한 메모리 덩어리를 재사용할 수도 있습니다. 물론 그들의 데이터가 새로 고쳐진다는 조건하에 말이죠. 이것은 '별칭화(aliasing)'로 알려져 있으며, 일부 Vulkan 함수에는 이를 명시적으로 지정할 수 있는 플래그가 있습니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/21_index_buffer.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.frag)