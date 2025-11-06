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

    uint32_t imgIndex;
    vkAcquireNextImageKHR(device,swapChain,UINT64_MAX,sem_imgAva,VK_NULL_HANDLE,&imgIndex);

    vkResetCommandBuffer(commandBuffer,0);
    vk_recordCommandBuffer(commandBuffer,imgIndex);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { sem_imgAva };
    VkSemaphore signalSemaphores[] = { sem_renderFin };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(VkResult r = vkQueueSubmit(graphicsQueue,1,&submitInfo,fen_inFlight);r != VK_SUCCESS){
        lg(LOG_ERROR) << "Failed to submit this frame:" << (int)r << endlog;
    }

    VkSwapchainKHR swapChains[] = {swapChain};
    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imgIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentQueue,&presentInfo);
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