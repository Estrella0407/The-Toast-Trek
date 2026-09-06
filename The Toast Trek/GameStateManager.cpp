#include "GameStateManager.h"
#include "SoundManager.h"
#include "Sprite.h"
#include "Inventory.h"
#include "Pochi.h"
#include "Player.h"
#include "Cheats.h"

static const int kScreenWidth = 1280;
static const int kScreenHeight = 720;

GameStateManager::GameStateManager()
    : sound(nullptr), player(nullptr), context{}, pendingPopCount(0), clearRequested(false)
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

void GameStateManager::Physics()
{
    if (!stateStack.empty()) stateStack.back()->Physics(context, *this);
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
    if (context.playerStats) { delete context.playerStats; context.playerStats = nullptr; }
    if (player) { delete player; player = nullptr; context.pochi = nullptr; }

    // Tilemaps are owned by `maps`; the cheat overlay owns its own font/tex.
    context.forestMap = context.mazeMap = nullptr;
    context.ruinsExteriorMap = context.ruinsInteriorMap = context.tarumtMap = nullptr;

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
// Assets (the player sprite still lives here until the GameObject retrofit)
// ---------------------------------------------------------------------------

void GameStateManager::LoadAssets()
{
    IDirect3DDevice9* device = d3d.GetDevice();

    cheatOverlay.Load(device);

    maps.Load(device);
    context.forestMap = maps.Forest();
    context.mazeMap = maps.Maze();
    context.ruinsExteriorMap = maps.RuinsExterior();
    context.ruinsInteriorMap = maps.RuinsInterior();
    context.tarumtMap = maps.Tarumt();

    player = new Player(device);
    context.pochi = player->GetSprite();   // scenes borrow the sprite; Player owns it

    context.inventory = new Inventory();
    context.playerStats = new Pochi(1);

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
