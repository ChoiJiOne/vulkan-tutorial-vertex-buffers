# Conclusion

이제 이전 챕터에서 다룬 모든 구조체와 객체를 결합하여 그래픽스 파이프라인을 생성할 수 있습니다! 현재 우리가 가진 객체 유형은 다음과 같습니다:

- 셰이더 단계(Shader stages): 그래픽스 파이프라인의 프로그래머블 단계의 기능을 정의하는 셰이더 모듈
- 고정 함수 상태(Fixed-function state): 입력 조합, 래스터라이저, 뷰포트, 색상 블렌딩과 같은 파이프라인의 고정 함수 단계를 정의하는 모든 구조체
파이프라인 레이아웃: 셰이더가 참조하는 유니폼과 푸시 값들로, 드로우 타임에 업데이트할 수 있음
- 렌더 패스(Render pass): 파이프라인 단계에서 참조하는 첨부물과 그 용도

이 모든 요소들이 결합되어 그래픽스 파이프라인의 기능을 완전히 정의하므로, 이제 `createGraphicsPipeline` 함수에서 `VkGraphicsPipelineCreateInfo` 구조체를 채우기 시작할 수 있습니다. 그러나 `vkDestroyShaderModule` 호출 이전에 이 구조체를 사용하는 것이므로, 먼저 이를 설정해야 합니다.

```C++
VkGraphicsPipelineCreateInfo pipelineInfo{};
pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
pipelineInfo.stageCount = 2;
pipelineInfo.pStages = shaderStages;
```

먼저, `VkPipelineShaderStageCreateInfo` 구조체 배열을 참조합니다.

```C++
pipelineInfo.pVertexInputState = &vertexInputInfo;
pipelineInfo.pInputAssemblyState = &inputAssembly;
pipelineInfo.pViewportState = &viewportState;
pipelineInfo.pRasterizationState = &rasterizer;
pipelineInfo.pMultisampleState = &multisampling;
pipelineInfo.pDepthStencilState = nullptr; // Optional
pipelineInfo.pColorBlendState = &colorBlending;
pipelineInfo.pDynamicState = &dynamicState;
```

그다음, 고정 함수 단계에 대한 모든 구조체를 참조합니다.

```C++
pipelineInfo.layout = pipelineLayout;
```

그 후, 파이프라인 레이아웃을 참조하는데, 이는 구조체 포인터가 아닌 Vulkan 핸들입니다.

```C++
pipelineInfo.renderPass = renderPass;
pipelineInfo.subpass = 0;
```

마지막으로, 렌더 패스에 대한 참조와 이 그래픽스 파이프라인이 사용될 서브패스의 인덱스를 참조합니다. 이 파이프라인은 특정 인스턴스 대신 다른 렌더 패스를 사용할 수 있지만, `renderPass`와 호환되어야 합니다. 호환성 요구 사항은 여기에서 설명되지만, 이번 튜토리얼에서는 [이 기능](https://docs.vulkan.org/spec/latest/chapters/renderpass.html#renderpass-compatibility)을 사용하지 않습니다.

```C++
pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
pipelineInfo.basePipelineIndex = -1; // Optional
```

실제로 두 가지 추가 매개변수도 있습니다: `basePipelineHandle`과 `basePipelineIndex`. Vulkan은 기존 파이프라인에서 파생하여 새 그래픽스 파이프라인을 생성할 수 있도록 허용합니다. 파이프라인 파생의 개념은 기존 파이프라인과 많은 기능을 공유하는 파이프라인을 설정하는 것이 더 비용 효율적이며, 동일한 부모에서 파이프라인을 전환하는 것도 더 빠르게 할 수 있다는 것입니다. 기존 파이프라인의 핸들을 `basePipelineHandle`로 지정하거나, 생성될 또 다른 파이프라인을 인덱스로 참조하는 방식으로 `basePipelineIndex`를 사용할 수 있습니다. 현재는 파이프라인이 하나뿐이므로, 우리는 단순히 널 핸들과 유효하지 않은 인덱스를 지정할 것입니다. 이 값들은 `VkGraphicsPipelineCreateInfo`의 flags 필드에서 `VK_PIPELINE_CREATE_DERIVATIVE_BIT` 플래그가 지정될 경우에만 사용됩니다.

이제 최종 단계를 준비하면서 `VkPipeline` 객체를 저장할 클래스 멤버를 생성합니다.

```C++
VkPipeline graphicsPipeline;
```

마지막으로 그래픽스 파이프라인을 생성합니다.

```C++
if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
}
```

`vkCreateGraphicsPipelines` 함수는 Vulkan에서 일반적인 객체 생성 함수보다 더 많은 매개변수를 가집니다. 이 함수는 여러 개의 `VkGraphicsPipelineCreateInfo` 객체를 받아 여러 개의 `VkPipeline` 객체를 한 번에 생성할 수 있도록 설계되었습니다.

두 번째 매개변수는 선택적인 `VkPipelineCache` 객체를 참조합니다. 이 매개변수에 `VK_NULL_HANDLE`을 전달했지만, `VkPipelineCache`는 파이프라인 생성에 관련된 데이터를 저장하고 재사용하는 데 사용됩니다. 이를 통해 여러 번의 `vkCreateGraphicsPipelines` 호출 또는 프로그램 실행 간에 파이프라인 생성을 빠르게 할 수 있습니다. 이 내용은 파이프라인 캐시 장에서 자세히 다룰 예정입니다.

그래픽스 파이프라인은 모든 일반적인 그리기 작업에 필요하므로 프로그램 종료 시에만 파괴해야 합니다.

```C++
void cleanup() {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    ...
}
```

이제 프로그램을 실행하여 그동안의 작업이 성공적으로 파이프라인을 생성했는지 확인하세요! 우리는 화면에 무언가가 나타나기를 매우 가깝게 만들었습니다. 다음 몇 개의 장에서는 스왑 체인 이미지를 사용해 실제 프레임버퍼를 설정하고 그리기 명령을 준비할 것입니다.

## Source Code
- [C++ code](https://vulkan-tutorial.com/code/12_graphics_pipeline_complete.cpp)
- [Vertex shader](https://vulkan-tutorial.com/code/09_shader_base.vert)
- [Fragment shader](https://vulkan-tutorial.com/code/09_shader_base.frag)