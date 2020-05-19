//
//  Swapchain.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 16/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
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

struct SwapChain {
    std::vector<VkImage> mImages;
    std::vector<VkImageView> mImageViews;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    
    
    void createSwapChain(VkPhysicalDevice physicalDevice, VkDevice& device, VkSurfaceKHR surface, std::vector<VkImage>& swapChainImages, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent, GLFWwindow* window);

    void cleanupSwapChain(VkDevice device, VkImageView depthImageView, VkImage depthImage, VkDeviceMemory depthImageMemory,     std::vector<VkFramebuffer> swapChainFramebuffers,VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews, std::vector<Buffer> uniformBuffers, VkDescriptorPool descriptorPool, std::vector<VkImage> swapChainImages);
};


/*
 //Swap chain SETUP requirements
  - Surface format (color depth)
  - Presentation mode (conditions for "swapping" images to the screen)
  - Swap extent (resolution of images in swap chain)
 
 TODO:: add the private funcs here
 */
