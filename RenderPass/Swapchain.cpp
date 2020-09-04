//
//  Swapchain.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 16/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "Swapchain.hpp"
#include "QueueFamiliy.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <algorithm>


//Swap chain support setup
SwapChainSupportDetails::SwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    ///querying the supported surface formats.
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
    }
    
    ///querying the supported presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
    }
}

//Swap chain SETUP requirements
/*
 - Surface format (color depth)
 - Presentation mode (conditions for "swapping" images to the screen)
 - Swap extent (resolution of images in swap chain)
 */
VkSurfaceFormatKHR  chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];//if it fails we return the first one
}
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;//triple buffering
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;//double buffering
}
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {//almost equal ot the window resolution
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
       int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        return actualExtent;
    }
}



//VkPhysicalDevice physicalDevice, VkDevice& device, VkSurfaceKHR surface, std::vector<Image>& swapChainImages, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent, GLFWwindow* window

void SwapChain::createSwapChain(VkPhysicalDevice physicalDevice, VkDevice& device, VkSurfaceKHR surface, std::vector<VkImage>& swapChainImages, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent, GLFWwindow* window){
    
    SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails(physicalDevice, surface);
        
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    //make sure to not exceed the maximum number of images
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    //-------swapchain struct info required to create a swapchain object---------
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    //Specify how to handle swap chain images that will be used across multiple queue families.
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    //We'll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue
    if (indices.graphicsFamily != indices.presentFamily) {
        ///If the queue families differ, then we'll be using the concurrent mode
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;/// Images can be used across multiple queue families without explicit ownership transfers.
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {///f the graphics queue family and presentation queue family are the same, which will be the case on most hardware,
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;/// An image is owned by one queue family at a time and ownership must be explicitly transfered before using it in another queue family. This option offers the best performance.
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;///can be used for rotations, etc or just current transform
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;///specifies if the alpha channel should be used
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;///if window was resized.
    
    
    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    //retrieve the image handles and store them in the swapChainImages vector
    ///we only specified a minimum number of images in the swap chain, so the implementation is allowed to create a swap chain with more
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    //store the format and extent we've chosen for the swap chain images
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    ///We now have a set of images that can be drawn onto and can be presented to the window
     mSwapchain = swapChain;
}

void SwapChain::cleanupSwapChain(VkDevice device, VkImageView depthImageView, VkImage depthImage, VkDeviceMemory depthImageMemory,     std::vector<VkFramebuffer> swapChainFramebuffers,VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews, std::vector<Buffer> uniformBuffers, VkDescriptorPool descriptorPool, std::vector<VkImage> swapChainImages) {
    
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, mSwapchain, nullptr);
    
    ///The descriptor pool should be destroyed when the swap chain is recreated because it depends on the number of images.
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers[i].mBuffer, nullptr);
        vkFreeMemory(device, uniformBuffers[i].mBufferMemory, nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}
