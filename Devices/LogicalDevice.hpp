//
//  LogicalDevice.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 21/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "PhysicalDevice.hpp"

class LogicalDevice {
public:
    LogicalDevice() = delete;
    LogicalDevice(const PhysicalDevice *physicalDevice, const VkSurfaceKHR* surface);
    
    void createLogicalDevice(VkQueue& graphicsQueue, VkQueue& presentQueue);
    const VkDevice &GetLogicalDevice() const { return mLogicalDevice; }
    VkDevice &GetLogicalDevice(){ return mLogicalDevice; }

    static const std::vector<const char*>& getDeviceExtensions() {return deviceExtensions;}
    
private:
    VkDevice mLogicalDevice = VK_NULL_HANDLE;
    const VkSurfaceKHR* _surface = VK_NULL_HANDLE;
    const PhysicalDevice* _physicalDevice = nullptr;
    
    ///list of required device extensions
    static const std::vector<const char*> deviceExtensions;
};
