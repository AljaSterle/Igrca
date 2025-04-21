#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "shader.h"
#include "texture.h"
#include "game_object.h"
#include "fire.h"
#include "text_renderer.h"

#include "GLUtils.h"

#include <cmath>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

SpriteRenderer* renderer;
GameObject* player;
TextRenderer* text;

const float spawn = 5.0f;
float fromSpawn = 0.0f;

const glm::vec2 PLAYER_SIZE(20, 20);
const float PLAYER_VELOCITY(300.0f);
const float OTHER_VELOCITY(300.0f);

std::ofstream replayWrite;
std::ifstream replayRead;

struct Poz {
	glm::vec2 pozicija;
};

struct Event
{
	float x, y1, y2;
	int type; // 1 start, -1 end

	bool operator<(const Event& other) const {
		if (x == other.x)
			return type > other.type;
		return x < other.x;
	}
};

float calculateBurntArea(const std::vector<Fire>& burnt) {
	std::vector<Event> events;
	for (const auto& fire : burnt) {
		events.push_back({ fire.Position.x, fire.Position.y, fire.Position.y + fire.Size.y, 1 });
		events.push_back({ fire.Position.x + fire.Size.x, fire.Position.y, fire.Position.y + fire.Size.y, -1 });
	}

	std::sort(events.begin(), events.end());

	std::multiset<std::pair<float, float>> activeIntervals;
	float prevX = 0;
	float burntArea = 0;

	for (const auto& event : events) {
		float currentX = event.x;

		// Calculate the total length of active intervals
		float totalY = 0;
		float prevY = -1;
		for (const auto& interval : activeIntervals) {
			if (interval.first > prevY) {
				totalY += interval.second - interval.first;
				prevY = interval.second;
			}
			else if (interval.second > prevY) {
				totalY += interval.second - prevY;
				prevY = interval.second;
			}
		}

		// Calculate the area contribution
		burntArea += totalY * (currentX - prevX);
		prevX = currentX;

		// Update the active intervals
		if (event.type == 1) {
			activeIntervals.insert({ event.y1, event.y2 });
		}
		else {
			activeIntervals.erase(activeIntervals.find({ event.y1, event.y2 }));
		}
	}

	return burntArea;
}
Game::Game(unsigned int width, unsigned int height)
	: Keys(), Width(width), Height(height), State(GAME_ACTIVE), level(0), startFires(false), kurjenje(0.0f), nextLevel(false), PrevState(GAME_MENU), burntPercentage(0.0f), won(false), lost(false), konc(false)
{

}
Game::~Game()
{
	delete player;
	delete renderer;
}

void Game::Init()
{
	srand(time(0));

	ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
	IsThereError();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

	Shader tmps = ResourceManager::GetShader("sprite");
	tmps.Use();
	tmps.SetInteger("image", 0);
	tmps.SetMatrix4("projection", projection);

	renderer = new SpriteRenderer(tmps);

	ResourceManager::LoadTexture("awesomeface.png", true, "face");
	ResourceManager::LoadTexture("block.png", true, "block");
	ResourceManager::LoadTexture("betterfire.png", true, "fire");
	ResourceManager::LoadTexture("burnt.png", true, "burnt");
	ResourceManager::LoadTexture("menu.png", true, "menu");
	ResourceManager::LoadTexture("native-american.png", true, "indijanec");
	ResourceManager::LoadTexture("ogpozigalec.png", false, "pozigalec");

	// load levels
	this->levels[0] = {glm::vec3(0.0f, 0.678f, 0.043f), 1.0f}; // Level 1: green background, normal speed
	this->levels[1] = {glm::vec3(0.0f, 0.588f, 0.035f), 1.0f}; // Level 2: red background, faster speed
	this->levels[2] = {glm::vec3(0.0f, 0.478f, 0.031f), 1.0f}; // Level 3: red background, even faster speed
	this->level = 0;
	//replay = false;

	glm::vec2 playerSize = glm::vec2(this->Width * 0.05f, this->Width * 0.05f);
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - playerSize.x / 2, this->Height / 2 - playerSize.y);
	
	player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("block"));
	fires.reserve(10000);
	burnt.reserve(10000);
	fires.clear();
	burnt.clear();
	for (int i = 0; i < 3; i++) {
		indijanci.push_back(GameObject(glm::vec2(50, this->Height-50), glm::vec2(50, 50), ResourceManager::GetTexture("indijanec")));
		pozigalci.push_back(GameObject(glm::vec2(this->Width - 50, 50), glm::vec2(50, 50), ResourceManager::GetTexture("pozigalec")));
	}

	text = new TextRenderer(this->Width, this->Height);
	text->Load("arial.ttf", 24);

	this->LevelInitialize();
	this->State = GAME_MENU;
	this->PrevState = this->State;
}

void Game::LevelInitialize(char c) {
	indijanci.clear();
	pozigalci.clear();

	if (c == 'p') {
		fires.clear();
		burnt.clear();
		won = false; lost = false;
	}

	glm::vec2 playerSize = glm::vec2(this->Width * 0.05f, this->Width * 0.05f);
	player->Position = glm::vec2(this->Width / 2 - playerSize.x / 2, this->Height / 2 - playerSize.y);
	
	for (int i = 0; i < 3; i++) {
		indijanci.push_back(GameObject(glm::vec2(50, this->Height - 50), glm::vec2(50, 50), ResourceManager::GetTexture("indijanec")));
		pozigalci.push_back(GameObject(glm::vec2(this->Width - 50, 50), glm::vec2(50, 50), ResourceManager::GetTexture("pozigalec")));
	}

	this->background = levels[level].color;
	this->State = GAME_ACTIVE;

	konc = false;

	replayWrite.open("replay.bin", std::ios::binary);
}

float randomNumber(int min, int max) {
	if (min > max) {
		std::swap(min, max);
	}
	float x = rand() % (max - min + 1) + min;
	return x;
}

template <typename Container, typename Object>
void HandleCollisions(Container& objects, Object& target, std::function<void()> onCollision = nullptr) {
	auto rem = std::remove_if(objects.begin(), objects.end(), [&target](auto& obj) {
		bool collisionX = target.Position.x + target.Size.x >= obj.Position.x &&
			target.Position.x <= obj.Position.x + obj.Size.x;
		bool collisionY = target.Position.y + target.Size.y >= obj.Position.y &&
			target.Position.y <= obj.Position.y + obj.Size.y;
		return collisionX && collisionY;
		});

	if (onCollision) {
		for (auto it = rem; it != objects.end(); ++it) {
			onCollision();
		}
	}

	objects.erase(rem, objects.end());
}

void Game::Resize(float width, float height)
{
	float wratio = width / this->Width;
	float hratio = height / this->Height;

	float min = std::min(width, height);

	// Adjust player position and size
	player->Position.x *= wratio;
	player->Position.y *= hratio;

	// Adjust positions and sizes of other game objects if necessary
	for (Fire& fire : fires) {
		fire.Position.x *= wratio;
		fire.Position.y *= hratio;
		fire.Size.x = 0.05f * min;
		fire.Size.y = 0.05f * min;
	}

	for (Fire& burnt : burnt) {
		burnt.Position.x *= wratio;
		burnt.Position.y *= hratio;
		burnt.Size.x = 0.05f * min;
		burnt.Size.y = 0.05f * min;
	}

	for (GameObject& indijanec : indijanci) {
		indijanec.Position.x *= wratio;
		indijanec.Position.y *= hratio;
		indijanec.Size.x = 0.05f * min;
		indijanec.Size.y = 0.05f * min;
	}

	for (GameObject& hejtr : pozigalci) {
		hejtr.Position.x *= wratio;
		hejtr.Position.y *= hratio;
		hejtr.Size.x = 0.05f * min;
		hejtr.Size.y = 0.05f * min;
	}
}

void ValidatePosition(GameObject& obj, int width, int height) {
	if (obj.Position.x < 0)
		obj.Position.x = 0;
	if (obj.Position.x > width - obj.Size.x)
		obj.Position.x = width - obj.Size.x;
	if (obj.Position.y < 0)
		obj.Position.y = 0;
	if (obj.Position.y > height - obj.Size.y)
		obj.Position.y = height - obj.Size.y;
}
bool Game::checkPosition(int x, int y) {
	int manjsi = std::min(this->Width, this->Height);
	if((x + manjsi/5)<this->Width && (x >= 0) && ((y + manjsi / 5)<this->Height && y >= 0))
		return true;
	return false;
}

void Game::StartReplay() {
	//std::cout << "REPLAYYYY" << std::endl;
	replayRead.open("replay.bin", std::ios::binary);
	this->PrevState = this->State;
	this->State = GAME_REPLAY;
	replayFrameCounter = 0;
}

void Game::ReplayUpdate() {
	//std::cout << "PREPLAY UPDEJTTTTTT" << std::endl;
	if (replayRead.is_open()) {
		struct Poz p;
		if (replayRead.read((char*)&p, sizeof(p))) {
			player->Position = p.pozicija;
		}
		else {
			// End of replay
			replayRead.close();
			this->State = this->PrevState;
		}
		
	}
}

struct Zapis {
	char name[30];
	int level;
	float proc;
	float score;
};

void Game::SaveToLeaderboard() {
	std::ofstream odata("new.bin", std::ios::binary);
	std::ifstream idata("leaderboard.bin", std::ios::binary);
	struct Zapis notr;
	strcpy_s(notr.name, playerName.c_str());
	notr.level = level;
	notr.proc = burntPercentage;
	notr.score = (level * 100) - (burntPercentage * 10);
	int i = 0;
	struct Zapis beri;  bool already = false;
	while (idata.read((char*)&beri, sizeof(beri)) && i < 5) {
		if (beri.score < notr.score && !already) {
			odata.write((char*)&notr, sizeof(notr));
			already = true;
			i++;
		}
		odata.write((char*)&beri, sizeof(beri));
		i++;
	}
	if (!already && i < 5) {
		odata.write((char*)&notr, sizeof(notr));
	}
	idata.close(); odata.close();
	remove("leaderboard.bin"); rename("new.bin", "leaderboard.bin");
	playerName.clear();
}

void Game::Update(float dt) 
{
	if (this->State == GAME_ACTIVE) {
		startFires = true;

		if (replayWrite.is_open()) {
			struct Poz p;
			p.pozicija = player->Position;
			replayWrite.write((char*)&p, sizeof(p));
		}
	}
	else if (this->State == GAME_MENU || this->State == GAME_LOST || this->State == GAME_MID_LEVEL)
		startFires = false;

	// player resizing
	float playerSize = std::min(this->Width, this->Height) * 0.05f;
	player->Size.x = playerSize;
	player->Size.y = playerSize;

	// fires & burns
	if (startFires == true) {	
		for (Fire& fire : fires) {

			// if it hasn't burnt forest yet
			if (fire.nekoc == false) { 
				fire.flekCounter += dt;
				if (fire.flekCounter >= fire.flek) { // should it make a burn?
					float x = fire.Position.x - (fire.Size.x/2);
					float y = fire.Position.y - (fire.Size.y / 2);
					burnt.push_back(Fire(glm::vec2(x, y), glm::vec2(100, 100), ResourceManager::GetTexture("burnt")));
					fire.nekoc = true; // it made a burn :)
				}
			
			}

			// fire multiplying after specific period
			if (fire.expand > fire.not_expanding_yet) {
				//std::cout << "Expansionnnn" << std::endl;
				// Expanding by spawning new fires beside old ones
				
				int x_temp = randomNumber(0, this->Width);
				int y_temp = randomNumber(0, this->Height);
				glm::vec2 direction = glm::normalize(glm::vec2(x_temp, y_temp));
				int x_pos = (fire.Position.x + direction.x * fire.Size.x) + 50.0f;
				int y_pos = (fire.Position.y + direction.y * fire.Size.y) + 50.0f;
				if (checkPosition(x_pos, y_pos)) {
					fires.push_back(Fire(glm::vec2(x_pos, y_pos), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
				}
				fire.expand = 0.0f;
				
				fire.expanded = true;
			}
			else {
				fire.expand += dt;
			}

			// fire resizing
			fire.Size.x = 0.05f * std::min(Width, Height);
			fire.Size.y = 0.05f * std::min(Width, Height);
		}

		// spawning fire
		fromSpawn += dt;
		if (fromSpawn >= spawn) {
			fromSpawn = 0.0f;
			float x = rand() % (this->Width);
			float y = rand() % (this->Height);
			fires.push_back(Fire(glm::vec2(x, y), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
		}
		// DoCollisions();
		HandleCollisions(fires, *player);
		//CollisionCigani();
		HandleCollisions(pozigalci, *player, [this]() { State = GAME_LOST; });

		int moverand;
		float radius = std::min(this->Width, this->Height) / 2;

		// indijanci movement
		for (GameObject& indijanec : indijanci) { // following
			bool inside = false;
			for (Fire& fire : fires) {
				float distance = glm::distance(indijanec.Position, fire.Position);
				if (distance < radius) {
					glm::vec2 direction = glm::normalize(fire.Position - indijanec.Position);
					indijanec.Position += direction * OTHER_VELOCITY * dt;
					inside = true;
				}
			}
			if (!inside) { // move randomly
				if (static_cast<float>(rand() / RAND_MAX < 0.3)) {
					indijanec.direction = static_cast<Direction>(rand() % 4);
				}
				moverand = randomNumber(10, -10);
				switch (indijanec.direction)
				{
				case UP:
					indijanec.Position.y += moverand * OTHER_VELOCITY * dt;
					break;
				case DOWN:
					indijanec.Position.y += moverand * OTHER_VELOCITY * dt;
					break;
				case RIGHT:
					indijanec.Position.x += moverand * OTHER_VELOCITY * dt;
					break;
				case LEFT:
					indijanec.Position.x += moverand * OTHER_VELOCITY * dt;
					break;
				default:
					break;
				}

				ValidatePosition(indijanec, this->Width, this->Height);

			}
			// Collison(fires, indijanec);
			HandleCollisions(fires, indijanec);
			// CollisonPeople(pozigalci, indijanec);
			HandleCollisions(pozigalci, indijanec);
		}		
		
		// pozigalci movement
		for (GameObject& pozigalec : pozigalci) {
			if (static_cast<float>(rand() / RAND_MAX < 0.3)) {
				pozigalec.direction = static_cast<Direction>(rand() % 4);
			}
			moverand = randomNumber(10, -10);
			switch (pozigalec.direction)
			{
			case UP:
				pozigalec.Position.y += moverand * OTHER_VELOCITY * dt;
				break;
			case DOWN:
				pozigalec.Position.y += moverand * OTHER_VELOCITY * dt;
				break;
			case RIGHT:
				pozigalec.Position.x += moverand * OTHER_VELOCITY * dt;
				break;
			case LEFT:
				pozigalec.Position.x += moverand * OTHER_VELOCITY * dt;
				break;
			default:
				break;
			}

			ValidatePosition(pozigalec, this->Width, this->Height);
		}

		// random pozigalec sproži ognj
		kurjenje += dt;
		if (kurjenje > 5.0f && !pozigalci.empty()) {
			int who = randomNumber(0, pozigalci.size()-1);
			fires.push_back(Fire(glm::vec2(pozigalci.at(who).Position), glm::vec2(50, 50), ResourceManager::GetTexture("fire")));
			kurjenje = 0;
		}

		if (pozigalci.size() == 0) {
			nextLevel = true;
		}
		// check if improving to next level
		if (nextLevel && this->State != GAME_WIN) {
			level++;
			std::cout << "Level: " << level << std::endl;
			if (level > 2) {
				this->State = GAME_WIN;
				won = true;
			} 
			else {
				nextLevel = 0;
				this->State = GAME_MID_LEVEL;
			}
		}
	}
	if ((this->State == GAME_MID_LEVEL || this->State == GAME_WIN || this->State == GAME_LOST)) {
		if (replayWrite.is_open()) {
			replayWrite.close();
		}
		//this->PrevState = this->State;
	}
	if (this->State != GAME_ACTIVE) {
		startFires = false;
	}
	if ((this->State == GAME_LOST || this->State == GAME_WIN) && !konc) {
		this->PrevState = this->State; // Save the previous state
		konc = true;
		this->State = GAME_NAME_INPUT; // Transition to name input
	}
}

void Game::ProcessInput(float dt)
{
	if ((this->State == GAME_MID_LEVEL || this->State == GAME_WIN || this->State == GAME_LOST)) {
		if (this->Keys[GLFW_KEY_R] && !this->KeysProcessed[GLFW_KEY_R]) {
			StartReplay();
			this->KeysProcessed[GLFW_KEY_R] = true;
		}
		if (this->Keys[GLFW_KEY_L] && !this->KeysProcessed[GLFW_KEY_L]) {
			this->PrevState = this->State;
			this->State = GAME_LEADERBOARD;
			this->KeysProcessed[GLFW_KEY_L] = true;
		}
	}
	if (this->State == GAME_MENU) {
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE]) {
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}
	}
	if (this->State == GAME_MID_LEVEL) {
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE]) {
			LevelInitialize();
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}

	}
	if (this->State == GAME_LOST) {
		if (this->Keys[GLFW_KEY_P] && !this->KeysProcessed[GLFW_KEY_P]) {
			this->level = 0;
			fires.clear();
			this->LevelInitialize('p');
		}
	}
	if (this->State == GAME_LEADERBOARD) {
		if (this->Keys[GLFW_KEY_C] && !this->KeysProcessed[GLFW_KEY_C]) {
			this->State = this->PrevState;
			this->KeysProcessed[GLFW_KEY_C] = true;
		}
	}
	if (this->State == GAME_REPLAY) {
		if (this->Keys[GLFW_KEY_C] && !this->KeysProcessed[GLFW_KEY_C]) {
			replayRead.close();
			this->State = this->PrevState;
			this->KeysProcessed[GLFW_KEY_C] = true;
		}
	}
	if (this->State == GAME_NAME_INPUT) {
		for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			if (this->Keys[key] && !this->KeysProcessed[key]) {
				this->playerName += static_cast<char>(key);
				this->KeysProcessed[key] = true;;
			}
		}
		if (this->Keys[GLFW_KEY_BACKSPACE] && !this->KeysProcessed[GLFW_KEY_BACKSPACE]) {
			if (!this->playerName.empty()) {
				this->playerName.pop_back();
			}
			this->KeysProcessed[GLFW_KEY_BACKSPACE] = true;
		}
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) {
			SaveToLeaderboard();    // Save the player's name and score
			this->State = GAME_LEADERBOARD; // Transition to the next state (e.g., leaderboard or menu)
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
	}
	if (this->State == GAME_ACTIVE) {
		float velocity = PLAYER_VELOCITY * dt * this->levels[level].playerSpeedMultiplier;
		if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE]) {
			this->State = GAME_MENU;
			this->KeysProcessed[GLFW_KEY_SPACE] = true;
		}
		if (this->Keys[GLFW_KEY_A]) {
			if (player->Position.x >= 0)
				player->Position.x -= velocity;
		}
		if (this->Keys[GLFW_KEY_D]) {
			if (player->Position.x <= this->Width - player->Size.x)
				player->Position.x += velocity;
		}
		if (this->Keys[GLFW_KEY_W]) {
			if (player->Position.y >= 0)
				player->Position.y -= velocity;
		}
		if (this->Keys[GLFW_KEY_S]) {
			if (player->Position.y <= this->Height - player->Size.y)
				player->Position.y += velocity;
		}
	}
}

void Game::Render()
{
	//std::cout << fires.size() << std::endl;
	float burntArea = calculateBurntArea(burnt);
	float totalArea = static_cast<float>(Width) * static_cast<float>(Height);
	burntPercentage = (burntArea / totalArea) * 100.0f;
	if (burntPercentage >= 25) {
		this->State = GAME_LOST;
	}
	// std::cout << "Burnt area: " << burntArea  << " (" << burntPercentage << "%)" << std::endl;
	std::cout << this->State << std::endl;
	glClearColor(background.x, background.y, background.z, 1.0f);

	if (this->State == GAME_REPLAY) {
		ReplayUpdate();
		player->Draw(*renderer);
		return;
	}
	if (this->State == GAME_NAME_INPUT) {
		text->RenderText("Enter your name: " + this->playerName, 20.0f, this->Height / 2.0f, 1.0f);
	}
	if (this->State != GAME_WIN) {
		for (Fire& burnt : burnt)
			burnt.Draw(*renderer);
		for (Fire& fire : fires)
			fire.Draw(*renderer);
		for (GameObject& indijanec : indijanci)
			indijanec.Draw(*renderer);
		for (GameObject& hejtr : pozigalci)
			hejtr.Draw(*renderer);
		player->Draw(*renderer);

		text->RenderText("Level: " + std::to_string(level + 1), 20.0f, 20.0f, 1.0f);
		text->RenderText("Burnt: " + std::to_string(burntPercentage), this->Width - 300.0f, 20.0f, 1.0f);
	}if (this->State == GAME_MENU) {
		Texture2D menu = ResourceManager::GetTexture("menu");
		
		std::string first = "GAME MENU!";
		std::string second = "click [SPACE] to continue";
		float textWidth1 = text->GetTextWidth(first, 1.0f);
		float textHeight1= text->GetTextWidth(first, 1.0f);
		float textWidth2 = text->GetTextWidth(second, 1.0f);
		float textHeight2 = text->GetTextHeight(second, 1.0f);

		// Calculate the position to center the text
		float x1 = (this->Width - textWidth1) / 2.0f;
		float y1 = (this->Height - textHeight1) / 2.0f;
		float x2 = (this->Width - textWidth2) / 2.0f;
		float y2 = (this->Height - textHeight2) / 2.0f;

		text->RenderText(first, x1, y1, 1.0f);
		text->RenderText(second, x2, y2 + 50.0f, 1.0f);
	}if (this->State == GAME_LOST) {
		std::string first = "YOU LOST THE GAME! ";
		float textWidth1 = text->GetTextWidth(first, 1.0f);
		float textHeight1 = text->GetTextWidth(first, 1.0f);

		// Calculate the position to center the text
		float x1 = (this->Width - textWidth1) / 2.0f;
		float y1 = (this->Height - textHeight1) / 2.0f;

		text->RenderText(first, x1, y1, 1.0f);
	}if (this->State == GAME_MID_LEVEL) {
		std::string first = "YOU CLEARED THIS LEVEL";
		std::string second = "click [SPACE] to continue on " + std::to_string(level+1) +  ". level";
		float textWidth1 = text->GetTextWidth(first, 1.0f);
		float textHeight1 = text->GetTextWidth(first, 1.0f);
		float textWidth2 = text->GetTextWidth(second, 1.0f);
		float textHeight2 = text->GetTextHeight(second, 1.0f);

		// Calculate the position to center the text
		float x1 = (this->Width - textWidth1) / 2.0f;
		float y1 = (this->Height - textHeight1) / 2.0f;
		float x2 = (this->Width - textWidth2) / 2.0f;
		float y2 = (this->Height - textHeight2) / 2.0f;

		text->RenderText(first, x1, y1, 1.0f);
		text->RenderText(second, x2, y2 + 50.0f, 1.0f);
	}if (this->State == GAME_WIN) {
		std::string first = "YOU WON THE GAME! ";
		float textWidth1 = text->GetTextWidth(first, 1.0f);
		float textHeight1 = text->GetTextWidth(first, 1.0f);

		// Calculate the position to center the text
		float x1 = (this->Width - textWidth1) / 2.0f;
		float y1 = (this->Height - textHeight1) / 2.0f;

		text->RenderText(first, x1, y1, 1.0f);
	}
	if (this->State == GAME_LEADERBOARD) {
		std::string ldb = "LEADERBOARD: ";
		float textWidth = text->GetTextWidth(ldb, 1.0f);
		float x = (this->Width - textWidth) / 2.0f;
		text->RenderText(ldb, x, 100, 1.0f);
		std::ifstream data("leaderboard.bin", std::ios::binary);
		struct Zapis a;
		int paddingY = 250;
		while (data.read((char*)&a, sizeof(a))) {
			std::string name = a.name;
			float score = a.score;
			std::string izpis = name +":   " + std::to_string(score);
			float textWidth1 = text->GetTextWidth(izpis, 1.0f);
			float textHeight1 = text->GetTextWidth(izpis, 1.0f);
			float x1 = (this->Width - textWidth1) / 2.0f;
			text->RenderText(izpis, x1, paddingY, 1.0f);
			paddingY += 50;
		}
		data.close();
	}
	IsThereError();
}
