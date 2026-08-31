#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include <vector>
#include "Enemy.h" // BossId

class Sprite;
class TileMap;
class Pochi;
class Inventory;

// How BattleState's last fight ended, so whichever state pushed it (the
// maze) can react once it's back on top of the stack. Set by BattleState
// right before it pops itself; consumed and reset to None by the reader.
enum class BattleOutcome {
    None,
    Victory,
    Defeat,
    Fled
};

// Data shared by all screens. States do not own these game resources.
struct GameContext {
    IDirect3DDevice9* device;
    LPD3DXSPRITE spriteBrush;
    Sprite* pochi;
    TileMap* forestMap;
    TileMap* mazeMap;
	Pochi* playerStats;
	Inventory* inventory;
    TileMap* ruinsExteriorMap;
    TileMap* ruinsInteriorMap;
    BYTE* keys;
    int moveSpeed;

    // Absolute cursor position in window client coordinates, and whether
    // the left button is currently held - refreshed every frame in
    // Main.cpp's GetInput(). Read via GetCursorPos/GetAsyncKeyState rather
    // than the DirectInput mouse device, which reports relative motion
    // deltas (not an absolute position) the way it's configured here.
    float mouseX;
    float mouseY;
    bool mouseLeftDown;

    BattleOutcome lastBattleOutcome;
    BossId lastBattleBoss;
};

class GameStateManager;

class GameState {
public:
    virtual ~GameState() {}
    virtual void Initialize(GameContext& context) {}
    virtual void HandleInput(GameContext& context, GameStateManager& manager) = 0;
    virtual void Update(GameContext& context, GameStateManager& manager) = 0;
    virtual void Render(GameContext& context) = 0;
    virtual D3DCOLOR ClearColor() const = 0;
};

// The only object that changes screens. State changes are queued until the
// current input/update call ends, so a state never deletes itself mid-method.
class GameStateManager {
private:
    GameContext& context;
    std::vector<std::unique_ptr<GameState>> stateStack;
    std::vector<std::unique_ptr<GameState>> pendingPushes;
    size_t pendingPopCount;
    bool clearRequested;

public:
    explicit GameStateManager(GameContext& gameContext);

    void Push(std::unique_ptr<GameState> state);
    void Pop();
    void ClearAndPush(std::unique_ptr<GameState> state);
    void ApplyPendingChanges();

    void HandleInput();
    void Update();
    void Render();
    D3DCOLOR ClearColor() const;
};

// Cross-file entry points - each defined next to the state it builds
// (CreateMainMenuState() in MainMenuState.cpp, CreateBattleState() in
// BattleState.cpp). Anything only ever pushed from within one file (e.g.
// the maze, pushed only from OverworldState.cpp's forest-exit trigger)
// doesn't need to live here - see OverworldState.h for those.
std::unique_ptr<GameState> CreateMainMenuState();
std::unique_ptr<GameState> CreateBattleState(BossId bossId);

