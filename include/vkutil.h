#ifndef VK_UTIL_H
#define VK_UTIL_H
#include <vulkan/vulkan.h>
#include <span>
#include <string_view>
#include <vector>

bool check_validation_layer_support(std::span<const char *> data);

std::vector<const char *> get_required_extensions(bool enableValidate);

#endif