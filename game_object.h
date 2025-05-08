#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm-master/glm-master/glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"

enum Direction {UP, DOWN, RIGHT, LEFT};

class GameObject
{
protected:
    // object state
    glm::vec2   Position, Size, Velocity;
	glm::vec4   Color; // RGBA
    Direction direction; // za NPC
    // render state
    Texture2D   Sprite;

public:
    // constructor(s)
    GameObject();
    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec4 color = glm::vec4(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f));

    // draw sprite
    virtual void Draw(SpriteRenderer& renderer);

    virtual glm::vec2 GetPosition() const ;
    virtual glm::vec2 GetSize() const;
    virtual Direction GetDirection();
    virtual void SetPosition(glm::vec2);
    virtual void SetSize(glm::vec2);
    virtual void SetDirection(Direction);
    virtual void SetSprite(Texture2D);
};

#endif