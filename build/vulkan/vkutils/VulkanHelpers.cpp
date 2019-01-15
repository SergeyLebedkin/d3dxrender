#include "VulkanHelpers.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
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
	mLayerProperties.reserve(layerPropertiesCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerPropertiesCount, mLayerProperties.data()));

	// VkExtensionProperties
	uint32_t extensionsPropertiesCount = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr));
	mExtensionProperties.reserve(extensionsPropertiesCount);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, mExtensionProperties.data()));

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
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &mInstance));
	assert(mInstance);

	// get physical devices count
	uint32_t physicalDevicesCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr));
	assert(physicalDevicesCount);
	// get physical devices list
	mPhysicalDevices.resize(physicalDevicesCount);
	VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, mPhysicalDevices.data()));

	// get physical device GPU
	mPhysicalDeviceGPU = FindPhysicalDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);;

#ifdef _DEBUG
	// vkCreateDebugUtilsMessengerEXT and vkDestroyDebugUtilsMessengerEXT
	fnCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
	fnDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT");

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
		VK_CHECK(fnCreateDebugReportCallbackEXT(mInstance, &callbackCreateInfo, nullptr, &debugReportCallbackEXT));
#endif
}

// DeInitialize
void VulkanInstanceInfo::DeInitialize()
{
#ifdef _DEBUG
	if (fnDestroyDebugReportCallbackEXT)
		fnDestroyDebugReportCallbackEXT(mInstance, debugReportCallbackEXT, nullptr);
	debugReportCallbackEXT = VK_NULL_HANDLE;
#endif

	// clear physical device GPU
	mPhysicalDeviceGPU = VK_NULL_HANDLE;

	// destroy instance
	vkDestroyInstance(mInstance, VK_NULL_HANDLE);
	mInstance = VK_NULL_HANDLE;
}

// FindPhysicalDevice
VkPhysicalDevice VulkanInstanceInfo::FindPhysicalDevice(VkPhysicalDeviceType physicalDeviceType)
{	
	VkPhysicalDeviceFeatures physicalDeviceFeaturesGPU;
	VkPhysicalDeviceProperties physicalDevicePropertiesGPU;
	for (const auto& physicalDevice : mPhysicalDevices)
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
	mPhysicalDevice = physicalDevice;
	mSurface = surface;

	// VkPhysicalDeviceMemoryProperties
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mDeviceMemoryProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &mDeviceFeatures);
	vkGetPhysicalDeviceProperties(physicalDevice, &mDeviceProperties);

	// get queue family properties count
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	assert(queueFamilyPropertiesCount);
	// get queue family properties list
	mQueueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, mQueueFamilyProperties.data());

	// get surface formats count
	uint32_t formatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatsCount, nullptr);
	assert(formatsCount);
	// get surface formats list
	mSurfaceFormats.resize(formatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatsCount, mSurfaceFormats.data());

	// get present modes count
	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
	assert(presentModesCount);
	// get present modes list
	mPresentModes.resize(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, mPresentModes.data());

	// find device local memory type index
	mMemoryDeviceLocalTypeIndex = FindMemoryHeapIndexByFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	mMemoryHostVisibleTypeIndex = FindMemoryHeapIndexByFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// get graphics, queue and transfer queue family property index
	mQueueFamilyIndexCompute = FindQueueFamilyIndexByFlags(VK_QUEUE_COMPUTE_BIT);
	mQueueFamilyIndexTransfer = FindQueueFamilyIndexByFlags(VK_QUEUE_TRANSFER_BIT);
	FindPresentQueueFamilyIndexes(mQueueFamilyIndexGraphics, mQueueFamilyIndexPresent);
	assert(mQueueFamilyIndexGraphics < UINT32_MAX);
	assert(mQueueFamilyIndexPresent < UINT32_MAX);

	// deviceQueueCreateInfos
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

	// VkDeviceQueueCreateInfo - graphics
	VkDeviceQueueCreateInfo deviceQueueCreateInfoGraphics = InitDeviceQueueCreateInfo(mQueueFamilyIndexGraphics);
	deviceQueueCreateInfos.push_back(deviceQueueCreateInfoGraphics);

	// VkDeviceQueueCreateInfo - compute
	VkDeviceQueueCreateInfo deviceQueueCreateInfoCompute = InitDeviceQueueCreateInfo(mQueueFamilyIndexCompute);
	if (mQueueFamilyIndexGraphics != mQueueFamilyIndexCompute)
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfoCompute);

	// VkDeviceQueueCreateInfo - transfer
	VkDeviceQueueCreateInfo deviceQueueCreateInfoTransfer = InitDeviceQueueCreateInfo(mQueueFamilyIndexTransfer);
	if (mQueueFamilyIndexGraphics != mQueueFamilyIndexTransfer)
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfoTransfer);

	// VkDeviceQueueCreateInfo - present
	VkDeviceQueueCreateInfo deviceQueueCreateInfoPresent = InitDeviceQueueCreateInfo(mQueueFamilyIndexPresent);
	if (mQueueFamilyIndexGraphics != mQueueFamilyIndexPresent)
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

	// vkCreateDevice
	VK_CHECK(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &mDevice));
	assert(mDevice);

	// vkGetDeviceQueue
	vkGetDeviceQueue(mDevice, mQueueFamilyIndexGraphics, 0, &mQueueGraphics);
	assert(mQueueGraphics);
	vkGetDeviceQueue(mDevice, mQueueFamilyIndexCompute, 0, &mQueueCompute);
	assert(mQueueCompute);
	vkGetDeviceQueue(mDevice, mQueueFamilyIndexTransfer, 0, &mQueueTransfer);
	assert(mQueueTransfer);
	vkGetDeviceQueue(mDevice, mQueueFamilyIndexPresent, 0, &mQueuePresent);
	assert(mQueuePresent);

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = mDevice;
	allocatorCreateInfo.flags = 0;
	VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &mAllocator));

	// VkCommandPoolCreateInfo
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = VK_NULL_HANDLE;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = mQueueFamilyIndexGraphics;
	VK_CHECK(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &mCommandPool));
	assert(mCommandPool);
}

// DeInitialize
void VulkanDeviceInfo::DeInitialize()
{
	vkDestroyCommandPool(mDevice, mCommandPool, VK_NULL_HANDLE);
	mCommandPool = VK_NULL_HANDLE;
	vmaDestroyAllocator(mAllocator);
	mAllocator = VK_NULL_HANDLE;
	vkDestroyDevice(mDevice, VK_NULL_HANDLE);
	mDevice = VK_NULL_HANDLE;
}

// FindPresentQueueFamilyIndex
void VulkanDeviceInfo::FindPresentQueueFamilyIndexes(uint32_t& graphicsIndex, uint32_t& presentIndex) const
{
	graphicsIndex = UINT32_MAX;
	presentIndex = UINT32_MAX;
	// find presentation queue family property index
	for (uint32_t i = 0; i < mQueueFamilyProperties.size(); i++)
	{
		if ((mQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			graphicsIndex = i;
			// get present queue family property index
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &presentSupport);
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
	for (uint32_t i = 0; i < mQueueFamilyProperties.size(); i++)
		if ((mQueueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// FindMemoryHeapIndexByFlags
uint32_t VulkanDeviceInfo::FindMemoryHeapIndexByFlags(VkMemoryPropertyFlags propertyFlags) const
{
	// find device local memory type index
	for (uint32_t i = 0; i < mDeviceMemoryProperties.memoryTypeCount; i++)
		if ((mDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// FindMemoryHeapIndexByBits
uint32_t VulkanDeviceInfo::FindMemoryHeapIndexByBits(uint32_t bits, VkMemoryPropertyFlags propertyFlags) const
{
	// find device local memory type index
	for (uint32_t i = 0; i < mDeviceMemoryProperties.memoryTypeCount; i++)
		if ((bits & 1 << i) && ((mDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
			return i;
	assert(0);
	// return default
	return UINT32_MAX;
}

// CheckMemoryHeapIndexByBits
uint32_t VulkanDeviceInfo::CheckMemoryHeapIndexByBits(uint32_t index, VkMemoryPropertyFlags propertyFlags) const
{
	assert(index < mDeviceMemoryProperties.memoryTypeCount);
	return ((mDeviceMemoryProperties.memoryTypes[index].propertyFlags & propertyFlags) == propertyFlags);
}

// FindSurfaceFormat
VkSurfaceFormatKHR VulkanDeviceInfo::FindSurfaceFormat() const
{
	// get default surface format
	if ((mSurfaceFormats.size() == 1) && (mSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
		VkSurfaceFormatKHR format;
		format.format = VK_FORMAT_R8G8B8A8_SNORM;
		format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		return format;
	};

	// try to find HDR format
	for (const auto& surfaceFormat : mSurfaceFormats)
		if ((surfaceFormat.format == VK_FORMAT_A2R10G10B10_SNORM_PACK32) || (surfaceFormat.format == VK_FORMAT_A2B10G10R10_SNORM_PACK32))
			return surfaceFormat;

	// try to find standard format
	for (const auto& surfaceFormat : mSurfaceFormats)
		if ((surfaceFormat.format == VK_FORMAT_R8G8B8A8_SNORM) || (surfaceFormat.format == VK_FORMAT_B8G8R8_SNORM))
			return surfaceFormat;

	// return default
	return mSurfaceFormats[0];
}

// FindPresentMode()
VkPresentModeKHR VulkanDeviceInfo::FindPresentMode() const
{
	// try to find MAILBOX mode
	for (const auto& presentMode : mPresentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;

	// try to find FIFO mode
	for (const auto& presentMode : mPresentModes)
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
			return presentMode;

	// return default
	return mPresentModes[0];
}

// CopyBuffers
void VulkanDeviceInfo::CopyBuffers(VkDeviceSize size, VkBuffer srcBuffer, VkBuffer dstBuffer) const
{
	// VkCommandBufferAllocateInfo
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = mCommandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	// VkCommandBuffer
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &commandBuffer));
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
	VK_CHECK(vkQueueSubmit(mQueueGraphics, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(mQueueGraphics));

	// free command buffer
	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

// CreateBuffer (without initialization)
void VulkanDeviceInfo::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation)
{
	// check size
	assert(size);

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

	// vmaCreateBuffer
	VK_CHECK(vmaCreateBuffer(mAllocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, VK_NULL_HANDLE));
	assert(buffer);
	assert(allocation);
}

// CreateBuffer (with initialization)
void VulkanDeviceInfo::CreateBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation)
{
	// check data
	assert(data);
	// create buffer
	CreateBuffer(size, usage, buffer, allocation);
	// write buffer
	WriteBuffer(data, size, buffer, allocation);
}

// WriteBuffer
void VulkanDeviceInfo::WriteBuffer(const void* data, VkDeviceSize size, VkBuffer buffer, VmaAllocation& allocation)
{
	// check data
	assert(data);

	// get memory buffer properties
	VmaAllocationInfo allocationInfo{};
	vmaGetAllocationInfo(mAllocator, allocation, &allocationInfo);
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(mAllocator, allocationInfo.memoryType, &memFlags);

	// if target device memory is host visible, then just map/unmap memory to device memory
	if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		void* mappedData = nullptr;
		vmaMapMemory(mAllocator, allocation, &mappedData);
		assert(mappedData);
		memcpy(mappedData, &data, size);
		vmaUnmapMemory(mAllocator, allocation);
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
		VK_CHECK(vmaCreateBuffer(mAllocator, &bufferCreateInfo, &allocCreateInfo, &stagingBuffer, &stagingBufferAlloc, VK_NULL_HANDLE));

		// map staging buffer and memory
		void* mappedData = nullptr;
		vmaMapMemory(mAllocator, stagingBufferAlloc, &mappedData);
		assert(mappedData);
		memcpy(mappedData, data, size);
		vmaUnmapMemory(mAllocator, stagingBufferAlloc);

		// copy buffers
		CopyBuffers(size, stagingBuffer, buffer);

		// destroy buffer and free memory
		vmaDestroyBuffer(mAllocator, stagingBuffer, stagingBufferAlloc);
	}
}

// CopyImages
void VulkanDeviceInfo::CopyImages(uint32_t width, uint32_t height, VkImage srcImage, VkImage dstImage) const
{
	// VkCommandBufferAllocateInfo
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = mCommandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	// VkCommandBuffer
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &commandBuffer));
	assert(commandBuffer);

	// VkCommandBufferBeginInfo
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	// VkImageMemoryBarrier
	VkImageMemoryBarrier imgMemBarrier{};
	imgMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemBarrier.pNext = VK_NULL_HANDLE;
	imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgMemBarrier.subresourceRange.baseMipLevel = 0;
	imgMemBarrier.subresourceRange.levelCount = 1;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;

	imgMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imgMemBarrier.image = srcImage;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);

	imgMemBarrier.srcAccessMask = 0;
	imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgMemBarrier.image = dstImage;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);

	// VkImageCopy
	VkImageCopy imageCopy{};
	imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopy.srcSubresource.mipLevel = 0;
	imageCopy.srcSubresource.baseArrayLayer = 0;
	imageCopy.srcSubresource.layerCount = 1;
	imageCopy.srcOffset.x = 0;
	imageCopy.srcOffset.y = 0;
	imageCopy.srcOffset.z = 0;
	imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopy.dstSubresource.mipLevel = 0;
	imageCopy.dstSubresource.baseArrayLayer = 0;
	imageCopy.dstSubresource.layerCount = 1;
	imageCopy.dstOffset.x = 0;
	imageCopy.dstOffset.y = 0;
	imageCopy.dstOffset.z = 0;
	imageCopy.extent.width = width;
	imageCopy.extent.height = height;
	imageCopy.extent.depth = 1;

	// vkCmdCopyImage
	vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

	imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgMemBarrier.image = dstImage;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);

	// vkEndCommandBuffer
	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	// submit and wait
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VK_CHECK(vkQueueSubmit(mQueueGraphics, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(mQueueGraphics));

	// free command buffer
	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

// CreateImage
void VulkanDeviceInfo::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation)
{
	// check width and height
	assert(width);
	assert(height);

	// VkImageCreateInfo
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
	imageCreateInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// VmaAllocationCreateInfo
	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocCreateInfo.flags = 0;

	// vmaCreateImage
	VK_CHECK(vmaCreateImage(mAllocator, &imageCreateInfo, &allocCreateInfo, &image, &allocation, VK_NULL_HANDLE));
	assert(image);
}

// CreateImage
void VulkanDeviceInfo::CreateImage(const void* data, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation)
{
	// create image
	CreateImage(width, height, format, usage, image, allocation);
	// write image
	WriteImage(data, width, height, format, usage, image, allocation);
}

// WriteImage
void VulkanDeviceInfo::WriteImage(const void* data, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation)
{
	// get memory buffer properties
	VmaAllocationInfo allocationInfo{};
	vmaGetAllocationInfo(mAllocator, allocation, &allocationInfo);
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(mAllocator, allocationInfo.memoryType, &memFlags);

	// if target device memory is host visible, then just map/unmap memory to device memory
	if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		void* mappedData = nullptr;
		vmaMapMemory(mAllocator, allocation, &mappedData);
		assert(mappedData);
		memcpy(mappedData, &data, width * height * 4);
		vmaUnmapMemory(mAllocator, allocation);
	}
	else // if target device memory is NOT host visible, then we need use staging image
	{
		// VkImageCreateInfo
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
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
		imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		// VmaAllocationCreateInfo
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		allocCreateInfo.flags = 0;

		// create staging image and memory
		VkImage stagingImage = VK_NULL_HANDLE;
		VmaAllocation stagingImageAlloc = VK_NULL_HANDLE;

		// vmaCreateImage
		VK_CHECK(vmaCreateImage(mAllocator, &imageCreateInfo, &allocCreateInfo, &stagingImage, &stagingImageAlloc, VK_NULL_HANDLE));
		assert(stagingImage);
		assert(stagingImageAlloc);

		// map staging buffer and memory
		void* mappedData = nullptr;
		vmaMapMemory(mAllocator, stagingImageAlloc, &mappedData);
		assert(mappedData);
		memcpy(mappedData, data, width * height * 4);
		vmaUnmapMemory(mAllocator, stagingImageAlloc);

		// copy images
		CopyImages(width, height, stagingImage, image);

		// destroy buffer and free memory
		vmaDestroyImage(mAllocator, stagingImage, stagingImageAlloc);
	}
}

// CreateSemaphore
VkSemaphore VulkanDeviceInfo::CreateSemaphore()
{
	// VkSemaphoreCreateInfo
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	semaphoreCreateInfo.flags = 0;

	// vkCreateSemaphore
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore));
	return semaphore;
}

// CreateSampler
VkSampler VulkanDeviceInfo::CreateSampler(VkFilter filter, VkSamplerAddressMode samplerAddressMode)
{
	// VkSamplerCreateInfo
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = VK_NULL_HANDLE;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = filter;
	samplerCreateInfo.minFilter = filter;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = samplerAddressMode;
	samplerCreateInfo.addressModeV = samplerAddressMode;
	samplerCreateInfo.addressModeW = samplerAddressMode;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = FLT_MAX;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	// vkCreateSampler
	VkSampler sampler;
	VK_CHECK(vkCreateSampler(mDevice, &samplerCreateInfo, VK_NULL_HANDLE, &sampler));
	return sampler;
}

// CreateImageView
VkImageView VulkanDeviceInfo::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
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
	VK_CHECK(vkCreateImageView(mDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView));
	return imageView;
}

// AllocateCommandBuffer
VkCommandBuffer VulkanDeviceInfo::AllocateCommandBuffer(VkCommandBufferLevel commandBufferLevel)
{
	// VkCommandBufferAllocateInfo
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
	commandBufferAllocateInfo.commandPool = mCommandPool;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = 1;

	// vkAllocateCommandBuffers
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, &commandBuffer));
	return commandBuffer;
}

// CreateFramebuffer
VkFramebuffer VulkanDeviceInfo::CreateFramebuffer(VkRenderPass renderPass, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height)
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
	VK_CHECK(vkCreateFramebuffer(mDevice, &framebufferCreateInfo, VK_NULL_HANDLE, &framebuffer));
	return framebuffer;
}

//////////////////////////////////////////////////////////////////////////
// VulkanSwapchainInfo
//////////////////////////////////////////////////////////////////////////

// Initialize
void VulkanSwapchainInfo::Initialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface)
{
	// store parameters
	mDeviceInfo = &deviceInfo;
	mSurface = surface;
	assert(mDeviceInfo->mDevice);
	assert(surface);

	// get parameters
	mPresentMode = mDeviceInfo->FindPresentMode();
	mSurfaceFormat = mDeviceInfo->FindSurfaceFormat();

	// VkSurfaceCapabilitiesKHR
	VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR{};
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDeviceInfo->mPhysicalDevice, surface, &surfaceCapabilitiesKHR));
	mViewportWidth = surfaceCapabilitiesKHR.currentExtent.width;
	mViewportHeight = surfaceCapabilitiesKHR.currentExtent.height;

	// VkSwapchainCreateInfoKHR
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR{};
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.surface = surface;
	swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.maxImageCount;
	swapchainCreateInfoKHR.imageFormat = mSurfaceFormat.format;
	swapchainCreateInfoKHR.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainCreateInfoKHR.imageExtent.width = surfaceCapabilitiesKHR.currentExtent.width;
	swapchainCreateInfoKHR.imageExtent.height = surfaceCapabilitiesKHR.currentExtent.height;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
	swapchainCreateInfoKHR.pQueueFamilyIndices = VK_NULL_HANDLE;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKHR.presentMode = mPresentMode;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

	// vkCreateSwapchainKHR
	VK_CHECK(vkCreateSwapchainKHR(mDeviceInfo->mDevice, &swapchainCreateInfoKHR, nullptr, &mSwapchain));
	assert(mSwapchain);

	//////////////////////////////////////////////////////////////////////////

	// swapChainImages
	uint32_t imageColorsCount = 0;
	vkGetSwapchainImagesKHR(mDeviceInfo->mDevice, mSwapchain, &imageColorsCount, nullptr);
	mImageColors.resize(imageColorsCount);
	vkGetSwapchainImagesKHR(mDeviceInfo->mDevice, mSwapchain, &imageColorsCount, mImageColors.data());

	// mSwapChainImageViews
	mImageViewColors.clear();
	mImageViewColors.reserve(imageColorsCount);
	for (const auto& imageColor : mImageColors) {
		// create image view
		VkImageView imageView = mDeviceInfo->CreateImageView(imageColor, mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		assert(imageView);

		// add image view
		mImageViewColors.push_back(imageView);
	}

	//////////////////////////////////////////////////////////////////////////

	// VmaAllocationCreateInfo
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationCreateInfo.flags = 0;

	// create image
	mDeviceInfo->CreateImage(mViewportWidth, mViewportHeight, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, mImageDepthStencil, mImageDepthStencilAllocation);
	assert(mImageDepthStencil);
	assert(mImageDepthStencilAllocation);

	// VkImageView
	mImageViewDepthStencil = mDeviceInfo->CreateImageView(mImageDepthStencil, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	assert(mImageViewDepthStencil);

	//////////////////////////////////////////////////////////////////////////

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

	// VkRenderPassCreateInfo
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = (uint32_t)subpassDescriptions.size();
	renderPassCreateInfo.pSubpasses = subpassDescriptions.data();

	// vkCreateRenderPass
	VK_CHECK(vkCreateRenderPass(mDeviceInfo->mDevice, &renderPassCreateInfo, VK_NULL_HANDLE, &mRenderPass));

	//////////////////////////////////////////////////////////////////////////

	// create framebuffers
	mFramebuffers.clear();
	mFramebuffers.reserve(imageColorsCount);
	for (const auto& imageViewColor : mImageViewColors) {
		// create framebuffer
		std::vector<VkImageView> imageViews = { imageViewColor, mImageViewDepthStencil };
		VkFramebuffer framebuffer = mDeviceInfo->CreateFramebuffer(mRenderPass, imageViews, mViewportWidth, mViewportWidth);
		assert(framebuffer);

		// add framebuffer
		mFramebuffers.push_back(framebuffer);
	}
}

// DeInitialize
void VulkanSwapchainInfo::DeInitialize()
{
	// destroy framebuffers
	for (const auto& framebuffer : mFramebuffers)
		vkDestroyFramebuffer(mDeviceInfo->mDevice, framebuffer, VK_NULL_HANDLE);

	// destroy render pass
	vkDestroyRenderPass(mDeviceInfo->mDevice, mRenderPass, VK_NULL_HANDLE);
	mRenderPass = VK_NULL_HANDLE;

	// destroy image depth stencil
	vmaDestroyImage(mDeviceInfo->mAllocator, mImageDepthStencil, mImageDepthStencilAllocation);
	mImageDepthStencil = VK_NULL_HANDLE;
	mImageDepthStencilAllocation = VK_NULL_HANDLE;

	// destroy image views
	vkDestroyImageView(mDeviceInfo->mDevice, mImageViewDepthStencil, VK_NULL_HANDLE);
	mImageViewDepthStencil = VK_NULL_HANDLE;

	// destroy image views
	for (const auto& imageViewColor : mImageViewColors)
		vkDestroyImageView(mDeviceInfo->mDevice, imageViewColor, VK_NULL_HANDLE);

	// destroy swapchain
	vkDestroySwapchainKHR(mDeviceInfo->mDevice, mSwapchain, VK_NULL_HANDLE);
	mSwapchain = VK_NULL_HANDLE;
}

// ReInitialize
void VulkanSwapchainInfo::ReInitialize(VulkanDeviceInfo& deviceInfo, VkSurfaceKHR surface)
{
	DeInitialize();
	Initialize(deviceInfo, surface);
}

// BeginFrame
VkFramebuffer VulkanSwapchainInfo::BeginFrame(VkSemaphore signalSemaphore)
{
	VK_CHECK(vkAcquireNextImageKHR(mDeviceInfo->mDevice, mSwapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &mCurrentFramebufferIndex));
	return mFramebuffers[mCurrentFramebufferIndex];
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
	presentInfo.pSwapchains = &mSwapchain;
	presentInfo.pImageIndices = &mCurrentFramebufferIndex;
	presentInfo.pResults = nullptr; // Optional
	VK_CHECK(vkQueuePresentKHR(mDeviceInfo->mQueuePresent, &presentInfo));
	VK_CHECK(vkQueueWaitIdle(mDeviceInfo->mQueuePresent));
}

//////////////////////////////////////////////////////////////////////////
// VulkanPipelineInfo
//////////////////////////////////////////////////////////////////////////

// InitVertexInputDescriptions
void VulkanPipelineInfo::InitVertexInputDescriptions()
{
	// VkVertexInputBindingDescription
	mVertexBindingDescriptions = {
		{ 0, 10 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX },
	};

	// VkVertexInputAttributeDescription
	mVertexAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT,  0 }, // position
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 16 }, // color
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT      , 32 }, // texCoord
	};
}

// InitPipelineLayoutHandles
void VulkanPipelineInfo::InitPipelineLayoutDescriptions()
{
	// VkDescriptorSetLayoutBinding
	mDescriptorSetLayoutBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE }, // texture
		{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_VERTEX_BIT  , VK_NULL_HANDLE }, // buffer
	};
}

// Initialize
void VulkanPipelineInfo::Initialize(VulkanDeviceInfo& deviceInfo, VkRenderPass renderPass, const char* pathVS, const char* pathFS) 
{
	// check device info and paths
	assert(renderPass);
	assert(pathVS);
	assert(pathFS);

	// copy handles
	mDeviceInfo = &deviceInfo;

	// init handles
	InitVertexInputDescriptions();
	InitPipelineLayoutDescriptions();

	// create shaders
	mShaderModuleVS = CreateShaderModuleFromFile(mDeviceInfo->mDevice, pathVS);
	assert(mShaderModuleVS);
	mShaderModuleFS = CreateShaderModuleFromFile(mDeviceInfo->mDevice, pathFS);
	assert(mShaderModuleFS);

	// VkPipelineShaderStageCreateInfo - shaderStages
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	// vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = VK_NULL_HANDLE;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = mShaderModuleVS;
	shaderStages[0].pName = "main";
	shaderStages[0].pSpecializationInfo = VK_NULL_HANDLE;
	// fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = VK_NULL_HANDLE;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = mShaderModuleFS;
	shaderStages[1].pName = "main";
	shaderStages[1].pSpecializationInfo = VK_NULL_HANDLE;

	//////////////////////////////////////////////////////////////////////////

	// VkPipelineVertexInputStateCreateInfo - vertexInputState
	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = VK_NULL_HANDLE;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = (uint32_t)mVertexBindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions = mVertexBindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = (uint32_t)mVertexAttributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions = mVertexAttributeDescriptions.data();

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
	viewport.width = 0.0f;
	viewport.height = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// VkRect2D - scissor
	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = 0;
	scissor.extent.height = 0;

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
	//rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
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

	// VkPipelineLayoutCreateInfo
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = VK_NULL_HANDLE;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)mDescriptorSetLayoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = mDescriptorSetLayoutBindings.data();

	// vkCreatePipelineLayout
	VK_CHECK(vkCreateDescriptorSetLayout(mDeviceInfo->mDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &mDescriptorSetLayout));

	//////////////////////////////////////////////////////////////////////////

		// VkPipelineLayoutCreateInfo
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = VK_NULL_HANDLE;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = VK_NULL_HANDLE; // Optional

	// vkCreatePipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(mDeviceInfo->mDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &mPipelineLayout));

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
	createInfo.layout = mPipelineLayout;
	createInfo.renderPass = renderPass;
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = 0;

	// vkCreateGraphicsPipelines
	VK_CHECK(vkCreateGraphicsPipelines(mDeviceInfo->mDevice, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &mPipeline));

	// get descriptor type counts
	std::map<VkDescriptorType, uint32_t> descriptorTypeCounts{};
	for (const auto& binding : mDescriptorSetLayoutBindings)
		descriptorTypeCounts[binding.descriptorType]++;

	// fill descriptor pool sizes
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes{};
	for (const auto& descriptorTypeCount : descriptorTypeCounts)
		descriptorPoolSizes.push_back({ descriptorTypeCount.first, descriptorTypeCount.second });

	// VkDescriptorPoolCreateInfo
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = VK_NULL_HANDLE;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	// vkCreateDescriptorPool
	vkCreateDescriptorPool(mDeviceInfo->mDevice, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &mDescriptorPool);

	// VkDescriptorSetAllocateInfo
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = VK_NULL_HANDLE;
	descriptorSetAllocateInfo.descriptorPool = mDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &mDescriptorSetLayout;

	// vkAllocateDescriptorSets
	VK_CHECK(vkAllocateDescriptorSets(mDeviceInfo->mDevice, &descriptorSetAllocateInfo, &mDescriptorSet));
}

// DeInitialize
void VulkanPipelineInfo::DeInitialize() 
{
	vkDestroyDescriptorPool(mDeviceInfo->mDevice, mDescriptorPool, VK_NULL_HANDLE);
	mDescriptorPool = VK_NULL_HANDLE;
	vkDestroyPipeline(mDeviceInfo->mDevice, mPipeline, VK_NULL_HANDLE);
	mPipeline = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(mDeviceInfo->mDevice, mPipelineLayout, VK_NULL_HANDLE);
	mPipelineLayout = VK_NULL_HANDLE;
	vkDestroyDescriptorSetLayout(mDeviceInfo->mDevice, mDescriptorSetLayout, VK_NULL_HANDLE);
	mDescriptorSetLayout = VK_NULL_HANDLE;
	vkDestroyShaderModule(mDeviceInfo->mDevice, mShaderModuleFS, VK_NULL_HANDLE);
	mShaderModuleFS = VK_NULL_HANDLE;
	vkDestroyShaderModule(mDeviceInfo->mDevice, mShaderModuleVS, VK_NULL_HANDLE);
	mShaderModuleVS = VK_NULL_HANDLE;
}

// BindImageView
void VulkanPipelineInfo::BindImageView(uint32_t binding, VkImageView imageView, VkSampler sampler)
{
	// VkDescriptorImageInfo
	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = imageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// VkWriteDescriptorSet
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = VK_NULL_HANDLE;
	writeDescriptorSet.dstSet = mDescriptorSet;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.pBufferInfo = VK_NULL_HANDLE;
	writeDescriptorSet.pTexelBufferView = VK_NULL_HANDLE;

	// vkUpdateDescriptorSets
	vkUpdateDescriptorSets(mDeviceInfo->mDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
}

// BindUnifromBuffer
void VulkanPipelineInfo::BindUnifromBuffer(uint32_t binding, VkBuffer buffer)
{
	// VkDescriptorImageInfo
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = VK_WHOLE_SIZE;

	// VkWriteDescriptorSet
	VkWriteDescriptorSet writeDescriptorSet{};
	// VkWriteDescriptorSet - uniform buffer
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = VK_NULL_HANDLE;
	writeDescriptorSet.dstSet = mDescriptorSet;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pImageInfo = VK_NULL_HANDLE;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pTexelBufferView = VK_NULL_HANDLE;

	// vkUpdateDescriptorSets
	vkUpdateDescriptorSets(mDeviceInfo->mDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
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
