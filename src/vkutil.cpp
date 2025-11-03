#include <vkutil.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <GLFW/glfw3.h>

static bool operator==(const VkLayerProperties & in,const char * d){
    return !std::strcmp(in.layerName,d);
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