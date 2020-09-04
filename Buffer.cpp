//
//  Buffer.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 09/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include <stdexcept>

void Buffer::createBuffer(VkDeviceSize bufferSize,VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice& device, VkPhysicalDevice& physicalDevice) {
    
    mBufferSize = bufferSize;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = mBufferSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;///The buffer will only be used from the graphics queue,
    
    ///create vertexBuffer object. Still need to allocate memory for it
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
    ///query its memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, mBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, mBuffer, mBufferMemory, 0);
}

void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue) {
    
    CommandBuffer commandBuffer;
    commandBuffer.beginSingleTimeCommands(commandPool, device);
    
    ///Transfer the  contents of buffers
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer.mCommandBuffer, srcBuffer.mBuffer, dstBuffer.mBuffer, 1, &copyRegion);
    
    commandBuffer.endSingleTimeCommands(graphicsQueue, device, commandPool);
    
}
