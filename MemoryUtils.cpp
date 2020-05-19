//
//  MemoryUtils.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 19/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "MemoryUtils.h"
#include <stdexcept>
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
