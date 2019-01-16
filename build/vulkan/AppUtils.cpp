#include "AppUtils.hpp"
#include "utils/stb_image.h"
#include "utils/tiny_obj_loader.h"

#include <iostream>

// LoadImageFromFile
bool AppUtils::LoadImageFromFile(VulkanHelpers::VulkanDeviceInfo& deviceInfo, const char* fileName, VkImage& image, VmaAllocation& allocation, VkImageView& imageView)
{
	// load image data
	int x, y, n;
	unsigned char *data = stbi_load(fileName, &x, &y, &n, 4);
	assert(data);

	// create image
	deviceInfo.CreateImage(data, x, y, VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_USAGE_SAMPLED_BIT, image, allocation);
	assert(image);
	assert(allocation);

	// create image view
	imageView = deviceInfo.CreateImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	assert(imageView);

	// free image data
	stbi_image_free(data);
	return true;
}

// LoadMeshesFromObjFile
bool AppUtils::LoadMeshesFromObjFile(VulkanHelpers::VulkanDeviceInfo& deviceInfo, const char* fileName, const char* baseDir, std::vector<VulkanMeshObj *>& meshes)
{
	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	tinyobj::LoadObj(&attribs, &shapes, &materials, &err, fileName, baseDir, true);
	std::cout << err << std::endl;

	// allocate buffers
	std::vector<float> bufferPos{};
	std::vector<float> bufferNorm{};
	std::vector<float> bufferTexCoord{};
	for (const auto& shape : shapes)
	{
		bufferPos.clear();
		bufferNorm.clear();
		bufferTexCoord.clear();
		bufferPos.reserve(shape.mesh.indices.size() * 3);
		bufferNorm.reserve(shape.mesh.indices.size() * 3);
		bufferTexCoord.reserve(shape.mesh.indices.size() * 2);

		// create buffers
		for (tinyobj::index_t index : shape.mesh.indices)
		{
			// if vertex exists
			if (index.vertex_index >= 0)
			{
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 0]);
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 1]);
				bufferPos.push_back(attribs.vertices[3 * index.vertex_index + 2]);
			}

			// if normal exists
			if (index.normal_index >= 0)
			{
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 0]);
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 1]);
				bufferNorm.push_back(attribs.normals[3 * index.normal_index + 2]);
			}

			// if texCoords exists
			if (index.texcoord_index >= 0)
			{
				bufferTexCoord.push_back(attribs.texcoords[2 * index.texcoord_index + 0]);
				bufferTexCoord.push_back(attribs.texcoords[2 * index.texcoord_index + 1]);
			}
		}

		VulkanMeshObj* mesh = new VulkanMeshObj();
		assert(mesh);

		deviceInfo.CreateBuffer(bufferPos.data(), bufferPos.size() * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh->mBufferPositions, mesh->mDeviceMemoryPositions);
		deviceInfo.CreateBuffer(bufferNorm.data(), bufferNorm.size() * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh->mBufferNormals, mesh->mDeviceMemoryNormals);
		deviceInfo.CreateBuffer(bufferTexCoord.data(), bufferTexCoord.size() * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh->mBufferTexCoords, mesh->mDeviceMemoryTexCoords);
		assert(mesh->mDeviceMemoryPositions);
		assert(mesh->mDeviceMemoryNormals);
		assert(mesh->mDeviceMemoryTexCoords);
	}

	return true;
}