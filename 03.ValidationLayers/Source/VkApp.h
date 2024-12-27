#pragma once

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

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
	bool checkValidationLayerSupport();
	void createInstance();

private:
	/** GLFW */
	GLFWwindow* window;

	/** Vulkan */
	VkInstance instance;
};