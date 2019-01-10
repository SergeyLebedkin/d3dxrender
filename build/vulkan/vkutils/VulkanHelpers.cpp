#include "VulkanHelpers.hpp"
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <array>

//////////////////////////////////////////////////////////////////////////
// VulkanInstanceInfo
//////////////////////////////////////////////////////////////////////////

// Initialize
void VulkanInstanceInfo::Initialize(
	const char* appName, uint32_t appVersion,
	const char* engineName, uint32_t engineVersion,
	std::vector<const char *> enabledLayerNames,
	std::vector<const char *> enabledExtensionNames,
	uint32_t apiVersion)
{
	// VkLayerProperties
	uint32_t layerPropertiesCount = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr));
	layerProperties.reserve(layerPropertiesCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties.data()));

	// VkExtensionProperties
	uint32_t extensionsPropertiesCount = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr));
	extensionProperties.reserve(extensionsPropertiesCount);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, extensionProperties.data()));

#ifdef _DEBUG
	// vkCreateDebugUtilsMessengerEXT and vkDestroyDebugUtilsMessengerEXT
	fnCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	fnDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	// VkDebugReportCallbackCreateInfoEXT
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags =
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	callbackCreateInfo.pUserData = nullptr;

	// fnCreateDebugReportCallbackEXT
	if (fnCreateDebugReportCallbackEXT)
		VK_CHECK(fnCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &debugReportCallbackEXT));
#endif

	// VkApplicationInfo
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = VK_NULL_HANDLE;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = engineName;
	applicationInfo.engineVersion = engineVersion;
	applicationInfo.apiVersion = apiVersion;

	// VkInstanceCreateInfo
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = VK_NULL_HANDLE;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();

	// vkCreateInstance
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance));
	assert(instance);

	// get physical devices count
	uint32_t physicalDevicesCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr));
	assert(physicalDevicesCount);
	// get physical devices list
	physicalDevices.resize(physicalDevicesCount);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices.data()));
}

// DeInitialize
void VulkanInstanceInfo::DeInitialize()
{
#ifdef _DEBUG
	if (fnDestroyDebugReportCallbackEXT)
		fnDestroyDebugReportCallbackEXT(instance, debugReportCallbackEXT, nullptr);
	debugReportCallbackEXT = VK_NULL_HANDLE;
#endif

	vkDestroyInstance(instance, VK_NULL_HANDLE);
	instance = VK_NULL_HANDLE;
}

// FindPhysicalDevice
VkPhysicalDevice VulkanInstanceInfo::FindPhysicalDevice(VkPhysicalDeviceType physicalDeviceType)
{	
	VkPhysicalDeviceFeatures physicalDeviceFeaturesGPU;
	VkPhysicalDeviceProperties physicalDevicePropertiesGPU;
	for (const auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		if (physicalDeviceProperties.deviceType & physicalDeviceType)
		{
			physicalDeviceFeaturesGPU = physicalDeviceFeatures;
			physicalDevicePropertiesGPU = physicalDeviceProperties;
			return physicalDevice;
		}
	}
	return VK_NULL_HANDLE;
}

#if _DEBUG
// MyDebugReportCallback
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstanceInfo::MyDebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char * pLayerPrefix,
	const char * pMessage,
	void * pUserData)
{
	std::cerr << pMessage << std::endl;
	return VK_TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////
// VulkanDeviceInfo
//////////////////////////////////////////////////////////////////////////

// Initialize
void VulkanDeviceInfo::Initialize(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
	VkPhysicalDeviceFeatures& physicalDeviceFeatures,
	std::vector<const char *>& enabledExtensionNames)
{
	// store parameters
	this->physicalDevice = physicalDevice;
	this->surface = surface;

	// VkPhysicalDeviceMemoryProperties
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	// get queue family properties count
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	assert(queueFamilyPropertiesCount);
	// get queue family properties list
	queueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	// get surface formats count
	uint32_t formatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);
	assert(formatsCount);
	// get surface formats list
	surfaceFormats.resize(formatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, surfaceFormats.data());

	// get present modes count
	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
	assert(presentModesCount);
	// get present modes list
	presentModes.resize(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());

	// find device local memory type index
	memoryDeviceLocalTypeIndex = FindMemoryHeapIndexByFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	memoryHostVisibleTypeIndex = FindMemoryHeapIndexByFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// get graphics, queue and transfer queue family property index
	queueFamilyIndexCompute = FindQueueFamilyIndexByFlags(VK_QUEUE_COMPUTE_BIT);
	queueFamilyIndexTransfer = FindQueueFamilyIndexByFlags(VK_QUEUE_TRANSFER_BIT);
	FindPresentQueueFamilyIndexes(queueFamilyIndexGraphics, queueFamilyIndexPresent);
	assert(queueFamilyIndexGraphics < UINT32_MAX);
	assert(queueFamilyIndexPresent < UINT32_MAX);

	// deviceQueueCreateInfos
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

	// VkDeviceQueueCreateInfo - graphics
	VkDeviceQueueCreateInfo deviceQueueCreateInfoGraphics = InitDeviceQueueCreateInfo(queueFamilyIndexGraphics);
	deviceQueueCreateInfos.push_back(deviceQueueCreateInfoGraphics);

	// VkDeviceQueueCreateInfo - compute
	VkDeviceQueueCreateInfo deviceQueueCreateInfoCompute = InitDeviceQueueCreateInfo(queueFamilyIndexCompute);
	if (queueFamilyIndexGraphics != queueFamilyIndexCompute)
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfoCompute);

	// VkDeviceQueueCreateInfo - transfer
	VkDeviceQueueCreateInfo deviceQueueCreateInfoTransfer = InitDeviceQueueCreateInfo(queueFamilyIndexTransfer);
	if (queueFamilyIndexGraphics != queueFamilyIndexTransfer)
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfoTransfer);

	// VkDeviceQueueCreateInfo - present
	VkDeviceQueueCreateInfo deviceQueueCreateInfoPresent = InitDeviceQueueCreateInfo(queueFamilyIndexPresent);
	if (queueFamilyIndexGraphics != queueFamilyIndexPresent)
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfoPresent);
		
	// VkDeviceCreateInfo
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = VK_NULL_HANDLE;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	// vkCreateImage
	VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &device));
	assert(device);

	// vkGetDeviceQueue
	vkGetDeviceQueue(device, queueFamilyIndexGraphics, 0, &queueGraphics);
	assert(queueGraphics);
	vkGetDeviceQueue(device, queueFamilyIndexCompute, 0, &queueCompute);
	assert(queueCompute);
	vkGetDeviceQueue(device, queueFamilyIndexTransfer, 0, &queueTransfer);
	assert(queueTransfer);
	vkGetDeviceQueue(device, queueFamilyIndexPresent, 0, &queuePresent);
	assert(queuePresent);

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.flags = 0;
	VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &allocator));

	// VkCommandPoolCreateInfo
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = VK_NULL_HANDLE;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndexGraphics;
	VK_CHECK(vkCreateCommandPool(device, &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool));
	assert(commandPool);
}

// DeInitialize
void VulkanDeviceInfo::DeInitialize()
{
	vkDestroyCommandPool(device, commandPool, VK_NULL_HANDLE);
	commandPool = VK_NULL_HANDLE;
	vmaDestroyAllocator(allocator);
	allocator = VK_NULL_HANDLE;
	vkDestroyDevice(device, VK_NULL_HANDLE);
	device = VK_NULL_HANDLE;
}

// FindPresentQueueFamilyIndex
void VulkanDeviceInfo::FindPresentQueueFamilyIndexes(uint32_t& graphicsIndex, uint32_t& presentIndex) const
{
	graphicsIndex = UINT32_MAX;
	presentIndex = UINT32_MAX;
	// find presentation queue family property index
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			graphicsIndex = i;
			// get present queue family property index
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
			if (presentSupport) 
			{
				graphicsIndex = i;
				presentIndex = i;
			}
		}
	}
	assert(graphicsIndex < UINT32_MAX);
	assert(presentIndex < UINT32_MAX);
}

// FindQueueFamilyIndexByFlags
uint32_t VulkanDeviceInfo::FindQueueFamilyIndexByFlags(uint32_t queueFlags) const
{
	// get graphics queue family property index
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// FindMemoryHeapIndexByFlags
uint32_t VulkanDeviceInfo::FindMemoryHeapIndexByFlags(VkMemoryPropertyFlags propertyFlags) const
{
	// find device local memory type index
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
		if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// FindMemoryHeapIndexByBits
uint32_t VulkanDeviceInfo::FindMemoryHeapIndexByBits(uint32_t bits, VkMemoryPropertyFlags propertyFlags) const
{
	// find device local memory type index
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
		if ((bits & 1 << i) && ((deviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// CheckMemoryHeapIndexByBits
uint32_t VulkanDeviceInfo::CheckMemoryHeapIndexByBits(uint32_t index, VkMemoryPropertyFlags propertyFlags) const
{
	assert(index < deviceMemoryProperties.memoryTypeCount);
	return ((deviceMemoryProperties.memoryTypes[index].propertyFlags & propertyFlags) == propertyFlags);
}

// FindSurfaceFormat
VkSurfaceFormatKHR VulkanDeviceInfo::FindSurfaceFormat() const
{
	// get default surface format
	if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
		VkSurfaceFormatKHR format;
		format.format = VK_FORMAT_R8G8B8A8_SNORM;
		format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		return format;
	};

	// try to find HDR format
	for (const auto& surfaceFormat : surfaceFormats)
		if ((surfaceFormat.format == VK_FORMAT_A2R10G10B10_SNORM_PACK32) || (surfaceFormat.format == VK_FORMAT_A2B10G10R10_SNORM_PACK32))
			return surfaceFormat;

	// try to find standard format
	for (const auto& surfaceFormat : surfaceFormats)
		if ((surfaceFormat.format == VK_FORMAT_R8G8B8A8_SNORM) || (surfaceFormat.format == VK_FORMAT_B8G8R8_SNORM))
			return surfaceFormat;

	// return default
	return surfaceFormats[0];
}

// FindPresentMode()
VkPresentModeKHR VulkanDeviceInfo::FindPresentMode() const
{
	// try to find MAILBOX mode
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;

	// try to find FIFO mode
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
			return presentMode;

	// return default
	return presentModes[0];
}

// CopyBuffers
void VulkanDeviceInfo::CopyBuffers(VkDeviceSize size, VkBuffer srcBuffer, VkBuffer dstBuffer) const
{
	// VkCommandBufferAllocateInfo
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	// VkCommandBuffer
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));
	assert(commandBuffer);

	// VkCommandBufferBeginInfo
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	// VkBufferCopy
	VkBufferCopy bufferCopy{};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

	// vkEndCommandBuffer
	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	// submit and wait
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VK_CHECK(vkQueueSubmit(queueGraphics, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(queueGraphics));

	// free command buffer
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

// AllocateBufferAndMemory
void VulkanDeviceInfo::AllocateBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation)
{
	// VkBufferCreateInfo
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// VmaAllocationCreateInfo
	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocCreateInfo.flags = 0;
	
	VK_CHECK(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, nullptr));
	assert(buffer);
	assert(allocation);
}

// UpdateBufferAndMemory
void VulkanDeviceInfo::UpdateBufferAndMemory(const void* data, VkDeviceSize size, VkBuffer buffer, VmaAllocation& allocation)
{
	// get memory buffer properties
	VmaAllocationInfo allocationInfo{};
	vmaGetAllocationInfo(allocator, allocation, &allocationInfo);
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(allocator, allocationInfo.memoryType, &memFlags);

	// if target device memory is host visible, then just map/unmap memory to device memory
	if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		void* mappedData = nullptr;
		vmaMapMemory(allocator, allocation, &mappedData);
		assert(mappedData);
		memcpy(mappedData, &data, size);
		vmaUnmapMemory(allocator, allocation);
	} 
	else // if target device memory is NOT host visible, then we need use staging buffer
	{
		// VkBufferCreateInfo
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// VmaAllocationCreateInfo
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		allocCreateInfo.flags = 0;

		// create staging buffer and memory
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
		VK_CHECK(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &stagingBuffer, &stagingBufferAlloc, nullptr));

		// create staging buffer and memory
		void* mappedData = nullptr;
		vmaMapMemory(allocator, stagingBufferAlloc, &mappedData);
		assert(mappedData);
		memcpy(mappedData, data, size);
		vmaUnmapMemory(allocator, stagingBufferAlloc);

		// copy buffers
		CopyBuffers(size, stagingBuffer, buffer);

		// destroy buffer and free memory
		vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
	}
}

//////////////////////////////////////////////////////////////////////////
// VulkanSwapchainInfo
//////////////////////////////////////////////////////////////////////////

// Initialize
void VulkanSwapchainInfo::Initialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface, VkRenderPass renderPass)
{
	// store parameters
	this->device = deviceInfo.device;
	this->allocator = deviceInfo.allocator;
	this->queuePresent = deviceInfo.queuePresent;
	this->surface = surface;
	this->renderPass = renderPass;

	// get parameters
	presentMode = deviceInfo.FindPresentMode();
	surfaceFormat = deviceInfo.FindSurfaceFormat();

	// VkSurfaceCapabilitiesKHR
	VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR{};
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceInfo.physicalDevice, surface, &surfaceCapabilitiesKHR));
	viewportWidth = surfaceCapabilitiesKHR.currentExtent.width;
	viewportHeight = surfaceCapabilitiesKHR.currentExtent.height;

	// VkSwapchainCreateInfoKHR
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR{};
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.surface = surface;
	swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.maxImageCount;
	swapchainCreateInfoKHR.imageFormat = surfaceFormat.format;
	swapchainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfoKHR.imageExtent.width = surfaceCapabilitiesKHR.currentExtent.width;
	swapchainCreateInfoKHR.imageExtent.height = surfaceCapabilitiesKHR.currentExtent.height;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
	swapchainCreateInfoKHR.pQueueFamilyIndices = VK_NULL_HANDLE;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKHR.presentMode = presentMode;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

	// vkCreateSwapchainKHR
	VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, nullptr, &swapchain));
	assert(swapchain);

	// swapChainImages
	uint32_t imageColorsCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageColorsCount, nullptr);
	imageColors.resize(imageColorsCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageColorsCount, imageColors.data());

	// mSwapChainImageViews
	imageViewColors.clear();
	imageViewColors.reserve(imageColorsCount);
	for (const auto& imageColor : imageColors) {
		// create image view
		VkImageView imageView = CreateImageView(device, imageColor, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		assert(imageView);

		// add image view
		imageViewColors.push_back(imageView);
	}

	// VmaAllocationCreateInfo
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationCreateInfo.flags = 0;

	// create image
	VkImageCreateInfo imageCreateInfo = InitImageCreateInfo(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, viewportWidth, viewportHeight);
	vmaCreateImage(deviceInfo.allocator, &imageCreateInfo, &allocationCreateInfo, &imageDepthStencil, &imageDepthStencilAllocation, VK_NULL_HANDLE);
	assert(imageDepthStencil);
	assert(imageDepthStencilAllocation);

	// VkImageView
	imageViewDepthStencil = CreateImageView(device, imageDepthStencil, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	assert(imageViewDepthStencil);

	// create framebuffers
	framebuffers.clear();
	framebuffers.reserve(imageColorsCount);
	for (const auto& imageViewColor : imageViewColors) {
		// create framebuffer
		std::vector<VkImageView> imageViews = { imageViewColor, imageViewDepthStencil };
		VkFramebuffer framebuffer = CreateFramebuffer(device, renderPass, imageViews, viewportWidth, viewportWidth);
		assert(framebuffer);

		// add framebuffer
		framebuffers.push_back(framebuffer);
	}
}

// DeInitialize
void VulkanSwapchainInfo::DeInitialize()
{
	vmaDestroyImage(allocator, imageDepthStencil, imageDepthStencilAllocation);
	imageDepthStencil = VK_NULL_HANDLE;
	imageDepthStencilAllocation = VK_NULL_HANDLE;
	vkDestroyImageView(device, imageViewDepthStencil, VK_NULL_HANDLE);
	imageViewDepthStencil = VK_NULL_HANDLE;
	for (const auto& imageViewColor : imageViewColors)
		vkDestroyImageView(device, imageViewColor, VK_NULL_HANDLE);
	for (const auto& framebuffer : framebuffers)
		vkDestroyFramebuffer(device, framebuffer, VK_NULL_HANDLE);
	vkDestroySwapchainKHR(device, swapchain, VK_NULL_HANDLE);
	swapchain = VK_NULL_HANDLE;
}

// ReInitialize
void VulkanSwapchainInfo::ReInitialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface, VkRenderPass renderPass)
{
	DeInitialize();
	Initialize(deviceInfo, surface, renderPass);
}

// BeginFrame
VkFramebuffer VulkanSwapchainInfo::BeginFrame(VkSemaphore signalSemaphore)
{
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &currentFramebufferIndex));
	return framebuffers[currentFramebufferIndex];
}

// EndFrame
void VulkanSwapchainInfo::EndFrame(VkSemaphore waitSemaphore)
{
	// VkPresentInfoKHR
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentFramebufferIndex;
	presentInfo.pResults = nullptr; // Optional
	VK_CHECK(vkQueuePresentKHR(queuePresent, &presentInfo));
	VK_CHECK(vkQueueWaitIdle(queuePresent));
}

//////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////

// InitDeviceQueueCreateInfo
VkDeviceQueueCreateInfo InitDeviceQueueCreateInfo(uint32_t queueIndex)
{
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.pNext = VK_NULL_HANDLE;
	deviceQueueCreateInfo.flags = 0;
	deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = nullptr;
	return deviceQueueCreateInfo;
}

// InitPipelineVertexInputState
VkPipelineVertexInputStateCreateInfo InitPipelineVertexInputStateCreateInfo(
	std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
	std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions)
{	
	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = VK_NULL_HANDLE;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexBindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
	return vertexInputState;
}

// CreateImageView
VkImageCreateInfo InitImageCreateInfo(VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = VK_NULL_HANDLE;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	return imageCreateInfo;
}

// CreateSurface
VkSurfaceKHR CreateSurface(VkInstance instance, HWND hWnd)
{
	// VkWin32SurfaceCreateInfoKHR
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR{};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.pNext = VK_NULL_HANDLE;
	win32SurfaceCreateInfoKHR.flags = 0;
	win32SurfaceCreateInfoKHR.hwnd = hWnd;
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);

	// vkCreateWin32SurfaceKHR
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfoKHR, nullptr, &surface));
	return surface;
}

// CreateImageView
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
{
	// VkImageViewCreateInfo
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = VK_NULL_HANDLE;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	// vkCreateImage
	VkImageView imageView = VK_NULL_HANDLE;
	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView));
	return imageView;
}

// CreateFramebuffer
VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height)
{
	// VkFramebufferCreateInfo 
	VkFramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = VK_NULL_HANDLE;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = (uint32_t)imageViews.size();
	framebufferCreateInfo.pAttachments = imageViews.data();
	framebufferCreateInfo.width = width;
	framebufferCreateInfo.height = height;
	framebufferCreateInfo.layers = 1;

	// vkCreateFramebuffer
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, VK_NULL_HANDLE, &framebuffer));
	return framebuffer;
}

// CreateBuffer
VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
	// VkBufferCreateInfo
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// vkCreateBuffer
	VkBuffer buffer = VK_NULL_HANDLE;
	VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));
	return buffer;
}

// CreateShaderModuleFromFile
VkShaderModule CreateShaderModuleFromFile(VkDevice device, const char* fileName)
{
	// open file
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	assert(file.is_open());

	// get file size
	auto fileSize = file.tellg();
	assert(fileSize > 0);

	// read data and close
	std::vector<char> code((size_t)fileSize);
	file.seekg(0, std::ios::beg);
	file.read(code.data(), fileSize);
	file.close();

	// VkShaderModuleCreateInfo - fragShaderModuleCreateInfo
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = (uint32_t)code.size();
	shaderModuleCreateInfo.pCode = (uint32_t *)code.data();

	// vkCreateShaderModule
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shaderModule));
	return shaderModule;
}

// CreateRenderPass
VkRenderPass CreateRenderPass(VkDevice device)
{
	// VkAttachmentDescription - color
	std::array<VkAttachmentDescription, 2> attachmentDescriptions;
	// color attachment
	attachmentDescriptions[0].flags = 0;
	attachmentDescriptions[0].format = VK_FORMAT_B8G8R8A8_UNORM;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// depth-stencil attachment
	attachmentDescriptions[1].flags = 0;
	attachmentDescriptions[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//////////////////////////////////////////////////////////////////////////

	// VkAttachmentReference - color
	std::array<VkAttachmentReference, 1> colorAttachmentReferences;
	colorAttachmentReferences[0].attachment = 0;
	colorAttachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// VkAttachmentReference - depth-stencil
	VkAttachmentReference depthStencilAttachmentReference{};
	depthStencilAttachmentReference.attachment = 1;
	depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// VkSubpassDescription - subpassDescriptions
	std::array<VkSubpassDescription, 1> subpassDescriptions;
	subpassDescriptions[0].flags = 0;
	subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptions[0].inputAttachmentCount = 0;
	subpassDescriptions[0].pInputAttachments = VK_NULL_HANDLE;
	subpassDescriptions[0].colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
	subpassDescriptions[0].pColorAttachments = colorAttachmentReferences.data();
	subpassDescriptions[0].pResolveAttachments = VK_NULL_HANDLE;
	subpassDescriptions[0].pDepthStencilAttachment = &depthStencilAttachmentReference;
	subpassDescriptions[0].preserveAttachmentCount = 0;
	subpassDescriptions[0].pPreserveAttachments = VK_NULL_HANDLE;

	//////////////////////////////////////////////////////////////////////////

	// VkRenderPassCreateInfo
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptions.size();
	renderPassCreateInfo.pSubpasses = subpassDescriptions.data();

	// vkCreateRenderPass
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VK_CHECK(vkCreateRenderPass(device, &renderPassCreateInfo, VK_NULL_HANDLE, &renderPass));
	return renderPass;
}

// CreatePipelineLayout
VkPipelineLayout CreatePipelineLayout(VkDevice device)
{
	// VkPipelineLayoutCreateInfo
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = VK_NULL_HANDLE;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = VK_NULL_HANDLE; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = VK_NULL_HANDLE; // Optional

	// vkCreatePipelineLayout;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, VK_NULL_HANDLE, &pipelineLayout));
	return pipelineLayout;
}

// CreateGraphicsPipeline
VkPipeline CreateGraphicsPipeline(
	VkDevice device,
	VkPipelineVertexInputStateCreateInfo vertexInputState,
	VkShaderModule vs, VkShaderModule fs,
	VkPipelineLayout layout,
	VkRenderPass renderPass,
	uint32_t width, uint32_t height)
{
	// VkPipelineShaderStageCreateInfo - shaderStages
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	// vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = VK_NULL_HANDLE;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vs;
	shaderStages[0].pName = "main";
	shaderStages[0].pSpecializationInfo = VK_NULL_HANDLE;
	// fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = VK_NULL_HANDLE;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fs;
	shaderStages[1].pName = "main";
	shaderStages[1].pSpecializationInfo = VK_NULL_HANDLE;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineInputAssemblyStateCreateInfo - inputAssemblyState
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext = VK_NULL_HANDLE;
	inputAssemblyState.flags = 0;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineTessellationStateCreateInfo - tessellationState
	VkPipelineTessellationStateCreateInfo tessellationState{};
	tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellationState.pNext = VK_NULL_HANDLE;
	tessellationState.flags = 0;
	tessellationState.patchControlPoints = 3;

	//////////////////////////////////////////////////////////////////////////

	// VkViewport - viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// VkRect2D - scissor
	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = width;
	scissor.extent.height = height;

	// VkPipelineViewportStateCreateInfo - viewportState
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = VK_NULL_HANDLE;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineRasterizationStateCreateInfo - rasterizationState
	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.pNext = VK_NULL_HANDLE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;
	rasterizationState.lineWidth = 1.0f;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineMultisampleStateCreateInfo - multisampleState
	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = VK_NULL_HANDLE;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = VK_NULL_HANDLE;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineDepthStencilStateCreateInfo - depthStencilState
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext = VK_NULL_HANDLE;
	depthStencilState.flags = 0;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_TRUE;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;
	depthStencilState.front.compareMask = 0;
	depthStencilState.front.writeMask = 0;
	depthStencilState.front.reference = 0;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_NEVER;
	depthStencilState.back.compareMask = 0;
	depthStencilState.back.writeMask = 0;
	depthStencilState.back.reference = 0;
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineColorBlendAttachmentState - attachments
	std::array<VkPipelineColorBlendAttachmentState, 1> attachments;
	attachments[0].blendEnable = VK_FALSE;
	attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
	attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
	attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	// VkPipelineColorBlendStateCreateInfo - colorBlendState
	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = VK_NULL_HANDLE;
	colorBlendState.flags = 0;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlendState.attachmentCount = (uint32_t)attachments.size();
	colorBlendState.pAttachments = attachments.data();
	colorBlendState.blendConstants[0] = 0.0f; // Optional
	colorBlendState.blendConstants[1] = 0.0f; // Optional
	colorBlendState.blendConstants[2] = 0.0f; // Optional
	colorBlendState.blendConstants[3] = 0.0f; // Optional

	//////////////////////////////////////////////////////////////////////////

	// VkDynamicState - dynamicStates
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	// VkPipelineDynamicStateCreateInfo - dynamicState
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = VK_NULL_HANDLE;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	//////////////////////////////////////////////////////////////////////////

	// VkGraphicsPipelineCreateInfo
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.flags = 0;
	createInfo.stageCount = (uint32_t)shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.pVertexInputState = &vertexInputState;
	createInfo.pInputAssemblyState = &inputAssemblyState;
	createInfo.pTessellationState = &tessellationState;
	createInfo.pViewportState = &viewportState;
	createInfo.pRasterizationState = &rasterizationState;
	createInfo.pMultisampleState = &multisampleState;
	createInfo.pDepthStencilState = &depthStencilState;
	createInfo.pColorBlendState = &colorBlendState;
	createInfo.pDynamicState = &dynamicState;
	createInfo.layout = layout;
	createInfo.renderPass = renderPass;
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = 0;

	// vkCreateGraphicsPipelines
	VkPipeline pipeline = VK_NULL_HANDLE;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &pipeline));
	return pipeline;
}

// CreateCommandPool
VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueIndex)
{
	// VkCommandPoolCreateInfo
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = VK_NULL_HANDLE;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = queueIndex;

	// vkCreateCommandPool
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VK_CHECK(vkCreateCommandPool(device, &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool));
	return commandPool;
}

// AllocateCommandBuffer
VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel)
{
	// VkCommandBufferAllocateInfo
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = 1;

	// vkAllocateCommandBuffers
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));
	return commandBuffer;
}

// CreateSemaphore
VkSemaphore CreateSemaphore(VkDevice device)
{
	// VkSemaphoreCreateInfo
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	semaphoreCreateInfo.flags = 0;

	// vkCreateSemaphore
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore));
	return semaphore;
}

// QueueSubmit
void QueueSubmit(VkQueue queue, VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
{
	// VkSubmitInfo
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;
	submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// vkQueueSubmit
	VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
}