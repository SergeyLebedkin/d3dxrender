#include "AppMain.hpp"
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
	VkDeviceSize offsets[] = { 0 };

	//VkBuffer buffers[] = { vertexBufferPos, vertexBufferNorm, vertexBufferTexCoords };

	// GO RENDER
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, VK_NULL_HANDLE);
	//vkCmdBindVertexBuffers(commandBuffer, 0, 3, buffers, offsets);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferPos, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
	//vkCmdDraw(commandBuffer, size, 1, 0, 0);
	
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
	std::vector<const char *> enabledDeviceExtensionNames{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// VkPhysicalDeviceFeatures
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	mInstanceInfo.Initialize("Vulkan app", VK_MAKE_VERSION(1, 0, 1), "Vulkan Engine", VK_MAKE_VERSION(1, 0, 1), enabledInstanceLayerNames, enabledInstanceExtensionNames, VK_API_VERSION_1_1);
	assert(mInstanceInfo.mInstance);

	mSurface = VulkanHelpers::CreateSurface(mInstanceInfo.mInstance, hWnd);
	assert(mSurface);

	mDeviceInfo.Initialize(mInstanceInfo.mPhysicalDeviceGPU, mSurface, physicalDeviceFeatures, enabledDeviceExtensionNames);
	assert(mDeviceInfo.mDevice);

	mSwapchainInfo.Initialize(mDeviceInfo, mSurface);
	assert(mSwapchainInfo.mSwapchain);

	mPipelineInfo.Initialize(mDeviceInfo, mSwapchainInfo.mRenderPass, "shaders/base.vert.spv", "shaders/base.frag.spv");
	assert(mPipelineInfo.mDescriptorSetLayout);
	assert(mPipelineInfo.mPipelineLayout);
	assert(mPipelineInfo.mPipeline);

	mCommandBuffer = mDeviceInfo.AllocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	assert(mCommandBuffer);

	mImageAvailableSemaphore = mDeviceInfo.CreateSemaphore();
	assert(mImageAvailableSemaphore);

	mRenderFinishedSemaphore = mDeviceInfo.CreateSemaphore();
	assert(mRenderFinishedSemaphore);

	mSampler = mDeviceInfo.CreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	assert(mSampler);

	// LoadImageFromFile
	AppUtils::LoadImageFromFile(mDeviceInfo, "./textures/texture.png", mModelImage, mModelImageMemory, mModelImageView);
	//AppUtils::LoadMeshesFromObjFile(mDeviceInfo, "./textures/texture.png", mModelImage, mModelImageMemory, mModelImageView);

	// loadModelObjFromFile("./models/tea.obj", "./models");
	mDeviceInfo.CreateBuffer(vertices, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mModelVertexBufferPos, mModelVertexMemoryPos);
	mDeviceInfo.CreateBuffer(indexes, sizeof(indexes), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mModelIndexBuffer, mModelIndexMemory);
	mDeviceInfo.CreateBuffer(&mWVP, sizeof(mWVP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mModelUniformMVP, mModelUniformMemoryMVP);

	// bind data
	// mPipelineInfo.BindImageView(0, mModelImageView, mSampler);
	mPipelineInfo.BindImageView(0, mModelImageView, mSampler);
	mPipelineInfo.BindUnifromBuffer(1, mModelUniformMVP);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vmaDestroyBuffer(mDeviceInfo.mAllocator, mModelUniformMVP, mModelUniformMemoryMVP);
	vkDestroyImageView(mDeviceInfo.mDevice, mModelImageView, VK_NULL_HANDLE);
	vmaDestroyImage(mDeviceInfo.mAllocator, mModelImage, mModelImageMemory);
	vmaDestroyBuffer(mDeviceInfo.mAllocator, mModelIndexBuffer, mModelIndexMemory);
	//vmaDestroyBuffer(mDeviceInfo.allocator, mModelVertexBufferTexCoord, mModelVertexMemoryTexCoord);
	//vmaDestroyBuffer(mDeviceInfo.allocator, mModelVertexBufferNorm, mModelVertexMemoryNorm);
	vmaDestroyBuffer(mDeviceInfo.mAllocator, mModelVertexBufferPos, mModelVertexMemoryPos);
	
	vkDestroySampler(mDeviceInfo.mDevice, mSampler, VK_NULL_HANDLE);
	vkDestroySemaphore(mDeviceInfo.mDevice, mRenderFinishedSemaphore, VK_NULL_HANDLE);
	vkDestroySemaphore(mDeviceInfo.mDevice, mImageAvailableSemaphore, VK_NULL_HANDLE);
	mPipelineInfo.DeInitialize();
	mSwapchainInfo.DeInitialize();
	vkDestroySurfaceKHR(mInstanceInfo.mInstance, mSurface, VK_NULL_HANDLE);
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
	extend2d.height = mSwapchainInfo.mViewportHeight;
	extend2d.width = mSwapchainInfo.mViewportWidth;

	// update MVP uniform buffer
	mDeviceInfo.WriteBuffer(&mWVP, sizeof(mWVP), mModelUniformMVP, mModelUniformMemoryMVP);

	// refill command buffer (RENDER CURRENT FRAME TO CURRENT FRAME BUFFER)
	FillCommandBuffer(mCommandBuffer, mPipelineInfo.mPipeline, mPipelineInfo.mPipelineLayout, mPipelineInfo.mDescriptorSet,
		mSwapchainInfo.mRenderPass, framebuffer, extend2d, 
		mModelVertexBufferPos, mModelVertexBufferNorm, mModelVertexBufferTexCoord, mModelIndexBuffer, mVertexCount);

	// submit render command buffer
	VulkanHelpers::QueueSubmit(mDeviceInfo.mQueueGraphics, mCommandBuffer, mImageAvailableSemaphore, mRenderFinishedSemaphore);

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
	DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(45.0f), (FLOAT)mSwapchainInfo.mViewportWidth / mSwapchainInfo.mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	mWVP = matWorld * matView * matProj;
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	VK_CHECK(vkQueueWaitIdle(mDeviceInfo.mQueuePresent));
	mSwapchainInfo.ReInitialize(mDeviceInfo, mSurface);
};
