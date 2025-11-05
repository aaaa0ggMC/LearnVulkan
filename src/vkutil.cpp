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

VkShaderModule create_shader_module(VkDevice dev,const std::vector<char>& code){
    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = (const uint32_t *)code.data();
    VkShaderModule mod;
    VkResult r = vkCreateShaderModule(dev,&info,nullptr,&mod);
    if(r != VK_SUCCESS)return VK_NULL_HANDLE;
    return mod;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR & cap,GLFWwindow * window){
    if(cap.currentExtent.width != UINT32_MAX){
        return cap.currentExtent;
    }else{
        int w,h;
        glfwGetFramebufferSize(window,&w,&h);
        VkExtent2D ret = {(uint32_t)w,(uint32_t)h};
        ret.width = std::clamp((uint32_t)w,cap.minImageExtent.width,cap.maxImageExtent.width);
        ret.height = std::clamp((uint32_t)h,cap.minImageExtent.height,cap.maxImageExtent.height);
        return ret;
    }
}

VkPresentModeKHR choose_swapchains_present_mode(std::span<VkPresentModeKHR> modes){
    if(std::find(modes.begin(),modes.end(),VK_PRESENT_MODE_MAILBOX_KHR) != modes.end())
        return VK_PRESENT_MODE_MAILBOX_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR choose_surface_format(std::span<VkSurfaceFormatKHR> formats){
    if(formats.empty())return {(VkFormat)0,(VkColorSpaceKHR)0};
    for(auto & fmt : formats){
        if(fmt.format == VK_FORMAT_R8G8B8A8_SRGB && 
        fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return fmt;
        }
    }
    return formats[0];
}

SwapChainsSupportDetails get_swapchains_support_detail(VkPhysicalDevice dev,VkSurfaceKHR surface){
    SwapChainsSupportDetails det {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev,surface,&det.capabilities);
    {
        uint32_t fmt_c;
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev,surface,&fmt_c,nullptr);
        det.formats.resize(fmt_c);
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev,surface,&fmt_c,det.formats.data());
    }
    {
        uint32_t mode_c;
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev,surface,&mode_c,nullptr);
        det.presentModes.resize(mode_c);
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev,surface,&mode_c,det.presentModes.data());
    }
    return det;
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
        if(q.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            ind.graphicsFamily = i; 
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev,i,surface,&presentSupport);
        if(presentSupport){
            ind.presentFamily = i;
        }
    }

    return ind;
}