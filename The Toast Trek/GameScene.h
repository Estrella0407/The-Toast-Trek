#pragma once
#include <d3d9.h>
#include <Windows.h>
#include "GameContext.h"

class GameStateManager;

// One screen / mode of the game (main menu, overworld, battle, ...).
// Subclasses supply the per-scene Initialize / HandleInput / Update / Render.
// The stack only ever ticks and renders its top entry.
class GameScene {
public:
    virtual ~GameScene() = default;

    virtual void Initialize(GameContext& context) {}
    virtual void HandleInput(GameContext& context, GameStateManager& manager) = 0;
    virtual void Update(GameContext& context, GameStateManager& manager) = 0;
    virtual void Render(GameContext& context) = 0;
    virtual D3DCOLOR ClearColor() const = 0;

    // True on the frame `key` goes from up to down. `wasDown` is the caller's
    // per-key latch, carried between frames. `key` is a name from Keys.h
    // (RETURN_KEY, ESCAPE_KEY, ...). For a plain held-state check use
    // KeyDown(keys, key) from Keys.h.
    static bool JustPressed(const BYTE* keys, int key, bool& wasDown);
};
