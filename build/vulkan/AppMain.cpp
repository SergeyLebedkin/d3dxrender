#include "AppMain.hpp"
#include "Meshes.teapot.h"
#include "utils/stb_image.h"
#include "utils/tiny_obj_loader.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <array>
#include <cmath>

// vertex structure
struct CUSTOMVERTEX { FLOAT X, Y, Z, W; FLOAT R, G, B, A; FLOAT U, V; };

// vertex array
CUSTOMVERTEX vertices[] = {
	{ +1.0f, -1.0f, +0.0f, +1.0, /**/+1.0f, +0.0f, +0.0f, +1.0, /**/+1, +0 },
	{ +1.0f, +1.0f, +0.0f, +1.0, /**/+0.0f, +1.0f, +0.0f, +1.0, /**/+1, +1 },
	{ -1.0f, -1.0f, +0.0f, +1.0, /**/+0.0f, +0.0f, +1.0f, +1.0, /**/+0, +0 },
	{ -1.0f, +1.0f, +0.0f, +1.0, /**/+1.0f, +1.0f, +0.0f, +1.0, /**/+0, +1 },
};

// index array
uint16_t indexes[] = { 0, 1, 2, 2, 1, 3 };

// FillCommandBuffer
void FillCommandBuffer(
	VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet,
	VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent2D,
	VkBuffer vertexBufferPos, VkBuffer vertexBufferNorm, VkBuffer vertexBufferTexCoords, VkBuffer indexBuffer, uint32_t size)
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
	viewport.y = (float)extent2D.height;
	viewport.width = (float)extent2D.width;
	viewport.height = -(float)extent2D.height;
	viewport.minDepth = 0.5f;
	viewport.maxDepth = 1.0f;

	// VkRect2D - scissor
	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = extent2D.width;
	scissor.extent.height = extent2D.height;

	// VkDeviceSize
	VkDeviceSize offsets[] = { 0, 0, 0 };

	VkBuffer buffers[] = { vertexBufferPos, vertexBufferNorm, vertexBufferTexCoords };

	// GO RENDER
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, VK_NULL_HANDLE);
	vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);
	//vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdDrawIndexed(commandBuffer, sizeof(teapot_indices)/4, 1, 0, 0, 0);
	vkCmdDraw(commandBuffer, size, 1, 0, 0);
	
	vkCmdEndRenderPass(commandBuffer);

	// vkEndCommandBuffer
	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

// loadModelObjFromFile
bool CAppMain::loadModelObjFromFile(const char * fileName, const char * baseDir)
{
	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	tinyobj::LoadObj(&attribs, &shapes, &materials, &err, fileName, baseDir, true);
	std::cout << err << std::endl;

	// allocate buffers
	std::vector<float> bufferPos{};
	std::vector<float> bufferNorm{};
	std::vector<float> bufferTexCoord{};
	//for (int i = 0; i < shapes.size(); i++)
	for (int i = 0; i < 1; i++)
	{	
		tinyobj::shape_t& shape = shapes[i];

		bufferPos.clear();
		bufferNorm.clear();
		bufferTexCoord.clear();
		bufferPos.reserve(shape.mesh.indices.size() * 3);
		bufferNorm.reserve(shape.mesh.indices.size() * 3);
		bufferTexCoord.reserve(shape.mesh.indices.size() * 2);
		mVertexCount = (uint32_t)shape.mesh.indices.size();

		// create buffers
		for (tinyobj::index_t& index : shape.mesh.indices)
		{
			// if vertex exists
			if (index.vertex_index >= 0)
			{
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 0]);
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 1]);
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 2]);
			}

			// if normal exists
			if (index.normal_index >= 0)
			{
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 0]);
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 1]);
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 2]);
			}

			// if texCoords exists
			if (index.texcoord_index >= 0)
			{
				bufferTexCoord.push_back(attribs.texcoords[2 * index.texcoord_index + 0]);
				bufferTexCoord.push_back(attribs.texcoords[2 * index.texcoord_index + 1]);
			}
		}

		mDeviceInfo.CreateBuffer(bufferPos.data(), bufferPos.size() * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mModelVertexBufferPos, mModelVertexMemoryPos);
		mDeviceInfo.CreateBuffer(bufferNorm.data(), bufferNorm.size() * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mModelVertexBufferNorm, mModelVertexMemoryNorm);
		mDeviceInfo.CreateBuffer(bufferTexCoord.data(), bufferTexCoord.size() * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mModelVertexBufferTexCoord, mModelVertexMemoryTexCoord);
		assert(mModelVertexMemoryPos);
		assert(mModelVertexMemoryNorm);
		assert(mModelVertexMemoryTexCoord);
	}

	return true;
}

// loadTextureFromFile
bool CAppMain::loadTextureFromFile(const char * fileName)
{
	// load image data
	int x, y, n;
	unsigned char *data = stbi_load(fileName, &x, &y, &n, 4);
	assert(data);

	// create image
	mDeviceInfo.CreateImage(data, x, y, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_USAGE_SAMPLED_BIT, mModelImage, mModelImageMemory);
	assert(mModelImage);
	assert(mModelImageMemory);

	// create image view
	mModelImageView = CreateImageView(mDeviceInfo.device, mModelImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	assert(mModelImageView);

	// free image data
	stbi_image_free(data);

	return true;
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
		{ 0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX },
		{ 1, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX },
		{ 2, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX },
	};

	// VkVertexInputAttributeDescription - vertexAttributeDescriptions
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, // position
		{ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 }, // normal
		{ 2, 2, VK_FORMAT_R32G32_SFLOAT, 0 },    // texture coordinates
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

	mDescriptorSetLayout = CreateDescriptorSetLayout(mDeviceInfo.device);
	assert(mDescriptorSetLayout);

	mPipelineLayout = CreatePipelineLayout(mDeviceInfo.device, mDescriptorSetLayout);
	assert(mPipelineLayout);

	mGraphicsPipeline = CreateGraphicsPipeline(mDeviceInfo.device, vertexInputState, mShaderModuleVS, mShaderModuleFS, mPipelineLayout,
		mRenderPass, mSwapchainInfo.viewportWidth, mSwapchainInfo.viewportWidth);
	assert(mGraphicsPipeline);

	mCommandBuffer = AllocateCommandBuffer(mDeviceInfo.device, mDeviceInfo.commandPool);
	assert(mCommandBuffer);

	mImageAvailableSemaphore = CreateSemaphore(mDeviceInfo.device);
	assert(mImageAvailableSemaphore);

	mRenderFinishedSemaphore = CreateSemaphore(mDeviceInfo.device);
	assert(mRenderFinishedSemaphore);

	mSampler = CreateSampler(mDeviceInfo.device);
	assert(mSampler);

	loadModelObjFromFile("./models/tea.obj", "./models");
	loadTextureFromFile("./models/rose.jpg");

	mDeviceInfo.CreateBuffer(sizeof(mWVP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mModelUniformMVP, mModelUniformMemoryMVP);

	mDescriptorPool = CreateDescriptorPool(mDeviceInfo.device);
	assert(mDescriptorPool);

 	mDescriptorSet = AllocateDescriptorSet(mDeviceInfo.device, mDescriptorPool, mDescriptorSetLayout);
 	assert(mDescriptorSet);
 	UpdateDescriptorSets(mDeviceInfo.device, mDescriptorSet, mModelImageView, mSampler, mModelUniformMVP);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroyDescriptorPool(mDeviceInfo.device, mDescriptorPool, VK_NULL_HANDLE);

	vmaDestroyBuffer(mDeviceInfo.allocator, mModelUniformMVP, mModelUniformMemoryMVP);
	vkDestroyImageView(mDeviceInfo.device, mModelImageView, VK_NULL_HANDLE);
	vmaDestroyImage(mDeviceInfo.allocator, mModelImage, mModelImageMemory);
	vmaDestroyBuffer(mDeviceInfo.allocator, mModelIndexBuffer, mModelIndexMemory);
	vmaDestroyBuffer(mDeviceInfo.allocator, mModelVertexBufferTexCoord, mModelVertexMemoryTexCoord);
	vmaDestroyBuffer(mDeviceInfo.allocator, mModelVertexBufferNorm, mModelVertexMemoryNorm);
	vmaDestroyBuffer(mDeviceInfo.allocator, mModelVertexBufferPos, mModelVertexMemoryPos);
	
	vkDestroySampler(mDeviceInfo.device, mSampler, VK_NULL_HANDLE);
	vkDestroySemaphore(mDeviceInfo.device, mRenderFinishedSemaphore, VK_NULL_HANDLE);
	vkDestroySemaphore(mDeviceInfo.device, mImageAvailableSemaphore, VK_NULL_HANDLE);
	vkDestroyPipeline(mDeviceInfo.device, mGraphicsPipeline, VK_NULL_HANDLE);
	vkDestroyPipelineLayout(mDeviceInfo.device, mPipelineLayout, VK_NULL_HANDLE);
	vkDestroyDescriptorSetLayout(mDeviceInfo.device, mDescriptorSetLayout, VK_NULL_HANDLE);
	vkDestroyShaderModule(mDeviceInfo.device, mShaderModuleFS, VK_NULL_HANDLE);
	vkDestroyShaderModule(mDeviceInfo.device, mShaderModuleVS, VK_NULL_HANDLE);
	vkDestroyRenderPass(mDeviceInfo.device, mRenderPass, VK_NULL_HANDLE);
	mSwapchainInfo.DeInitialize();
	vkDestroySurfaceKHR(mInstanceInfo.instance, mSurface, VK_NULL_HANDLE);
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

	// update MVP uniform buffer
	mDeviceInfo.WriteBuffer(&mWVP, sizeof(mWVP), mModelUniformMVP, mModelUniformMemoryMVP);

	// refill command buffer (RENDER CURRENT FRAME TO CURRENT FRAME BUFFER)
	FillCommandBuffer(mCommandBuffer, mGraphicsPipeline, mPipelineLayout, mDescriptorSet,
		mRenderPass, framebuffer, extend2d, 
		mModelVertexBufferPos, mModelVertexBufferNorm, mModelVertexBufferTexCoord, mModelIndexBuffer, mVertexCount);

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
	//DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationX(angle) * DirectX::XMMatrixRotationY(angle) * DirectX::XMMatrixRotationZ(angle += deltaTime);
	DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationX(angle/16.0f) * DirectX::XMMatrixRotationY(angle += deltaTime);
	DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX matTranslate = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX matWorld = matRotate * matScale * matTranslate;

	// mat view
	DirectX::XMMATRIX matView = DirectX::XMMatrixLookAtRH(
		DirectX::XMVectorSet(0.0f, 0.0f, 40.0f, 1.0f), // the camera position
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
