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
#include <glad/glad.h>
#include <GLFW/glfw3.h>

SpriteRenderer* Renderer;
GameObject* Player;

const float spawn = 5.0f;
float fromSpawn = 0.0f;

const glm::vec2 PLAYER_SIZE(20, 20);
const float PLAYER_VELOCITY(300.0f);

Game::Game(unsigned int width, unsigned int height)
	: Keys(), Width(width), Height(height), State(GAME_ACTIVE), Level(0)
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

	// load levels
	GameLevel one;
	GameLevel two;
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Level = 0;

	glm::vec2 playerSize = glm::vec2(this->Width * 0.05f, this->Width * 0.05f);
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - playerSize.x / 2, this->Height - playerSize.y);
	
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("block"));
	Fires.clear();
	Burnt.clear();
}

void Game::DoCollisions() {
	for (auto it = Fires.begin(); it != Fires.end(); ) {
		bool collisionX = Player->Position.x + Player->Size.x >= it->Position.x &&
			Player->Position.x <= it->Position.x + it->Size.x;
		bool collisionY = Player->Position.y + Player->Size.y >= it->Position.y &&
			Player->Position.y <= it->Position.y + it->Size.y;
		if (collisionX && collisionY) {
			it = Fires.erase(it); // Erase the element and get the next iterator
			break;
		}
		else {
			++it; // Move to the next element
		}
	}
	/*for (Fire& fire : Fires) {
		bool collisionX = Player->Position.x + Player->Size.x >= fire.Position.x + fire.Size.x >= Player->Position.x;
		bool collisionY = Player->Position.y + Player->Size.y >= fire.Position.y + fire.Size.y >= Player->Position.y;
		if (collisionX && collisionY) {
			Fires.erase(std::remove(Fires.begin(), Fires.end(), fire), Fires.end());
			break;
		}
		
	}*/
}

void Game::Update(float dt)
{
	Player->Size.x = this->Width * 0.05f;
	Player->Size.y = this->Width * 0.05f;

	for (Fire& fire : Fires) {
		fire.flekCounter += dt;

		if (fire.flekCounter >= fire.flek && fire.nekoc == false) {
			float x = fire.Position.x - (fire.Size.x/2);
			float y = fire.Position.y - (fire.Size.y / 2);
			Burnt.push_back(Fire(glm::vec2(x, y), glm::vec2(100, 100), ResourceManager::GetTexture("burnt")));
			fire.nekoc = true;
		}
	}
	fromSpawn += dt;
	if (fromSpawn >= spawn) {
		fromSpawn = 0.0f;
		float x = rand() % (this->Width);
		float y = rand() % (this->Height);
		Fires.push_back(Fire(glm::vec2(x, y), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
	}
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_ACTIVE) {

		float velocity = PLAYER_VELOCITY * dt;
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
	if (this->State == GAME_ACTIVE) {
		for (Fire burnt : Burnt) {
			burnt.Draw(*Renderer);
		}
		for (Fire fire : Fires)
			fire.Draw(*Renderer);
		Player->Draw(*Renderer);
	}
	
	IsThereError();
}