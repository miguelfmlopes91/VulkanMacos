//
//  MemoryUtils.h
//  VulkanMacos
//
//  Created by Miguel Lopes on 09/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include "VulkanHeaders.h"
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);
