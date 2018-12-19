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
	std::vector<VkLayerProperties>     mLayerProperties;
	std::vector<char*>                 mEnabledLayerNames;
	std::vector<VkExtensionProperties> mExtensionProperties;
	std::vector<char*>                 mEnabledExtensionNames;
	VkInstance                         mInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT           mDebugUtilsMessengerEXT;
	std::vector<VkPhysicalDevice>      mPhysicalDevices;
	VkPhysicalDevice                   mPhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures           mPhysicalDeviceFeatures;
	VkDevice                           mDevice = VK_NULL_HANDLE;
	VkQueue                            mQueue = VK_NULL_HANDLE;

	// scene variables
	DirectX::XMMATRIX mWVP;

	// debug
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