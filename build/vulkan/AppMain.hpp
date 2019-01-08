#pragma once

#include <DirectXMath.h>
#include <windows.h>
#include <vector>
#include "VulkanHelpers.hpp"

// created SL-160225
class CAppMain
{
private:
	// vulkan handle holders
	VulkanDeviceInfo             mDeviceInfo;
	VulkanSwapchainInfo          mSwapchainInfo;

	// vulkan handlers
	VkInstance                   mInstance = VK_NULL_HANDLE;
	VkSurfaceKHR                 mSurface = VK_NULL_HANDLE;
	VkShaderModule               mShaderModuleVS = VK_NULL_HANDLE;
	VkShaderModule               mShaderModuleFS = VK_NULL_HANDLE;
	VkPipelineLayout             mPipelineLayout = VK_NULL_HANDLE;
	VkRenderPass                 mRenderPass = VK_NULL_HANDLE;
	VkPipeline                   mGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandPool                mCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer              mCommandBuffer = VK_NULL_HANDLE;
	VkSemaphore                  mImageAvailableSemaphore;
	VkSemaphore                  mRenderFinishedSemaphore;

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