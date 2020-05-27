//
//  LogicalDevice.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 21/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "LogicalDevice.hpp"
#include "QueueFamiliy.hpp"
#include <set>


const std::vector<const char*> LogicalDevice::deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

LogicalDevice::LogicalDevice(std::shared_ptr<PhysicalDevice> device, std::shared_ptr<VkSurfaceKHR> surface):_physicalDevice(device),_surface(surface){
    
}


void LogicalDevice::createLogicalDevice(VkQueue& graphicsQueue, VkQueue& presentQueue) {
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(_physicalDevice->getDevice(), *_surface);

   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>( LogicalDevice::deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    auto instance = _physicalDevice->getInstance();
    if (instance.areLayersEnabled()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(_physicalDevice->getDevice(), &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}
