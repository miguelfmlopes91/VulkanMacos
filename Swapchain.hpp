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
#include <optional>
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

struct QueueFamilyIndices {
   std::optional<uint32_t> graphicsFamily;;
   std::optional<uint32_t> presentFamily;
   bool isComplete() {
       return graphicsFamily.has_value() && presentFamily.has_value();
   }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface); //TODO: move out, doesnt belong to swapchain
//cleanupSwapChain
