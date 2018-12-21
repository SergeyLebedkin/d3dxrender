#include "VulkanUtils.hpp"
#include <vector>

// Created SL-181221
VkInstance VulkanCreateInstance()
{
	// vulkan handles
	VkResult vkResult = VK_SUCCESS;
	VkInstance vkInstance = VK_NULL_HANDLE;

	// VkExtensionProperties
	uint32_t extensionsPropertiesCount = 0;
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(extensionsPropertiesCount);
	vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsPropertiesCount, extensionProperties.data());

	// enabledExtensionNames
	std::vector<char *> enabledExtensionNames;
	enabledExtensionNames.reserve(extensionsPropertiesCount);
	for (auto& extensionProperty : extensionProperties)
		enabledExtensionNames.push_back(extensionProperty.extensionName);

	// VkLayerProperties
	uint32_t layerPropertiesCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);
	std::vector<VkLayerProperties> layerProperties(layerPropertiesCount);
	vkResult = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties.data());

	// enabledLayerNames
	std::vector<char *> enabledLayerNames;
	enabledLayerNames.reserve(layerPropertiesCount);
	for (auto& layerProperty : layerProperties)
		enabledLayerNames.push_back(layerProperty.layerName);

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
	vkResult = vkCreateInstance(&instanceCreateInfo, NULL, &vkInstance);

	return vkInstance;
}

// Created SL-181221
VkSurfaceKHR VulkanCreateSurface(VkInstance vkInstance, HWND hWnd)
{
	// vulkan handles
	VkResult vkResult = VK_SUCCESS;
	VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

	// VkWin32SurfaceCreateInfoKHR
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.hwnd = hWnd;
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);
	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR);

	return vkSurfaceKHR;
}

// Created SL-181221
VkPhysicalDevice VulkanGetPhysicalDevice(VkInstance vkInstance)
{
	// vulkan handles
	VkResult vkResult = VK_SUCCESS;

	// VkPhysicalDevice and VkPhysicalDeviceFeatures
	uint32_t physicalDevicesCount = 0;
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, physicalDevices.data());

	// find discrete GPU physical device and physical device features
	for (auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return physicalDevice;
	}

	return VK_NULL_HANDLE;
}

// Created SL-181221
VkDevice VulkanCreateDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKHR)
{
	// vulkan handles
	VkResult vkResult = VK_SUCCESS;
	VkDevice vkDevice = VK_NULL_HANDLE;

	// VkQueueFamilyProperties
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &physicalDeviceFeatures);

	// find graphics and present queue family property index
	uint32_t queueFamilyPropertieIndexGraphics = MAXUINT32;
	uint32_t queueFamilyPropertieIndexPresent = MAXUINT32;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		// get graphics queue family property index
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			queueFamilyPropertieIndexGraphics = i;

		// get present queue family property index
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurfaceKHR, &presentSupport);
		if (queueFamilyProperties[i].queueCount > 0 && presentSupport)
			queueFamilyPropertieIndexPresent = i;
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
	deviceCreateInfo.ppEnabledExtensionNames = nullptr;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	vkResult = vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice);

	return vkDevice;
}