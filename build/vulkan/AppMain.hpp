#pragma once

#include <DirectXMath.h>
#include <windows.h>
#include <vector>
#include "VulkanHelpers.hpp"

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// Vulkan handlers
	VkInstance                   mInstance = VK_NULL_HANDLE;
	VkSurfaceKHR                 mSurface = VK_NULL_HANDLE;
	VkDevice                     mDevice = VK_NULL_HANDLE;
	VkQueue                      mQueueGraphics = VK_NULL_HANDLE;
	VkQueue                      mQueueCompute = VK_NULL_HANDLE;
	VkQueue                      mQueueTransfer = VK_NULL_HANDLE;
	VkQueue                      mQueuePresent = VK_NULL_HANDLE;
	VkSurfaceFormatKHR           mSurfaceFormat;
	VkPresentModeKHR             mPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	VkSwapchainKHR               mSwapChain = VK_NULL_HANDLE;
	VkImage                      mDepthStencilImage = VK_NULL_HANDLE;
	VkImageView                  mDepthStencilImageView = VK_NULL_HANDLE;
	VkDeviceMemory               mDepthStencilImageMem = VK_NULL_HANDLE;
	std::vector<VkImageView>     mSwapChainImageViews;
	std::vector<VkFramebuffer>   mSwapChainFramebuffers;
	VkShaderModule               mShaderModuleVS = VK_NULL_HANDLE;
	VkShaderModule               mShaderModuleFS = VK_NULL_HANDLE;
	VkPipelineLayout             mPipelineLayout = VK_NULL_HANDLE;
	VkRenderPass                 mRenderPass = VK_NULL_HANDLE;
	VkPipeline                   mGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandPool                mCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer              mCommandBuffer = VK_NULL_HANDLE;
	VkSemaphore                  mImageAvailableSemaphore;
	VkSemaphore                  mRenderFinishedSemaphore;

	BufferMemoryHandle           mVertexBuffer;
	BufferMemoryHandle           mIndexBuffer;

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