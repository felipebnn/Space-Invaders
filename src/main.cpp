#include "engine.h"

#include <thread>

constexpr int FRAME_RATE = 30;

class SpaceInvaders : public Engine {
	size_t playerIndex;
	size_t playerBulletIndex;
	size_t playerBulletCount;

	size_t enemyBulletIndex;
	size_t enemyBulletCount;

	size_t bossIndex;

	size_t enemy1Index;
	size_t enemy2Index;
	size_t enemy3Index;

	const size_t enemy1Count = 22;
	const size_t enemy2Count = 22;
	const size_t enemy3Count = 11;

	const size_t spacing = 15;

	void setModelPos(size_t index, const glm::vec3& pos) {
		Model &m = models[index];

		m.modelMatrix = glm::translate(glm::mat4{}, pos - m.size / 2.0f);
		updateModelMatrix(m);
	}

	void translateModelPos(size_t index, const glm::vec3& delta) {
		Model &m = models[index];

		m.modelMatrix = glm::translate(m.modelMatrix, delta);
		updateModelMatrix(m);
	}

	glm::vec3 getModelPos(size_t index) {
		Model &m = models[index];

		return glm::vec3(m.modelMatrix * glm::vec4{0,0,0,1}) + m.size / 2.0f;
	}

	void tick(double duration) {
		uint32_t sleepTime = (1000-duration) / FRAME_RATE;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime > 1 ? sleepTime : 1));

		#ifndef NDEBUG
		measureFramerate();
		#endif
	}

	void updateCamera() {
		UniformBufferObject ubo;
		ubo.view = glm::lookAt(glm::vec3(spacing * 5, -20.0f, 200.0f), glm::vec3(spacing * 5, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 1000.0f);
		ubo.proj[1][1] *= -1;
		updateUniformBuffers(ubo);
	}

	void setup() {
		updateCamera();

		setModelPos(playerIndex, {spacing * 5, -60, 0});
		setModelPos(bossIndex, {spacing * 5, 80, 0});

		for (uint32_t i=0; i<playerBulletCount; ++i) {
			setModelPos(playerBulletIndex + i, {0, 0, 10000});
		}

		for (uint32_t i=0; i<enemyBulletCount; ++i) {
			setModelPos(playerBulletIndex + i, {0, 0, 10000});
		}

		for (uint32_t i=0; i<11; ++i) {
			setModelPos(enemy1Index+i, {i * spacing, 0, 0});
			setModelPos(enemy1Index+11+i, {i * spacing, spacing, 0});

			setModelPos(enemy2Index+i, {i * spacing, 2 * spacing, 0});
			setModelPos(enemy2Index+11+i, {i * spacing, 3 * spacing, 0});

			setModelPos(enemy3Index+i, {i * spacing, 4 * spacing, 0});
		}
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
				playerBulletCount = count;
			} else if (modelName == "enemy_bullet") {
				enemyBulletIndex = models.size();
				enemyBulletCount = count;
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
			m.size[1] = height;

			for (uint32_t i=height; i-- > 0; ) {
				std::string line;
				modelsFile >> line;

				m.size[0] = std::max(m.size[0], (float) line.length());

				for (uint32_t j=0; j<line.length(); ++j) {
					if (line[j] != '.') {
						const Vertex v1 {{j     , i     , 0.0f}, {1.0f, 1.0f, 1.0f}, {    textureBase, 0}};
						const Vertex v2 {{j+1.0f, i     , 0.0f}, {1.0f, 1.0f, 1.0f}, {    textureBase, 1}};
						const Vertex v3 {{j+1.0f, i+1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {nextTextureBase, 1}};
						const Vertex v4 {{j     , i+1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {nextTextureBase, 0}};

						addVertex(v1, uniqueVertices);
						addVertex(v2, uniqueVertices);
						addVertex(v4, uniqueVertices);

						addVertex(v2, uniqueVertices);
						addVertex(v3, uniqueVertices);
						addVertex(v4, uniqueVertices);

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