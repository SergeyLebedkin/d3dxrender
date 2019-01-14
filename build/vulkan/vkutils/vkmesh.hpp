#pragma once

#include "VulkanHelpers.hpp"
#include <DirectXMath.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// interfaces
//////////////////////////////////////////////////////////////////////////

// VulkanMesh
class VulkanMesh
{
public:
	// Init/DeInit
	virtual void Initialize(VulkanDeviceInfo* vulkanDeviceInfo) = 0;
	virtual void DeInitialize() = 0;
};

// VulkanModel
class VulkanModel
{
public:
	// transformations
	DirectX::XMMATRIX mModelMatrix{};
};

//////////////////////////////////////////////////////////////////////////
// realizations
//////////////////////////////////////////////////////////////////////////

// VulkanMeshObj
class VulkanMeshObj : public VulkanMesh
{
protected:
	VulkanDeviceInfo * vulkanDeviceInfo = nullptr;
public:
	// buffers
	VkBuffer       mBufferPositions = VK_NULL_HANDLE;
	VkBuffer       mBufferNormals = VK_NULL_HANDLE;
	VkBuffer       mBufferTexCoords = VK_NULL_HANDLE;
	VkDeviceMemory mDeviceMemoryPositions = VK_NULL_HANDLE;
	VkDeviceMemory mDeviceMemoryNormals = VK_NULL_HANDLE;
	VkDeviceMemory mDeviceMemoryTexCoords = VK_NULL_HANDLE;

	// texture
	VkImage     mImage = VK_NULL_HANDLE;
	VkImageView mImageView = VK_NULL_HANDLE;
	VkSampler   mSamples = VK_NULL_HANDLE;

	// Init/DeInit
	void Initialize(VulkanDeviceInfo* vulkanDeviceInfo) override {};
	void DeInitialize() override {};
};

// VulkanModelObj
class VulkanModelObj : public VulkanModel
{
public:
	// meshes
	std::vector<VulkanMeshObj *> mMeshes{};
};
