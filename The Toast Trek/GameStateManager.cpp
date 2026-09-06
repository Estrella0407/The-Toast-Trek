#include "GameStateManager.h"
#include "SoundManager.h"
#include "Inventory.h"
#include "Pochi.h"
#include "Cheats.h"

static const int kScreenWidth = 1280;
static const int kScreenHeight = 720;

GameStateManager::GameStateManager()
    : sound(nullptr), context{}, pendingPopCount(0), clearRequested(false)
{
}

GameStateManager::~GameStateManager()
{
    Shutdown();
}

// ---------------------------------------------------------------------------
// Frame facade
// ---------------------------------------------------------------------------

void GameStateManager::Init()
{
    window.Create("Let's Gooooo", kScreenWidth, kScreenHeight);
    d3d.CreateDevice(window.GetHandle(), kScreenWidth, kScreenHeight);
    input.Create(window.GetHandle());

    context.device = d3d.GetDevice();
    context.spriteBrush = d3d.GetSpriteBrush();
    context.keys = const_cast<BYTE*>(input.Keys());
    context.moveSpeed = 5;

    LoadAssets();

    timer.Init(10);

    // Run the scene(s) pushed before Init()
    ApplyPendingChanges();
}

bool GameStateManager::WindowIsRunning()
{
    return window.IsRunning();
}

void GameStateManager::GetInput()
{
    input.Update(window.GetHandle(), d3d.BackBufferWidth(), d3d.BackBufferHeight());

    context.mouseX = input.MouseX();
    context.mouseY = input.MouseY();
    context.mouseLeftDown = input.MouseLeftDown();

    // Developer cheat switch (F5) - reads the same key buffer
    Cheats::Update(const_cast<BYTE*>(input.Keys()));

    if (sound) sound->Update();
}

void GameStateManager::Update()
{
    if (!stateStack.empty()) stateStack.back()->HandleInput(context, *this);
    ApplyPendingChanges();
    if (!stateStack.empty()) stateStack.back()->Update(context, *this);
    ApplyPendingChanges();
}

void GameStateManager::Render()
{
    d3d.BeginFrame(ClearColor());

    if (!stateStack.empty()) stateStack.back()->Render(context);

    cheatOverlay.Draw(d3d.GetSpriteBrush());

    d3d.EndFrame();
}

void GameStateManager::Shutdown()
{
    stateStack.clear();
    pendingPushes.clear();

    if (context.inventory) { delete context.inventory; context.inventory = nullptr; }
    if (context.pochi) { delete context.pochi; context.pochi = nullptr; }

    // `maps` owns the tilemaps; CheatOverlay owns its own font/texture.
    context.maps = nullptr;

    if (sound) {
        sound->Shutdown();
        delete sound;
        sound = nullptr;
    }

    d3d.Cleanup();
    input.Cleanup();
    window.Destroy();
}

// ---------------------------------------------------------------------------
// Assets - each class loads its own; GameStateManager just wires them into
// the shared GameContext.
// ---------------------------------------------------------------------------

void GameStateManager::LoadAssets()
{
    IDirect3DDevice9* device = d3d.GetDevice();

    cheatOverlay.Load(device);

    maps.Load(device);
    context.maps = &maps;

    context.pochi = new Pochi(device, 1);   // sprite + stats in one
    context.inventory = new Inventory();

    sound = new SoundManager();
    sound->Initialize();
    sound->LoadGameSounds();
    context.sound = sound;
}

// ---------------------------------------------------------------------------
// Scene stack
// ---------------------------------------------------------------------------

void GameStateManager::Push(std::unique_ptr<GameScene> scene) {
    if (scene != NULL) pendingPushes.push_back(std::move(scene));
}

void GameStateManager::Pop() {
    ++pendingPopCount;
}

void GameStateManager::ClearAndPush(std::unique_ptr<GameScene> scene) {
    clearRequested = true;
    pendingPopCount = 0;
    pendingPushes.clear();
    if (scene != NULL) pendingPushes.push_back(std::move(scene));
}

void GameStateManager::ApplyPendingChanges() {
    if (clearRequested) stateStack.clear();
    clearRequested = false;
    while (pendingPopCount > 0 && !stateStack.empty()) {
        stateStack.pop_back();
        --pendingPopCount;
    }
    pendingPopCount = 0;
    for (size_t i = 0; i < pendingPushes.size(); ++i) {
        pendingPushes[i]->Initialize(context);
        stateStack.push_back(std::move(pendingPushes[i]));
    }
    pendingPushes.clear();
}

D3DCOLOR GameStateManager::ClearColor() const {
    return stateStack.empty() ? D3DCOLOR_XRGB(0, 0, 0) : stateStack.back()->ClearColor();
}
