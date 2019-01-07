#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#ifdef _DEBUG
#define VK_CHECK(func) { VkResult result = func; assert(result == VK_SUCCESS); };
#else
#define VK_CHECK(func)
#endif

// InitDeviceQueueCreateInfo
VkDeviceQueueCreateInfo InitDeviceQueueCreateInfo(uint32_t queueIndex);

// CreatePipelineVertexInputState
VkPipelineVertexInputStateCreateInfo InitPipelineVertexInputStateCreateInfo(
	std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
	std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions);

// FindPhysicalDevice
VkPhysicalDevice FindPhysicalDevice(VkInstance instance, VkPhysicalDeviceType physicalDeviceType);
// FindPhysicalDeviceMemoryIndex
uint32_t FindPhysicalDeviceMemoryIndex(VkPhysicalDevice physicalDevice, uint32_t propertyFlags);
// FindPhysicalDeviceQueueFamilyIndex
uint32_t FindPhysicalDeviceQueueFamilyIndex(VkPhysicalDevice physicalDevice, uint32_t queueFlags);
// FindSurfaceFormat
VkSurfaceFormatKHR FindSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
// FindPresentMode
VkPresentModeKHR FindPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
// FindPresentQueueMode
uint32_t FindPresentQueueMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

// CreateInstance
VkInstance CreateInstance(
	const char* appName, uint32_t appVersion,
	const char* engineName, uint32_t engineVersion,
	std::vector<const char *> enabledLayerNames,
	std::vector<const char *> enabledExtensionNames,
	uint32_t apiVersion);

// CreateSurface
VkSurfaceKHR CreateSurface(VkInstance instance, HWND hWnd);

// CreateDevice
VkDevice CreateDevice(
	VkPhysicalDevice physicalDevice,
	std::vector<VkDeviceQueueCreateInfo>& deviceQueueCreateInfos,
	std::vector<const char *> enabledExtensionNames,
	const VkPhysicalDeviceFeatures physicalDeviceFeatures);

// CreateSwapchain
VkSwapchainKHR CreateSwapchain(
	VkDevice device, VkSurfaceKHR surface, 
	VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode,
	uint32_t imageCount, uint32_t width, uint32_t height);

// CreateImage
VkImage CreateImage(VkDevice device, VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height);

// CreateImageView
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

// CreateFramebuffer
VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);

// CreateBuffer
VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);

// CreateShaderModuleFromFile
VkShaderModule CreateShaderModuleFromFile(VkDevice device, const char* fileName);

// CreateRenderPass
VkRenderPass CreateRenderPass(VkDevice device);

// CreatePipelineLayout
VkPipelineLayout CreatePipelineLayout(VkDevice device);

// CreateGraphicsPipeline
VkPipeline CreateGraphicsPipeline(
	VkDevice device, 
	VkPipelineVertexInputStateCreateInfo vertexInputState, 
	VkShaderModule vs, VkShaderModule fs, 
	VkPipelineLayout layout, 
	VkRenderPass renderPass, 
	uint32_t width, uint32_t height);

// CreateCommandPool
VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueIndex);

// AllocateCommandBuffer
VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

// CreateSemaphore
VkSemaphore CreateSemaphore(VkDevice device);

// InitVulkanDebug
void InitVulkanDebug(VkInstance instance);

// DeInitVulkanDebug
void DeInitVulkanDebug(VkInstance instance);