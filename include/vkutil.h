#ifndef VK_UTIL_H
#define VK_UTIL_H
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <span>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool ok(){
        return graphicsFamily && presentFamily;
    }

    inline std::vector<uint32_t> gen(){
        if(!ok())return {};
        std::set<uint32_t> unique_data;

        unique_data.insert(*presentFamily);
        unique_data.insert(*graphicsFamily);

        return std::vector(unique_data.begin(),unique_data.end());
    }
};

struct SwapChainsSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    inline bool ok(){
        return !formats.empty() && !presentModes.empty();
    }
};

bool check_validation_layer_support(std::span<const char *> data);

std::vector<const char *> get_required_extensions(bool enableValidate);

QueueFamilyIndices find_queue_family(VkPhysicalDevice dev,VkSurfaceKHR surface);

bool check_device_extension_support(VkPhysicalDevice device,std::span<const char*>);

SwapChainsSupportDetails get_swapchains_support_detail(VkPhysicalDevice dev,VkSurfaceKHR surface);

VkSurfaceFormatKHR choose_surface_format(std::span<VkSurfaceFormatKHR> formats);

VkPresentModeKHR choose_swapchains_present_mode(std::span<VkPresentModeKHR> modes);

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR & cap,GLFWwindow*);

VkShaderModule create_shader_module(VkDevice,const std::vector<char>& code);

#endif