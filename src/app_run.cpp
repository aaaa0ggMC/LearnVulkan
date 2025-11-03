#include "application.h"

int Application::run(){
    
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }

    return 0;
}