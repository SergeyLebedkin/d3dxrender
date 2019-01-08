#include "AppMain.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <array>
#include <cmath>

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
void FillCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent2D)
{
	// VkCommandBufferBeginInfo
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	// VkClearValue
	VkClearValue clearColors[2];
	clearColors[0].color = { 0.0f, 0.125f, 0.3f, 1.0f };
	clearColors[1].depthStencil.depth = 1.0f;
	clearColors[1].depthStencil.stencil = 0;

	// VkRenderPassBeginInfo
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = extent2D;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearColors;

	// GO RENDER
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	//vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, VK_NULL_HANDLE);
	//vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	// vkEndCommandBuffer
	VK_CHECK(vkEndCommandBuffer(commandBuffer));
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
VkResult MapDeviceMemory(VkDevice device, VkDeviceMemory deviceMemory, const void* srcData, VkDeviceSize size)
{
	// handles
	VkResult result = VK_SUCCESS;

	// vkMapMemory/vkUnmapMemory
	void* data = nullptr;
	result = vkMapMemory(device, deviceMemory, 0, size, 0, &data);
	memcpy(data, srcData, (size_t)size);
	vkUnmapMemory(device, deviceMemory);

	// return
	return result;
}

// Created SL-160225
void CAppMain::Init(const HWND hWnd)
{
	// enabledInstanceLayerNames
	std::vector<const char *> enabledInstanceLayerNames{
		"VK_LAYER_LUNARG_standard_validation"
	};

	// enabledInstanceExtensionNames
	std::vector<const char *> enabledInstanceExtensionNames{
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
	};

	// extensions
	std::vector<const char *> enabledDeviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// VkPhysicalDeviceFeatures
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	// CreateInstance
	mInstance = CreateInstance("Vulkan app", VK_MAKE_VERSION(1, 0, 1), "Vulkan Engine", VK_MAKE_VERSION(1, 0, 1), enabledInstanceLayerNames, enabledInstanceExtensionNames, VK_API_VERSION_1_1);
	assert(mInstance);

	// InitVulkanDebug
	InitVulkanDebug(mInstance);

	// VkSurface
	mSurface = CreateSurface(mInstance, hWnd);
	assert(mSurface);

	// VkPhysicalDevice
	VkPhysicalDevice physicalDevice = FindPhysicalDevice(mInstance, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
	assert(physicalDevice);

	// VulkanDeviceInfo
	VulkanDeviceInfo deviceInfo;
	deviceInfo.Initialize(physicalDevice, mSurface, physicalDeviceFeatures, enabledDeviceExtensionNames);
	assert(deviceInfo.device);

	// VulkanSwapchainInfo
	VulkanSwapchainInfo swapChainInfo;
	swapChainInfo.Initialize(deviceInfo, mSurface);
	assert(swapChainInfo.swapchain);

	// VkVertexInputBindingDescription - vertexBindingDescriptions
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions{
		{ 0, sizeof(CUSTOMVERTEX), VK_VERTEX_INPUT_RATE_VERTEX }
	};

	// VkVertexInputAttributeDescription - vertexAttributeDescriptions
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R8G8B8A8_SNORM, 0 },
		{ 1, 0, VK_FORMAT_R8G8B8A8_SNORM, 16 },
		{ 2, 0, VK_FORMAT_R8G8_SNORM, 32 },
	};

	// VkPipelineVertexInputStateCreateInfo
	VkPipelineVertexInputStateCreateInfo vertexInputState = InitPipelineVertexInputStateCreateInfo(vertexBindingDescriptions, vertexAttributeDescriptions);

	mShaderModuleVS = CreateShaderModuleFromFile(mDevice, "shaders/base.vert.spv");
	assert(mShaderModuleVS);

	mShaderModuleFS = CreateShaderModuleFromFile(mDevice, "shaders/base.frag.spv");
	assert(mShaderModuleFS);

	mPipelineLayout = CreatePipelineLayout(mDevice);
	assert(mPipelineLayout);

	mGraphicsPipeline = CreateGraphicsPipeline(mDevice, vertexInputState, mShaderModuleVS, mShaderModuleFS, mPipelineLayout, mRenderPass, mViewportWidth, mViewportHeight);
	assert(mGraphicsPipeline);

	mCommandPool = CreateCommandPool(mDevice, queueFamilyPropertieIndexGraphics);
	assert(mCommandPool);

	mCommandBuffer = AllocateCommandBuffer(mDevice, mCommandPool);
	assert(mCommandBuffer);

	mImageAvailableSemaphore = CreateSemaphore(mDevice);
	assert(mImageAvailableSemaphore);

	mRenderFinishedSemaphore = CreateSemaphore(mDevice);
	assert(mRenderFinishedSemaphore);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
	vkDestroySemaphore(mDevice, mImageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	for (auto& swapChainFramebuffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mDevice, swapChainFramebuffer, nullptr);
	vkDestroyImageView(mDevice, mDepthStencilImageView, nullptr);
	vkFreeMemory(mDevice, mDepthStencilImageMem, nullptr);
	vkDestroyImage(mDevice, mDepthStencilImage, nullptr);
	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyShaderModule(mDevice, mShaderModuleFS, nullptr);
	vkDestroyShaderModule(mDevice, mShaderModuleVS, nullptr);
	for (auto& swapChainImageView : mSwapChainImageViews)
		vkDestroyImageView(mDevice, swapChainImageView, nullptr);
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	DeInitVulkanDebug(mInstance);
	vkDestroyInstance(mInstance, nullptr);
}

// Created SL-160225
void CAppMain::Render()
{
	uint32_t imageIndex = 0;
	VkResult result = VK_SUCCESS;

	// vkAcquireNextImageKHR
	result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	// VkExtent2D
	VkExtent2D extend2d;
	extend2d.height = mViewportHeight;
	extend2d.width = mViewportWidth;

	// refill command buffer (RENDER CURRENT FRAME TO CURRENT FRAME BUFFER)
	FillCommandBuffer(mCommandBuffer, mGraphicsPipeline, mRenderPass, mSwapChainFramebuffers[imageIndex], extend2d);

	// VkSubmitInfo
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &mImageAvailableSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &mRenderFinishedSemaphore;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer;

	// vkQueueSubmit
	result = vkQueueSubmit(mQueueGraphics, 1, &submitInfo, VK_NULL_HANDLE);

	// VkPresentInfoKHR
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapChain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	result = vkQueuePresentKHR(mQueuePresent, &presentInfo);
	result = vkQueueWaitIdle(mQueuePresent);
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

