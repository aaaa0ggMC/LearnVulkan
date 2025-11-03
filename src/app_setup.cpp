#include "application.h"
#include "vkutil.h"

extern std::vector<const char *> app_validation_layers;

void Application::setupVulkan(){
    vk_createInstance();
    vk_setupDebugMessenger();
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