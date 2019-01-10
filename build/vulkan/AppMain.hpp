#pragma once

#include <DirectXMath.h>
#include <windows.h>
#include <vector>
#include "vkutils/VulkanHelpers.hpp"

// created SL-160225
class CAppMain
{
private:
	// vulkan handle holders
	VulkanInstanceInfo  mInstanceInfo;
	VulkanDeviceInfo    mDeviceInfo;
	VulkanSwapchainInfo mSwapchainInfo;

	// vulkan handlers
	VkSurfaceKHR     mSurface = VK_NULL_HANDLE;
	VkRenderPass     mRenderPass = VK_NULL_HANDLE;
	VkShaderModule   mShaderModuleVS = VK_NULL_HANDLE;
	VkShaderModule   mShaderModuleFS = VK_NULL_HANDLE;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
	VkPipeline       mGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandBuffer  mCommandBuffer = VK_NULL_HANDLE;
	VkSemaphore      mImageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore      mRenderFinishedSemaphore = VK_NULL_HANDLE;

	// vertex
	VkBuffer         mModelVertexBufferPos = VK_NULL_HANDLE;
	VkBuffer         mModelVertexBufferNorm = VK_NULL_HANDLE;
	VmaAllocation    mModelVertexMemoryPos = VK_NULL_HANDLE;
	VmaAllocation    mModelVertexMemoryNorm = VK_NULL_HANDLE;
	// index
	VkBuffer         mModelIndexBuffer = VK_NULL_HANDLE;
	VmaAllocation    mModelIndexMemory = VK_NULL_HANDLE;

	// scene variables
	DirectX::XMMATRIX mWVP;
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