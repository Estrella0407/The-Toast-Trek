#pragma once
#include <d3d9.h>
#include <memory>
#include <vector>
#include "GameScene.h"

// The only object that changes screens.
// Changes are queued until the current input/update call ends, so a scene
// never deletes itself mid-method.
class GameStateManager {
private:
    GameContext& context;
    std::vector<std::unique_ptr<GameScene>> stateStack;
    std::vector<std::unique_ptr<GameScene>> pendingPushes;
    size_t pendingPopCount;
    bool clearRequested;

public:
    explicit GameStateManager(GameContext& gameContext);

    void Push(std::unique_ptr<GameScene> scene);
    void Pop();
    void ClearAndPush(std::unique_ptr<GameScene> scene);
    void ApplyPendingChanges();

    void HandleInput();
    void Update();
    void Render();
    D3DCOLOR ClearColor() const;
};
