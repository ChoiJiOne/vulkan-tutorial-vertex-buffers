# Presentation: Image views

`VkImage`를 포함하여 스왑 체인의 모든 이미지를 렌더링 파이프라인에서 사용하려면, 반드시 `VkImageView` 객체를 생성해야 합니다. 이미지 뷰는 말 그대로 이미지에 대한 "뷰"입니다. 이는 이미지에 어떻게 접근할지와 어느 부분에 접근할지를 설명합니다. 예를 들어, 이를 2D 텍스처나 깊이 텍스처로 처리할지, 혹은 어떤 mipmapping 레벨도 사용하지 않을지를 정의합니다.

이번 챕터에서는 스왑 체인의 각 이미지에 대해 기본적인 이미지 뷰를 생성하는 `createImageViews` 함수를 작성할 것입니다. 이를 통해 나중에 이러한 이미지를 색상 타겟(color target)으로 사용할 수 있게 됩니다.

먼저, 이미지 뷰를 저장할 클래스 멤버를 추가합니다:

```C++
std::vector<VkImageView> swapChainImageViews;
```

`createImageViews` 함수를 작성하고, 이를 스왑 체인을 생성한 직후에 호출하세요.

```C++
void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
}

void createImageViews() {

}
```

먼저, 생성할 모든 이미지 뷰를 저장할 수 있도록 리스트 크기를 조정해야 합니다.

```C++
void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

}
```

그다음, 스왑 체인 이미지를 반복 처리하는 루프를 설정합니다.

```C++
for (size_t i = 0; i < swapChainImages.size(); i++) {

}
```

이미지 뷰 생성에 필요한 매개변수는 `VkImageViewCreateInfo` 구조체에 지정됩니다. 초기 몇 가지 매개변수는 간단합니다.

```C++
VkImageViewCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
createInfo.image = swapChainImages[i];
```

`viewType`과 `format` 필드는 이미지 데이터를 어떻게 해석할지를 지정합니다. `viewType` 매개변수는 이미지를 1D 텍스처, 2D 텍스처, 3D 텍스처 또는 큐브 맵으로 처리할 수 있도록 합니다.

```C++
createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
createInfo.format = swapChainImageFormat;
```

`components` 필드는 색상 채널을 재배치(swizzle)할 수 있도록 해줍니다. 예를 들어, 모든 채널을 빨간색 채널에 매핑하여 단색 텍스처를 만들 수 있습니다. 또한, 0이나 1과 같은 상수 값을 채널에 매핑할 수도 있습니다. 그러나 이번 경우에는 기본 매핑을 사용할 것입니다.

```C++
createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
```

`subresourceRange` 필드는 이미지의 목적과 접근할 이미지의 부분을 설명합니다. 우리의 이미지는 mipmapping 레벨이나 여러 레이어 없이 색상 타겟으로 사용될 것입니다.

```C++
createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
createInfo.subresourceRange.baseMipLevel = 0;
createInfo.subresourceRange.levelCount = 1;
createInfo.subresourceRange.baseArrayLayer = 0;
createInfo.subresourceRange.layerCount = 1;
```

만약 스테레오그래픽 3D 애플리케이션을 작업 중이라면, 여러 레이어로 구성된 스왑 체인을 생성해야 할 것입니다. 그런 다음, 각 이미지에 대해 왼쪽 눈과 오른쪽 눈을 위한 서로 다른 레이어를 접근하여 여러 이미지 뷰를 생성할 수 있습니다.

이미지 뷰를 생성하려면 이제 `vkCreateImageView`를 호출하면 됩니다.

```C++
if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image views!");
}
```

이미지와는 달리, 이미지 뷰는 우리가 명시적으로 생성했기 때문에 프로그램 종료 시 이를 다시 파괴하는 유사한 루프를 추가해야 합니다.

```C++
void cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    ...
}
```

이미지 뷰만으로 이미지를 텍스처로 사용하는 것은 충분하지만, 이를 렌더 타겟으로 사용하려면 한 단계 더 필요합니다. 이를 프레임버퍼(framebuffer)라고 하는 간접적인 개체로 설정해야 합니다. 그러나 그 전에 그래픽 파이프라인을 먼저 설정해야 합니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/07_image_views.cpp)