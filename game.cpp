#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "shader.h"
#include "texture.h"
#include "game_object.h"

#include "GLUtils.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

SpriteRenderer* Renderer;
GameObject* Player;

const glm::vec2 PLAYER_SIZE(100, 100);
const float PLAYER_VELOCITY(500.0f);

Game::Game(unsigned int width, unsigned int height)
    : Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

void Game::Init()
{
	ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
	
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

	Shader tmps = ResourceManager::GetShader("sprite");
	tmps.Use();
	tmps.SetInteger("image", 0);
	tmps.SetMatrix4("projection", projection);

	Renderer = new SpriteRenderer(tmps);

	ResourceManager::LoadTexture("awesomeface.png", true, "face");
	ResourceManager::LoadTexture("block.png", true, "block");

	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Texture2D ttmp3 = ResourceManager::GetTexture("block");
	Player = new GameObject(playerPos, PLAYER_SIZE, ttmp3);
}

void Game::Update(float dt)
{

}

void Game::ProcessInput(float dt)
{
	float velocity = PLAYER_VELOCITY * dt;
	if (this->Keys[GLFW_KEY_A])
	{
		if (Player->Position.x >= 0)
			Player->Position.x -= velocity;
	}
	if (this->Keys[GLFW_KEY_D])
	{
		if (Player->Position.x <= this->Width - Player->Size.x)
			Player->Position.x += velocity;
	}
	if (this->Keys[GLFW_KEY_W])
	{
		if (Player->Position.y >= 0)
			Player->Position.y -= velocity;
	}
	if (this->Keys[GLFW_KEY_S])
	{
		if (Player->Position.y <= this->Height - Player->Size.y)
			Player->Position.y += velocity;
	}

}

void Game::Render()
{
	Texture2D ttmp = ResourceManager::GetTexture("face");
	Texture2D ttmp2 = Player->Sprite;
	Renderer->DrawSprite(ttmp,
		// position					size					rotation		color	
		glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	 //Player->Draw(*Renderer);
	// std::cout << Player->Color.x << " " << Player->Color.y << " " << Player->Color.z << std::endl;
	
	Renderer->DrawSprite(ttmp2,
		// position					size					rotation		color	
	Player->Position, Player->Size, 0.0f, Player->Color);
	IsThereError();
}