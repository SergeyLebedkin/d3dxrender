#pragma once

#include <DirectXMath.h>
#include "AppUtils.hpp"

// created SL-160225
class CAppMain
{
private:
	// vulkan handle holders
	VulkanHelpers::VulkanInstanceInfo  mInstanceInfo;
	VulkanHelpers::VulkanDeviceInfo    mDeviceInfo;
	VulkanHelpers::VulkanSwapchainInfo mSwapchainInfo;
	VulkanHelpers::VulkanPipelineInfo  mPipelineInfo;

	// vulkan handlers
	VkSurfaceKHR          mSurface = VK_NULL_HANDLE;
	VkCommandBuffer       mCommandBuffer = VK_NULL_HANDLE;
	VkSemaphore           mImageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore           mRenderFinishedSemaphore = VK_NULL_HANDLE;

	// texture
	VkImage          mModelImage = VK_NULL_HANDLE;
	VmaAllocation    mModelImageMemory = VK_NULL_HANDLE;
	VkImageView      mModelImageView = VK_NULL_HANDLE;
	VkSampler        mSampler = VK_NULL_HANDLE;
	// uniforms
	VkBuffer         mModelUniformMVP = VK_NULL_HANDLE;
	VmaAllocation    mModelUniformMemoryMVP = VK_NULL_HANDLE;
	// vertex
	VkBuffer         mModelVertexBufferPos = VK_NULL_HANDLE;
	VkBuffer         mModelVertexBufferNorm = VK_NULL_HANDLE;
	VkBuffer         mModelVertexBufferTexCoord = VK_NULL_HANDLE;
	VmaAllocation    mModelVertexMemoryPos = VK_NULL_HANDLE;
	VmaAllocation    mModelVertexMemoryNorm = VK_NULL_HANDLE;
	VmaAllocation    mModelVertexMemoryTexCoord = VK_NULL_HANDLE;
	// index
	VkBuffer         mModelIndexBuffer = VK_NULL_HANDLE;
	VmaAllocation    mModelIndexMemory = VK_NULL_HANDLE;
	// vertex count
	uint32_t         mVertexCount = 0;

	// scene variables
	DirectX::XMMATRIX mWVP;
private:
	bool loadModelObjFromFile(const char * fileName, const char * baseDir);
public:
	CAppMain() {};
	virtual ~CAppMain() {};

	// main functions
	void Init(const HWND hWnd);
	void Destroy();
	void Render();
	void Update(float deltaTime);

	// SetViewportSize
	void SetViewportSize(WORD viewportWidth, WORD viewportHeight);
};