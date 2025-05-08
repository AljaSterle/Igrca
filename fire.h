#ifndef FIRE_H
#define FIRE_H

#include "game_object.h"
#include "sprite_renderer.h"

class Fire : public GameObject { 
	const float flek = 2.0f;
	bool nekoc;
	float flekCounter;

    const float not_expanding_yet = 3.0f;
    //const float expansion_value = 2.0f;
    bool expanded;
    float expand = 0.0f;

public:
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

    glm::vec2 GetPosition() const;
    glm::vec2 GetSize() const ;
    Direction GetDirection();
    void SetPosition(glm::vec2);
    void SetSize(glm::vec2);
    void SetDirection(Direction);
    void SetSprite(Texture2D);

    float GetFlek();
    float GetNot_expanding_yet();

    bool GetExpanded();
    float GetExpand();
    bool GetNekoc();
    float GetFlekCounter();
    void SetExpanded(bool);
    void SetExpand(float);
    void SetNekoc(bool);
    void AddFlekCounter(float);
};

#endif // !FIRE_H