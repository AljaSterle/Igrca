#ifndef FIRE_H
#define FIRE_H

#include "game_object.h"
#include "sprite_renderer.h"

class Fire : public GameObject {
public: 
	const float flek = 2.0f;
	bool nekoc;
	float flekCounter;
	Fire(glm::vec2, glm::vec2, Texture2D);
	void Draw(SpriteRenderer& renderer);
};

#endif // !FIRE_H