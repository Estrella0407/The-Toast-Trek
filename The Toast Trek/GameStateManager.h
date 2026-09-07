#pragma once
#include <d3d9.h>
#include <memory>
#include <vector>
#include "GameScene.h"
#include "Window.h"
#include "Direct3D.h"
#include "InputManager.h"
#include "FrameTimer.h"
#include "MapLibrary.h"
#include "CheatOverlay.h"

class SoundManager;

// The lecture-notes "GameStateStackManager": owns the engine subsystems
// (window, device, input, sound) AND the scene stack, and drives the frame.
// WinMain talks only to this class.
//
//   Init / Update  - "must code per instance": engine bring-up, then the
//                    top scene's own logic.
//   GetInput / Physics / Render - "code once and forget": fixed engine work
//                    delegated to the subsystems below.
class GameStateManager {
private:
    // --- Engine subsystems ---------------------------------------------
    Window window;
    Direct3D d3d;
    InputManager input;
    FrameTimer timer;
    SoundManager* sound;

    MapLibrary maps;             // owns the overworld tilemaps
    CheatOverlay cheatOverlay;   // red "CHEAT MODE" plate over every scene

    // --- Game data owned for the whole run ---------------------------
    GameContext context;

    // --- Scene stack -------------------------------------------------
    std::vector<std::unique_ptr<GameScene>> stateStack;
    std::vector<std::unique_ptr<GameScene>> pendingPushes;
    size_t pendingPopCount;
    bool clearRequested;

    void LoadAssets();

public:
    GameStateManager();
    ~GameStateManager();

    // Stack operations (queued, applied by ApplyPendingChanges)
    void Push(std::unique_ptr<GameScene> scene);
    void Pop();
    void ClearAndPush(std::unique_ptr<GameScene> scene);
    void ApplyPendingChanges();

    // --- Frame facade, called by WinMain --------------------------
    void Init();                 // engine bring-up + top scene Initialize()
    bool WindowIsRunning();       // -> Window::IsRunning()
    void GetInput();              // engine: input devices + cursor + sound tick
    void Update();                // top scene HandleInput + Update (scenes call PhysicsManager here)
    void Render();                // engine: clear / draw scene / overlay / present
    void Shutdown();             // reverse-order teardown

    D3DCOLOR ClearColor() const;
};
