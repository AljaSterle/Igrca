#include "fire.h"
#include "game_object.h"

Fire::Fire(glm::vec2 pos, glm::vec2 size, Texture2D sprite)
	: GameObject(pos, size, sprite, glm::vec4(1.0), glm::vec2(0.0, 0.0)), flekCounter(0), nekoc(false)
{
}

void Fire::Draw(SpriteRenderer& renderer)
{
	renderer.DrawSprite(this->Sprite, this->Position, this->Size, 0.0, this->Color);
}