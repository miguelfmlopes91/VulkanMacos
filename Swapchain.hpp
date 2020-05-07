//
//  Swapchain.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 16/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include "VulkanHeaders.h"
#include <vector>
/*
 Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
 Surface formats (pixel format, color space)
 Available presentation modes
 */
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice& device, VkSurfaceKHR surface, std::vector<VkImage>& swapChainImages, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent, GLFWwindow* window);
