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

// Full definition in OverworldState.h; opaque here so GameContext can hold a
// std::set<MapId> without the whole overworld header
enum class MapId : int;

// How BattleState's last fight ended - set just before it pops itself,
// read (and reset to None) by whichever map pushed it
enum class BattleOutcome {
    None,
    Victory,
    Defeat,
    Fled
};

// Shared game resources; states borrow these, they don't own them
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

    // Absolute cursor position + left-button state, refreshed each frame in
    // Main.cpp (GetCursorPos / GetAsyncKeyState, not the DirectInput mouse)
    float mouseX;
    float mouseY;
    bool mouseLeftDown;

    BattleOutcome lastBattleOutcome;
    BossId lastBattleBoss;

    // One-shot spawn override for the next map, so the arriving map places Pochi on the connecting seam
    // The arriving Initialize() consumes it
    D3DXVECTOR2 pendingSpawn = D3DXVECTOR2(0.0f, 0.0f);
    bool hasPendingSpawn = false;

    // Maps whose bosses are all beaten - persists for the run so backtracking doesn't respawn enemies
    // Cleared at the start of a new run
    std::set<MapId> clearedMaps;

    // Per-slot run progress, kept so a picked-up item / beaten boss stays
    // gone after a map reload or Continue. Encoded as (int)mapId * 16 + index
    std::set<int> collectedItems;
    std::set<int> clearedBosses;

    // The map Pochi is currently on - set by OverworldState::Initialize, used
    // by SaveCurrentRun() so "Continue" knows where to drop the player back
    MapId currentMapId{};
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
// Changes are queued until the current input/update call ends, so a state never deletes itself mid-method
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

// Entry points defined next to the state they build (CreateMainMenuState in MainMenuState.cpp)
// States only ever pushed from one file aren't listed
std::unique_ptr<GameState> CreateMainMenuState();
std::unique_ptr<GameState> CreateBattleState(BossId bossId);
std::unique_ptr<GameState> CreateGameOverState(SoundManage* sound);
std::unique_ptr<GameState> CreateEndingState();

// Level 1 Pochi, empty pack, every map re-locked - for each "new run" entry
void ResetRunProgress(GameContext& context);

// Snapshot the current run (map, position, level, pack, cleared maps) to
// savegame.txt. Call on any real progress event - map entry, item pickup,
// battle win, exit to menu.
void SaveCurrentRun(const GameContext& context);
