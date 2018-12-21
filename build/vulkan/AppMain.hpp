#pragma once

#include <vulkan/vulkan.h>
#include <windows.h>
#include <DirectXMath.h>
#include <vector>

void FillCommandBuffer(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer);

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
	VkQueue                      mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue                      mPresentQueue = VK_NULL_HANDLE;
	VkSwapchainKHR               mSwapChain = VK_NULL_HANDLE;
	std::vector<VkImageView>     mSwapChainImageViews;
	std::vector<VkFramebuffer>   mSwapChainFramebuffers;
	VkShaderModule               mVertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule               mFragmentShaderModule = VK_NULL_HANDLE;
	VkPipelineLayout             mPipelineLayout = VK_NULL_HANDLE;
	VkRenderPass                 mRenderPass = VK_NULL_HANDLE;
	VkPipeline                   mGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandPool                mCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> mCommandBuffers;
	VkSemaphore                  mImageAvailableSemaphore;
	VkSemaphore                  mRenderFinishedSemaphore;

	// scene variables
	DirectX::XMMATRIX mWVP;

	// debug and extensions
	VkDebugUtilsMessengerEXT              mDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	PFN_vkCreateDebugUtilsMessengerEXT    vkCreateDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugUtilsMessengerEXT   vkDestroyDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT             messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*                                       pUserData);
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