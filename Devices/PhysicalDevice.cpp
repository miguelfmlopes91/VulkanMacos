//
//  PhysicalDevice.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 21/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "PhysicalDevice.hpp"
#include "QueueFamiliy.hpp"
#include "Swapchain.hpp"
#include "LogicalDevice.hpp"

#include <set>
#include <string>

PhysicalDevice::PhysicalDevice(std::shared_ptr<Instance> instance, std::shared_ptr<VkSurfaceKHR> surface):_instance(instance), _surface(surface){
    
}

void PhysicalDevice::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, nullptr);
    //std::cout << "Number of available devices: " << deviceCount<<std::endl;

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance->getInstance(), &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) {
    /*
    //TODO: give each device a score and pick the highest one.That way you could favor a dedicated graphics card by giving it a higher score, but fall back to an integrated GPU if that's the only available one.
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader;
    */
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device, *_surface);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    ///verify that swap chain support is adequate.
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails(device, *_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    ///It is important that we only try to query for swap chain support after verifying that the extension is available
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(LogicalDevice::getDeviceExtensions().begin(), LogicalDevice::getDeviceExtensions().end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
