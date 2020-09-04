//
//  Buffer.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 09/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include "MemoryUtils.h"

struct Buffer
{
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    void* mData;
    VkDeviceSize mBufferSize; ///Can it be size_t?
    
    void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice& device, VkPhysicalDevice& physicalDevice);
    
};
void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue);
