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

SpriteRenderer* Renderer;
GameObject* Player;

const float spawn = 5.0f;
float fromSpawn = 0.0f;

const glm::vec2 PLAYER_SIZE(20, 20);
const float PLAYER_VELOCITY(300.0f);

Game::Game(unsigned int width, unsigned int height)
	: Keys(), Width(width), Height(height), State(GAME_ACTIVE), Level(0), startFires(false)
{

}

Game::~Game()
{

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

	Renderer = new SpriteRenderer(tmps);

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
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Level = 0;

	glm::vec2 playerSize = glm::vec2(this->Width * 0.05f, this->Width * 0.05f);
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - playerSize.x / 2, this->Height / 2 - playerSize.y);
	
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("block"));
	Fires.clear();
	Burnt.clear();
	for (int i = 0; i < 3; i++) {
		indijanci.push_back(GameObject(glm::vec2(50, this->Height-50), glm::vec2(50, 50), ResourceManager::GetTexture("indijanec")));
		pozigalci.push_back(GameObject(glm::vec2(this->Width - 50, 50), glm::vec2(50, 50), ResourceManager::GetTexture("pozigalec")));
	}

	this->State = GAME_MENU;
}

void Game::DoCollisions() {
	auto rem = std::remove_if(Fires.begin(), Fires.end(), [this](Fire& fire) {
		bool collisionX = Player->Position.x + Player->Size.x >= fire.Position.x &&
			Player->Position.x <= fire.Position.x + fire.Size.x;
		bool collisionY = Player->Position.y + Player->Size.y >= fire.Position.y &&
			Player->Position.y <= fire.Position.y + fire.Size.y;
		return collisionX && collisionY;
		});
	Fires.erase(rem, Fires.end());
}

void Game::Resize(float width, float height)
{
	float wratio = width / this->Width;
	float hratio = height / this->Height;

	float min = std::min(width, height);

	// Adjust player position and size
	Player->Position.x *= wratio;
	Player->Position.y *= hratio;

	// Adjust positions and sizes of other game objects if necessary
	for (Fire& fire : Fires) {
		fire.Position.x *= wratio;
		fire.Position.y *= hratio;
		fire.Size.x = 0.05f * min;
		fire.Size.y = 0.05f * min;
	}

	for (Fire& burnt : Burnt) {
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

void Game::Update(float dt)
{
	float playerSize = std::min(this->Width, this->Height) * 0.05f;
	Player->Size.x = playerSize;
	Player->Size.y = playerSize;
	if (this->State == GAME_ACTIVE)
		startFires = true;
	else if (this->State == GAME_MENU)
		startFires = false;
	if (startFires == true) {
		for (Fire& fire : Fires) {
			fire.flekCounter += dt;

			if (fire.flekCounter >= fire.flek && fire.nekoc == false) {
				float x = fire.Position.x - (fire.Size.x/2);
				float y = fire.Position.y - (fire.Size.y / 2);
				Burnt.push_back(Fire(glm::vec2(x, y), glm::vec2(100, 100), ResourceManager::GetTexture("burnt")));
				fire.nekoc = true;
			}
			fire.Size.x = 0.05f * std::min(Width, Height);
			fire.Size.y = 0.05f * std::min(Width, Height);
		}
		for (Fire& burnt : Burnt) {
			burnt.Size.x = 0.1f * std::min(Width, Height);
			burnt.Size.y = 0.1f * std::min(Width, Height);
		}
		fromSpawn += dt;
		if (fromSpawn >= spawn) {
			fromSpawn = 0.0f;
			float x = rand() % (this->Width);
			float y = rand() % (this->Height);
			Fires.push_back(Fire(glm::vec2(x, y), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
		}
		DoCollisions();
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
			if (Player->Position.x >= 0)
				Player->Position.x -= velocity;
		}
		if (this->Keys[GLFW_KEY_D]) {
			if (Player->Position.x <= this->Width - Player->Size.x)
				Player->Position.x += velocity;
		}
		if (this->Keys[GLFW_KEY_W]) {
			if (Player->Position.y >= 0)
				Player->Position.y -= velocity;
		}
		if (this->Keys[GLFW_KEY_S]) {
			if (Player->Position.y <= this->Height - Player->Size.y)
				Player->Position.y += velocity;
		}
	}
}

void Game::Render()
{
	glClearColor(0.13f, 0.56f, 0.13f, 1.0f);

	if (this->State == GAME_ACTIVE || this->State == GAME_MENU) {
		for (Fire& burnt : Burnt)
			burnt.Draw(*Renderer);
		for (Fire& fire : Fires)
			fire.Draw(*Renderer);
		for (GameObject& indijanec : indijanci)
			indijanec.Draw(*Renderer);
		for (GameObject& hejtr : pozigalci)
			hejtr.Draw(*Renderer);
		Player->Draw(*Renderer);
	}
	if (this->State == GAME_MENU) {
		Texture2D menu = ResourceManager::GetTexture("menu");
		Renderer->DrawSprite(menu, glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
	}
	
	IsThereError();
}
