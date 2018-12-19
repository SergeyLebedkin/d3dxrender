#include "AppMain.hpp"
#include <iostream>

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

// Created SL-160225
void CAppMain::Init(const HWND hWnd)
{
	// get client rect
	RECT clientWindowRect{ 0 };
	GetClientRect(hWnd, &clientWindowRect);
	mViewportWidth = (WORD)clientWindowRect.right - (WORD)clientWindowRect.left;
	mViewportHeight = (WORD)clientWindowRect.bottom - (WORD)clientWindowRect.top;

	// console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// VkInstance
	VkResult vkResult = VK_SUCCESS;

	// VkExtensionProperties
	uint32_t extensionsPropertiesCount = 0;
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(extensionsPropertiesCount);
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, extensionProperties.data());

	// enabledExtensionNames
	std::vector<char *> enabledExtensionNames;
	enabledExtensionNames.reserve(extensionsPropertiesCount);
	for (auto& extensionPropertie : extensionProperties)
		enabledExtensionNames.push_back(extensionPropertie.extensionName);

	// VkLayerProperties
	uint32_t layerPropertiesCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
	std::vector<VkLayerProperties> layerProperties(layerPropertiesCount);
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties.data());

	// enabledLayerNames
	std::vector<char *> enabledLayerNames;
	enabledLayerNames.reserve(layerPropertiesCount);
	for (auto& layerPropertie : layerProperties)
		enabledLayerNames.push_back(layerPropertie.layerName);

	// VkApplicationInfo
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = VK_NULL_HANDLE;
	applicationInfo.pApplicationName = "Vulkan App";
	applicationInfo.applicationVersion = 0;
	applicationInfo.pEngineName = "Vulkan Engine";
	applicationInfo.engineVersion = 0;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	
	// VkInstanceCreateInfo
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = VK_NULL_HANDLE;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
	vkResult = vkCreateInstance(&instanceCreateInfo, NULL, &mInstance);

	// vkCreateDebugUtilsMessengerEXT and vkDestroyDebugUtilsMessengerEXT
	vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");

	// VkDebugUtilsMessengerCreateInfoEXT
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {};
	debugUtilsMessengerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfoEXT.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCreateInfoEXT.pfnUserCallback = DebugCallback;
	debugUtilsMessengerCreateInfoEXT.pUserData = nullptr;
	if (vkCreateDebugUtilsMessengerEXT)
		vkCreateDebugUtilsMessengerEXT(mInstance, &debugUtilsMessengerCreateInfoEXT, nullptr, &mDebugUtilsMessengerEXT);

	// VkWin32SurfaceCreateInfoKHR
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.hwnd = hWnd;
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);
	vkResult = vkCreateWin32SurfaceKHR(mInstance, &win32SurfaceCreateInfoKHR, nullptr, &mSurface);

	// VkPhysicalDevice and VkPhysicalDeviceFeatures
	uint32_t physicalDevicesCount = 0;
	vkResult = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	vkResult = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, physicalDevices.data());

	// find discrete gpu physical device and physical device features
	VkPhysicalDevice physicalDeviceGPU = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures physicalDeviceFeaturesGPU;
	for (auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDeviceGPU = physicalDevice;
			physicalDeviceFeaturesGPU = physicalDeviceFeatures;
		}
	}

	// VkSurfaceCapabilitiesKHR
	VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceGPU, mSurface, &surfaceCapabilitiesKHR);

	// VkSurfaceFormatKHR
	uint32_t physicalDeviceSurfaceFormatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfaceFormatsCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(physicalDeviceSurfaceFormatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfaceFormatsCount, surfaceFormats.data());

	// VkSurfaceFormatKHR
	uint32_t physicalDeviceSurfacePresentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfacePresentModesCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(physicalDeviceSurfacePresentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceGPU, mSurface, &physicalDeviceSurfacePresentModesCount, presentModes.data());

	// VkQueueFamilyProperties
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceGPU, &queueFamilyPropertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceGPU, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	// find graphics and present queue family propertie index
	int32_t queueFamilyPropertieIndexGraphics = -1;
	int32_t queueFamilyPropertieIndexPresent = -1;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		// get graphics queue family propertie index
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			queueFamilyPropertieIndexGraphics = i;

		// get present queue family propertie index
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDeviceGPU, i, mSurface, &presentSupport);
		if (queueFamilyProperties[i].queueCount > 0 && presentSupport)
			queueFamilyPropertieIndexPresent = i;
	}

	// queue family propertie debug output
	std::cout << "Queue Family Properties : " << std::endl;
	std::cout << "-------------------------" << std::endl;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		std::cout << "index              : " << i << std::endl;
		std::cout << "timestampValidBits : " << queueFamilyProperties[i].timestampValidBits << std::endl;
		std::cout << "queueFlags         : " << queueFamilyProperties[i].queueFlags << std::endl;
		std::cout << "queueCount         : " << queueFamilyProperties[i].queueCount << std::endl;
		std::cout << "-----------------------" << std::endl;
	}

	// VkDeviceQueueCreateInfo
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo deviceQueueCreateInfos[2];
	// VkDeviceQueueCreateInfo - graphics
	deviceQueueCreateInfos[0].pNext = VK_NULL_HANDLE;
	deviceQueueCreateInfos[0].flags = 0;
	deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyPropertieIndexGraphics;
	deviceQueueCreateInfos[0].queueCount = 1;
	deviceQueueCreateInfos[0].pQueuePriorities = &queuePriority;
	// VkDeviceQueueCreateInfo - present
	deviceQueueCreateInfos[1].pNext = VK_NULL_HANDLE;
	deviceQueueCreateInfos[1].flags = 0;
	deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyPropertieIndexPresent;
	deviceQueueCreateInfos[1].queueCount = 1;
	deviceQueueCreateInfos[1].pQueuePriorities = &queuePriority;

	// VkDeviceCreateInfo
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 2;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.enabledLayerCount = (uint32_t)enabledLayerNames.size();
	deviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeaturesGPU;
	vkResult = vkCreateDevice(physicalDeviceGPU, &deviceCreateInfo, nullptr, &mDevice);

	// vkGetDeviceQueue
	vkGetDeviceQueue(mDevice, queueFamilyPropertieIndexGraphics, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, queueFamilyPropertieIndexPresent, 0, &mPresentQueue);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroyDevice(mDevice, nullptr);
	if (vkDestroyDebugUtilsMessengerEXT)
		vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessengerEXT, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

// Created SL-160225
void CAppMain::Render()
{
}

// Created SL-160225
void CAppMain::Update(float deltaTime)
{
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
	DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(45.0f), (FLOAT)mViewportWidth / mViewportHeight, 1.0f, 1000.0f);

	// WorldViewProjection
	mWVP = matWorld * matView * matProj;
}

// Created SL-160225
void CAppMain::SetViewportSize(WORD viewportWidth, WORD viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;
};

// DebugCallback
VKAPI_ATTR VkBool32 VKAPI_CALL CAppMain::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT             messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                       pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		std::cout << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}