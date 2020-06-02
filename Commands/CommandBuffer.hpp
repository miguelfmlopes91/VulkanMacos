//
//  CommandBuffer.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 10/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include "VulkanHeaders.h"

struct CommandBuffer {
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;//TODO:move to private
    
public:
    const VkCommandBuffer &GetCommandBuffer() const { return mCommandBuffer; }

    void beginSingleTimeCommands(VkCommandPool& commandPool, VkDevice& device);
    void endSingleTimeCommands(VkQueue& graphicsQueue, VkDevice& device, VkCommandPool& commandPool);
};
