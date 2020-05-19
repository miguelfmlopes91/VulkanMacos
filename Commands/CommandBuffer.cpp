//
//  CommandBuffer.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 10/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "CommandBuffer.hpp"


void CommandBuffer::beginSingleTimeCommands(VkCommandPool& commandPool, VkDevice& device) {
   ///Memory transfer operations are executed using command buffers, just like drawing commands.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//could be extracted
    allocInfo.commandPool = commandPool;///TODO: use a different command pool
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    //TODO: move this to a begin func. No need to begin command buffer if we're just idle
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;///We're only going to use the command buffer once and wait with returning from the function until the copy operation has finished executing. It's good practice to tell the driver about our intent using
    
    ///start recording the command buffer
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    mCommandBuffer = commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(VkQueue& graphicsQueue, VkDevice& device, VkCommandPool& commandPool) {
    ///Stop recording
    vkEndCommandBuffer(mCommandBuffer);
    
    ///Now execute the command buffer to complete the transfer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffer;

    ///Unlike the draw commands, there are no events we need to wait on this time. We just want to execute the transfer on the buffers immediately.
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    ///There are again two possible ways to wait on this transfer to complete. We could use a fence and wait with vkWaitForFences, or simply wait for the transfer queue to become idle with vkQueueWaitIdle. A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete, instead of executing one at a time. That may give the driver more opportunities to optimize.
    
    vkFreeCommandBuffers(device, commandPool, 1, &mCommandBuffer);
}
