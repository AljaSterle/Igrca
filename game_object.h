#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm-master/glm-master/glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"

enum Direction {UP, DOWN, RIGHT, LEFT};

class GameObject
{
public:
    // object state
    glm::vec2   Position, Size, Velocity;
	glm::vec4   Color; // RGBA
    Direction direction; // za NPC
    // render state
    Texture2D   Sprite;
    // constructor(s)
    GameObject();
    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec4 color = glm::vec4(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f));

    void move(); // also za NPC
    // draw sprite
    virtual void Draw(SpriteRenderer& renderer);
};

#endif