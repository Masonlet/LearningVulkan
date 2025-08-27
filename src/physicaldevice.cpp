#include "engine.hpp"

/*
* Mason L'Etoile
* March 20, 2025
* Vulkan 3D Engine
* Handles Vulkan physical device selection
*/

#include <map>
#include <set>

static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> deviceExtensions) {
    // Get the number of extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // Get the extensions
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // Check if all required extensions are supported
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}
static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, auto& engine) {
    // Physical device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // Physical device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Check if physical device is suitable
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, engine->deviceExtensions);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        Engine::SwapChainSupportDetails swapChainSupport = engine->querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    Engine::QueueFamilyIndices indices = engine->findQueueFamilies(physicalDevice);
    return indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
}
static int rateDeviceSuitability(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures) {
    // Physical device properties
    int deviceScore = 0;
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) deviceScore += 1000;
    // Maximum possible size of textures affects graphics quality
    deviceScore += deviceProperties.limits.maxImageDimension2D;  
    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) return 0; 

    //Debugging
    if (enableValidationLayers) {
        std::cout << "rateDeviceSuitability\n";
        std::cout << "Device: " << deviceProperties.deviceName << '\n';
        std::cout << "Device Type: " << deviceProperties.deviceType << '\n';
        std::cout << "Max Image Dimension 2D: " << deviceProperties.limits.maxImageDimension2D << '\n';
        std::cout << "Supports Geometry Shader: " << deviceFeatures.geometryShader << '\n';
        std::cout << "Score: " << deviceScore << '\n';
    }

    return deviceScore;
}
static VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void Engine::pickPhysicalDevice() {
    //Debugging
    if (enableValidationLayers) std::cout << "pickPhysicalDevice\n";
    
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    // Score the physical devices
    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto& candidatePhysicalDevice : physicalDevices) {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (isDeviceSuitable(candidatePhysicalDevice, app)) {
            int score = rateDeviceSuitability(candidatePhysicalDevice, deviceFeatures);
            candidates.insert(std::make_pair(score, candidatePhysicalDevice));
        }
    }

    // Select the best physical device if one exists
    if (!candidates.empty() && candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
        msaaSamples = getMaxUsableSampleCount(physicalDevice);
    } else throw std::runtime_error("failed to find a suitable GPU!");
}