#include <vector>
#include "game_object.h"
#include "fire.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN, 
    GAME_LOST, 
    GAME_MID_LEVEL,
    GAME_REPLAY, 
    GAME_NAME_INPUT,
    GAME_LEADERBOARD
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
    GameState    PrevState;
    bool         Keys[1024];
	bool 	     KeysProcessed[1024];
    bool         startFires;
    bool         nextLevel;
    bool         won, lost, konc;
    glm::vec3    background;
    unsigned int Width, Height;

    GameLevel levels[3];
    unsigned int level;
    float burntPercentage;
    float kurjenje;
    int replayFrameCounter;
    std::string playerName;

    std::vector<Fire> fires;
    std::vector<Fire> burnt;

	std::vector<GameObject> indijanci;
	std::vector<GameObject> pozigalci;

    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // initialize game state (load all shaders/textures/levels)
    void Init();
    void LevelInitialize(char c = 'n');
    bool checkPosition(int, int);
    // game loop
    void Resize(float, float);
    void StartReplay();
    void ReplayUpdate();
    void SaveToLeaderboard();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
};