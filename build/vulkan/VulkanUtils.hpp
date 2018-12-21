#pragma once

#include <vulkan/vulkan.h>

// Create Vulcan Handlers
VkInstance VulkanCreateInstance();
VkSurfaceKHR VulkanCreateSurface(VkInstance vkInstance, HWND hWnd);
VkPhysicalDevice VulkanGetPhysicalDevice(VkInstance vkInstance);
VkDevice VulkanCreateDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKHR);
