#include <GLFW/glfw3.h>
#include <iostream>

// Branch 0.1 : Bare Minimum GLFW Window

int main() {
	// Initialize GLFW
	// Must be called before any other GLFW function
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return EXIT_FAILURE;
	}

	// Configure GLFW context using glfwWindowHint
	// This first argument is the option to be configured
	// The second argument is the value to set the option to
	// More information can be found at: https://www.glfw.org/docs/latest/window.html#window_hints
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We won't use OpenGL context

	// Create a window object -- Holds all the windowing data, required by most of GLFW's functions
	// glfwCreateWindow(width, height, title, monitor, share)
	// Monitor = NULL -> windowed mode, non-NULL -> full screen mode
	// Share = NULL -> do not share resources with other context, non-NULL -> share resources with the specified context
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearningVulkan", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate(); // Clean up any GLFW resources
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(window)) {
		// Poll inputs/events (keyboard, mouse, window resize, etc.)
		glfwPollEvents(); 
	}

	// Destroy the window
	glfwDestroyWindow(window); 
	// Terminates GLFW, clearing any resources allocated by GLFW
	glfwTerminate(); 
	return EXIT_SUCCESS;
}
