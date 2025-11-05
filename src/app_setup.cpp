#define GLFW_INCLUDE_VULKAN
#include "application.h"
#include "vkutil.h"

extern std::vector<const char *> app_validation_layers;
extern std::vector<const char*> app_device_extensions;

void Application::setupVulkan(){
    vk_createInstance();
    vk_setupDebugMessenger();
    vk_createSurface();
    vk_pickPhysicalDevice();
    vk_createLogicalDevice();
    vk_createSwapChain();
    vk_createImageViews();
    vk_createGraphicePipeline();
}

static std::vector<char> readFile(std::string_view fp){
    std::ifstream ifs(std::string(fp),std::ios::ate | std::ios::binary);
    if(!ifs.is_open()){
        return {};
    }
    size_t fsize = ifs.tellg();
    std::vector<char> buf (fsize,0);
    ifs.seekg(0);
    ifs.read(buf.data(),fsize);
    ifs.close();
    return buf;
}

void Application::vk_createGraphicePipeline(){
    VkShaderModule vert = create_shader_module(device,readFile("data/shaders/vert.spv"));
    VkShaderModule frag = create_shader_module(device,readFile("data/shaders/frag.spv"));

    if(!frag || !vert){
        lg(LOG_CRITI) << "Failed to create vertex or fragment shaders!" << endlog;
        std::exit(-1);
    }
    lg(LOG_INFO) << "vkShaderModule:OK" << endlog;

    VkPipelineShaderStageCreateInfo infos[2];
    infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    infos[0].module = vert;
    infos[0].pName = "main";
    infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    infos[1].module = frag;
    infos[1].pName = "main";

    std::vector<VkDynamicState> dynamicStatse = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dinfo {};
    dinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dinfo.dynamicStateCount = dynamicStatse.size();
    dinfo.pDynamicStates = dynamicStatse.data();

    VkPipelineVertexInputStateCreateInfo vin {};
    vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vin.vertexBindingDescriptionCount = 0;
    vin.pVertexBindingDescriptions = nullptr;
    vin.vertexAttributeDescriptionCount = 0;
    vin.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo assem {};
    assem.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assem.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assem.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor {};
    scissor.offset = {0,0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo vps {};
    vps.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vps.viewportCount = 1;
    vps.pViewports = &viewport;
    vps.scissorCount = 1;
    vps.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo raster {};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.depthClampEnable = VK_FALSE;
    raster.rasterizerDiscardEnable = VK_FALSE;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;
    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.depthBiasEnable = VK_FALSE;
    raster.depthBiasClamp = 0.0f;
    raster.depthBiasConstantFactor = 0.f;
    raster.depthBiasSlopeFactor = 0.f;

    VkPipelineMultisampleStateCreateInfo msamp;
    msamp.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msamp.sampleShadingEnable = VK_FALSE;
    msamp.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    msamp.minSampleShading = 1.0f;
    msamp.pSampleMask = nullptr;
    msamp.alphaToCoverageEnable = VK_FALSE;
    msamp.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState cblend {};
    cblend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cblend.blendEnable = VK_FALSE;

    vkDestroyShaderModule(device,vert,nullptr);
    vkDestroyShaderModule(device,frag,nullptr);
}

void Application::vk_createImageViews(){
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0;i < swapChainImages.size();++i){
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(VkResult r = vkCreateImageView(device,&createInfo,nullptr,&swapChainImageViews[i]);
            r != VK_SUCCESS){
            lg(LOG_CRITI) << "Failed to create image view for index " << i << ":" << (int)r << endlog; 
            std::exit(-1);
        }
    }
    lg(LOG_INFO) << "vkImageViews:OK" << endlog;
}

void Application::vk_createSwapChain(){
    SwapChainsSupportDetails det = get_swapchains_support_detail(physicalDevice,surface);
    auto fmt = choose_surface_format(det.formats);
    auto mode = choose_swapchains_present_mode(det.presentModes);
    auto extent = choose_swap_extent(det.capabilities,window);

    uint32_t imageCount = det.capabilities.minImageCount + 1;
    if(det.capabilities.maxImageCount > 0 && imageCount > det.capabilities.maxImageCount){
        imageCount = det.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageColorSpace = fmt.colorSpace;
    createInfo.imageFormat = fmt.format;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices ind = find_queue_family(physicalDevice,surface);
    uint32_t queues[] = {*ind.graphicsFamily,*ind.presentFamily};
    if(ind.graphicsFamily != ind.presentFamily){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queues;
    }else{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = det.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = mode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(VkResult result = vkCreateSwapchainKHR(device,&createInfo,nullptr,&swapChain);result != VK_SUCCESS){
        lg(LOG_CRITI) << "Failed to create Vulkan swapchains:" << (int)result << endlog;
        std::exit(-1);
    }else lg(LOG_INFO) << "vkSwapChain:OK" << endlog;

    vkGetSwapchainImagesKHR(device,swapChain,&imageCount,nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device,swapChain,&imageCount,swapChainImages.data());

    swapChainImageFormat = fmt.format;
    swapChainExtent = extent;
}

void Application::vk_createSurface(){
    if(VkResult result = glfwCreateWindowSurface(instance,window,nullptr,&surface);
        result != VK_SUCCESS){
        lg(LOG_CRITI) << "Failed to create Vulkan surface:" << (int)result << endlog;
        std::exit(-1);
    }else lg(LOG_INFO) << "vkSurface:OK" << endlog;
}

void Application::vk_createLogicalDevice(){
    QueueFamilyIndices ind = find_queue_family(physicalDevice,surface);
    if(!ind.ok()){
        lg(LOG_CRITI) << "Failed to find related queue family in current physical device!" << endlog;
        std::exit(-1);
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};
    VkPhysicalDeviceFeatures deviceFeatures {};
    float queuePriority = 1;

    for(auto family : ind.gen()){
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = app_device_extensions.size();
    createInfo.ppEnabledExtensionNames = app_device_extensions.data();

    if(app_enable_validation){
        createInfo.enabledLayerCount = app_validation_layers.size();
        createInfo.ppEnabledLayerNames = app_validation_layers.data();
    }else createInfo.enabledLayerCount = 0;

    if(VkResult result = vkCreateDevice(physicalDevice,&createInfo,nullptr,&device);result != VK_SUCCESS){
        lg(LOG_CRITI) << "Failed to create Vulkan logical device:" << (int)result << endlog;
        std::exit(-1);
    }else lg(LOG_INFO) << "vkDevice:Ok" << endlog;

    vkGetDeviceQueue(device,*ind.graphicsFamily,0,&graphicsQueue);
    vkGetDeviceQueue(device,*ind.presentFamily,0,&presentQueue);
}

void Application::vk_pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);
    if(!deviceCount){
        lg(LOG_ERROR) << "No physical device found!" << endlog;
    }
    std::vector<VkPhysicalDevice> devices (deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount,devices.data());

    int maxScore = 0;
    int s_i = -1;
    VkPhysicalDevice selected = VK_NULL_HANDLE;

    int i = 0;
    for(auto & dev : devices){
        int ranking = 0;
        VkPhysicalDeviceProperties devProperty;
        VkPhysicalDeviceFeatures devFeatures;

        vkGetPhysicalDeviceProperties(dev,&devProperty);
        vkGetPhysicalDeviceFeatures(dev,&devFeatures);

        lg(LOG_INFO) << "GPU" << i << ":" << devProperty.deviceName << endlog;
        ++i;

        /// find queue families
        if(!find_queue_family(dev,surface).ok() || !check_device_extension_support(dev,app_device_extensions)
          || !get_swapchains_support_detail(dev,surface).ok()){
            continue;
        }

        if(devFeatures.geometryShader){
            if(devProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
                ranking += 1000;
            }

            ranking += devProperty.limits.maxImageDimension2D; 
        }

        if(ranking >= maxScore){
            maxScore = ranking;
            selected = dev;
            s_i = i-1;
        }
    }
    if(s_i < 0){
        lg(LOG_CRITI) << "Failed to select a GPU!" << endlog;
        std::exit(-1);
    }

    lg(LOG_INFO) << "GPU seleted:GPU" << s_i << endlog;
    physicalDevice = selected;

}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    LogFactory & lg = *((LogFactory*)pUserData);
    
    int serverity;
    switch(messageSeverity){
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        serverity = LOG_ERROR;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        serverity = LOG_WARN;
        break;
    default:
        serverity = LOG_INFO;
        break;
    }

    lg(serverity) << pCallbackData->pMessage << endlog;

    return VK_FALSE;
}

static void populateDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo,LogFactory & lg_v){
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pUserData = &lg_v;
    createInfo.pfnUserCallback = debugCallback;
}

void Application::vk_setupDebugMessenger(){
    if(!app_enable_validation)return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    populateDebugCreateInfo(createInfo,lg_v);

    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");

    if(fn != nullptr && fn(instance,&createInfo,nullptr,&debugMessenger) == VK_SUCCESS){
        lg(LOG_INFO) << "vkDebugUtilsMessenger:OK" << endlog;
    }else{
        lg(LOG_CRITI) << "Failed to create debug messenger!" << endlog;
        std::exit(-1);
    }
}

void Application::vk_createInstance(){
    /// check validation compatibility
    if(app_enable_validation){
        if(!check_validation_layer_support(app_validation_layers)){
            lg(LOG_ERROR) << "validation layers requested, but not available!" << std::endl;
            std::exit(-1);
        }else{
            lg(LOG_INFO) << "vkValidationLayer:OK" << endlog;
        }
    }


    VkApplicationInfo appInfo {};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = app_name;
    appInfo.applicationVersion = app_version;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = app_api_version;

    VkInstanceCreateInfo createInfo {};
    auto ext = get_required_extensions(app_enable_validation);
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    //// fetch GLFW extensions
    {
        createInfo.enabledExtensionCount = ext.size();
        createInfo.ppEnabledExtensionNames = ext.data();
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(app_enable_validation){
        createInfo.enabledLayerCount = app_validation_layers.size();
        createInfo.ppEnabledLayerNames = app_validation_layers.data();
    
        populateDebugCreateInfo(debugCreateInfo,lg_v);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }else{
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if(VkResult result = vkCreateInstance(&createInfo,nullptr,&instance); 
        result != VK_SUCCESS){
        lg(LOG_CRITI) << "Failed to create VkInstance:" << (int)result << endlog;
    }else lg(LOG_INFO) << "vkInstance:OK" << endlog;

    /// fetch Vulkan Extensions
    {
        uint32_t vk_extc;
        vkEnumerateInstanceExtensionProperties(nullptr,&vk_extc,nullptr);

        std::vector<VkExtensionProperties> extensions (vk_extc);
        vkEnumerateInstanceExtensionProperties(nullptr,&vk_extc,extensions.data());

        lg(LOG_INFO) << "available vulkan extensions:";
        for(auto & ext : extensions){
            lg << ext.extensionName << " ";
        }
        lg << endlog;
    }
}