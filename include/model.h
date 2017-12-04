#pragma once

#include <glm/glm.hpp>

struct Model {
	VkDescriptorSet descriptorSet;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	glm::mat4 modelMatrix;
	
	glm::vec3 position;
	glm::vec3 size;
	float radius;

	uint32_t indexCount;
	uint32_t firstIndex;
};
