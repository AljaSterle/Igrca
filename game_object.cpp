#include "game_object.h"


GameObject::GameObject()
    : Position(0.0f, 0.0f), Size(1.0f, 1.0f), Velocity(0.0f), Color(1.0f), direction(static_cast<Direction>(rand() % 4)) { }

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec4 color, glm::vec2 velocity)
    : Position(pos), Size(size), Velocity(velocity), Color(color), Sprite(sprite), direction(static_cast<Direction>(rand() % 4)) {}

void GameObject::Draw(SpriteRenderer& renderer)
{
    renderer.DrawSprite(this->Sprite, this->Position, this->Size, 0.0, this->Color);
}
void GameObject::SetPosition(glm::vec2 p) {
    this->Position = p;
}
void GameObject::SetSize(glm::vec2 s) {
    this->Size = s;
}
void GameObject::SetDirection(Direction d) {
    this->direction = d;
}
void GameObject::SetSprite(Texture2D s) {
    this->Sprite = s;
}
glm::vec2 GameObject::GetPosition() const {
    return this->Position;
}
glm::vec2 GameObject::GetSize() const {
    return this->Size;
}
Direction GameObject::GetDirection() {
    return this->direction;
}