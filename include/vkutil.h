#ifndef VK_UTIL_H
#define VK_UTIL_H
#include <vulkan/vulkan.h>
#include <span>
#include <string_view>
#include <vector>
#include <optional>

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool ok(){
        return graphicsFamily && presentFamily;
    }

    inline std::vector<uint32_t> gen(){
        if(!ok())return {};
        return {
            *graphicsFamily,
            *presentFamily
        };
    }
};

bool check_validation_layer_support(std::span<const char *> data);

std::vector<const char *> get_required_extensions(bool enableValidate);

QueueFamilyIndices find_queue_family(VkPhysicalDevice dev,VkSurfaceKHR surface);

bool check_device_extension_support(VkPhysicalDevice device,std::span<const char*>);

#endif