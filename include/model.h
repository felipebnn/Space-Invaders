#pragma once

#include <glm/glm.hpp>

struct Model {
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	glm::mat4 modelMatrix;

	uint32_t indexCount;
	uint32_t firstIndex;
};
