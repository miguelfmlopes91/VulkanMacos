//
//  QueueFamiliy.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 07/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include "VulkanHeaders.h"
#include <optional>

struct QueueFamilyIndices {
   std::optional<uint32_t> graphicsFamily;;
   std::optional<uint32_t> presentFamily;
   bool isComplete() {
       return graphicsFamily.has_value() && presentFamily.has_value();
   }
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};


//cleanupSwapChain

