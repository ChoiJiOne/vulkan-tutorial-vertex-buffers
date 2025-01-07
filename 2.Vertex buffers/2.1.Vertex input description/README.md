# Vertex input description

## Introduction

다음 몇 개의 챕터에서는, 정점 셰이더에 하드코딩된 정점 데이터를 메모리에 있는 정점 버퍼로 교체할 것입니다. 가장 쉬운 접근 방식인 CPU에서 접근 가능한 버퍼를 생성하고, memcpy를 사용해 정점 데이터를 직접 복사하는 방법부터 시작한 뒤, 이후 고성능 메모리로 정점 데이터를 복사하기 위해 스테이징 버퍼를 사용하는 방법을 살펴보겠습니다.

## Vertex shader

먼저, 정점 셰이더에서 정점 데이터를 더 이상 셰이더 코드 내에 포함하지 않도록 변경하세요. 정점 셰이더는 `in` 키워드를 사용하여 정점 버퍼에서 입력을 받습니다.

```GLSL
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
```

`inPosition`과 `inColor` 변수는 정점 속성(vertex attributes)입니다. 이는 정점 버퍼에서 정점당 지정되는 속성으로, 우리가 이전에 두 개의 배열을 사용하여 정점당 위치와 색상을 수동으로 지정했던 것과 동일합니다. 정점 셰이더를 반드시 다시 컴파일하세요!

`fragColor`와 마찬가지로, `layout(location = x)` 주석은 입력에 인덱스를 할당하여 나중에 이를 참조할 수 있도록 합니다. 여기서 중요한 점은, `dvec3`와 같은 64비트 벡터 타입은 여러 슬롯을 사용한다는 것입니다. 따라서, 그 이후의 인덱스는 최소 2 이상이어야 합니다.

```GLSL
layout(location = 0) in dvec3 inPosition;
layout(location = 2) in vec3 inColor;
```

[OpenGL 위키](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL))에서 layout 한정자에 대한 더 많은 정보를 찾을 수 있습니다.

## Vertex data

우리는 정점 데이터를 셰이더 코드에서 프로그램 코드의 배열로 옮길 것입니다. 먼저 GLM 라이브러리를 포함하세요. 이 라이브러리는 벡터와 행렬 같은 선형 대수 관련 타입을 제공합니다. 우리는 이러한 타입을 사용하여 위치와 색상 벡터를 지정할 것입니다.

```C++
#include <glm/glm.hpp>
```

정점 셰이더에서 사용할 두 가지 속성을 포함한 `Vertex`라는 새 구조체를 생성하세요.

```C++
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};
```

GLM은 셰이더 언어에서 사용하는 벡터 타입과 정확히 일치하는 C++ 타입을 편리하게 제공합니다.

```C++
const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
```

이제 `Vertex` 구조체를 사용하여 정점 데이터 배열을 지정하세요. 이전과 정확히 동일한 위치와 색상 값을 사용하지만, 이제는 하나의 정점 배열에 결합됩니다. 이것은 정점 속성의 인터리빙(interleaving)으로 알려져 있습니다.

## Binding descriptions

다음 단계는 정점 데이터가 GPU 메모리에 업로드된 후, Vulkan이 이를 정점 셰이더에 전달하는 방식을 정의하는 것입니다. 이 정보를 전달하기 위해 두 가지 구조체가 필요합니다.

첫 번째 구조체는 `VkVertexInputBindingDescription`이며, 이를 채우기 위해 Vertex 구조체에 멤버 함수를 추가할 것입니다.

```C++
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        return bindingDescription;
    }
};
```

정점 바인딩(binding)은 메모리에서 정점을 통해 데이터를 어느 속도로 로드할지를 설명합니다. 이는 데이터 항목 간의 바이트 수와 각 정점 또는 각 인스턴스 후 다음 데이터 항목으로 이동할지 여부를 지정합니다.

```C++
VkVertexInputBindingDescription bindingDescription{};
bindingDescription.binding = 0;
bindingDescription.stride = sizeof(Vertex);
bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
```

모든 정점별 데이터가 하나의 배열에 함께 패킹되어 있으므로, 우리는 단 하나의 바인딩만 사용합니다. `binding` 매개변수는 바인딩 배열에서 해당 바인딩의 인덱스를 지정하고, `stride` 매개변수는 각 항목 간 바이트 수를 지정합니다. `inputRate` 매개변수는 다음 중 하나의 값을 가질 수 있습니다:

- `VK_VERTEX_INPUT_RATE_VERTEX`: 각 정점 후에 다음 데이터 항목으로 이동
- `VK_VERTEX_INPUT_RATE_INSTANCE`: 각 인스턴스 후에 다음 데이터 항목으로 이동

인스턴싱 렌더링은 사용하지 않을 것이므로, 정점별 데이터를 유지할 것입니다.

## Attribute descriptions

정점 입력을 처리하는 방식을 설명하는 두 번째 구조체는 `VkVertexInputAttributeDescription`입니다. 이 구조체를 채우기 위해 `Vertex`에 또 다른 도우미 함수를 추가할 것입니다.

```C++
#include <array>

...

static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    return attributeDescriptions;
}
```

함수 프로토타입에서 알 수 있듯이, 이러한 구조체는 두 개가 필요합니다. 속성 설명 구조체(attribute description struct)는 바인딩 설명에서 유래한 정점 데이터 조각에서 정점 속성을 추출하는 방법을 설명합니다. 우리는 두 개의 속성, 위치(position)와 색상(color)을 가지고 있으므로 두 개의 속성 설명 구조체가 필요합니다.

```C++
attributeDescriptions[0].binding = 0;
attributeDescriptions[0].location = 0;
attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
attributeDescriptions[0].offset = offsetof(Vertex, pos);
```

`binding` 매개변수는 정점별 데이터가 어느 바인딩에서 오는지를 Vulkan에 알립니다. `location` 매개변수는 정점 셰이더에서 입력의 `location` 지시문을 참조합니다. 정점 셰이더에서 `location`이 `0`인 입력은 위치이며, 이는 두 개의 32비트 부동소수점(float) 구성 요소를 가집니다.

`format` 매개변수는 속성 데이터의 타입을 설명합니다. 다소 혼란스럽게도, 포맷은 색상 포맷과 동일한 열거형을 사용하여 지정됩니다. 다음은 셰이더 타입과 일반적으로 함께 사용되는 포맷입니다:

- `float`: `VK_FORMAT_R32_SFLOAT`
- `vec2`: `VK_FORMAT_R32G32_SFLOAT`
- `vec3`: `VK_FORMAT_R32G32B32_SFLOAT`
- `vec4`: `VK_FORMAT_R32G32B32A32_SFLOAT`

위에서 볼 수 있듯이, 셰이더 데이터 타입의 구성 요소 수와 일치하는 색상 채널 수를 가진 포맷을 사용해야 합니다. 셰이더 구성 요소 수보다 더 많은 채널을 사용하는 것은 허용되지만, 초과된 채널은 묵시적으로 무시됩니다. 채널 수가 셰이더 구성 요소 수보다 적으면 부족한 BGA 구성 요소는 기본값 (0, 0, 1)을 사용합니다. 색상 타입(SFLOAT, UINT, SINT)과 비트 폭도 셰이더 입력 타입과 일치해야 합니다. 다음 예제를 참조하세요:

- `ivec2`: `VK_FORMAT_R32G32_SINT`, 32비트 부호 있는 정수로 구성된 2-컴포넌트 벡터
- `uvec4`: `VK_FORMAT_R32G32B32A32_UINT`, 32비트 부호 없는 정수로 구성된 4-컴포넌트 벡터
- `double`: `VK_FORMAT_R64_SFLOAT`, 64비트 정밀도(double-precision)의 부동소수점

`format` 매개변수는 속성 데이터의 바이트 크기를 암묵적으로 정의하며, `offset` 매개변수는 정점별 데이터의 시작점으로부터 몇 바이트를 읽을지를 지정합니다. 바인딩은 한 번에 하나의 `Vertex`를 로드하며, 위치 속성(`pos`)은 구조체의 시작점으로부터 0바이트의 오프셋에 위치합니다. 이는 `offsetof` 매크로를 사용하여 자동으로 계산됩니다.

```C++
attributeDescriptions[1].binding = 0;
attributeDescriptions[1].location = 1;
attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[1].offset = offsetof(Vertex, color);
```

색상 속성(color)도 이와 비슷한 방식으로 설명됩니다.

## Pipeline vertex input

이제 `createGraphicsPipeline`에서 해당 구조체를 참조하여 그래픽 파이프라인이 이 형식의 정점 데이터를 수용하도록 설정해야 합니다. `vertexInputInfo` 구조체를 찾아 두 설명 구조체를 참조하도록 수정하세요:

```C++
auto bindingDescription = Vertex::getBindingDescription();
auto attributeDescriptions = Vertex::getAttributeDescriptions();

vertexInputInfo.vertexBindingDescriptionCount = 1;
vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
```

이제 파이프라인은 `vertices` 컨테이너의 형식으로 된 정점 데이터를 수용하고 이를 정점 셰이더에 전달할 준비가 되었습니다. 현재 프로그램을 실행하면, 검증 레이어가 활성화된 상태에서 바인딩에 연결된 정점 버퍼가 없다는 경고를 표시할 것입니다. 다음 단계는 정점 버퍼를 생성하고 정점 데이터를 해당 버퍼로 이동시켜 GPU가 이를 액세스할 수 있도록 하는 것입니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/18_vertex_input.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/18_shader_vertexbuffer.frag)