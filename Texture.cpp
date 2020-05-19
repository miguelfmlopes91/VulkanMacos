//
//  Texture.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 08/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "stb_image.h"

#include "Texture.hpp"
#include "CommandBuffer.hpp"


#include <cmath>
#include <string>


Texture::Texture(std::string_view path){
    mTexturePath = path;
}

void Texture::createTextureImage(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& graphicsQueue){
    int texWidth, texHeight, texChannels;
    
    stbi_uc* pixels = stbi_load(std::string(mTexturePath).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
   VkDeviceSize size  = texWidth * texHeight * 4;
    mTextureImage.mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    ///This calculates the number of levels in the mip chain. The max function selects the largest dimension.
    ///The log2 function calculates how many times that dimension can be divided by 2.
    ///The floor function handles cases where the largest dimension is not a power of 2. 1 is added so that the original image has a mip level.
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    
    mStagingBuffer.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, device, physicalDevice );
    

    vkMapMemory(device, mStagingBuffer.mBufferMemory, 0, mStagingBuffer.mBufferSize, 0, &mStagingBuffer.mData);
    memcpy(mStagingBuffer.mData, pixels, static_cast<size_t>(mStagingBuffer.mBufferSize));
    vkUnmapMemory(device, mStagingBuffer.mBufferMemory);

    stbi_image_free(pixels);
  //TODO::
//    if (mTextureImage==nullptr) {
//        throw std::runtime_error("Texture Image is null.");
//    }
 
    ///creating the texture image.
    mTextureImage.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device, physicalDevice);

    
    //Preparing the texture image
    ///copy the staging buffer to the texture image. This involves two steps:
    ///Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ///Execute the buffer to image copy operation
    
    ///To be able to start sampling from the texture image in the shader, we need one last transition to prepare it for shader access:
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandPool, device, graphicsQueue);
    
    ///The image was created with the VK_IMAGE_LAYOUT_UNDEFINED layout, so that one should be specified as old layout when transitioning textureImage
    copyBufferToImage(mStagingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), commandPool, device, graphicsQueue);
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    vkDestroyBuffer(device, mStagingBuffer.mBuffer, nullptr);
    vkFreeMemory(device, mStagingBuffer.mBufferMemory, nullptr);
    
    generateMipmaps(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, physicalDevice, graphicsQueue, device, commandPool);
}
void Texture::createTextureImageView(VkImageAspectFlags aspectFlags, VkDevice& device) {
    mTextureImage.createImageView(VK_FORMAT_R8G8B8A8_SRGB,aspectFlags, device);
}

void Texture::createTextureSampler(VkDevice& device){
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;///Repeat the texture when going beyond the image dimensions.
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;///There is no graphics hardware available today that will use more than 16 samples
    //samplerInfo.anisotropyEnable = VK_FALSE;
    //samplerInfo.maxAnisotropy = 1;
    
    
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;///The borderColor field specifies which color is returned when sampling beyond the image with clamp to border addressing mode
    samplerInfo.unnormalizedCoordinates = VK_FALSE;///The unnormalizedCoordinates field specifies which coordinate system you want to use to address texels in an image.
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    ///for mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;//static_cast<float>(_mipLevels/2);///0.0f;//Optional
    samplerInfo.maxLod = static_cast<float>(mTextureImage.mMipLevels);///Level of detail
    samplerInfo.mipLodBias = 0;//Optional
    
    if (vkCreateSampler(device, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    
    ///Note the sampler does not reference a VkImage anywhere.
    ///The sampler is a distinct object that provides an interface to extract colors from a texture. It can be applied to any image you want, whether it is 1D, 2D or 3D.
    ///This is different from many older APIs, which combined texture images and filtering into a single state.
}

#pragma region HelperFunctions
void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue) {
    
    CommandBuffer cmdBuffer;
    cmdBuffer.beginSingleTimeCommands(commandPool, device);
    
    ///A pipeline barrier like that is generally used to synchronize access to resources, like ensuring that a write to a buffer completes before reading from it, but it can also be used to transition image layouts and transfer queue family ownership when VK_SHARING_MODE_EXCLUSIVE is used.
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ///The image and subresourceRange specify the image that is affected and the specific part of the image.
    ///Our image is not an array and does not have mipmapping levels, so only one level and layer are specified.
    barrier.image = mTextureImage.mImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mTextureImage.mMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    //Transition barrier masks
    ///There are two transitions we need to handle:
    ///Undefined → transfer destination: transfer writes that don't need to wait on anything
    ///Transfer destination → shader reading: shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    ///The image will be written in the same pipeline stage and subsequently read by the fragment shader,
    /// which is why we specify shader reading access in the fragment shader pipeline stage.
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(
        cmdBuffer.GetCommandBuffer(),
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    cmdBuffer.endSingleTimeCommands(graphicsQueue, device, commandPool );
    
    ///All of the helper functions that submit commands so far have been set up to execute synchronously by waiting for the queue to become idle. For practical applications it is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput, especially the transitions and copy in the createTextureImage function.
    ///Try to experiment with this by creating a setupCommandBuffer that the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been recorded so far. It's best to do this after the texture mapping works to check if the texture resources are still set up correctly.
}

void Texture::copyBufferToImage(Buffer& buffer, uint32_t width, uint32_t height, VkCommandPool& commandPool, VkDevice& device, VkQueue& graphicsQueue) {
    CommandBuffer cmdBuffer;
    cmdBuffer.beginSingleTimeCommands(commandPool, device);
    ///need to specify which part of the buffer is going to be copied to which part of the image.
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };
    
    vkCmdCopyBufferToImage(cmdBuffer.GetCommandBuffer(), buffer.mBuffer, mTextureImage.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    cmdBuffer.endSingleTimeCommands(graphicsQueue, device, commandPool );
}
#pragma endregion HelperFunctions

#pragma region mipmaps
void Texture::generateMipmaps(int32_t texWidth, int32_t texHeight, VkFormat format, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkDevice& device, VkCommandPool& commandPool) {
    ///There are two alternatives in this case. You could implement a function that searches common texture image formats for one that does support linear blitting, or you could implement the mipmap generation in software with a library like stb_image_resize. Each mip level can then be loaded into the image in the same way that you loaded the original image.

    ///It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway. Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed. Implementing resizing in software and loading multiple levels from a file is left as an exercise to the reader.
    
    
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }
    CommandBuffer cmdBuffer;
    cmdBuffer.beginSingleTimeCommands(commandPool, device);
    
    ///We're going to make several transitions, so we'll reuse this VkImageMemoryBarrier.
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = mTextureImage.mImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    
    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    ///This loop will record each of the VkCmdBlitImage commands. Note that the loop variable starts at 1, not 0.
    for (uint32_t i = 1; i < mTextureImage.mMipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        ///This transition will wait for level i - 1 to be filled, either from the previous blit command, or from vkCmdCopyBufferToImage.
        ///The current blit command will wait on this transition.
        vkCmdPipelineBarrier(cmdBuffer.GetCommandBuffer(),
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        ///Now, we record the blit command. Note that textureImage is used for both the srcImage and dstImage parameter. This is because we're blitting between different levels of the same image.
        ///The source mip level was just transitioned to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and the destination level is still in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL from createTextureImage.
        vkCmdBlitImage(cmdBuffer.GetCommandBuffer(),
        mTextureImage.mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        mTextureImage.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &blit,
        VK_FILTER_LINEAR);///The last parameter allows us to specify a VkFilter to use in the blit. We have the same filtering options here that we had when making the VkSampler. We use the VK_FILTER_LINEAR to enable interpolation.
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuffer.GetCommandBuffer(),
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        ///At the end of the loop, we divide the current mip dimensions by two. We check each dimension before the division to ensure that dimension never becomes 0. This handles cases where the image is not square, since one of the mip dimensions would reach 1 before the other dimension. When this happens, that dimension should remain 1 for all remaining levels.
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = mTextureImage.mMipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    ///Before we end the command buffer, we insert one more pipeline barrier. This barrier transitions the last mip level from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
    ///This wasn't handled by the loop, since the last mip level is never blitted from.
    vkCmdPipelineBarrier(cmdBuffer.GetCommandBuffer(),
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    cmdBuffer.endSingleTimeCommands(graphicsQueue, device, commandPool);
}
#pragma endregion mipmaps
