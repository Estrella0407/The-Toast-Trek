#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include <set>
#include <vector>
#include "Enemy.h" // BossId

class Sprite;
class TileMap;
class Pochi;
class Inventory;
class SoundManage;

// Defined in OverworldState.h
// The fixed underlying type makes this a complete type here
// GameContext can hold a std::set<MapId> without pulling in the whole overworld header
enum class MapId : int;

// How BattleState's last fight ended
// Whichever state pushed it (the Maze) can react once it's back on top of the stack
// Set by BattleState right before it pops itself; consumed and reset to None by the reader
enum class BattleOutcome {
    None,
    Victory,
    Defeat,
    Fled
};

// Data shared by all screens
// States do not own these game resources
struct GameContext {
    IDirect3DDevice9* device;
    LPD3DXSPRITE spriteBrush;
    Sprite* pochi;
    TileMap* forestMap;
    TileMap* mazeMap;
	Pochi* playerStats;
	Inventory* inventory;
    SoundManage* sound;         // May be null if audio failed to init
    TileMap* ruinsExteriorMap;
    TileMap* ruinsInteriorMap;
    TileMap* tarumtMap;         // Secret-boss area off the forest's top-left; may be null
    BYTE* keys;
    int moveSpeed;

    // Absolute cursor position in window client coordinates, and whether the left button is currently held
    // Read via GetCursorPos/GetAsyncKeyState instead of DirectInput mouse device
    // which reports relative motion deltas (not an absolute position) the way it's configured
    float mouseX;
    float mouseY;
    bool mouseLeftDown;

    BattleOutcome lastBattleOutcome;
    BossId lastBattleBoss;

    // One-shot spawn override for the next overworld map
    // A departing OverworldState sets this
    // ("put Pochi at the right edge of the Forest")
    // so the arriving map places her at the connecting seam instead of its own default spawn
    // The arriving Initialize() consumes it and clears the flag
    D3DXVECTOR2 pendingSpawn = D3DXVECTOR2(0.0f, 0.0f);
    bool hasPendingSpawn = false;

    // Maps whose bosses are all beaten
    // Persists for the run so backtracking into a cleared map doesn't respawn its enemies / re-lock its gate
    // Cleared at the start of a new run (main menu -> play, game-over retry)
    std::set<MapId> clearedMaps;
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

// The only object that changes screens
// State changes are queued until the current input/update call ends, so a state never deletes itself mid-method
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
// CreateMainMenuState() in MainMenuState.cpp
// CreateBattleState() in BattleState.cpp
// Anything only ever pushed from within one file
// (the maze, pushed only from OverworldState.cpp's forest-exit trigger)
std::unique_ptr<GameState> CreateMainMenuState();
std::unique_ptr<GameState> CreateBattleState(BossId bossId);
std::unique_ptr<GameState> CreateGameOverState(SoundManage* sound);
std::unique_ptr<GameState> CreateEndingState();

// Level 1 Pochi, empty pack, every map re-locked
// A clean playthrough called from each "new run" entry point (main menu Enter, Game Over retry)
void ResetRunProgress(GameContext& context);

