//
//  Image.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 09/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include "MemoryUtils.h"
struct Image
{
    VkImage mImage;
    VkDeviceMemory mDeviceMemory;//TODO:free after allocate
    uint32_t mMipLevels = 0;
    VkImageView mImageView;

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,VkDevice& device, VkPhysicalDevice& physicalDevice);
    void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, VkDevice& device);
    
    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkDevice& device);
    VkImageView& getImageView(){ return mImageView;}
};

