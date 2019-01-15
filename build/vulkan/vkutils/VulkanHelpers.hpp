#pragma once

#include "VmaUsage.h"
#include <vector>
#include <list>
#include <map>

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
	VkInstance       mInstance = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDeviceGPU = VK_NULL_HANDLE;

	// properties
	std::vector<VkPhysicalDevice>      mPhysicalDevices{};
	std::vector<VkLayerProperties>     mLayerProperties{};
	std::vector<VkExtensionProperties> mExtensionProperties{};

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
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR     mSurface = VK_NULL_HANDLE;
	VkDevice         mDevice = VK_NULL_HANDLE;
	VmaAllocator     mAllocator = VK_NULL_HANDLE;

	// properties
	VkPhysicalDeviceFeatures             mDeviceFeatures;
	VkPhysicalDeviceProperties           mDeviceProperties;
	VkPhysicalDeviceMemoryProperties     mDeviceMemoryProperties;
	std::vector<VkQueueFamilyProperties> mQueueFamilyProperties{};
	std::vector<VkSurfaceFormatKHR>      mSurfaceFormats{};
	std::vector<VkPresentModeKHR>        mPresentModes{};

	// memory type indexes
	uint32_t mMemoryDeviceLocalTypeIndex = UINT32_MAX;
	uint32_t mMemoryHostVisibleTypeIndex = UINT32_MAX;

	// queue family indexes
	uint32_t mQueueFamilyIndexGraphics = UINT32_MAX;
	uint32_t mQueueFamilyIndexCompute = UINT32_MAX;
	uint32_t mQueueFamilyIndexTransfer = UINT32_MAX;
	uint32_t mQueueFamilyIndexPresent = UINT32_MAX;

	// queues
	VkQueue mQueueGraphics = VK_NULL_HANDLE;
	VkQueue mQueueCompute = VK_NULL_HANDLE;
	VkQueue mQueueTransfer = VK_NULL_HANDLE;
	VkQueue mQueuePresent = VK_NULL_HANDLE;

	// command pool
	VkCommandPool mCommandPool = VK_NULL_HANDLE;

	// Init/DeInit functions
	void Initialize(
		VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
		VkPhysicalDeviceFeatures& physicalDeviceFeatures,
		std::vector<const char *>& enabledExtensionNames);
	void DeInitialize();

	// utilities functions
	void FindPresentQueueFamilyIndexes(uint32_t& graphicsIndex, uint32_t& presentIndex) const;
	uint32_t FindQueueFamilyIndexByFlags(uint32_t queueFlags) const;
	uint32_t FindMemoryHeapIndexByFlags(VkMemoryPropertyFlags propertyFlags) const;
	uint32_t FindMemoryHeapIndexByBits(uint32_t bits, VkMemoryPropertyFlags propertyFlags) const;
	uint32_t CheckMemoryHeapIndexByBits(uint32_t index, VkMemoryPropertyFlags propertyFlags) const;
	VkSurfaceFormatKHR FindSurfaceFormat() const;
	VkPresentModeKHR FindPresentMode() const;

	// buffer functions
	void CopyBuffers(VkDeviceSize size, VkBuffer srcBuffer, VkBuffer dstBuffer) const;
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation);
	void CreateBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation);
	void WriteBuffer(const void* data, VkDeviceSize size, VkBuffer buffer, VmaAllocation& allocation);

	// image functions
	void CopyImages(uint32_t width, uint32_t height, VkImage srcImage, VkImage dstImage) const;
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation);
	void CreateImage(const void* data, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation);
	void WriteImage(const void* data, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation);
};

// VulkanSwapchainInfo
struct VulkanSwapchainInfo
{
private:
	// base handles
	VulkanDeviceInfo* mDeviceInfo = nullptr;
	VkSurfaceKHR      mSurface = VK_NULL_HANDLE;

	// framebuffer formats
	VkSurfaceFormatKHR mSurfaceFormat{};
	VkFormat           mDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	VkPresentModeKHR   mPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	// framebuffer data
	VkImage                    mImageDepthStencil = VK_NULL_HANDLE;
	VkImageView                mImageViewDepthStencil = VK_NULL_HANDLE;
	VmaAllocation              mImageDepthStencilAllocation = VK_NULL_HANDLE;
	std::vector<VkImage>       mImageColors{};
	std::vector<VkImageView>   mImageViewColors{};
	std::vector<VkFramebuffer> mFramebuffers{};
public:
	// swapchain
	VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
	VkRenderPass   mRenderPass = VK_NULL_HANDLE;

	// current frame index
	uint32_t mCurrentFramebufferIndex = UINT32_MAX;

	// viewport size
	uint32_t mViewportWidth = UINT32_MAX;
	uint32_t mViewportHeight = UINT32_MAX;
	
	// Init/DeInit functions
	void Initialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface);
	void DeInitialize();
	void ReInitialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface);

	// frame processing
	VkFramebuffer BeginFrame(VkSemaphore signalSemaphore);
	void EndFrame(VkSemaphore waitSemaphore);
};

// VulkanPipelineInfo
class VulkanPipelineInfo
{
protected:
	// device info
	VulkanDeviceInfo* mDeviceInfo = nullptr;

	// vertex input descriptions
	std::vector<VkVertexInputBindingDescription>   mVertexBindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> mVertexAttributeDescriptions{};
	std::vector<VkDescriptorSetLayoutBinding>      mDescriptorSetLayoutBindings{};

	// shaders
	VkShaderModule mShaderModuleVS = VK_NULL_HANDLE;
	VkShaderModule mShaderModuleFS = VK_NULL_HANDLE;

	// MUST fill mVertexBindingDescriptions and mVertexAttributeDescriptions
	virtual void InitVertexInputDescriptions();

	// MUST create mPipelineLayout, mDescriptorSetLayout , mDescriptorPool and mDescriptorSet
	virtual void InitPipelineLayoutDescriptions();
public:
	// base handles (own)
	VkPipeline            mPipeline = VK_NULL_HANDLE;
	VkPipelineLayout      mPipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool      mDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet       mDescriptorSet = VK_NULL_HANDLE;

	// Init/DeInit functions
	void Initialize(VulkanDeviceInfo& deviceInfo, VkRenderPass renderPass, const char* pathVS, const char* pathFS);
	void DeInitialize();

	// bind functions
	void BindImageView(uint32_t binding, VkImageView imageView, VkSampler sampler);
	void BindUnifromBuffer(uint32_t binding, VkBuffer buffer);
};

// InitDeviceQueueCreateInfo
VkDeviceQueueCreateInfo InitDeviceQueueCreateInfo(uint32_t queueIndex);

// CreateSurface
VkSurfaceKHR CreateSurface(VkInstance instance, HWND hWnd);

// CreateImageView
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

// CreateFramebuffer
VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);

// CreateBuffer
VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);

// CreateSampler
VkSampler CreateSampler(VkDevice device);

// CreateShaderModuleFromFile
VkShaderModule CreateShaderModuleFromFile(VkDevice device, const char* fileName);

// AllocateCommandBuffer
VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

// CreateSemaphore
VkSemaphore CreateSemaphore(VkDevice device);

// QueueSubmit
void QueueSubmit(VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore);
