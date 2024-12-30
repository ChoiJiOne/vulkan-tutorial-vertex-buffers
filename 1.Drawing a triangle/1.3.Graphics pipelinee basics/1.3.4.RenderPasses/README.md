# Render passes 

파이프라인을 완성하기 전에 렌더링 중에 사용할 프레임버퍼 첨부물에 대해 Vulkan에 알려야 합니다. 몇 개의 색상 버퍼와 깊이 버퍼가 있을지, 각각에 사용할 샘플의 개수, 그리고 렌더링 작업 동안 해당 버퍼의 내용을 어떻게 처리할지를 지정해야 합니다. 이 모든 정보는 렌더 패스 객체에 포함되며, 이를 위해 새로운 `createRenderPass` 함수를 생성할 것입니다. 이 함수는 `createGraphicsPipeline` 함수보다 먼저 `initVulkan`에서 호출하도록 합니다.

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
}

...

void createRenderPass() {

}
```

## Attachment description

우리의 경우, 단일 색상 버퍼 첨부물이 스왑체인의 이미지 중 하나로 표현됩니다.

```C++
void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
}
```

색상 첨부물의 포맷은 스왑체인 이미지의 포맷과 일치해야 하며, 멀티샘플링을 사용하지 않을 것이므로 샘플 수는 1로 설정합니다.

```C++
colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
```

`loadOp`와 `storeOp`는 렌더링 전에 첨부물의 데이터를 어떻게 처리할지와 렌더링 후에 어떻게 처리할지를 결정합니다. `loadOp`에 대해 선택할 수 있는 옵션은 다음과 같습니다:

- `VK_ATTACHMENT_LOAD_OP_LOAD`: 첨부물의 기존 내용을 유지합니다.
- `VK_ATTACHMENT_LOAD_OP_CLEAR`: 렌더링 시작 시 값을 일정 상수로 초기화합니다.
- `VK_ATTACHMENT_LOAD_OP_DONT_CARE`: 기존 내용은 정의되지 않으며, 신경 쓰지 않습니다.

우리의 경우, 새로운 프레임을 그리기 전에 프레임버퍼를 검은색으로 초기화하기 위해 Clear 작업을 사용할 것입니다.

`storeOp`의 경우 선택할 수 있는 옵션은 다음 두 가지뿐입니다:

- `VK_ATTACHMENT_STORE_OP_STORE`: 렌더링된 내용이 메모리에 저장되며, 나중에 읽을 수 있습니다.
- `VK_ATTACHMENT_STORE_OP_DONT_CARE`: 렌더링 작업 후 프레임버퍼의 내용은 정의되지 않습니다.

우리는 화면에 렌더링된 삼각형을 확인하려고 하므로, 여기서는 Store 작업을 선택할 것입니다.

```C++
colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
```

`loadOp`와 `storeOp`는 색상 및 깊이 데이터에 적용되며, `stencilLoadOp`와 `stencilStoreOp`는 스텐실 데이터에 적용됩니다. 우리의 애플리케이션은 스텐실 버퍼를 사용하지 않을 것이므로, 스텐실 데이터의 로드 및 저장 결과는 무의미합니다.

```C++
colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
```

Vulkan에서 텍스처와 프레임버퍼는 특정 픽셀 포맷을 가진 `VkImage` 객체로 표현됩니다. 하지만 이미지에 대해 수행하려는 작업에 따라 메모리에서 픽셀이 배치되는 레이아웃은 변경될 수 있습니다.

가장 일반적으로 사용되는 레이아웃은 다음과 같습니다:

- `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`: 색상 첨부물로 사용되는 이미지.
- `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`: 스왑체인에서 화면에 표시될 이미지.
- `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`: 메모리 복사 작업의 대상으로 사용될 이미지.

이 주제는 텍스처링 챕터에서 더 깊이 다룰 예정이지만, 지금 중요한 점은 이미지가 다음에 수행할 작업에 적합한 특정 레이아웃으로 전환되어야 한다는 것입니다.

`initialLayout는` 렌더 패스가 시작되기 전에 이미지가 가질 레이아웃을 지정합니다. `finalLayout`은 렌더 패스가 종료될 때 이미지가 자동으로 전환될 레이아웃을 지정합니다.`initialLayout`에 `VK_IMAGE_LAYOUT_UNDEFINED`를 사용하면 이미지가 이전에 어떤 레이아웃에 있었는지 신경 쓰지 않겠다는 의미입니다. 하지만 이 특별한 값을 사용할 경우 이미지의 내용이 보존되지 않을 수 있다는 단점이 있습니다. 그러나 우리는 이미지를 초기화할 예정이므로 이는 문제가 되지 않습니다. 렌더링 후 이미지를 스왑체인을 사용해 화면에 표시할 준비를 해야 하므로, `finalLayout`으로 `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`를 사용합니다.

## Subpasses and attachment references

단일 렌더 패스는 여러 서브패스로 구성될 수 있습니다. 서브패스는 이전 패스의 프레임버퍼 내용에 의존하는 연속적인 렌더링 작업을 의미합니다. 예를 들어, 연속적으로 적용되는 후처리 효과의 시퀀스가 이에 해당합니다. 이러한 렌더링 작업을 하나의 렌더 패스로 그룹화하면, Vulkan은 작업을 재정렬하고 메모리 대역폭을 절약하여 성능을 향상시킬 가능성이 있습니다. 그러나 첫 번째 삼각형을 렌더링할 때는 단일 서브패스만 사용할 것입니다.

각 서브패스는 이전 섹션에서 설명한 첨부물 중 하나 이상을 참조합니다. 이러한 참조는 `VkAttachmentReference` 구조체로 표현되며, 그 구조는 다음과 같습니다:

```C++
VkAttachmentReference colorAttachmentRef{};
colorAttachmentRef.attachment = 0;
colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
```

`attachment` 매개변수는 첨부물을 참조할 때 그 인덱스를 첨부물 설명 배열에서 지정합니다. 우리의 배열은 단일 `VkAttachmentDescription`으로 구성되므로, 인덱스는 0입니다. layout은 서브패스에서 이 참조를 사용할 때 첨부물이 가질 레이아웃을 지정합니다. Vulkan은 서브패스가 시작될 때 이 레이아웃으로 첨부물을 자동으로 전환합니다. 우리는 이 첨부물을 색상 버퍼로 사용할 계획이며, `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` 레이아웃은 성능 측면에서 가장 최적화된 레이아웃이므로 이를 사용합니다.

서브패스는 `VkSubpassDescription` 구조체를 사용하여 설명됩니다:

```C++
VkSubpassDescription subpass{};
subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
```

Vulkan은 미래에 컴퓨트 서브패스를 지원할 수 있으므로, 우리가 사용하는 서브패스가 그래픽스 서브패스임을 명확히 해야 합니다. 그 다음으로, 색상 첨부물에 대한 참조를 지정합니다:

```C++
subpass.colorAttachmentCount = 1;
subpass.pColorAttachments = &colorAttachmentRef;
```

이 배열에서 첨부물의 인덱스는 프래그먼트 셰이더에서 `layout(location = 0) out vec4 outColor` 지시어를 사용하여 직접 참조됩니다!

서브패스는 다음과 같은 다른 종류의 첨부물을 참조할 수 있습니다:

- `pInputAttachments`: 셰이더에서 읽는 첨부물
- `pResolveAttachments`: 멀티샘플링 색상 첨부물에 사용되는 첨부물
- `pDepthStencilAttachment`: 깊이 및 스텐실 데이터에 사용되는 첨부물
- `pPreserveAttachments`: 이 서브패스에서 사용되지 않지만 데이터가 보존되어야 하는 첨부물

## Render pass

첨부물과 이를 참조하는 기본 서브패스를 설명했으므로, 이제 렌더 패스를 생성할 수 있습니다. `VkRenderPass` 객체를 저장할 새 클래스 멤버 변수를 `pipelineLayout` 변수 바로 위에 추가합니다:

```C++
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
```

렌더 패스 객체는 `VkRenderPassCreateInfo` 구조체를 채우고 첨부물과 서브패스 배열을 포함하여 생성할 수 있습니다. `VkAttachmentReference` 객체는 이 배열의 인덱스를 사용하여 첨부물을 참조합니다.

```C++
VkRenderPassCreateInfo renderPassInfo{};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
renderPassInfo.attachmentCount = 1;
renderPassInfo.pAttachments = &colorAttachment;
renderPassInfo.subpassCount = 1;
renderPassInfo.pSubpasses = &subpass;

if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
}
```

파이프라인 레이아웃처럼, 렌더 패스는 프로그램 전체에서 참조되므로 마지막에만 정리되어야 합니다.

```C++
void cleanup() {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    ...
}
```

많은 작업이었지만, 다음 챕터에서는 모든 것이 합쳐져 그래픽스 파이프라인 객체를 최종적으로 생성할 수 있습니다!

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/11_render_passes.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)
