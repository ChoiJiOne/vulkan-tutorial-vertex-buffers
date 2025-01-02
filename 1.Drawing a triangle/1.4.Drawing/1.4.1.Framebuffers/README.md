# Framebuffers

지금까지의 챕터에서 프레임버퍼에 대해 많이 다뤘고, 렌더 패스를 설정하여 스왑체인 이미지와 동일한 형식의 단일 프레임버퍼를 기대하도록 구성했습니다. 하지만 실제로 프레임버퍼를 생성한 적은 없습니다.

렌더 패스를 생성할 때 지정한 첨부물은 이를 `VkFramebuffer` 객체에 래핑하여 바인딩됩니다. 프레임버퍼 객체는 첨부물을 나타내는 모든 `VkImageView` 객체를 참조합니다. 우리의 경우, 첨부물은 단 하나, 즉 색상 첨부물(color attachment)뿐입니다. 그러나 첨부물로 사용해야 하는 이미지는 표시를 위해 스왑체인에서 반환된 이미지에 따라 달라집니다. 이는 스왑체인의 모든 이미지에 대해 프레임버퍼를 생성하고, 그중 반환된 이미지에 해당하는 프레임버퍼를 렌더링 시점에 사용해야 함을 의미합니다.

이를 위해, 프레임버퍼를 저장할 또 다른 `std::vector `클래스 멤버를 생성하세요.

```C++
std::vector<VkFramebuffer> swapChainFramebuffers;
```

우리는 이 배열의 객체를 그래픽 파이프라인을 생성한 직후 `initVulkan`에서 호출되는 새로운 함수 `createFramebuffers`에서 생성할 것입니다.

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
}

...

void createFramebuffers() {

}
```

먼저, 모든 프레임버퍼를 저장할 수 있도록 컨테이너의 크기를 조정합니다:

```C++
void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
}
```

그런 다음 이미지 뷰를 반복하며 이를 기반으로 프레임버퍼를 생성합니다:

```C++
for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {
        swapChainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}
```

보시다시피, 프레임버퍼 생성은 비교적 간단합니다. 먼저, 프레임버퍼가 호환되어야 하는 `renderPass`를 지정해야 합니다. 프레임버퍼는 자신과 호환되는 렌더 패스에서만 사용할 수 있으며, 이는 대략적으로 동일한 개수와 유형의 첨부물을 사용하는 것을 의미합니다.

`attachmentCount`와 `pAttachments` 매개변수는 렌더 패스의 `pAttachment` 배열에서 해당 첨부물 설명과 연결되어야 하는 `VkImageView` 객체를 지정합니다.

`width`와 `height` 매개변수는 직관적이며, `layers`는 이미지 배열의 레이어 수를 나타냅니다. 우리의 스왑체인 이미지는 단일 이미지이므로 레이어 수는 `1`입니다.

프레임버퍼는 이를 기반으로 하는 이미지 뷰 및 렌더 패스를 삭제하기 전에 삭제해야 하지만, 렌더링이 완료된 이후에만 가능합니다.

```C++
void cleanup() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    ...
}
```

이제 렌더링에 필요한 모든 객체를 준비하는 이정표에 도달했습니다. 다음 장에서는 첫 번째 실제 드로잉 명령을 작성할 것입니다.

## Source code
- [C++ code](https://vulkan-tutorial.com/code/13_framebuffers.cpp)
- [Vertex Shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)