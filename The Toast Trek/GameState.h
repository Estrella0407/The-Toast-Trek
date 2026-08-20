#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>
#include <vector>

class Sprite;
class TileMap;

// Data shared by all screens. States do not own these game resources.
struct GameContext {
    IDirect3DDevice9* device;
    LPD3DXSPRITE spriteBrush;
    Sprite* pochi;
    TileMap* forestMap;
    BYTE* keys;
    int moveSpeed;
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

std::unique_ptr<GameState> CreateMainMenuState();

