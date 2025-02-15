#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "shader.h"
#include "texture.h"
#include "game_object.h"
#include "fire.h"

#include "GLUtils.h"

#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

SpriteRenderer* renderer;
GameObject* player;

const float spawn = 5.0f;
float fromSpawn = 0.0f;

const glm::vec2 PLAYER_SIZE(20, 20);
const float PLAYER_VELOCITY(300.0f);

Game::Game(unsigned int width, unsigned int height)
	: Keys(), Width(width), Height(height), State(GAME_ACTIVE), level(0), startFires(false), kurjenje(0.0f)
{

}

Game::~Game()
{
	delete player;
	delete renderer;
}

void Game::Init()
{
	srand(time(0));

	ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
	
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

	Shader tmps = ResourceManager::GetShader("sprite");
	tmps.Use();
	tmps.SetInteger("image", 0);
	tmps.SetMatrix4("projection", projection);

	renderer = new SpriteRenderer(tmps);

	ResourceManager::LoadTexture("awesomeface.png", true, "face");
	ResourceManager::LoadTexture("block.png", true, "block");
	ResourceManager::LoadTexture("ogenjcek.jpg", false, "fire");
	ResourceManager::LoadTexture("burnt.png", true, "burnt");
	ResourceManager::LoadTexture("menu.png", true, "menu");
	ResourceManager::LoadTexture("native-american.png", true, "indijanec");
	ResourceManager::LoadTexture("pozigalec.png", true, "pozigalec");

	// load levels
	GameLevel one;
	GameLevel two;
	this->levels.push_back(one);
	this->levels.push_back(two);
	this->level = 0;

	glm::vec2 playerSize = glm::vec2(this->Width * 0.05f, this->Width * 0.05f);
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - playerSize.x / 2, this->Height / 2 - playerSize.y);
	
	player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("block"));
	fires.clear();
	burnt.clear();
	for (int i = 0; i < 3; i++) {
		indijanci.push_back(GameObject(glm::vec2(50, this->Height-50), glm::vec2(50, 50), ResourceManager::GetTexture("indijanec")));
		pozigalci.push_back(GameObject(glm::vec2(this->Width - 50, 50), glm::vec2(50, 50), ResourceManager::GetTexture("pozigalec")));
	}

	this->State = GAME_MENU;
}

float randomNumber(int min, int max) {
	if (min > max) {
		std::swap(min, max);
	}
	float x = rand() % (max - min + 1) + min;
	return x;
}

void Collison(std::vector<Fire>& fires, GameObject& obj) {
	auto rem = std::remove_if(fires.begin(), fires.end(), [&obj](Fire& fire) {
		bool collisionX = obj.Position.x + obj.Size.x >= fire.Position.x &&
			obj.Position.x <= fire.Position.x + fire.Size.x;
		bool collisionY = obj.Position.y + obj.Size.y >= fire.Position.y &&
			obj.Position.y <= fire.Position.y + fire.Size.y;
		return collisionX && collisionY;
		});
	fires.erase(rem, fires.end());
}


void Game::DoCollisions() {
	auto rem = std::remove_if(fires.begin(), fires.end(), [this](Fire& fire) {
		bool collisionX = player->Position.x + player->Size.x >= fire.Position.x &&
			player->Position.x <= fire.Position.x + fire.Size.x;
		bool collisionY = player->Position.y + player->Size.y >= fire.Position.y &&
			player->Position.y <= fire.Position.y + fire.Size.y;
		return collisionX && collisionY;
		});
	fires.erase(rem, fires.end());
}

void Game::Resize(float width, float height)
{
	float wratio = width / this->Width;
	float hratio = height / this->Height;

	float min = std::min(width, height);

	// Adjust player position and size
	player->Position.x *= wratio;
	player->Position.y *= hratio;

	// Adjust positions and sizes of other game objects if necessary
	for (Fire& fire : fires) {
		fire.Position.x *= wratio;
		fire.Position.y *= hratio;
		fire.Size.x = 0.05f * min;
		fire.Size.y = 0.05f * min;
	}

	for (Fire& burnt : burnt) {
		burnt.Position.x *= wratio;
		burnt.Position.y *= hratio;
		burnt.Size.x = 0.05f * min;
		burnt.Size.y = 0.05f * min;
	}

	for (GameObject& indijanec : indijanci) {
		indijanec.Position.x *= wratio;
		indijanec.Position.y *= hratio;
		indijanec.Size.x = 0.05f * min;
		indijanec.Size.y = 0.05f * min;
	}

	for (GameObject& hejtr : pozigalci) {
		hejtr.Position.x *= wratio;
		hejtr.Position.y *= hratio;
		hejtr.Size.x = 0.05f * min;
		hejtr.Size.y = 0.05f * min;
	}
}

void checkPosition(GameObject& obj, int width, int height) {
	if (obj.Position.x < 0)
		obj.Position.x = 0;
	if (obj.Position.x > width - obj.Size.x)
		obj.Position.x = width - obj.Size.x;
	if (obj.Position.y < 0)
		obj.Position.y = 0;
	if (obj.Position.y > height - obj.Size.y)
		obj.Position.y = height - obj.Size.y;
}

void Game::Update(float dt) 
{
	if (this->State == GAME_ACTIVE)
		startFires = true;
	else if (this->State == GAME_MENU)
		startFires = false;

	// player resizing
	float playerSize = std::min(this->Width, this->Height) * 0.05f;
	player->Size.x = playerSize;
	player->Size.y = playerSize;

	// fires & burns
	if (startFires == true) {	
		for (Fire& fire : fires) {

			// if it hasn't burnt forest yet
			if (fire.nekoc == false) { 
				fire.flekCounter += dt;
				if (fire.flekCounter >= fire.flek) { // should it make a burn?
					float x = fire.Position.x - (fire.Size.x/2);
					float y = fire.Position.y - (fire.Size.y / 2);
					burnt.push_back(Fire(glm::vec2(x, y), glm::vec2(100, 100), ResourceManager::GetTexture("burnt")));
					fire.nekoc = true; // it made a burn :)
				}
			
			}

			// fire multiplying after specific period
			if (fire.expand > fire.not_expanding_yet) {
				std::cout << "Expansionnnn" << std::endl;
				// Expanding by spawning new fires beside old ones
				
				int x_temp = randomNumber(0, this->Width);
				int y_temp = randomNumber(0, this->Height);
				glm::vec2 direction = glm::normalize(glm::vec2(x_temp, y_temp));
				int x_pos = fire.Position.x + direction.x * fire.Size.x;
				int y_pos = fire.Position.y + direction.y * fire.Size.y;
				
				fires.push_back(Fire(glm::vec2(x_pos, y_pos), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
				fire.expand = 0.0f;
				
				fire.expanded = true;
			}
			else {
				fire.expand += dt;
			}

			// fire resizing
			fire.Size.x = 0.05f * std::min(Width, Height);
			fire.Size.y = 0.05f * std::min(Width, Height);
		}

		// burns resizing
		for (Fire& burnt : burnt) {
			burnt.Size.x = 0.1f * std::min(Width, Height);
			burnt.Size.y = 0.1f * std::min(Width, Height);
		}

		// spawning fire
		fromSpawn += dt;
		if (fromSpawn >= spawn) {
			fromSpawn = 0.0f;
			float x = rand() % (this->Width);
			float y = rand() % (this->Height);
			fires.push_back(Fire(glm::vec2(x, y), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
		}
		DoCollisions();

		int moverandx;
		int moverandy;
		float radius = std::min(this->Width, this->Height) / 5;

		// indijanci movement
		for (GameObject& indijanec : indijanci) { 
			bool inside = false;
			for (Fire& fire : fires) {
				float distance = glm::distance(indijanec.Position, fire.Position);
				if (distance < radius) {
					glm::vec2 direction = glm::normalize(fire.Position - indijanec.Position);
					indijanec.Position += direction * PLAYER_VELOCITY * dt;
					inside = true;
				}
			}
			if (!inside) { // move randomly
				int kir = randomNumber(2, 1);
				if (kir == 1) {
					moverandx = randomNumber(20, -20);
					indijanec.Position.x += moverandx;
				}
				else {
					moverandy = randomNumber(20, -20);
					indijanec.Position.y += moverandy;
				}
				checkPosition(indijanec, this->Width, this->Height);

			}
			Collison(fires, indijanec);
		}		
		
		// pozigalci movement
		for (GameObject& pozigalec : pozigalci) {
			int kir = randomNumber(2, 1);
			if (kir == 1) {
				moverandx = randomNumber(20, -20);
				pozigalec.Position.x += moverandx;
			}
			else {
				moverandy = randomNumber(20, -20);
				pozigalec.Position.y += moverandy;
			}
			checkPosition(pozigalec, this->Width, this->Height);
		}

		// random pozigalec sproži ognj
		kurjenje += dt;
		if (kurjenje > 5.0f) {
			int who = randomNumber(0, pozigalci.size()-1);
			fires.push_back(Fire(glm::vec2(pozigalci.at(who).Position), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
			kurjenje = 0;
		}
	}
}


void Game::ProcessInput(float dt)
{
	if (this->State == GAME_MENU) {
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE]) {
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}
	}
	if (this->State == GAME_ACTIVE) {
		float velocity = PLAYER_VELOCITY * dt;
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE]) {
			this->State = GAME_MENU;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}
		if (this->Keys[GLFW_KEY_A]) {
			if (player->Position.x >= 0)
				player->Position.x -= velocity;
		}
		if (this->Keys[GLFW_KEY_D]) {
			if (player->Position.x <= this->Width - player->Size.x)
				player->Position.x += velocity;
		}
		if (this->Keys[GLFW_KEY_W]) {
			if (player->Position.y >= 0)
				player->Position.y -= velocity;
		}
		if (this->Keys[GLFW_KEY_S]) {
			if (player->Position.y <= this->Height - player->Size.y)
				player->Position.y += velocity;
		}
	}
}

void Game::Render()
{
	glClearColor(0.13f, 0.56f, 0.13f, 1.0f);

	if (this->State == GAME_ACTIVE || this->State == GAME_MENU) {
		for (Fire& burnt : burnt)
			burnt.Draw(*renderer);
		for (Fire& fire : fires)
			fire.Draw(*renderer);
		for (GameObject& indijanec : indijanci)
			indijanec.Draw(*renderer);
		for (GameObject& hejtr : pozigalci)
			hejtr.Draw(*renderer);
		player->Draw(*renderer);
	}
	if (this->State == GAME_MENU) {
		Texture2D menu = ResourceManager::GetTexture("menu");
		renderer->DrawSprite(menu, glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
	}
	
	IsThereError();
}
