#include "application.h"

void Application::setup(){
    glfwInit();

    setupLogger();
    setupWindow();
    setupVulkan();
}

void Application::drawFrame(){
    vkWaitForFences(device,1,&fen_inFlight,VK_TRUE,UINT64_MAX);
    vkResetFences(device,1,&fen_inFlight);
}

void Application::cleanup(){
    vkDestroySemaphore(device,sem_imgAva,nullptr);
    vkDestroySemaphore(device,sem_renderFin,nullptr);
    vkDestroyFence(device,fen_inFlight,nullptr);
    vkDestroyCommandPool(device,pool,nullptr);
    for(auto framebuffer : swapChainFramebuffers){
        vkDestroyFramebuffer(device,framebuffer,nullptr);
    }
    vkDestroyPipeline(device,graphicsPipeline,nullptr);
    vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
    vkDestroyRenderPass(device,renderPass,nullptr);
    for(auto iv : swapChainImageViews){
        vkDestroyImageView(device,iv,nullptr);
    }
    vkDestroySwapchainKHR(device,swapChain,nullptr);
    vkDestroyDevice(device,nullptr);

    if(app_enable_validation){
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
        if(func != nullptr)func(instance,debugMessenger,nullptr);
    }

    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    // destroy window
    if(window)glfwDestroyWindow(window);
    glfwTerminate();
}

Application::~Application(){

}

void Application::setupWindow(){
    // check GLFW Vulkan Support
    if(glfwVulkanSupported()){
        lg(LOG_INFO) << "GLFW supports Vulkan." << endlog;
    }else{
        lg(LOG_CRITI) << "GLFW doesnt support Vulkan!" << endlog;
        std::exit(-1);
    }

    // no GL API
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

    // create GLFW context
    window = glfwCreateWindow(win_width,win_height,win_title,nullptr,nullptr);
}

void Application::setupLogger(){
    logger.appendLogOutputTarget("console",std::make_shared<lot::Console>());
}