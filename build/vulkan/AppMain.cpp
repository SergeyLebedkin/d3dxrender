#include "AppMain.hpp"
#include <iostream>

// vertex structure
struct CUSTOMVERTEX { FLOAT X, Y, Z, W; FLOAT R, G, B, A; FLOAT U, V; };

// vertex array
CUSTOMVERTEX vertices[] = {
	{ +1.0f, -1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+1, +0 },
	{ +1.0f, +1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+1, +1 },
	{ -1.0f, -1.0f, +0.0f, +1.0, /**/+1.0f, +0.0f, +0.0f, +1.0, /**/+0, +0 },
	{ -1.0f, +1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+0, +1 },
};

// index array
uint16_t indexes[] = { 0, 1, 2, 2, 1, 3 };

// FillCommandBuffer
void FillCommandBuffer(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent2D)
{
	// handles
	VkResult result = VK_SUCCESS;

	// VkCommandBufferBeginInfo
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional

	// vkBeginCommandBuffer
	result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	// VkRenderPassBeginInfo
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	VkClearValue clearColor = { 0.0f, 0.125f, 0.3f, 1.0f };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = extent2D;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	//vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	//vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	// vkEndCommandBuffer
	result = vkEndCommandBuffer(commandBuffer);
}

// CreateRenderPass
VkRenderPass CreateRenderPass(VkDevice device)
{
	// handles
	VkResult result = VK_SUCCESS;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	// VkAttachmentDescription
	VkAttachmentDescription attachmentDescription = {};
	attachmentDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// attachmentDescriptions
	VkAttachmentDescription attachmentDescriptions[]{ attachmentDescription };

	//////////////////////////////////////////////////////////////////////////

	// VkAttachmentReference
	VkAttachmentReference attachmentReference = {};
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentReference.attachment = 0;

	// attachmentReferences
	VkAttachmentReference attachmentReferences[]{ attachmentReference };

	//////////////////////////////////////////////////////////////////////////

	// VkSubpassDescription
	VkSubpassDescription subpassDescription = {};
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = attachmentReferences;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	// VkSubpassDescription
	VkSubpassDescription subpassDescriptions[] = { subpassDescription };

	//////////////////////////////////////////////////////////////////////////

	// VkRenderPassCreateInfo
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = attachmentDescriptions;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = subpassDescriptions;
	result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);

	// return
	return renderPass;
}

// CreateFramebuffer
VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView imageView, VkExtent2D extent)
{
	// handles
	VkResult result = VK_SUCCESS;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;

	// image views
	VkImageView imageViews[]{ imageView };

	// VkFramebufferCreateInfo 
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = VK_NULL_HANDLE;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.pAttachments = imageViews;
	framebufferCreateInfo.width = extent.width;
	framebufferCreateInfo.height = extent.height;
	framebufferCreateInfo.layers = 1;
	result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer);
	
	// return
	return framebuffer;
}

// CreateBuffer
VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
	// handles
	VkResult result = VK_SUCCESS;
	VkBuffer buffer = VK_NULL_HANDLE;

	// VkBufferCreateInfo
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

	// return
	return buffer;
}

// AllocateAndBindDeviceMemoryForBuffer
VkDeviceMemory AllocateAndBindDeviceMemoryForBuffer(VkDevice device, VkBuffer buffer, uint32_t memoryTypeIndex = 0)
{
	// handles
	VkResult result = VK_SUCCESS;
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;

	// VkMemoryRequirements
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	// VkMemoryAllocateInfo
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
	result = vkBindBufferMemory(device, buffer, deviceMemory, 0);

	// return
	return deviceMemory;
}

// MapDeviceMemory
VkResult MapDeviceMemory(VkDevice device, VkDeviceMemory deviceMemory, void* srcData, VkDeviceSize size)
{
	// handles
	VkResult result = VK_SUCCESS;

	// vkMapMemory/vkUnmapMemory
	void* data = nullptr;
	result = vkMapMemory(device, deviceMemory, 0, size, 0, &data);
	memcpy(data, srcData, size);
	vkUnmapMemory(device, deviceMemory);

	// return
	return result;
}

// CreateImage
VkImage CreateImage(VkDevice device)
{
	// create buffer and device memory
	VkBuffer buffer = CreateBuffer(device, 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	VkDeviceMemory deviceMemory = AllocateAndBindDeviceMemoryForBuffer(device, buffer);

	// map device memory
	VkResult result = MapDeviceMemory(device, deviceMemory, nullptr, 1024);

	// free/destroy device memory and buffer
	vkFreeMemory(device, deviceMemory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);
	return VK_NULL_HANDLE;
}

// Created SL-160225
void CAppMain::Init(const HWND hWnd)
{
	// get client rect
	RECT clientWindowRect{ 0 };
	GetClientRect(hWnd, &clientWindowRect);
	mViewportWidth = (WORD)clientWindowRect.right - (WORD)clientWindowRect.left;
	mViewportHeight = (WORD)clientWindowRect.bottom - (WORD)clientWindowRect.top;

	// console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// VkInstance
	VkResult result = VK_SUCCESS;

	// VkExtensionProperties
	uint32_t extensionsPropertiesCount = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(extensionsPropertiesCount);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, extensionProperties.data());

	// enabledExtensionNames
	std::vector<const char *> enabledExtensionNames;
	enabledExtensionNames.reserve(extensionsPropertiesCount);
	for (auto& extensionProperty : extensionProperties)
		enabledExtensionNames.push_back(extensionProperty.extensionName);
// 	for (auto& extensionProperty : extensionProperties)	{
// 		if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extensionProperty.extensionName))
// 			enabledExtensionNames.push_back(extensionProperty.extensionName);
// 		if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extensionProperty.extensionName))
// 			enabledExtensionNames.push_back(extensionProperty.extensionName);
// 	}

	// VkLayerProperties
	uint32_t layerPropertiesCount = 0;
	result = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
	std::vector<VkLayerProperties> layerProperties(layerPropertiesCount);
	result = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties.data());

	// enabledLayerNames
	std::vector<char *> enabledLayerNames;
	enabledLayerNames.reserve(layerPropertiesCount);
	for (auto& layerProperty : layerProperties)
		enabledLayerNames.push_back(layerProperty.layerName);

	// VkApplicationInfo
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = VK_NULL_HANDLE;
	applicationInfo.pApplicationName = "Vulkan App";
	applicationInfo.applicationVersion = 0;
	applicationInfo.pEngineName = "Vulkan Engine";
	applicationInfo.engineVersion = 0;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	
	// VkInstanceCreateInfo
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = VK_NULL_HANDLE;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
	result = vkCreateInstance(&instanceCreateInfo, NULL, &mInstance);

	// vkCreateDebugUtilsMessengerEXT and vkDestroyDebugUtilsMessengerEXT
	vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");

	// VkDebugUtilsMessengerCreateInfoEXT
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {};
	debugUtilsMessengerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfoEXT.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.pfnUserCallback = DebugCallback;
	debugUtilsMessengerCreateInfoEXT.pUserData = nullptr;
	if (vkCreateDebugUtilsMessengerEXT)
		vkCreateDebugUtilsMessengerEXT(mInstance, &debugUtilsMessengerCreateInfoEXT, nullptr, &mDebugUtilsMessengerEXT);

	// VkPhysicalDevice and VkPhysicalDeviceFeatures
	uint32_t physicalDevicesCount = 0;
	result = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	result = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, physicalDevices.data());

	// find discrete GPU physical device and physical device features
	VkPhysicalDevice physicalDeviceGPU = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures physicalDeviceFeaturesGPU;
	for (auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDeviceGPU = physicalDevice;
			physicalDeviceFeaturesGPU = physicalDeviceFeatures;
		}
	}

	// VkWin32SurfaceCreateInfoKHR
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.hwnd = hWnd;
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);
	result = vkCreateWin32SurfaceKHR(mInstance, &win32SurfaceCreateInfoKHR, nullptr, &mSurface);

	// VkSurfaceCapabilitiesKHR
	VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceGPU, mSurface, &surfaceCapabilitiesKHR);

	// VkSurfaceFormatKHR
	uint32_t physicalDeviceSurfaceFormatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfaceFormatsCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(physicalDeviceSurfaceFormatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfaceFormatsCount, surfaceFormats.data());

	// VkSurfaceFormatKHR
	uint32_t physicalDeviceSurfacePresentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfacePresentModesCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(physicalDeviceSurfacePresentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfacePresentModesCount, presentModes.data());

	// VkQueueFamilyProperties
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceGPU, &queueFamilyPropertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceGPU, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	// VkPhysicalDeviceMemoryProperties
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDeviceGPU, &memProperties);

	// find graphics and present queue family property index
	uint32_t queueFamilyPropertieIndexGraphics = MAXUINT32;
	uint32_t queueFamilyPropertieIndexPresent = MAXUINT32;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		// get graphics queue family property index
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilyPropertieIndexGraphics = i;

			// get present queue family property index
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDeviceGPU, i, mSurface, &presentSupport);
			if (presentSupport)
				queueFamilyPropertieIndexPresent = i;
		}
	}

	// queue family properties debug output
	std::cout << "Queue Family Properties : " << std::endl;
	std::cout << "=========================" << std::endl;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		std::cout << "index              : " << i << std::endl;
		std::cout << "timestampValidBits : " << queueFamilyProperties[i].timestampValidBits << std::endl;
		std::cout << "queueFlags         : " << queueFamilyProperties[i].queueFlags << std::endl;
		std::cout << "queueCount         : " << queueFamilyProperties[i].queueCount << std::endl;
		std::cout << "-----------------------" << std::endl;
	}

	// VkDeviceQueueCreateInfo
	float queuePriority = 0.0f;
	VkDeviceQueueCreateInfo deviceQueueCreateInfos[2];
	// VkDeviceQueueCreateInfo - graphics
	deviceQueueCreateInfos[0].pNext = VK_NULL_HANDLE;
	deviceQueueCreateInfos[0].flags = 0;
	deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyPropertieIndexGraphics;
	deviceQueueCreateInfos[0].queueCount = 1;
	deviceQueueCreateInfos[0].pQueuePriorities = &queuePriority;
	// VkDeviceQueueCreateInfo - present
	deviceQueueCreateInfos[1].pNext = VK_NULL_HANDLE;
	deviceQueueCreateInfos[1].flags = 0;
	deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyPropertieIndexPresent;
	deviceQueueCreateInfos[1].queueCount = 1;
	deviceQueueCreateInfos[1].pQueuePriorities = &queuePriority;

	// re-assign enabledExtensionNames
	enabledExtensionNames.clear();
	enabledExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// VkDeviceCreateInfo
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
	deviceCreateInfo.enabledLayerCount = 0;// (uint32_t)enabledLayerNames.size();
	deviceCreateInfo.ppEnabledLayerNames = 0;// enabledLayerNames.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeaturesGPU;
	result = vkCreateDevice(physicalDeviceGPU, &deviceCreateInfo, nullptr, &mDevice);

	// vkGetDeviceQueue
	vkGetDeviceQueue(mDevice, queueFamilyPropertieIndexGraphics, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, queueFamilyPropertieIndexPresent, 0, &mPresentQueue);
	uint32_t queueFamilyIndices[]{ queueFamilyPropertieIndexGraphics, queueFamilyPropertieIndexPresent };
	bool queueFamiliesEqual = queueFamilyPropertieIndexGraphics == queueFamilyPropertieIndexPresent;

	// VkSwapchainCreateInfoKHR
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.surface = mSurface;
	swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.minImageCount + 1;
	swapchainCreateInfoKHR.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainCreateInfoKHR.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfoKHR.imageExtent = surfaceCapabilitiesKHR.currentExtent;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//queueFamiliesEqual ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;//queueFamiliesEqual ? 0 : 2;
	swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;//queueFamiliesEqual ? nullptr : queueFamilyIndices;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKHR.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;
	result = vkCreateSwapchainKHR(mDevice, &swapchainCreateInfoKHR, nullptr, &mSwapChain);

	// swapChainImages
	uint32_t swapChainImagesCount = 0;
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapChainImagesCount, nullptr);
	std::vector<VkImage> swapChainImages(swapChainImagesCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &swapChainImagesCount, swapChainImages.data());

	// mSwapChainImageViews
	mSwapChainImageViews.reserve(swapChainImagesCount);
	for (auto& swapChainImage : swapChainImages)
	{
		// VkImageViewCreateInfo
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapChainImage;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		// VkImageView
		VkImageView imageView = VK_NULL_HANDLE;
		result = vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &imageView);
		mSwapChainImageViews.push_back(imageView);
	}

	// VkShaderModuleCreateInfo - vertex shader
	VkShaderModuleCreateInfo vertShaderModuleCreateInfo = {};
	vertShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertShaderModuleCreateInfo.codeSize = 0;
	vertShaderModuleCreateInfo.pCode = nullptr;
	//result = vkCreateShaderModule(mDevice, &vertexShaderModuleCreateInfo, nullptr, &mVertexShaderModule);

	// VkShaderModuleCreateInfo - pixel shader
	VkShaderModuleCreateInfo fragShaderModuleCreateInfo = {};
	fragShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragShaderModuleCreateInfo.codeSize = 0;
	fragShaderModuleCreateInfo.pCode = nullptr;
	//result = vkCreateShaderModule(mDevice, &fragmentShaderModuleCreateInfo, nullptr, &mFragmentShaderModule);

	// VkPipelineShaderStageCreateInfo - vertex shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = mVertexShaderModule;
	vertShaderStageInfo.pName = "main";

	// VkPipelineShaderStageCreateInfo - fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = mFragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	// shaderStages
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// VkPipelineVertexInputStateCreateInfo
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	// VkPipelineInputAssemblyStateCreateInfo
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// VkViewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchainCreateInfoKHR.imageExtent.width;
	viewport.height = (float)swapchainCreateInfoKHR.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// scissor
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainCreateInfoKHR.imageExtent;

	// VkPipelineViewportStateCreateInfo
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// VkPipelineRasterizationStateCreateInfo
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// VkPipelineMultisampleStateCreateInfo
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// VkPipelineColorBlendAttachmentState
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// VkPipelineColorBlendStateCreateInfo
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// VkPipelineDynamicStateCreateInfo
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// VkPipelineLayoutCreateInfo
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	result = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout);

	mRenderPass = CreateRenderPass(mDevice);

	// VkGraphicsPipelineCreateInfo
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	graphicsPipelineCreateInfo.pViewportState = &viewportState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr; // Optional
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
	graphicsPipelineCreateInfo.pDynamicState = nullptr; // Optional
	graphicsPipelineCreateInfo.layout = mPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = mRenderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional
	//result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &mGraphicsPipeline);

	// mSwapChainFramebuffers
	mSwapChainFramebuffers.reserve(swapChainImagesCount);
	for (auto& swapChainImageView : mSwapChainImageViews)
		mSwapChainFramebuffers.push_back(CreateFramebuffer(mDevice, mRenderPass, swapChainImageView, swapchainCreateInfoKHR.imageExtent));

	// VkCommandPoolCreateInfo
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyPropertieIndexGraphics;
	commandPoolCreateInfo.flags = 0; // Optional
	result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);

	// VkCommandBufferAllocateInfo
	mCommandBuffers.resize(mSwapChainFramebuffers.size());
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = mCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
	result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, mCommandBuffers.data());

	// fill command buffers
	for (size_t i = 0; i < mCommandBuffers.size(); i++)
		FillCommandBuffer(mCommandBuffers[i], mRenderPass, mSwapChainFramebuffers[i], swapchainCreateInfoKHR.imageExtent);

	// VkSemaphoreCreateInfo
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	result = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore);
	result = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphore);

	//CreateImage(mDevice);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
	vkDestroySemaphore(mDevice, mImageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	for (auto& swapChainFramebuffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mDevice, swapChainFramebuffer, nullptr);
	//vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	//vkDestroyShaderModule(mDevice, mFragmentShaderModule, nullptr);
	//vkDestroyShaderModule(mDevice, mVertexShaderModule, nullptr);
	for (auto& swapChainImageView : mSwapChainImageViews)
		vkDestroyImageView(mDevice, swapChainImageView, nullptr);
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	if (vkDestroyDebugUtilsMessengerEXT)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessengerEXT, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

// Created SL-160225
void CAppMain::Render()
{
	uint32_t imageIndex = 0;
	VkResult result = VK_SUCCESS;

	// vkAcquireNextImageKHR
	result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	// VkSubmitInfo
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

	// vkQueueSubmit
	result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	// VkPresentInfoKHR
	VkPresentInfoKHR presentInfo = {};
	VkSwapchainKHR swapChains[] = { mSwapChain };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
	result = vkQueueWaitIdle(mPresentQueue);
}

// Created SL-160225
void CAppMain::Update(float deltaTime)
{
	// mat world
	static float angle = 0.0f;
	DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationZ(angle += deltaTime);
	DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX matTranslate = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX matWorld = matRotate * matScale * matTranslate;

	// mat view
	DirectX::XMMATRIX matView = DirectX::XMMatrixLookAtRH(
		DirectX::XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f), // the camera position
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),  // the look-at position
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)   // the up direction
	);

	// mat projection
	DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(45.0f), (FLOAT)mViewportWidth / mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	mWVP = matWorld * matView * matProj;
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};

// DebugCallback
VKAPI_ATTR VkBool32 VKAPI_CALL CAppMain::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT             messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                       pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		std::cout << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}