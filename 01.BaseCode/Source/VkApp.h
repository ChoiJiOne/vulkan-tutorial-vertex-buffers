#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class VkApp
{
public:
	void run();

private:
	void initVulkan();
	void mainLoop();
	void cleanup();
};