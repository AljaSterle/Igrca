#ifndef FIRE_H
#define FIRE_H

#include "game_object.h"
#include "sprite_renderer.h"

class SideObject : public GameObject {
public: 
	const float flek = 2.0f;
	bool nekoc;
	float flekCounter;
	SideObject(glm::vec2, glm::vec2, Texture2D);
	void Draw(SpriteRenderer& renderer);

    // Define the copy constructor
    SideObject(const SideObject& other)
        : GameObject(other), flek(other.flek), nekoc(other.nekoc), flekCounter(other.flekCounter) {
        // Copy other members if necessary
    }

    // Define the copy assignment operator
    SideObject& operator=(const SideObject& other) {
        if (this != &other) {
            GameObject::operator=(other);
            nekoc = other.nekoc;
            flekCounter = other.flekCounter;
            // Copy other members if necessary
        }
        return *this;
    }

    // Define the equality operator
    bool operator==(const SideObject& other) const {
        return this->Position == other.Position && this->Size == other.Size && this->Sprite.ID == other.Sprite.ID;
    }
};

#endif // !FIRE_H