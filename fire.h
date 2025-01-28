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

    // Define the copy constructor
    Fire(const Fire& other)
        : GameObject(other), flek(other.flek), nekoc(other.nekoc), flekCounter(other.flekCounter) {
        // Copy other members if necessary
    }

    // Define the copy assignment operator
    Fire& operator=(const Fire& other) {
        if (this != &other) {
            GameObject::operator=(other);
            nekoc = other.nekoc;
            flekCounter = other.flekCounter;
            // Copy other members if necessary
        }
        return *this;
    }

    // Define the equality operator
    bool operator==(const Fire& other) const {
        return this->Position == other.Position && this->Size == other.Size && this->Sprite.ID == other.Sprite.ID;
    }
};

#endif // !FIRE_H