#include "VulkanHelpers.hpp"
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <array>

// InitDeviceQueueCreateInfo
VkDeviceQueueCreateInfo InitDeviceQueueCreateInfo(uint32_t queueIndex)
{
	// VkDeviceQueueCreateInfo - present
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
	// VkPipelineVertexInputStateCreateInfo
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

// FindPhysicalDevice
VkPhysicalDevice FindPhysicalDevice(VkInstance instance, VkPhysicalDeviceType physicalDeviceType)
{
	// VkPhysicalDevice and VkPhysicalDeviceFeatures
	uint32_t physicalDevicesCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr));
	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices.data()));

	// find discrete GPU physical device and physical device features and properties
	VkPhysicalDeviceFeatures physicalDeviceFeaturesGPU;
	VkPhysicalDeviceProperties physicalDevicePropertiesGPU;
	for (const auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{	
			physicalDeviceFeaturesGPU = physicalDeviceFeatures;
			physicalDevicePropertiesGPU = physicalDeviceProperties;
			return physicalDevice;
		}
	}
	return VK_NULL_HANDLE;
}

// FindPhysicalDeviceMemoryIndex
uint32_t FindPhysicalDeviceMemoryIndex(VkPhysicalDevice physicalDevice, uint32_t propertyFlags)
{
	// VkPhysicalDeviceMemoryProperties
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// find device local memory type index
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			return i;

	// return default
	return ~0U;
}
// FindPhysicalDeviceFamilyQueueIndex
uint32_t FindPhysicalDeviceQueueFamilyIndex(VkPhysicalDevice physicalDevice, uint32_t queueFlags)
{
	// VkQueueFamilyProperties
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	// get graphics queue family property index
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
		if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			return i;

	// return default
	return ~0U;
}

// FindSurfaceFormat
VkSurfaceFormatKHR FindSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// get surface formats count
	uint32_t formatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);

	assert(formatsCount > 0);

	// get surface formats list
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, surfaceFormats.data());

	// get default surface format
	if ((formatsCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
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

// FindPresentMode
VkPresentModeKHR FindPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// get present modes count
	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);

	assert(presentModesCount > 0);

	// get present modes list
	std::vector<VkPresentModeKHR> presentModes(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());

	// try to find mailbox mode
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;

	// try to find fifo mode
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
			return presentMode;

	// return defualt
	return presentModes[0];
}

// FindPresentQueueMode
uint32_t FindPresentQueueMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// VkQueueFamilyProperties
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	// find presentation queue family property index
	uint32_t queueFamilyPropertieIndexPresent = MAXUINT32;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		// get present queue family property index
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport)
			return i;
	}

	// return default
	return ~0U;
}

// CreateInstance
VkInstance CreateInstance(
	const char* appName, uint32_t appVersion,
	const char* engineName, uint32_t engineVersion,
	std::vector<const char *> enabledLayerNames,
	std::vector<const char *> enabledExtensionNames,
	uint32_t apiVersion)
{
	// VkLayerProperties
	uint32_t instanceLayerPropertiesCount = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, nullptr));
	std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertiesCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, instanceLayerProperties.data()));

	// VkExtensionProperties
	uint32_t instanceExtensionsPropertiesCount = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsPropertiesCount, nullptr));
	std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionsPropertiesCount);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsPropertiesCount, instanceExtensionProperties.data()));

	// VkApplicationInfo
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = VK_NULL_HANDLE;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = engineName;
	applicationInfo.engineVersion = engineVersion;
	applicationInfo.apiVersion = apiVersion;

	// VkInstanceCreateInfo
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = VK_NULL_HANDLE;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();

	// vkCreateInstance
	VkInstance instance = VK_NULL_HANDLE;
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance));
	return instance;
}

// CreateSurface
VkSurfaceKHR CreateSurface(VkInstance instance, HWND hWnd)
{
	// VkWin32SurfaceCreateInfoKHR
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
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

// CreateDevice
VkDevice CreateDevice(
	VkPhysicalDevice physicalDevice,
	std::vector<VkDeviceQueueCreateInfo>& deviceQueueCreateInfos,
	std::vector<const char *> enabledExtensionNames,
	const VkPhysicalDeviceFeatures physicalDeviceFeatures)
{
	// VkDeviceCreateInfo
	VkDeviceCreateInfo deviceCreateInfo = {};
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
	VkDevice device = VK_NULL_HANDLE;
	VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &device));
	return device;
}

// CreateSwapchain
VkSwapchainKHR CreateSwapchain(
	VkDevice device, VkSurfaceKHR surface,
	VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode,
	uint32_t imageCount, uint32_t width, uint32_t height)
{
	// VkSwapchainCreateInfoKHR
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.surface = surface;
	swapchainCreateInfoKHR.minImageCount = imageCount + 1;
	swapchainCreateInfoKHR.imageFormat = surfaceFormat.format;
	swapchainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfoKHR.imageExtent.width = width;
	swapchainCreateInfoKHR.imageExtent.height = height;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
	swapchainCreateInfoKHR.pQueueFamilyIndices = VK_NULL_HANDLE;
	swapchainCreateInfoKHR.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfoKHR.presentMode = presentMode;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

	// vkCreateSwapchainKHR
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, nullptr, &swapchain));
	return swapchain;
}

// CreateImageView
VkImage CreateImage(VkDevice device, VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height)
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
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// vkCreateImage
	VkImage image = VK_NULL_HANDLE;
	VK_CHECK(vkCreateImage(device, &imageCreateInfo, VK_NULL_HANDLE, &image));
	return image;
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
	VkFramebufferCreateInfo framebufferCreateInfo = {};
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
	VkBufferCreateInfo bufferCreateInfo = {};
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
	std::vector<char> code(fileSize);
	file.seekg(0, std::ios::beg);
	file.read(code.data(), fileSize);
	file.close();

	// VkShaderModuleCreateInfo - fragShaderModuleCreateInfo
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
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
	// color attachement
	attachmentDescriptions[0].flags = 0;
	attachmentDescriptions[0].format = VK_FORMAT_B8G8R8A8_UNORM;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// depth-stencil attachement
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
	subpassDescriptions[0].pInputAttachments = nullptr;
	subpassDescriptions[0].colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
	subpassDescriptions[0].pColorAttachments = colorAttachmentReferences.data();
	subpassDescriptions[0].pResolveAttachments = nullptr;
	subpassDescriptions[0].pDepthStencilAttachment = &depthStencilAttachmentReference;
	subpassDescriptions[0].preserveAttachmentCount = 0;
	subpassDescriptions[0].pPreserveAttachments = nullptr;

	//////////////////////////////////////////////////////////////////////////

	// VkRenderPassCreateInfo
	VkRenderPassCreateInfo renderPassCreateInfo = {};
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
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = VK_NULL_HANDLE;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
	colorBlendState.pNext = VK_NULL_HANDLE;;
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
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_LINE_WIDTH };

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
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
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
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
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
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	semaphoreCreateInfo.flags = 0;

	// vkCreateSemaphore
	VkSemaphore semaphore;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore));
	return semaphore;
}

#if _DEBUG
static VkDebugReportCallbackEXT debugReportCallbackEXT = VK_NULL_HANDLE;
static PFN_vkCreateDebugReportCallbackEXT fnCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
static PFN_vkDestroyDebugReportCallbackEXT fnDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char * pLayerPrefix,
	const char * pMessage,
	void * pUserData);

// InitVulkanDebug
void InitVulkanDebug(VkInstance instance)
{
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
}

// DeInitVulkanDebug
void DeInitVulkanDebug(VkInstance instance)
{
	if (fnDestroyDebugReportCallbackEXT)
		fnDestroyDebugReportCallbackEXT(instance, debugReportCallbackEXT, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
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
#else
void InitVulkanDebug(VkInstance instance);
void DeInitVulkanDebug(VkInstance instance)
#endif