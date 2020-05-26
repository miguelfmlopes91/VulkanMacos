//
//  Instance.hpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 19/05/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#pragma once
#include <vulkan/vulkan.h>
#include <vector>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Instance {
public:
    Instance() = default;
    ~Instance();
    
    void createInstance();
    void setupDebugMessenger();
    
    const VkInstance& getInstance() const { return _instance; }
    VkInstance& getInstance(){ return _instance;}
    
    std::vector<const char*> getRequiredExtensions();

    bool areLayersEnabled(){return enableValidationLayers;}
    
private:
    
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    bool checkValidationLayerSupport();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger;
};
