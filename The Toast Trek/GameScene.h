#pragma once
#include <d3d9.h>
#include "GameContext.h"

class GameStateManager;

// One screen / mode of the game (main menu, overworld, battle, ...).
// Subclasses supply the per-scene Initialize / HandleInput / Update / Render.
// The stack only ever ticks and renders its top entry.
class GameScene {
public:
    virtual ~GameScene() {}
    virtual void Initialize(GameContext& context) {}
    virtual void HandleInput(GameContext& context, GameStateManager& manager) = 0;
    virtual void Update(GameContext& context, GameStateManager& manager) = 0;
    // Optional fixed-step hook, called once per frame before Update(). Most
    // scenes resolve their own collisions inside Update() and leave this empty.
    virtual void Physics(GameContext& context, GameStateManager& manager) {}
    virtual void Render(GameContext& context) = 0;
    virtual D3DCOLOR ClearColor() const = 0;
};
