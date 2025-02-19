#include <vector>
#include "game_object.h"
#include "fire.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN, 
    GAME_LOST, 
    GAME_MID_LEVEL
};

struct GameLevel {
    glm::vec3 color;
    float playerSpeedMultiplier;
};

class Game
{
public:
    // game state
    GameState    State;
    bool         Keys[1024];
	bool 	     KeysProcessed[1024];
    bool         startFires;
    bool         nextLevel;
    glm::vec3    background;
    unsigned int Width, Height;

    GameLevel levels[3];
    unsigned int level;
    float kurjenje;

    std::vector<Fire> fires;
    std::vector<Fire> burnt;

	std::vector<GameObject> indijanci;
	std::vector<GameObject> pozigalci;

    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // initialize game state (load all shaders/textures/levels)
    void Init();
    void LevelInitialize();
    // game loop
    void Resize(float, float);
    void DoCollisions();
    void CollisionCigani();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
};