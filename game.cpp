#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "shader.h"
#include "texture.h"

SpriteRenderer* Renderer;

Game::Game(unsigned int width, unsigned int height)
    : Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

void Game::Init()
{
	ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

	Shader tmps = ResourceManager::GetShader("sprite");
	tmps.Use();
	tmps.SetInteger("image", 0);
	tmps.SetMatrix4("projection", projection);
	Renderer = new SpriteRenderer(tmps);
	ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");

}

void Game::Update(float dt)
{

}

void Game::ProcessInput(float dt)
{

}

void Game::Render()
{
	Texture2D ttmp = ResourceManager::GetTexture("face");
	Renderer->DrawSprite(ttmp,
		// size						position				rotation		color	
		glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}