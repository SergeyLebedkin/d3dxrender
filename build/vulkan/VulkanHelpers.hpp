#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#ifdef _DEBUG
#define VK_CHECK(func) { VkResult result = func; assert(result == VK_SUCCESS); };
#else
#define VK_CHECK(func)
#endif

// VulkanInstanceInfo
struct VulkanInstanceInfo
{
private:
#ifdef _DEBUG
	// debug
	VkDebugReportCallbackEXT debugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkCreateDebugReportCallbackEXT fnCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT fnDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;
	static VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char * pLayerPrefix,
		const char * pMessage,
		void * pUserData);
#endif
public:
	// base handles
	VkInstance instance = VK_NULL_HANDLE;

	// properties
	std::vector<VkPhysicalDevice> physicalDevices{};
	std::vector<VkLayerProperties> layerProperties{};
	std::vector<VkExtensionProperties> extensionProperties{};

	// Init/DeInit functions
	void Initialize(
		const char* appName, uint32_t appVersion,
		const char* engineName, uint32_t engineVersion,
		std::vector<const char *> enabledLayerNames,
		std::vector<const char *> enabledExtensionNames,
		uint32_t apiVersion);
	void DeInitialize();

	// find functions
	VkPhysicalDevice FindPhysicalDevice(VkPhysicalDeviceType physicalDeviceType);
};

// VulkanDeviceInfo
struct VulkanDeviceInfo 
{
	// base handles
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR     surface = VK_NULL_HANDLE;
	VkDevice         device = VK_NULL_HANDLE;

	// properties
	VkPhysicalDeviceFeatures             deviceFeatures;
	VkPhysicalDeviceProperties           deviceProperties;
	VkPhysicalDeviceMemoryProperties     deviceMemoryProperties;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
	std::vector<VkSurfaceFormatKHR>      surfaceFormats{};
	std::vector<VkPresentModeKHR>        presentModes{};

	// memory type indexes
	uint32_t memoryDeviceLocalTypeIndex = UINT32_MAX;
	uint32_t memoryHostVisibleTypeIndex = UINT32_MAX;

	// queue family indexes
	uint32_t queueFamilyIndexGraphics = UINT32_MAX;
	uint32_t queueFamilyIndexCompute = UINT32_MAX;
	uint32_t queueFamilyIndexTransfer = UINT32_MAX;
	uint32_t queueFamilyIndexPresent = UINT32_MAX;

	// queues
	VkQueue queueGraphics = VK_NULL_HANDLE;
	VkQueue queueCompute = VK_NULL_HANDLE;
	VkQueue queueTransfer = VK_NULL_HANDLE;
	VkQueue queuePresent = VK_NULL_HANDLE;

	// command pool
	VkCommandPool commandPool = VK_NULL_HANDLE;

	// Init/DeInit functions
	void Initialize(
		VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
		VkPhysicalDeviceFeatures& physicalDeviceFeatures,
		std::vector<const char *>& enabledExtensionNames);
	void DeInitialize();

	// util functions
	void FindPresentQueueFamilyIndexes(uint32_t& graphicsIndex, uint32_t& presentIndex) const;
	uint32_t FindQueueFamilyIndexByFlags(uint32_t queueFlags) const;
	uint32_t FindMemoryHeapIndexByFlags(VkMemoryPropertyFlags propertyFlags) const;
	uint32_t FindMemoryHeapIndexByBits(uint32_t bits, VkMemoryPropertyFlags propertyFlags) const;
	uint32_t CheckMemoryHeapIndexByBits(uint32_t index, VkMemoryPropertyFlags propertyFlags) const;
	VkSurfaceFormatKHR FindSurfaceFormat() const;
	VkPresentModeKHR FindPresentMode() const;

	// allocate functions
	void AllocateBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& deviceMemory);
	void UpdateBufferAndMemory(const void* data, VkDeviceSize size, VkBuffer buffer, VkDeviceMemory deviceMemory);
};

// VulkanSwapchainInfo
struct VulkanSwapchainInfo
{
private:
	// base handles
	VkDevice     device = VK_NULL_HANDLE;
	VkQueue      queuePresent = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	// current frame index
	uint32_t currentFramebufferIndex = UINT32_MAX;
public:
	// swapchain
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	// viewport size
	uint32_t viewportWidth = UINT32_MAX;
	uint32_t viewportHeight = UINT32_MAX;

	// framebuffer formats
	VkSurfaceFormatKHR surfaceFormat{};
	VkFormat           depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkPresentModeKHR   presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	// framebuffer data
	VkImage                    imageDepthStencil = VK_NULL_HANDLE;
	VkImageView                imageViewDepthStencil = VK_NULL_HANDLE;
	VkDeviceMemory             memoryDepthStencil = VK_NULL_HANDLE;
	std::vector<VkImage>       imageColors{};
	std::vector<VkImageView>   imageViewColors{};
	std::vector<VkFramebuffer> framebuffers{};
	
	// Init/DeInit functions
	void Initialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface, VkRenderPass renderPass);
	void DeInitialize();
	void ReInitialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface, VkRenderPass renderPass);

	// frame processing
	VkFramebuffer BeginFrame(VkSemaphore signalSemaphore);
	void EndFrame(VkSemaphore waitSemaphore);
};

// InitDeviceQueueCreateInfo
VkDeviceQueueCreateInfo InitDeviceQueueCreateInfo(uint32_t queueIndex);

// CreatePipelineVertexInputState
VkPipelineVertexInputStateCreateInfo InitPipelineVertexInputStateCreateInfo(
	std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
	std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions);

// CreateSurface
VkSurfaceKHR CreateSurface(VkInstance instance, HWND hWnd);

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

// QueueSubmit
void QueueSubmit(VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore);
