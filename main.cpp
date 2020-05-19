//
//  main.cpp
//  VulkanMacos
//
//  Created by Miguel Lopes on 02/04/2020.
//  Copyright © 2020 Miguel Lopes. All rights reserved.
//

#include "Vertex.hpp"

#include "tiny_obj_loader.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <set>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm>
#include <fstream>


#include "Swapchain.hpp"
#include "QueueFamiliy.hpp"
#include "Texture.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::string MODEL_PATH = "Resources/models/chalet.obj";
const std::string TEXTURE_PATH = "Resources/textures/chalet.jpg";

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
///list of required device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};





/*const std::vector<Vertex> vertices = {
    ///position                 ///color                                ///coordinates
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};
*/
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};



#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    ///determine the size of the file and allocate a buffer
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    ///
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

class HelloTriangleApplication {
public:
    void run() {//FAÇADE
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
        
private:
    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR _surface;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;//logical device
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    //VkSwapchainKHR _swapChain;
    std::vector<VkImage> _swapChainImages;
    std::vector<Image> _swapChainImagesN;
    std::vector<VkImageView> _swapChainImageViews;
    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkFormat _swapChainImageFormat;
    VkExtent2D _swapChainExtent;
    VkRenderPass _renderPass;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout;///for storing uniform variables for shaders
    VkPipeline _graphicsPipeline;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    
//    VkBuffer _vertexBuffer;
//    VkDeviceMemory _vertexBufferMemory;
    
//    VkBuffer _indexBuffer;
//    VkDeviceMemory _indexBufferMemory;
    
    //std::vector<VkBuffer> _uniformBuffers;
    //std::vector<VkDeviceMemory> _uniformBuffersMemory;
    std::vector<Buffer> _uniformBuffers;
    
    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
    uint32_t _mipLevels;
    VkImage _textureImage;//TODO::remove
    VkDeviceMemory _textureImageMemory;
    
    //VkImage _depthImage;
    //VkDeviceMemory _depthImageMemory;
    //VkImageView _depthImageView;
    size_t _currentFrame = 0;
    bool _framebufferResized = false;
    
    Texture _texture = Texture(TEXTURE_PATH);
    Image _depthImageN;
    
    Buffer _vertexBuffer;
    Buffer _indexBuffer;
    
    SwapChain _swapChain;
private:
    void initWindow() {
        glfwInit();
        
        int result = glfwInit();
        printf("glfwInit result: %d\n", result);
        if (!glfwVulkanSupported()) {
             printf("Vulkan support not found\n");
        }else{
            printf("Vulkan is supported, yay!\n");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);///get a reference to the GLFWwindow
        glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);

    }
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        
        _swapChain.createSwapChain(_physicalDevice,_device,_surface,_swapChainImages,_swapChainImageFormat,_swapChainExtent, _window);
        
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        createDepthResources();
        createFramebuffers();
        
            
        _texture.createTextureImage(_device, _physicalDevice, _commandPool, _graphicsQueue);
        _texture.createTextureImageView(VK_IMAGE_ASPECT_COLOR_BIT, _device);
        _texture.createTextureSampler(_device);
        
        
        loadModel();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }
    void recreateSwapChain() {
        ///deal with minimizaition
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_window, &width, &height);
            glfwWaitEvents();
        }
        
        ///we shouldn't touch resources that may still be in use.
        vkDeviceWaitIdle(_device);

        _swapChain.cleanupSwapChain(_device, _depthImageN.mImageView, _depthImageN.mImage, _depthImageN.mDeviceMemory, _swapChainFramebuffers, _commandPool, _commandBuffers, _graphicsPipeline, _pipelineLayout, _renderPass, _swapChainImageViews, _uniformBuffers,_descriptorPool, _swapChainImages);
        
        _swapChain.createSwapChain(_physicalDevice,_device,_surface,_swapChainImages,_swapChainImageFormat,_swapChainExtent,_window);
        createRenderPass();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
    }
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        
        //load global extensions
        auto extensions = getRequiredExtensions();
        
        ///-------Vulkan utilizes strcuts instead of parameters in functions, so we need to fill some of those so we can have something to work with--------
        //non-obligatory struct, provides some usefull information for optimization
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//Vulkan structures require that you specify the strutc type (sType)
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        //obligatory struct, tells the global(needed in the entire code) extensions needed and the validation layers
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;//TODO: figure out way
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
                
        /* Info vkCreateInstance:
         - Pointer to struct with creation info
         - Pointer to custom allocator callbacks, always nullptr in this tutorial
         - Pointer to the variable that stores the handle to the new object
         */
        VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);

        if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
            std::cout << "cannot find a compatible Vulkan ICD\n";
            throw std::runtime_error("failed to create instance!");
        } else  if (result != VK_SUCCESS) {
            std::cout << "Failed with error: " << result << std::endl;
            throw std::runtime_error("failed to create instance!");
        }
    }
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;
        
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        
        if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void createSurface() {
        if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        //std::cout << "Number of available devices: " << deviceCount<<std::endl;

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());
        
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, _surface);

       std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
       std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        
        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }
        
        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        
        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
    }
    void createRenderPass() {
        VkAttachmentDescription colorAttachment = {};
        ///The format of the color attachment should match the format of the swap chain images, and we're not doing anything with multisampling yet, so we'll stick to 1 sample.
        colorAttachment.format = _swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        ///loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;///Clear the values to a constant at the start
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;///Rendered contents will be stored in memory and can be read later
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;///specifies which layout the image will have before the render pass begins
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;///: Images to be presented in the swap chain
        
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        //Subpasses and attachment references
        ///A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes
        ///Every subpass references one or more of the attachments. (attachment references)
        // subpasses in a render pass automatically take care of image layout transitions
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;///specifies which attachment to reference by its index in the attachment descriptions array (The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!)
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        ///Unlike color attachments, a subpass can only use a single depth (+stencil) attachment. It wouldn't really make any sense to do depth tests on multiple buffers.
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        
        //Subpass dependencies
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        ///We need to wait for the swap chain to finish reading from the image before we can access it.
        ///This can be accomplished by waiting on the color attachment output stage itself.
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        
        //Creating the render pass
        if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        

    }
    void createDescriptorSetLayout(){
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;///transformation is in a single uniform buffer object,
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;///referencing the descriptor from the vertex shader.
        uboLayoutBinding.pImmutableSamplers = nullptr; /// Optiona l///relevant for image sampling related descriptors,
        
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};


        ///all of descriptor bindings are combined into a single VkDescriptorSetLayout object
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        
        
    }
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("Resources/shaders/vert.spv");
        auto fragShaderCode = readFile("Resources/shaders/frag.spv");
        
        //Creating shader modules
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
        
        //Shader creation stage
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
        
        //Shader input stage
        ///describes the format of the vertex data that will be passed to the vertex shader.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        
        //Input assembly stage
        ///what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        
        //Viewports and scissors
        /// region of the framebuffer that the output will be rendered to
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) _swapChainExtent.width;
        viewport.height = (float) _swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        ///scissor rectangles define in which regions pixels will actually be stored
        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = _swapChainExtent;
        ///now we combine viewport and scissors
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        
        //Rasterizer
        ///takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader.
        /// also performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;///if true, ragments  beyond the near and far planes are clamped to them as opposed to discarding them. Good for shadow maps
        rasterizer.rasterizerDiscardEnable = VK_FALSE;///if true then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;///determines how fragments are generated for geometry. Fill, Line, Point.
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;///again, use true for shadow mapping
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
        
        //MultiSampling
        ///one of the ways to perform anti-aliasing
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional
        
        //Depth and stencil testing
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;///specifies if the depth of new fragments should be compared to the depth buffer to see if they should be discarded.
        depthStencil.depthWriteEnable = VK_TRUE;///pecifies if the new depth of fragments that pass the depth test should actually be written to the depth buffer.
        /// This is useful for drawing transparent objects. They should be compared to the previously rendered opaque objects, but not cause further away transparent objects to not be drawn.
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional
        
        
        
        //Color blending
        ///After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
        
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        
        //Dynamic state
        ///A limited amount of the state can actually be changed without recreating the pipeline. Examples are the size of the viewport, line width and blend constants
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;
        
        //Pipeline layout
        ///To create and store  uniform values
        ///tell Vulkan which descriptors the shaders will be using.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // Optional
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        ///create pipelineLayout object
        if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        //Finally we can now begin filling in the VkGraphicsPipelineCreateInfo structure
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        ///the fixed-function stage.
        pipelineInfo.layout = _pipelineLayout;
        ///reference to the render pass and the index of the sub pass where this graphics pipeline will be used
        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.subpass = 0;
        ///Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        
        //And finally create the graphics pipeline:
        if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        
        //Clear the used shader modules
        vkDestroyShaderModule(_device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_device, vertShaderModule, nullptr);
    }
    void createFramebuffers(){
        ///The attachments specified during render pass creation are bound by wrapping them into a VkFramebuffer object.
        ///A framebuffer object references all of the VkImageView objects that represent the attachments. In our case that will be only a single one: the color attachment.
        ///However, the image that we have to use for the attachment depends on which image the swap chain returns when we retrieve one for presentation.
        ///That means that we have to create a framebuffer for all of the images in the swap chain and use the one that corresponds to the retrieved image at drawing time.
        
        _swapChainFramebuffers.resize(_swapChainImageViews.size());
        
        ///iterate through the image views and create framebuffers from them
        for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                _swapChainImageViews[i],
                _depthImageN.mImageView// _depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = _swapChainExtent.width;
            framebufferInfo.height = _swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
            ///The color attachment differs for every swap chain image, but the same depth image can be used by all of them because only a single subpass is running at the same time due to our semaphores.
        }
    }
    void createCommandPool() {
    ///We have to create a command pool before we can create command buffers.
    ///Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice, _surface);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();///record commands for drawing, choose the graphics queue family.
        poolInfo.flags = 0; // Optional
        
        if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void createDepthResources() {
        ///Creating a depth image is fairly straightforward. It should have the same resolution as the color attachment, defined by the swap chain extent, an image usage appropriate for a depth attachment, optimal tiling and device local memory.
        
        VkFormat depthFormat = findDepthFormat();
        _depthImageN.mMipLevels = 1;
        _depthImageN.createImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _device, _physicalDevice);
        _depthImageN.createImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, _device);
        
        //transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
        
    }
    void createImageViews() {
        ///An image view is sufficient to start using an image as a texture, but it's not quite ready to be used as a render target just yet. Need framebuffer
        _swapChainImageViews.resize(_swapChainImages.size());

        for (size_t i = 0; i < _swapChainImages.size(); i++) {
            _swapChainImageViews[i] = Image::createImageView(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, _device);
        }
    }
    VkFormat findDepthFormat() {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void createVertexBuffer() {
        Buffer stagingBuffer;
        VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
        
        stagingBuffer.createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _device, _physicalDevice);

        ///copy the vertex data to the buffer
        //void* data;
        ///This function allows us to access a region of the specified memory resource defined by an offset and size. The offset and size here are 0 and bufferInfo.size, respectively. It is also possible to specify the special value VK_WHOLE_SIZE to map all of the memory.
        ///The second to last parameter can be used to specify flags, but there aren't any available yet in the current API. It must be set to the value 0. The last parameter specifies the output for the pointer to the mapped memory.
        vkMapMemory(_device, stagingBuffer.mBufferMemory, 0, stagingBuffer.mBufferSize, 0, &stagingBuffer.mData);
        memcpy(stagingBuffer.mData, _vertices.data(), (size_t) stagingBuffer.mBufferSize);///cpy the vertex data to the mapped memory
        vkUnmapMemory(_device, stagingBuffer.mBufferMemory);///free the memory
        
        _vertexBuffer.mBufferSize =sizeof(_vertices[0]) * _vertices.size();
        
        _vertexBuffer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _device, _physicalDevice);

        copyBuffer(stagingBuffer, _vertexBuffer, bufferSize, _commandPool, _device, _graphicsQueue);

        vkDestroyBuffer(_device, stagingBuffer.mBuffer, nullptr);
        vkFreeMemory(_device, stagingBuffer.mBufferMemory, nullptr);
        
        //TODO: check if below is still valid
        //Warning::Unfortunately the driver may not immediately copy the data into the buffer memory, for example because of caching.
        //It is also possible that writes to the buffer are not visible in the mapped memory yet.
        //There are two ways to deal with that problem:
        /// - Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        /// - Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
        
        //We went for the first approach, which ensures that the mapped memory always matches the contents of the allocated memory. Do keep in mind that this may lead to slightly worse performance than explicit flushing.
        
        //Flushing memory ranges or using a coherent memory heap means that the driver will be aware of our writes to the buffer, but it doesn't mean that they are actually visible on the GPU yet. The transfer of data to the GPU is an operation that happens in the background and the specification simply tells us that it is guaranteed to be complete as of the next call to vkQueueSubmit.

    }
    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

        Buffer stagingBuffer;
        stagingBuffer.createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _device, _physicalDevice);

        vkMapMemory(_device, stagingBuffer.mBufferMemory, 0, stagingBuffer.mBufferSize, 0, &stagingBuffer.mData);
        memcpy(stagingBuffer.mData, _indices.data(), (size_t) bufferSize);
        vkUnmapMemory(_device, stagingBuffer.mBufferMemory);

        _indexBuffer.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _device, _physicalDevice);

        copyBuffer(stagingBuffer, _indexBuffer, bufferSize, _commandPool, _device, _graphicsQueue);

        vkDestroyBuffer(_device, stagingBuffer.mBuffer, nullptr);
        vkFreeMemory(_device, stagingBuffer.mBufferMemory, nullptr);
    }
    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        //_uniformBuffers.resize(_swapChainImages.size());
        
        for (size_t i = 0; i < _swapChainImages.size(); i++) {
            Buffer tempBuffer;
            tempBuffer.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _device, _physicalDevice);
            _uniformBuffers.push_back(tempBuffer);
        }
    }
    void loadModel() {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(warn + err);
        }
        
        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
        
        ///We're going to combine all of the faces in the file into a single model, so just iterate over all of the shapes:
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex = {};

                ///Unfortunately the attrib.vertices array is an array of float values instead of something like glm::vec3, so you need to multiply the index by 3.
                ///Similarly, there are two texture coordinate components per entry. The offsets of 0, 1 and 2 are used to access the X, Y and Z components, or the U and V components in the case of texture coordinates.
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                ///The OBJ format assumes a coordinate system where a vertical coordinate of 0 means the bottom of the image, however we've uploaded our image into Vulkan in a top to bottom orientation where 0 means the top of the image. Solve this by flipping the vertical component of the texture coordinates:
                                
                
                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                    _vertices.push_back(vertex);
                }

                _vertices.push_back(vertex);
                _indices.push_back(uniqueVertices[vertex]);///If you check the size of vertices, then you'll see that it has shrunk down from 1,500,000 to 265,645! That means that each vertex is reused in an average number of ~6 triangles. This definitely saves us a lot of GPU memory.
            }
        }
        
    }
    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes = {};
              poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
              poolSizes[0].descriptorCount = static_cast<uint32_t>(_swapChainImages.size());
              poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
              poolSizes[1].descriptorCount = static_cast<uint32_t>(_swapChainImages.size());

              VkDescriptorPoolCreateInfo poolInfo = {};
              poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
              poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
              poolInfo.pPoolSizes = poolSizes.data();
              poolInfo.maxSets = static_cast<uint32_t>(_swapChainImages.size());

              if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
                  throw std::runtime_error("failed to create descriptor pool!");
              }
        
    }
    void createDescriptorSets(){
        ///In our case we will create one descriptor set for each swap chain image, all with the same layout
        std::vector<VkDescriptorSetLayout> layouts(_swapChainImages.size(), _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();
        
        _descriptorSets.resize(_swapChainImages.size());
        if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        
        ///The descriptor sets have been allocated now, but the descriptors within still need to be configured.
        for (size_t i = 0; i < _swapChainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = _uniformBuffers[i].mBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = _texture.mTextureImage.mImageView;
            imageInfo.sampler = _texture.mTextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = _descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = _descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            
            //The descriptors must be updated with this image info, just like the buffer.
            //This time we're using the pImageInfo array instead of pBufferInfo. The descriptors are now ready to be used by the shaders!

            vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createCommandBuffers() {
        ///Commands in Vulkan, like drawing operations and memory transfers, are not executed directly using function calls.
        ///You have to record all of the operations you want to perform in command buffer objects.
        ///The advantage of this is that all of the hard work of setting up the drawing commands can be done in advance and in multiple threads.
        ///After that, you just have to tell Vulkan to execute the commands in the main loop.
        _commandBuffers.resize(_swapChainFramebuffers.size());
        
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;///Can be submitted to a queue for execution, but cannot be called from other command buffers.
        allocInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

        if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        
        //Starting command buffer recording
        for (size_t i = 0; i < _commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }
            //Starting a render pass
            ///Drawing starts by beginning the render pass with vkCmdBeginRenderPass.
            ///The render pass is configured using some parameters in a VkRenderPassBeginInfo struct.
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = _renderPass;
            renderPassInfo.framebuffer = _swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = _swapChainExtent;
            
            //Clear Color
            std::array<VkClearValue, 2> clearValues = {};
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            
            //The render pass can now begin
            vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            //We can now bind the graphics pipeline
            ///The second parameter specifies if the pipeline object is a graphics or compute pipeline
            vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
            
            ///binding the vertex buffer
            VkBuffer vertexBuffers[] = {_vertexBuffer.mBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(_commandBuffers[i], _indexBuffer.mBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);///Unlike vertex and index buffers, descriptor sets are not unique to graphics pipelines.  Therefore we need to specify if we want to bind descriptor sets to the graphics or compute pipeline. createGraphicsPipeline

            //The ACTUAL DRAW FUNC
            /*
             - the current command buffer object
             - vertexCount: vertex buffer size (3)
             - instanceCount: Used for instanced rendering, use 1 if you're not doing that.
             - firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
             - firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
             */
            //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);

            
            //The render pass can now be ended:
            vkCmdEndRenderPass(_commandBuffers[i]);

            //Now check if we're able to finish recording the command buffer correctly
            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
        

    }
    void createSyncObjects(){
        //Synchronization
        ///There are two ways of synchronizing swap chain events: fences and semaphores.
        _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        _imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);///Initially not a single frame is using an image so we explicitly initialize it to no fence.

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;///nitialize it in the signaled state as if we had rendered an initial frame that finished to avoid stuck on waitForFences
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
    void mainLoop() {
        double previousTime = glfwGetTime();
        int frameCount = 0;
        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();
            drawFrame();
            // Measure speed
            double currentTime = glfwGetTime();
            frameCount++;
            // If a second has passed.
            if ( currentTime - previousTime >= 1.0 ){
                // Display the frame count here any way you want.
                std::cout <<"FPS: " << frameCount<<std::endl;

                frameCount = 0;
                previousTime = currentTime;
            }

        }
        vkDeviceWaitIdle(_device);
    }
    void drawFrame() {
        /*The drawFrame function will perform the following operations:
            -Wait for fences to
            - Acquire an image from the swap chain
            - Execute the command buffer with that image as attachment in the framebuffer
            - Return the image to the swap chain for presentation
         */
        
        /// takes an array of fences and waits for either any or all of them to be signaled before returning.
        /// wait for the frame to be finished
        vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

        //Acquire an image from the swap chain
        ///using semaphores to specify synchronization
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device, _swapChain.mSwapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

        ///check if swap chain recreation is necessary
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }


        updateUniformBuffer(imageIndex);
        /// Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        
        /// Mark the image as now being in use by this frame
        _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];
        
        //Submitting the command buffer
        ///Queue submission and synchronization is configured through parameters in the VkSubmitInfo structure.
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        ///We want to wait with writing colors to the image until it's available
        VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        ///specify which command buffers to actually submit for execution
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];
        ///specify which semaphores to signal once the command buffer(s) have finished execution
        VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        ///restore the fence to the unsignaled state
        ///Unlike the semaphores, we manually need to restore the fence to the unsignaled state by resetting
        vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
        
        ///We can now submit the command buffer to the graphics queue using vkQueueSubmit.
        ///The function takes an array of VkSubmitInfo structures as argument for efficiency when the workload is much larger.
        if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        //Presentation
        ///The last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up on the screen
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        
        VkSwapchainKHR swapChains[] = {_swapChain.mSwapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; /// Optional: allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful.

        ///submits the request to present an image to the swap chain.
        result = vkQueuePresentKHR(_presentQueue, &presentInfo);
        
        ///check if swap chain recreation is necessary
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
            _framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        ///shouldn't forget to advance to the next frame every time:
        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    }
    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        // define the model, view and projection transformations in the uniform buffer object
        UniformBufferObject ubo = {};
        ///model rotation will be a simple rotation around the Z-axis using the time variable:
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        /// the view transformation  looks at the geometry from above at a 45 degree angle. The glm::lookAt function takes the eye position, center position and up axis as parameters.
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ///perspective projection with a 45 degree vertical field-of-view
        ubo.proj = glm::perspective(glm::radians(45.0f), _swapChainExtent.width / (float) _swapChainExtent.height, 0.1f, 10.0f);
        ///GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted.
        ubo.proj[1][1] *= -1;
        
        void* data;
        vkMapMemory(_device, _uniformBuffers[currentImage].mBufferMemory, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(_device, _uniformBuffers[currentImage].mBufferMemory);

    }
    void cleanup() {
        ///cleanupSwapChain(_device, _depthImageView, _depthImage, _depthImageMemory, _swapChainFramebuffers, _commandPool, _commandBuffers, _graphicsPipeline, _pipelineLayout, _renderPass, _swapChainImageViews, _swapChain, _uniformBuffers, _uniformBuffersMemory, _descriptorPool, _swapChainImages);
        _swapChain.cleanupSwapChain(_device, _depthImageN.mImageView, _depthImageN.mImage, _depthImageN.mDeviceMemory, _swapChainFramebuffers, _commandPool, _commandBuffers, _graphicsPipeline, _pipelineLayout, _renderPass, _swapChainImageViews, _uniformBuffers, _descriptorPool, _swapChainImages);
        
        //vkDestroySampler(_device, _textureSampler, nullptr);//TODO
        //vkDestroyImageView(_device, _textureImageView, nullptr);//TODO:
        vkDestroyImage(_device, _textureImage, nullptr);
        vkFreeMemory(_device, _textureImageMemory, nullptr);
        vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
        //vkDestroyBuffer(_device, _indexBuffer, nullptr);
        //vkFreeMemory(_device, _indexBufferMemory, nullptr);
        //vkDestroyBuffer(_device, _vertexBuffer, nullptr);
        //vkFreeMemory(_device, _vertexBufferMemory, nullptr);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(_device, _inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(_device, _commandPool, nullptr);

        vkDestroyDevice(_device, nullptr);
        //TODO:INSTANCE É O RENDERER
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, debugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        /*
        //TODO: give each device a score and pick the highest one.That way you could favor a dedicated graphics card by giving it a higher score, but fall back to an integrated GPU if that's the only available one.
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
               deviceFeatures.geometryShader;
        */
        QueueFamilyIndices indices = findQueueFamilies(device, _surface);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        
        ///verify that swap chain support is adequate.
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        
        ///It is important that we only try to query for swap chain support after verifying that the extension is available
        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        //std::cout <<"Number of layers fround supported: "<< layerCount << std::endl;

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//        std::cout << "available layers:" << std::endl;
//        for (const char* layerName : validationLayers) {
//            std::cout << "\t" << layerName << std::endl;
//        }
        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();    }
    std::vector<const char*> getRequiredExtensions() {
        //total extension count
        /*
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionsVec(extensionCount);
        std::cout << "Number of extensions supported: "<< extensionCount << std::endl;

        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsVec.data());
        std::cout << "available extensions:" << std::endl;
        for (const auto& extension : extensionsVec) {
            std::cout << "\t" << extension.extensionName << std::endl;
        }
         */
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        /* TO COMPILE COCOA WINDOW SURFACE
         #define GLFW_EXPOSE_NATIVE_COCOA
         #include <GLFW/glfw3native.h>

         #define VK_USE_PLATFORM_MACOS_MVK
         #include <vulkan/vulkan.h>

         #include "ObjC-interface.h"
         ....
         // Adding KHR surface extension and MVK_MACOS_SURFACE_EXTENSION
         if (enableValidationLayers) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
         extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
         }*/
        return extensions;
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        ///takes a list of candidate formats in order from most desirable to least desirable, and checks which is the first one that is supported:
        
        //The support of a format depends on the tiling mode and usage, so we must also include these as parameters.
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
            
            ///The VkFormatProperties struct contains three fields:
            ///linearTilingFeatures: Use cases that are supported with linear tiling
            ///optimalTilingFeatures: Use cases that are supported with optimal tiling
            ///bufferFeatures: Use cases that are supported for buffers
            
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Message is important enough to show
        }

        return VK_FALSE;
    }
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    //Swap chain support setup
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

        ///querying the supported surface formats.
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
        }
        
        ///querying the supported presentation modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
        }
        
        
        return details;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        
        return shaderModule;
    }
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->_framebufferResized = true;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
