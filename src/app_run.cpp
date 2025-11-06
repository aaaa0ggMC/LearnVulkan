#include "application.h"

int Application::run(){
    
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
    return 0;
}