#include <vkutil.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <GLFW/glfw3.h>

static bool operator==(const VkLayerProperties & in,const char * d){
    return !std::strcmp(in.layerName,d);
}

static bool operator==(const VkExtensionProperties & in,const char * d){
    return !std::strcmp(in.extensionName,d);
}

bool check_validation_layer_support(std::span<const char *> data){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());

    for(const char * c : data){
        if(std::find(availableLayers.begin(),availableLayers.end(),c)
             == availableLayers.end()){ // 没找到，直接返回bad
            return false;
        }
    }
    return true;
}

std::vector<const char *> get_required_extensions(bool enableValidate){
    uint32_t ge_c = 0;
    const char ** ge_v = glfwGetRequiredInstanceExtensions(&ge_c);

    std::vector<const char*> exts (ge_v,ge_v + ge_c);

    if(enableValidate){
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return exts;
}

bool check_device_extension_support(VkPhysicalDevice device,std::span<const char*> data){
    uint32_t ext_c;
    vkEnumerateDeviceExtensionProperties(device,nullptr,&ext_c,nullptr);
    std::vector<VkExtensionProperties> exts (ext_c);
    vkEnumerateDeviceExtensionProperties(device,nullptr,&ext_c,exts.data());

    for(auto d : data){
        if(std::find(exts.begin(),exts.end(),d) == exts.end()){
            return false;
        }
    }
    return true;
}

QueueFamilyIndices find_queue_family(VkPhysicalDevice dev,VkSurfaceKHR surface){
    QueueFamilyIndices ind;
    uint32_t qe_c;
    vkGetPhysicalDeviceQueueFamilyProperties(dev,&qe_c,nullptr);
    std::vector<VkQueueFamilyProperties> qeFamilies (qe_c);
    vkGetPhysicalDeviceQueueFamilyProperties(dev,&qe_c,qeFamilies.data());
    int i = -1;
    for(auto & q : qeFamilies){
        ++i;
        if(!ind.graphicsFamily && q.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            ind.graphicsFamily = i; 
            continue;
        }

        if(!ind.presentFamily){
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev,i,surface,&presentSupport);
            if(presentSupport){
                ind.presentFamily = i;
                continue;
            }
        }
    }

    return ind;
}