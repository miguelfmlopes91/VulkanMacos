//
//  PhysicalDevice.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 21/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Instance.hpp"

class PhysicalDevice{
public:
    PhysicalDevice() = delete;
    PhysicalDevice(const Instance* instance, VkSurfaceKHR* surface);
    PhysicalDevice(const std::shared_ptr<Instance> instance, VkSurfaceKHR* surface);
    void pickPhysicalDevice();//TODO: Make this the constructor
    const VkPhysicalDevice& getDevice() const { return mPhysicalDevice; }
    VkPhysicalDevice& getDevice() { return mPhysicalDevice; }
    const Instance& getInstance() const{ return *_instance; }

private:
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    const Instance* _instance;
    VkSurfaceKHR* _surface;
    
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties mProperties = {};
    VkPhysicalDeviceFeatures mFeatures = {};
    VkPhysicalDeviceMemoryProperties mMemoryProperties = {};
    VkSampleCountFlagBits mMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    
};
