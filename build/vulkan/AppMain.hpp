#pragma once

#include <vulkan/vulkan.h>
#include <windows.h>
#include <DirectXMath.h>
#include <vector>

// created SL-160225
class CAppMain
{
private:
	// window parameters
	WORD mViewportWidth = 0;
	WORD mViewportHeight = 0;

	// Vulkan handlers
	VkInstance               mInstance = VK_NULL_HANDLE;
	VkDevice                 mDevice = VK_NULL_HANDLE;
	VkQueue                  mQueue = VK_NULL_HANDLE;
	VkSurfaceKHR             mSurface = VK_NULL_HANDLE;

	// scene variables
	DirectX::XMMATRIX mWVP;

	// debug and extentions
	VkDebugUtilsMessengerEXT            mDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = VK_NULL_HANDLE;
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