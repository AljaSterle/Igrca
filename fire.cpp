#include "fire.h"
#include "game_object.h"

Fire::Fire(glm::vec2 pos, glm::vec2 size, Texture2D sprite)
	: GameObject(pos, size, sprite, glm::vec4(1.0), glm::vec2(0.0, 0.0)), flekCounter(0), nekoc(false), expanded(false)
{
}

void Fire::Draw(SpriteRenderer& renderer)
{
	renderer.DrawSprite(this->Sprite, this->Position, this->Size, 0.0, this->Color);
}
void Fire::SetPosition(glm::vec2 p) {
    this->Position = p;
}
void Fire::SetSize(glm::vec2 s) {
    this->Size = s;
}
void Fire::SetDirection(Direction d) {
    this->direction = d;
}
void Fire::SetSprite(Texture2D s) {
    this->Sprite = s;
}
glm::vec2 Fire::GetPosition() const {
    return this->Position;
}
glm::vec2 Fire::GetSize() const {
    return this->Size;
}
Direction Fire::GetDirection() {
    return this->direction;
}
float Fire::GetFlek() {
    return this->flek;
}
float Fire::GetNot_expanding_yet() {
    return not_expanding_yet;
}
bool Fire::GetExpanded() {
    return this->expanded;
}
float Fire::GetExpand() {
    return this->expand;
}
bool Fire::GetNekoc() {
    return this->nekoc;
}
float Fire::GetFlekCounter() {
    return this->flekCounter;
}
void Fire::SetExpanded(bool b) {
    this->expanded = b;
}
void Fire::SetExpand(float f) {
    if (f == 0.0f)   this->expand = f;
    else this->expand += f;
}
void Fire::SetNekoc(bool b) {
    nekoc = b;
}
void Fire::AddFlekCounter(float f) {
    this->flekCounter += f;
}