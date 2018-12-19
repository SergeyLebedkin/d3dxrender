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

	// Extension Properties
	uint32_t extensionsPropertiesCount = 0;
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr);
	mExtensionProperties.resize(extensionsPropertiesCount);
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, mExtensionProperties.data());

	// create enabled extension names array
	mEnabledExtensionNames.reserve(extensionsPropertiesCount);
	for (auto& extensionPropertie : mExtensionProperties)
		mEnabledExtensionNames.push_back(extensionPropertie.extensionName);

	// Layer Properties
	uint32_t layerPropertiesCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
	mLayerProperties.resize(layerPropertiesCount);
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, mLayerProperties.data());

	// create enabled extension names array
	mEnabledLayerNames.reserve(layerPropertiesCount);
	for (auto& layerPropertie : mLayerProperties)
		mEnabledLayerNames.push_back(layerPropertie.layerName);

	// VkApplicationInfo
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = NULL;
	applicationInfo.pApplicationName = "Vulkan App";
	applicationInfo.applicationVersion = 0;
	applicationInfo.pEngineName = "Vulkan Engine";
	applicationInfo.engineVersion = 0;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	
	// VkInstanceCreateInfo
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)mEnabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = mEnabledLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)mEnabledExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = mEnabledExtensionNames.data();
	vkResult = vkCreateInstance(&instanceCreateInfo, NULL, &mInstance);

	// VkDebugUtilsMessengerCreateInfoEXT
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
	vkResult = func(mInstance, &createInfo, nullptr, &mDebugUtilsMessengerEXT);

	// mPhysicalDevices
	uint32_t physicalDevicesCount = 0;
	vkResult = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, nullptr);
	mPhysicalDevices.resize(physicalDevicesCount);
	vkResult = vkEnumeratePhysicalDevices(mInstance, &physicalDevicesCount, mPhysicalDevices.data());
	for (auto& physicalDevice : mPhysicalDevices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			mPhysicalDevice = physicalDevice;
			mPhysicalDeviceFeatures = deviceFeatures;
		}
	}

	// VkQueueFamilyProperties
	int32_t queueFamilyIndex = -1;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilies.data());
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			std::cout << queueFamilies[i].timestampValidBits << std::endl;
			std::cout << queueFamilies[i].queueFlags << std::endl;
			std::cout << queueFamilies[i].queueCount << std::endl;
			std::cout << i << std::endl;
			std::cout << "=========================" << std::endl;
			queueFamilyIndex = i;
		}
	}

	// VkDeviceQueueCreateInfo
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// VkDeviceCreateInfo
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.enabledExtensionCount = 0;
	instanceCreateInfo.enabledLayerCount = (uint32_t)mEnabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = mEnabledLayerNames.data();
	deviceCreateInfo.pEnabledFeatures = &mPhysicalDeviceFeatures;
	vkResult = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
	vkGetDeviceQueue(mDevice, queueFamilyIndex, 0, &mQueue);

	// VkSurfaceKHR
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.hwnd = hWnd;
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);
	vkResult = vkCreateWin32SurfaceKHR(mInstance, &win32SurfaceCreateInfoKHR, nullptr, &surface);
}

// Created SL-160225
void CAppMain::Destroy()
{
	vkDestroyDevice(mDevice, nullptr);
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
	func(mInstance, mDebugUtilsMessengerEXT, nullptr);
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