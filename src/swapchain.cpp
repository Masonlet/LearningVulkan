#include "engine.hpp"

/*
* Mason L'Etoile
* March 20, 2025
* Vulkan 3D Engine
* Handles vulkan swapchain creation
*/

#include <algorithm> 

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Debugging
    if (enableValidationLayers) std::cout << "chooseSwapSurfaceFormat\n";
    
	// Check surface format
    for (const auto& availableFormat : availableFormats) 
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            return availableFormat;
    
    return VkSurfaceFormatKHR();
}
static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // Debugging
    if (enableValidationLayers) std::cout << "chooseSwapPresentMode\n";
    
    // Check present mode
    for (const auto& availablePresentMode : availablePresentModes) 
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;

    return VK_PRESENT_MODE_FIFO_KHR;
}
static VkExtent2D chooseSwapExtent(GLFWwindow*& window, const VkSurfaceCapabilitiesKHR& capabilities) {
    // Debugging
    if (enableValidationLayers) std::cout << "chooseSwapExtent\n";
    
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

Engine::SwapChainSupportDetails Engine::querySwapChainSupport(VkPhysicalDevice device) const {
    // Debugging
    if (enableValidationLayers) std::cout << "querySwapChainSupport\n";
    
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void Engine::createSwapChain(VkSwapchainKHR oldSwapChain) {
    // Debugging
    if (enableValidationLayers) std::cout << "createSwapChain\n";

    // Create Info
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
        imageCount = swapChainSupport.capabilities.maxImageCount;
    createInfo.minImageCount = imageCount;

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;

    VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
	if (oldSwapChain != nullptr) createInfo.oldSwapchain = oldSwapChain;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

	if (oldSwapChain != nullptr) vkDestroySwapchainKHR(device, oldSwapChain, nullptr);

    // Get swap chain image count
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);

    // Get swap chain images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}
void Engine::recreateSwapChain() {
    // Debugging
    if (enableValidationLayers) std::cout << "recreateSwapChain\n";

    // Minimized window handling
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // Wait for device to idle
    vkDeviceWaitIdle(device);

    VkSwapchainKHR oldSwapChain = swapChain;

	cleanupSwapChain(false);

    createSwapChain(oldSwapChain);
    createImageViews();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}