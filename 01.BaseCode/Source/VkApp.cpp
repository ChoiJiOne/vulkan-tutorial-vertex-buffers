#include "VkApp.h"

void VkApp::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VkApp::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Tutorial", nullptr, nullptr);
}

void VkApp::initVulkan()
{
}

void VkApp::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void VkApp::cleanup()
{
	glfwDestroyWindow(window);
	window = nullptr;

	glfwTerminate();
}