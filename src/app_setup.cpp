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
}

void Application::vk_createSurface(){
    if(VkResult result = glfwCreateWindowSurface(instance,window,nullptr,&surface);
        result != VK_SUCCESS){
        lg(LOG_CRITI) << "Failed to create Vulkan surface!" << endlog;
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
        if(!find_queue_family(dev,surface).ok() || !check_device_extension_support(dev,app_device_extensions)){
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