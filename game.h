#include <vector>
#include "game_object.h"
#include "fire.h"

#include "game_level.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

class Game
{
public:
    // game state
    GameState    State;
    bool         Keys[1024];
	bool 	     KeysProcessed[1024];
    bool         startFires;
    unsigned int Width, Height;

    std::vector<GameLevel> Levels;
    unsigned int Level;

    std::vector<Fire> Fires;
    std::vector<Fire> Burnt;

	std::vector<GameObject> indijanci;
	std::vector<GameObject> pozigalci;

    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // initialize game state (load all shaders/textures/levels)
    void Init();
    // game loop
    void Resize(float, float);
    void DoCollisions();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
};