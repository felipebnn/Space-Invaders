#pragma once

#include <glm/glm.hpp>

struct Model {
	VkDescriptorSet descriptorSet;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	glm::mat4 modelMatrix;
	glm::vec3 size;

	uint32_t indexCount;
	uint32_t firstIndex;
};
