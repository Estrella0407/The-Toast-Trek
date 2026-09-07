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

    // Shared keyboard helpers - every scene reads the same DirectInput key
    // buffer (context.keys). `dikCode` is a DIK_* scancode. Static and public
    // so scene-local helper functions can use them too.
    static bool IsKeyDown(const BYTE* keys, int dikCode);

    // True on the frame `dikCode` goes from up to down. `wasDown` is the
    // caller's per-key latch, carried between frames.
    static bool JustPressed(const BYTE* keys, int dikCode, bool& wasDown);
};
