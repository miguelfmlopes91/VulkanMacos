//
//  Texture.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 08/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once

#include <string_view>
#include "Image.hpp"
#include "Buffer.hpp"

class Texture{
public: //TODO::get accessors
    std::string_view mTexturePath;
    Image mTextureImage;
    Buffer mStagingBuffer;
    VkSampler mTextureSampler;
    
public:
    Texture(std::string_view texturePath);
    
    const Image& getImageTexture()const{ return mTextureImage;}
    void createTextureImage(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue);
    void createTextureImageView(VkImageAspectFlags aspectFlags, VkDevice& device);
    void createTextureSampler(VkDevice& device);

private:
    void copyBufferToImage(Buffer& buffer, uint32_t width, uint32_t height, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue);
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue);
    void generateMipmaps(int32_t texWidth, int32_t texHeight, VkFormat format, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkDevice& device, VkCommandPool& commandPool);
};


