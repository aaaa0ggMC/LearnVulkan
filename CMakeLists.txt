cmake_minimum_required(VERSION 3.10)

project(learn_vulkan)

aux_source_directory("./src" LV_MAIN)
aux_source_directory("./src/alib-g3" ALIB_MAIN)

include_directories(./include)

add_library(alib-g3 ${ALIB_MAIN})
target_compile_options(alib-g3 PUBLIC -std=c++26)

add_executable(learn_vulkan ${LV_MAIN})
target_link_libraries(learn_vulkan PUBLIC -std=c++26)
target_link_libraries(learn_vulkan PRIVATE glfw alib-g3 vulkan)