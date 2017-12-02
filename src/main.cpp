#include "engine.h"

class SpaceInvaders : public Engine {
	void tick() {

	}

	void setup() {
		UniformBufferObject ubo;
		ubo.view = glm::lookAt(glm::vec3(5.5f, 15.0f, 15.0f), glm::vec3(5.5f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 30.0f);
		ubo.proj[1][1] *= -1;
		updateUniformBuffers(ubo);
	}



	void addVertex(const Vertex& v, std::unordered_map<Vertex, uint32_t> &uniqueVertices) {
		if (uniqueVertices.count(v) == 0) {
			uniqueVertices[v] = (uint32_t) vertices.size();
			vertices.push_back(v);
		}

		indices.push_back(uniqueVertices[v]);
	}

	void loadModel() {
		indices = {};
		models = {};

		std::ifstream modelsFile("models/models.txt");

		if (!modelsFile) {
			throw std::runtime_error("failed to open file models !");
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		uint32_t textureCount;
		modelsFile >> textureCount;

		while (modelsFile) {
			std::string line;
			modelsFile >> line;

			if (line == ">") {
				uint32_t height, count, textureId;
				modelsFile >> height >> count >> textureId;

				double textureBase = (double) textureId / textureCount;
				double nextTextureBase = (double) (textureId + 1) / textureCount;

				Model m {};
				m.firstIndex = indices.size();

				for (uint32_t i=0; i<height; ++i) {
					modelsFile >> line;

					for (uint32_t j=0; j<line.length(); ++j) {
						if (line[j] != '.') {
							const Vertex v1 {{j     , 0.0f, i     }, {1.0f, 1.0f, 1.0f}, {    textureBase, 0}};
							const Vertex v2 {{j+1.0f, 0.0f, i     }, {1.0f, 1.0f, 1.0f}, {    textureBase, 1}};
							const Vertex v3 {{j+1.0f, 0.0f, i+1.0f}, {1.0f, 1.0f, 1.0f}, {nextTextureBase, 1}};
							const Vertex v4 {{j     , 0.0f, i+1.0f}, {1.0f, 1.0f, 1.0f}, {nextTextureBase, 0}};

							addVertex(v1, uniqueVertices);
							addVertex(v4, uniqueVertices);
							addVertex(v2, uniqueVertices);

							addVertex(v2, uniqueVertices);
							addVertex(v4, uniqueVertices);
							addVertex(v3, uniqueVertices);

							m.indexCount += 6;
						}
					}
				}

				for (uint32_t i=0; i<count; ++i) {
					models.push_back(m);
				}
			}
		}
	}
};


int main() {
	SpaceInvaders app;

	try {
		app.run();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}