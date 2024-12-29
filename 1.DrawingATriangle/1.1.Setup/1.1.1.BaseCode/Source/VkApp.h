#pragma once

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 800;

class VkApp
{
public:
	void run();

private:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

private:
	GLFWwindow* window;
};