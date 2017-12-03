#include "engine.h"

#include <thread>

constexpr int FRAME_RATE = 30;

class SpaceInvaders : public Engine {
	size_t playerIndex;
	size_t playerBulletIndex;

	size_t enemyBulletIndex;
	const size_t enemyBulletCount = 3;

	size_t bossIndex;

	size_t enemy1Index;
	size_t enemy2Index;
	size_t enemy3Index;

	const size_t enemy1Count = 22;
	const size_t enemy2Count = 22;
	const size_t enemy3Count = 11;

	void tick(double duration) {
		uint32_t sleepTime = (1000-duration) / FRAME_RATE;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime > 1 ? sleepTime : 1));

		#ifndef NDEBUG
		measureFramerate();
		#endif
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
		vertices = {};
		indices = {};
		models = {};

		std::ifstream modelsFile("models/models.txt");

		if (!modelsFile) {
			throw std::runtime_error("failed to open file 'models/models.txt'!");
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		uint32_t textureCount;
		modelsFile >> textureCount;

		std::string modelName;
		uint32_t height, count, textureId;
		while (modelsFile >> modelName >> height >> textureId >> count) {
			double textureBase = (double) textureId / textureCount;
			double nextTextureBase = (double) (textureId + 1) / textureCount;

			if (modelName == "player") {
				playerIndex = models.size();
			} else if (modelName == "player_bullet") {
				playerBulletIndex = models.size();
			} else if (modelName == "enemy_bullet") {
				enemyBulletIndex = models.size();
			} else if (modelName == "boss") {
				bossIndex = models.size();
			} else if (modelName == "enemy_1") {
				enemy1Index = models.size();
			} else if (modelName == "enemy_2") {
				enemy2Index = models.size();
			} else if (modelName == "enemy_3") {
				enemy3Index = models.size();
			} else {
				throw std::runtime_error("unknown model name '" + modelName + "'");
			}

			Model m {};
			m.firstIndex = indices.size();

			for (uint32_t i=0; i<height; ++i) {
				std::string line;
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

	void measureFramerate() {
		static double frameRate = FRAME_RATE;
		static int frames = 0;
		static auto previousTime = std::chrono::high_resolution_clock::now();

		++frames;

		auto currTime = std::chrono::high_resolution_clock::now();
		double delta = std::chrono::duration<double, std::chrono::milliseconds::period>(currTime - previousTime).count();

		if (delta > 1000.0f) {
			frameRate = (double)frames*0.5f + frameRate*0.5f;
			std::cout << "Frame rate was " << frames << " average is " << frameRate << "\n";

			frames = 0;
			previousTime = currTime;
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