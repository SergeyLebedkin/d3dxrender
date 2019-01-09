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

	// VkViewport - viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent2D.width;
	viewport.height = (float)extent2D.height;
	viewport.minDepth = 0.5f;
	viewport.maxDepth = 1.0f;

	// VkRect2D - scissor
	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = extent2D.width;
	scissor.extent.height = extent2D.height;

	// GO RENDER
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	
	vkCmdEndRenderPass(commandBuffer);

	// vkEndCommandBuffer
	VK_CHECK(vkEndCommandBuffer(commandBuffer));
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
	std::vector<const char *> enabledDeviceExtensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// VkPhysicalDeviceFeatures
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

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

	mInstanceInfo.Initialize("Vulkan app", VK_MAKE_VERSION(1, 0, 1), "Vulkan Engine", VK_MAKE_VERSION(1, 0, 1), enabledInstanceLayerNames, enabledInstanceExtensionNames, VK_API_VERSION_1_1);
	assert(mInstanceInfo.instance);

	mSurface = CreateSurface(mInstanceInfo.instance, hWnd);
	assert(mSurface);

	VkPhysicalDevice physicalDevice = mInstanceInfo.FindPhysicalDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
	assert(physicalDevice);

	mDeviceInfo.Initialize(physicalDevice, mSurface, physicalDeviceFeatures, enabledDeviceExtensionNames);
	assert(mDeviceInfo.device);

	mRenderPass = CreateRenderPass(mDeviceInfo.device);
	assert(mRenderPass);

	mSwapchainInfo.Initialize(mDeviceInfo, mSurface, mRenderPass);
	assert(mSwapchainInfo.swapchain);

	mShaderModuleVS = CreateShaderModuleFromFile(mDeviceInfo.device, "shaders/base.vert.spv");
	assert(mShaderModuleVS);

	mShaderModuleFS = CreateShaderModuleFromFile(mDeviceInfo.device, "shaders/base.frag.spv");
	assert(mShaderModuleFS);

	mPipelineLayout = CreatePipelineLayout(mDeviceInfo.device);
	assert(mPipelineLayout);

	mGraphicsPipeline = CreateGraphicsPipeline(mDeviceInfo.device, vertexInputState, mShaderModuleVS, mShaderModuleFS, mPipelineLayout,
		mRenderPass, mSwapchainInfo.viewportWidth, mSwapchainInfo.viewportWidth);
	assert(mGraphicsPipeline);

	mCommandPool = CreateCommandPool(mDeviceInfo.device, mDeviceInfo.queueFamilyIndexGraphics);
	assert(mCommandPool);

	mCommandBuffer = AllocateCommandBuffer(mDeviceInfo.device, mCommandPool);
	assert(mCommandBuffer);

	mImageAvailableSemaphore = CreateSemaphore(mDeviceInfo.device);
	assert(mImageAvailableSemaphore);

	mRenderFinishedSemaphore = CreateSemaphore(mDeviceInfo.device);
	assert(mRenderFinishedSemaphore);

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	//mDeviceInfo.AllocateBuffer(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffer, deviceMemory);
	mDeviceInfo.AllocateBufferAndMemory(1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffer, deviceMemory);
	vkFreeMemory(mDeviceInfo.device, deviceMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(mDeviceInfo.device, buffer, VK_NULL_HANDLE);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroySemaphore(mDeviceInfo.device, mRenderFinishedSemaphore, nullptr);
	vkDestroySemaphore(mDeviceInfo.device, mImageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(mDeviceInfo.device, mCommandPool, nullptr);
	vkDestroyPipeline(mDeviceInfo.device, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDeviceInfo.device, mPipelineLayout, nullptr);
	vkDestroyShaderModule(mDeviceInfo.device, mShaderModuleFS, nullptr);
	vkDestroyShaderModule(mDeviceInfo.device, mShaderModuleVS, nullptr);
	vkDestroyRenderPass(mDeviceInfo.device, mRenderPass, nullptr);
	mSwapchainInfo.DeInitialize();
	vkDestroySurfaceKHR(mInstanceInfo.instance, mSurface, nullptr);
	mDeviceInfo.DeInitialize();
	mInstanceInfo.DeInitialize();
}

// Created SL-160225
void CAppMain::Render()
{
	// begin frame
	VkFramebuffer framebuffer = mSwapchainInfo.BeginFrame(mImageAvailableSemaphore);

	// VkExtent2D
	VkExtent2D extend2d;
	extend2d.height = mSwapchainInfo.viewportHeight;
	extend2d.width = mSwapchainInfo.viewportWidth;

	// refill command buffer (RENDER CURRENT FRAME TO CURRENT FRAME BUFFER)
	FillCommandBuffer(mCommandBuffer, mGraphicsPipeline, mRenderPass, framebuffer, extend2d);

	// submit render command buffer
	QueueSubmit(mDeviceInfo.queueGraphics, mCommandBuffer, mImageAvailableSemaphore, mRenderFinishedSemaphore);

	// end frame
	mSwapchainInfo.EndFrame(mRenderFinishedSemaphore);
}

// Created SL-160225
void CAppMain::Update(float deltaTime)
{
	static float time = 0.0f;
	static uint32_t frames = 0;
	time += deltaTime;
	frames++;
	if (time >= 1.0f) {
		std::cout << "frames " << frames << " in " << time << " seconds" << std::endl;
		time = 0.0f;
		frames = 0;
	}

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
	DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(45.0f), (FLOAT)mSwapchainInfo.viewportWidth / mSwapchainInfo.viewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	mWVP = matWorld * matView * matProj;
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	VK_CHECK(vkQueueWaitIdle(mDeviceInfo.queuePresent));
	mSwapchainInfo.ReInitialize(mDeviceInfo, mSurface, mRenderPass);
};
