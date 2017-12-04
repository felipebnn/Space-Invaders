#include "engine.h"

#include <random>
#include <thread>

constexpr int FRAME_RATE = 30;
constexpr float TEXTURE_WIDTH = 32 * 4;
constexpr float PLAYER_MOVE_SPEED = 64;
constexpr float BULLET_SPEED = 128;

constexpr float MOB_MOVE_SPEED = 4;
constexpr float MOB_ANIMATION_DURATION = 2;

constexpr float BOSS_MOVE_SPEED = 32;
constexpr float BOSS_ANIMATION_DURATION = 6;

constexpr float SHOOTING_PERCENTAGE_CHANCE = 0.001;

enum class AnimationState {
	Right, Left, Down1, Down2
};

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

	const size_t spacing = 20;

	size_t playerCurrentBullet = 0;
	size_t enemyCurrentBullet = 0;
	int moveDirection = 0;

	AnimationState mobState = AnimationState::Right;
	float mobStateTime = MOB_ANIMATION_DURATION / 2;

	AnimationState bossState = AnimationState::Left;
	float bossStateTime = BOSS_ANIMATION_DURATION / 2;

	std::vector<size_t> mobIndices;

	void onKeyDown(int key, int scancode, int mods) {
		switch (key) {
			case GLFW_KEY_LEFT:
				moveDirection = -1;
				break;

			case GLFW_KEY_RIGHT:
				moveDirection = 1;
				break;

			case GLFW_KEY_UP:
			case GLFW_KEY_SPACE:
				shoot(playerIndex, playerBulletIndex, playerBulletCount, playerCurrentBullet);
		}
	};

	void onKeyUp(int key, int scancode, int mods) {
		switch (key) {
			case GLFW_KEY_LEFT:
				if (moveDirection == -1) {
					moveDirection = 0;
				}
				break;

			case GLFW_KEY_RIGHT:
				if (moveDirection == 1) {
					moveDirection = 0;
				}
				break;
		}
	};

	void setModelPos(size_t index, const glm::vec3& pos) {
		Model &m = models[index];

		m.modelMatrix = glm::translate(glm::mat4{}, pos - m.size / 2.0f);
		m.position = pos;
		updateModelMatrix(m);
	}

	void translateModelPos(size_t index, const glm::vec3& delta) {
		Model &m = models[index];

		m.modelMatrix = glm::translate(m.modelMatrix, delta);
		m.position += delta;
		updateModelMatrix(m);
	}

	bool inBounds(const glm::vec3& pos) {
		return pos[1] <= 95 && pos[1] >= -55 && pos[2] >= 0;
	}

	void shoot(size_t shooterIndex, size_t bulletIndex, size_t bulletCount, size_t& currentBullet) {
		if (!inBounds(models[bulletIndex + currentBullet].position)) { // out of bounds
			setModelPos(bulletIndex + currentBullet, models[shooterIndex].position + glm::vec3{0,0,1});
			currentBullet = (currentBullet + 1) % bulletCount;
		}
	}

	bool detectCollision(size_t bulletIndex, size_t targetIndex) {
		const Model &bulletModel = models[bulletIndex];
		const Model &targetModel = models[targetIndex];

		float distance = glm::distance(bulletModel.position - glm::vec3{0,0,1}, targetModel.position);

		if (distance < targetModel.radius) {
			setModelPos(bulletIndex, {0, 0, -100000});
			setModelPos(targetIndex, {0, 0, 100000});
			return true;
		}

		return false;
	}

	void detectCollisions() {
		for (size_t i=0; i<playerBulletCount; ++i) {
			for (size_t mobIndex : mobIndices) {
				if (detectCollision(playerBulletIndex + i, mobIndex)) {
					break;
				}
			}

			detectCollision(playerBulletIndex + i, bossIndex);
		}

		for (size_t i=0; i<enemyBulletCount; ++i) {
			if (detectCollision(enemyBulletIndex + i, playerIndex)) {
				//TODO: gameover
				running = false;
				break;
			}
		}
	}

	void doMobMove(float duration) {
		glm::vec3 mobDir = ((mobState == AnimationState::Right) ? glm::vec3{1,0,0}
							: (mobState == AnimationState::Left) ? glm::vec3{-1,0,0}
							: glm::vec3{0,-1,0}) * (MOB_MOVE_SPEED * duration);

		for (size_t mobIndex : mobIndices) {
			translateModelPos(mobIndex, mobDir);
		}
	}

	void doBossMove(float duration) {
		glm::vec3 bossDir = ((bossState == AnimationState::Right ? glm::vec3{1,0,0} : glm::vec3{-1,0,0})) * (BOSS_MOVE_SPEED * duration);
		translateModelPos(bossIndex, bossDir);
	}

	void doPlayerAnimation(float duration) {
		translateModelPos(playerIndex, glm::vec3{1,0,0} * (PLAYER_MOVE_SPEED * moveDirection * duration));
	}

	void doMobAnimation(float duration) {
		if (mobStateTime > duration) {
			doMobMove(duration);
		} else {
			doMobMove(mobStateTime);

			switch (mobState) {
				case AnimationState::Right:
					mobState = AnimationState::Down1;
					doMobMove(duration - mobStateTime);
					mobStateTime += MOB_ANIMATION_DURATION / 2;
					break;

				case AnimationState::Left:
					mobState = AnimationState::Down2;
					doMobMove(duration - mobStateTime);
					mobStateTime += MOB_ANIMATION_DURATION / 2;
					break;

				case AnimationState::Down1:
					mobState = AnimationState::Left;
					doMobMove(duration - mobStateTime);
					mobStateTime += MOB_ANIMATION_DURATION;
					break;

				case AnimationState::Down2:
					mobState = AnimationState::Right;
					doMobMove(duration - mobStateTime);
					mobStateTime += MOB_ANIMATION_DURATION;
					break;
			}
		}
		mobStateTime -= duration;
	}

	void doBossAnimation(float duration) {
		if (bossStateTime > duration) {
			doBossMove(duration);
		} else {
			doBossMove(bossStateTime);

			switch (bossState) {
				case AnimationState::Right:
					bossState = AnimationState::Left;
					doBossMove(duration - bossStateTime);
					bossStateTime += BOSS_ANIMATION_DURATION;
					break;

				case AnimationState::Left:
					bossState = AnimationState::Right;
					doBossMove(duration - bossStateTime);
					bossStateTime += BOSS_ANIMATION_DURATION;
					break;

				default:
					bossState = AnimationState::Right;
					bossStateTime = 0;
			}
		}
		bossStateTime -= duration;
	}

	void doBulletAnimation(float duration) {
		for (size_t i=0; i<playerBulletCount; ++i) {
			translateModelPos(playerBulletIndex+i, glm::vec3{0,1,0} * (BULLET_SPEED * duration));
		}

		for (size_t i=0; i<enemyBulletCount; ++i) {
			translateModelPos(enemyBulletIndex+i, glm::vec3{0,-1,0} * (BULLET_SPEED * duration));
		}
	}

	void doAnimation(float duration) {
		doPlayerAnimation(duration);
		doMobAnimation(duration);
		doBossAnimation(duration);
		doBulletAnimation(duration);
	}

	void doRandomShooting() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		for (size_t mobIndex : mobIndices) {
			if (inBounds(models[mobIndex].position) && dis(gen) < SHOOTING_PERCENTAGE_CHANCE) {
				shoot(mobIndex, enemyBulletIndex, enemyBulletCount, enemyCurrentBullet);
			}
		}
	}

	void tick(float duration) {
		detectCollisions();

		doAnimation(duration);
		doRandomShooting();

		uint32_t sleepTime = 1000 * (1-duration) / FRAME_RATE;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime > 1 ? sleepTime : 1));

		#ifndef NDEBUG
		measureFramerate();
		#endif
	}

	void updateCamera() {
		UniformBufferObject ubo;
		ubo.view = glm::lookAt(glm::vec3(spacing * 5, -10.0f, 200.0f), glm::vec3(spacing * 5, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 1000.0f);
		ubo.proj[1][1] *= -1;
		updateUniformBuffers(ubo);
	}

	void setup() {
		updateCamera();

		setModelPos(playerIndex, {5 * spacing, -50, 0});
		setModelPos(bossIndex, {5 * spacing, 5 * spacing, 0});

		for (size_t i=0; i<playerBulletCount; ++i) {
			setModelPos(playerBulletIndex + i, {0, 0, -100000});
		}

		for (size_t i=0; i<enemyBulletCount; ++i) {
			setModelPos(enemyBulletIndex + i, {0, 0, -100000});
		}

		for (size_t i=0; i<11; ++i) {
			setModelPos(enemy1Index+i, {i * spacing, 0, 0});
			setModelPos(enemy1Index+11+i, {i * spacing, spacing, 0});

			setModelPos(enemy2Index+i, {i * spacing, 2 * spacing, 0});
			setModelPos(enemy2Index+11+i, {i * spacing, 3 * spacing, 0});

			setModelPos(enemy3Index+i, {i * spacing, 4 * spacing, 0});
		}
	}

	void addVertex(const Vertex& v, std::unordered_map<Vertex, size_t> &uniqueVertices) {
		if (uniqueVertices.count(v) == 0) {
			uniqueVertices[v] = (size_t) vertices.size();
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

		std::unordered_map<Vertex, size_t> uniqueVertices;

		size_t textureCount;
		modelsFile >> textureCount;

		std::string modelName;
		uint32_t height, count, textureId;
		while (modelsFile >> modelName >> height >> textureId >> count) {
			double textureBase = (double) textureId / textureCount;
			double nextTextureBase = (double) (textureId + 1) / textureCount - 1 / TEXTURE_WIDTH;

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

			for (size_t i=height; i-- > 0; ) {
				std::string line;
				modelsFile >> line;

				m.size[0] = std::max(m.size[0], (float) line.length());

				for (size_t j=0; j<line.length(); ++j) {
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

			m.radius = glm::length(m.size) / 2;

			for (size_t i=0; i<count; ++i) {
				models.push_back(m);
			}
		}

		for (size_t i=0; i<enemy1Count; ++i) {
			mobIndices.push_back(enemy1Index + i);
		}

		for (size_t i=0; i<enemy2Count; ++i) {
			mobIndices.push_back(enemy2Index + i);
		}

		for (size_t i=0; i<enemy3Count; ++i) {
			mobIndices.push_back(enemy3Index + i);
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
			std::cout << "\rFrame rate was " << frames << " average is " << frameRate;
			std::cout.flush();

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