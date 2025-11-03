#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <alib-g3/alogger.h>

using namespace alib::g3;

constexpr int win_width = 800;
constexpr int win_height = 600;
constexpr const char * win_title = "Learn Vulkan";
constexpr const char * app_name = win_title;
constexpr uint32_t app_version = VK_MAKE_VERSION(1,0,0);
constexpr uint32_t app_api_version = VK_API_VERSION_1_0; 
constexpr bool app_enable_validation = true;

static std::vector<const char *> app_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

struct Application{
    Logger logger;
    LogFactory lg;
    LogFactory lg_v;
    GLFWwindow * window { nullptr };

    /// Vulkan Data
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice { VK_NULL_HANDLE };

    inline Application():
    lg ("LearnVK",logger),
    lg_v ("Vulkan",logger)
    {}

    void setup();
    int run();
    void cleanup();

    /// setups
    void setupLogger();
    void setupWindow();
    void setupVulkan();

    /// vulkan setups
    void vk_createInstance();
    void vk_setupDebugMessenger();
    void vk_pickPhysicalDevice();

    ~Application();
};

#endif