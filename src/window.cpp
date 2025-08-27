#include "engine.hpp"

/*
* Mason L'Etoile
* March 20, 2025
* Vulkan 3D Engine
* Handles window creation
*/

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    // Debugging
    if (enableValidationLayers) std::cout << "framebufferResizeCallback\n";
    
	// Get app
    auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Engine::initWindow() {
	// Debugging
    if (enableValidationLayers) std::cout << "initWindow\n";

	// Initialize GLFW
    glfwInit();

	// Set GLFW options
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Create window
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Engine::createSurface() {
    // Debugging
    if (enableValidationLayers) std::cout << "createSurface\n";

    // Create Surface
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}
