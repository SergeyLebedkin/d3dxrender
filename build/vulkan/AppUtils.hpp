#pragma once

#include "vkutils/vkmesh.hpp"

namespace AppUtils {
	// LoadTextureFromFile (this function ALWAYS create VK_FORMAT_R8G8B8A8_SNORM texture)
	bool LoadImageFromFile(VulkanHelpers::VulkanDeviceInfo& deviceInfo, const char* fileName, VkImage& image, VmaAllocation& allocation, VkImageView& imageView);

	// LoadMeshesFromObjFile (this function ALWAYS create VK_FORMAT_R8G8B8A8_SNORM texture)
	bool LoadMeshesFromObjFile(VulkanHelpers::VulkanDeviceInfo& deviceInfo, const char* filePath, const char* baseDir, std::vector<VulkanMeshObj *>& meshes);
}