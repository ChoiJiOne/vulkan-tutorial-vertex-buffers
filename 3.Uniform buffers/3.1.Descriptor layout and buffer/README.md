# Descriptor layout and buffer

## Introduction

이제 각 버텍스에 대해 임의의 속성을 버텍스 셰이더로 전달할 수 있지만, 그렇다면 글로벌 변수는 어떻게 처리할까요? 이 장부터는 3D 그래픽스로 넘어가며 모델-뷰-프로젝션 행렬이 필요합니다. 이를 버텍스 데이터에 포함시킬 수도 있지만, 이는 메모리를 낭비하는 것이며 변환이 변경될 때마다 버텍스 버퍼를 업데이트해야 합니다. 변환은 매 프레임마다 쉽게 변경될 수 있습니다.

Vulkan에서 이를 처리하는 올바른 방법은 리소스 디스크립터를 사용하는 것입니다. 디스크립터는 셰이더가 버퍼나 이미지와 같은 리소스에 자유롭게 접근할 수 있는 방법입니다. 우리는 변환 행렬을 포함하는 버퍼를 설정하고, 버텍스 셰이더가 이를 디스크립터를 통해 접근할 수 있게 할 것입니다. 디스크립터 사용은 세 가지 부분으로 나눠집니다:

- 파이프라인 생성 시 디스크립터 레이아웃 지정
- 디스크립터 풀에서 디스크립터 세트 할당
- 렌더링 중 디스크립터 세트 바인딩

디스크립터 레이아웃은 파이프라인이 접근할 리소스의 유형을 지정합니다. 이는 렌더 패스가 접근할 첨부 파일 유형을 지정하는 것과 비슷합니다. 디스크립터 세트는 실제로 바인딩될 버퍼나 이미지 리소스를 지정합니다. 이는 프레임버퍼가 렌더 패스 첨부 파일에 바인딩될 실제 이미지 뷰를 지정하는 것과 비슷합니다. 그런 다음 디스크립터 세트는 버텍스 버퍼나 프레임버퍼처럼 드로우 명령에서 바인딩됩니다.

디스크립터에는 여러 유형이 있지만, 이번 장에서는 유니폼 버퍼 객체(UBO)와 작업할 것입니다. 다른 유형의 디스크립터는 향후 장에서 다룰 예정이지만, 기본 과정은 동일합니다. 이제 우리는 버텍스 셰이더가 가져야 할 데이터를 C 구조체로 다음과 같이 정의한다고 가정해 보겠습니다:

```C++
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

그런 다음 데이터를 `VkBuffer`에 복사하고 유니폼 버퍼 객체 디스크립터를 통해 버텍스 셰이더에서 이를 접근할 수 있습니다:

```GLSL
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

우리는 모델, 뷰, 프로젝션 행렬을 매 프레임마다 업데이트하여 이전 장에서 만든 사각형이 3D에서 회전하도록 만들 것입니다.

## Vertex shader

위에서 지정한 유니폼 버퍼 객체를 포함하도록 버텍스 셰이더를 수정하십시오. MVP 변환에 대해 이미 알고 있다고 가정하겠습니다. 만약 모르신다면, [첫 번째 장에서 언급된 자료](https://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/)를 참조하십시오.

```GLSL
#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

디스크립터의 `uniform`, `in`, `out` 선언 순서는 중요하지 않다는 점에 유의하십시오. 바인딩 지시어는 속성에 대한 `location` 지시어와 유사합니다. 우리는 이 바인딩을 디스크립터 레이아웃에서 참조할 것입니다. `gl_Position`이 포함된 라인은 변환을 사용하여 최종 위치를 클립 좌표에서 계산하도록 변경됩니다. 2D 삼각형과 달리 클립 좌표의 마지막 구성 요소는 1이 아닐 수 있으며, 이는 최종적으로 화면의 정규화된 장치 좌표로 변환될 때 나누기를 발생시킵니다. 이는 원근 투영에서 원근 분할(perspective division)로 사용되며, 가까운 물체가 멀리 있는 물체보다 더 크게 보이도록 하는 데 필수적입니다.

## Descriptor set layout

다음 단계는 C++ 측에서 UBO를 정의하고 Vulkan에 이 디스크립터를 버텍스 셰이더에 알려주는 것입니다.

```C++
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

우리는 GLM의 데이터 타입을 사용하여 셰이더에서 정의한 것과 정확히 일치하는 정의를 만들 수 있습니다. 행렬의 데이터는 셰이더에서 예상하는 방식과 바이너리 호환되므로, 나중에 `UniformBufferObject`를 `VkBuffer`에 `memcpy`로 복사할 수 있습니다.

우리는 파이프라인 생성을 위해 셰이더에서 사용되는 모든 디스크립터 바인딩에 대한 세부 정보를 제공해야 합니다. 이는 모든 버텍스 속성과 그 위치 인덱스에 대해 했던 것처럼 해야 합니다. 이 모든 정보를 정의하기 위한 새로운 함수인 `createDescriptorSetLayout`을 설정해야 합니다. 이 함수는 파이프라인 생성 직전에 호출되어야 합니다. 왜냐하면 그곳에서 이 정보를 필요로 하기 때문입니다.

```C++
void initVulkan() {
    ...
    createDescriptorSetLayout();
    createGraphicsPipeline();
    ...
}

...

void createDescriptorSetLayout() {

}
```

각 바인딩은 `VkDescriptorSetLayoutBinding` 구조체를 통해 설명되어야 합니다.

```C++
void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
}
```

첫 번째 두 필드는 셰이더에서 사용되는 바인딩과 디스크립터의 유형을 지정합니다. 여기서 디스크립터 유형은 유니폼 버퍼 객체(Uniform Buffer Object, UBO)입니다. 셰이더 변수는 유니폼 버퍼 객체 배열을 나타낼 수 있으며, `descriptorCount`는 배열의 값 수를 지정합니다. 예를 들어, 이 배열은 골격 애니메이션의 각 뼈에 대한 변환을 지정하는 데 사용할 수 있습니다. 우리 MVP 변환은 하나의 유니폼 버퍼 객체에 있기 때문에 `descriptorCount`는 `1`로 설정합니다.

```C++
uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
```

또한 디스크립터가 참조될 셰이더 단계도 지정해야 합니다. `stageFlags` 필드는 `VkShaderStageFlagBits` 값들의 조합이거나 `VK_SHADER_STAGE_ALL_GRAPHICS` 값을 가질 수 있습니다. 우리의 경우, 디스크립터는 버텍스 셰이더에서만 참조되므로 해당 셰이더 단계만 지정합니다.

```C++
uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
```

`pImmutableSamplers` 필드는 이미지 샘플링 관련 디스크립터에서만 의미가 있으며, 이는 나중에 살펴볼 것입니다. 기본값을 그대로 두면 됩니다.

모든 디스크립터 바인딩은 하나의 `VkDescriptorSetLayout` 객체로 결합됩니다. 이 객체는 `pipelineLayout` 위에 새로운 클래스 멤버로 정의합니다.

```C++
VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
```

그런 다음 `vkCreateDescriptorSetLayout`을 사용하여 이를 생성할 수 있습니다. 이 함수는 간단한 `VkDescriptorSetLayoutCreateInfo`를 받아 바인딩 배열을 포함합니다.

```C++
VkDescriptorSetLayoutCreateInfo layoutInfo{};
layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
layoutInfo.bindingCount = 1;
layoutInfo.pBindings = &uboLayoutBinding;

if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
}
```

파이프라인 생성을 통해 디스크립터 세트 레이아웃을 지정해야 Vulkan에 셰이더가 사용할 디스크립터를 알려줄 수 있습니다. 디스크립터 세트 레이아웃은 파이프라인 레이아웃 객체에서 지정됩니다. `VkPipelineLayoutCreateInfo`를 수정하여 레이아웃 객체를 참조하도록 합니다.

```C++
VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipelineLayoutInfo.setLayoutCount = 1;
pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
```

여기서 여러 개의 디스크립터 세트 레이아웃을 지정할 수 있는 이유에 대해 궁금할 수 있습니다. 하나의 디스크립터 세트 레이아웃이 모든 바인딩을 이미 포함하고 있기 때문입니다. 이 부분은 다음 챕터에서 다룰 예정인데, 그때는 디스크립터 풀과 디스크립터 세트에 대해 다룰 것입니다.

디스크립터 레이아웃은 새로운 그래픽 파이프라인을 생성할 때까지, 즉 프로그램이 끝날 때까지 유지되어야 합니다.

```C++
void cleanup() {
    cleanupSwapChain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    ...
}
```

## Uniform buffer

다음 챕터에서는 셰이더를 위한 UBO 데이터를 포함하는 버퍼를 지정할 예정이지만, 이 버퍼를 먼저 생성해야 합니다. 매 프레임마다 새로운 데이터를 유니폼 버퍼에 복사할 예정이므로, 스테이징 버퍼를 두는 것은 의미가 없습니다. 오히려 추가적인 오버헤드를 발생시켜 성능을 저하시킬 수 있습니다.

여러 개의 버퍼가 필요합니다. 여러 프레임이 동시에 진행될 수 있기 때문에 이전 프레임이 여전히 버퍼를 읽고 있는 동안, 다음 프레임을 준비하기 위해 버퍼를 업데이트하는 것은 바람직하지 않습니다! 따라서, 우리가 사용하는 각 프레임에 대해 하나의 유니폼 버퍼가 필요하며, GPU가 현재 읽고 있지 않은 유니폼 버퍼에 쓰는 방식으로 작업해야 합니다.

이를 위해 `uniformBuffers`와 `uniformBuffersMemory`라는 새로운 클래스 멤버를 추가해야 합니다.

```C++
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;
std::vector<void*> uniformBuffersMapped;
```

유사하게, `createUniformBuffers`라는 새로운 함수를 생성하여 `createIndexBuffer` 뒤에 호출하고, 이 함수가 버퍼를 할당하도록 합니다.

```C++
void initVulkan() {
    ...
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    ...
}

...

void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}
```

버퍼를 생성한 직후에 `vkMapMemory`를 사용하여 버퍼를 매핑하고, 이후 데이터를 쓸 수 있는 포인터를 얻습니다. 버퍼는 애플리케이션의 전체 수명 동안 이 포인터에 매핑된 상태로 유지됩니다. 이 기술을 "지속적인 매핑(persistent mapping)"이라고 하며, 모든 Vulkan 구현에서 작동합니다. 매번 버퍼를 매핑하지 않아도 되므로 성능이 향상됩니다. 매핑은 비용이 들기 때문입니다.

유니폼 데이터는 모든 드로우 콜에 사용되므로, 이를 포함하는 버퍼는 렌더링을 멈출 때까지 파괴되지 않아야 합니다.

```C++
void cleanup() {
    ...

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    ...

}
```

## Updating uniform data

새로운 `updateUniformBuffer` 함수를 만들고, 그 함수를 `drawFrame` 함수에서 호출하여 다음 프레임을 제출하기 전에 유니폼 데이터를 갱신하도록 하세요:

```C++
void drawFrame() {
    ...

    updateUniformBuffer(currentFrame);

    ...

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    ...
}

...

void updateUniformBuffer(uint32_t currentImage) {

}
```

이 함수는 매 프레임마다 새로운 변환을 생성하여 기하학이 회전하도록 합니다. 이를 구현하기 위해 두 개의 새로운 헤더를 포함해야 합니다:

```C++
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
```

`glm/gtc/matrix_transform.hpp` 헤더는 `glm::rotate`, `glm::lookAt`, `glm::perspective`와 같은 모델 변환, 뷰 변환, 프로젝션 변환을 생성할 수 있는 함수들을 제공합니다. `GLM_FORCE_RADIANS` 정의는 `glm::rotate`와 같은 함수들이 인수로 라디안을 사용하도록 보장하기 위해 필요합니다.

`chrono` 표준 라이브러리 헤더는 정밀한 시간 측정을 위한 함수들을 제공합니다. 우리는 이 라이브러리를 사용하여 프레임 속도와 관계없이 기하학이 초당 90도 회전하도록 할 것입니다.

```C++
void updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}
```

`updateUniformBuffer` 함수는 렌더링이 시작된 이후의 시간을 초 단위로 계산하는 로직으로 시작됩니다.

이제 유니폼 버퍼 객체에서 모델, 뷰 및 프로젝션 변환을 정의합니다. 모델 회전은 시간 변수를 사용하여 Z축을 중심으로 간단하게 회전합니다:

```C++
UniformBufferObject ubo{};
ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
```

`glm::rotate` 함수는 기존 변환, 회전 각도 및 회전 축을 매개변수로 받습니다. `glm::mat4(1.0f)` 생성자는 항등 행렬을 반환합니다. `time * glm::radians(90.0f)`의 회전 각도를 사용하면 초당 90도 회전하는 목적을 달성할 수 있습니다.

```C++
ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
```

뷰 변환은 기하학을 45도 각도로 위에서 보는 방식으로 설정했습니다. `glm::lookAt` 함수는 눈의 위치, 중심 위치 및 위쪽 축을 매개변수로 받습니다.

```C++
ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
```

프로젝션 변환은 45도의 수직 시야각을 가진 원근 투영을 사용하기로 했습니다. 다른 매개변수로는 종횡비, 근거리 및 원거리 뷰 평면이 있습니다. 창 크기 조정 후 새로운 창의 너비와 높이를 반영하기 위해 현재 스왑 체인 크기를 사용하여 종횡비를 계산하는 것이 중요합니다.

```C++
ubo.proj[1][1] *= -1;
```

GLM은 원래 OpenGL을 위해 설계되었으며, 그곳에서 클립 좌표의 Y 좌표는 반전되어 있습니다. 이를 보상하는 가장 쉬운 방법은 프로젝션 행렬에서 Y 축의 스케일링 계수의 부호를 반전시키는 것입니다. 이를 하지 않으면 이미지가 거꾸로 렌더링됩니다.

이제 모든 변환이 정의되었으므로, 유니폼 버퍼 객체의 데이터를 현재 유니폼 버퍼에 복사할 수 있습니다. 이는 우리가 버텍스 버퍼에서 했던 것과 정확히 동일한 방식으로 이루어지며, 스테이징 버퍼는 사용하지 않습니다. 앞서 언급한 대로, 유니폼 버퍼는 한 번만 매핑하므로, 다시 매핑할 필요 없이 직접 쓸 수 있습니다.

```C++
memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
```

이렇게 UBO를 사용하는 방식은 자주 변하는 값을 셰이더에 전달하는 가장 효율적인 방법은 아닙니다. 작은 데이터 버퍼를 셰이더로 전달하는 더 효율적인 방법은 푸시 상수(push constants)입니다. 푸시 상수에 대해서는 다음 장에서 살펴볼 수 있습니다.

다음 장에서는 실제로 `VkBuffer`를 유니폼 버퍼 디스크립터에 바인딩하여 셰이더가 이 변환 데이터를 접근할 수 있도록 하는 디스크립터 세트(descriptor sets)에 대해 살펴보겠습니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/22_descriptor_layout.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/22_shader_ubo.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/22_shader_ubo.frag)