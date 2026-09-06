#include "GameStateManager.h"
#include "SoundManager.h"
#include "Font.h"
#include "Sprite.h"
#include "TileMap.h"
#include "Inventory.h"
#include "Pochi.h"
#include "Cheats.h"
#include "UiFill.h"
#include "SaveGame.h"

static const int kScreenWidth = 1280;
static const int kScreenHeight = 720;

GameStateManager::GameStateManager()
    : sound(nullptr), cheatFont(nullptr), cheatPlateTex(nullptr),
      context{}, pendingPopCount(0), clearRequested(false)
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

    DrawCheatOverlay();

    d3d.EndFrame();
}

void GameStateManager::Shutdown()
{
    stateStack.clear();
    pendingPushes.clear();

    if (context.inventory) { delete context.inventory; context.inventory = nullptr; }
    if (context.playerStats) { delete context.playerStats; context.playerStats = nullptr; }

    delete context.forestMap;         context.forestMap = nullptr;
    delete context.mazeMap;           context.mazeMap = nullptr;
    delete context.ruinsExteriorMap;  context.ruinsExteriorMap = nullptr;
    delete context.ruinsInteriorMap;  context.ruinsInteriorMap = nullptr;
    delete context.tarumtMap;         context.tarumtMap = nullptr;
    delete context.pochi;             context.pochi = nullptr;

    if (cheatFont) { delete cheatFont; cheatFont = nullptr; }
    if (cheatPlateTex) { cheatPlateTex->Release(); cheatPlateTex = nullptr; }

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
// Assets (temporary home - Stage 3 moves these into MapLibrary / Player / etc.)
// ---------------------------------------------------------------------------

void GameStateManager::LoadAssets()
{
    IDirect3DDevice9* device = d3d.GetDevice();

    // Wide rect so the banner still renders when drawn far to the right
    cheatFont = new Font(device, 0.0f, 0.0f, 1600, 30, 18, "Arial");
    cheatPlateTex = ui::MakeWhiteTexture(device);

    context.forestMap = new TileMap(device, "Assets/TileMap/Forest.tmx", "Assets/TileMap/");
    context.forestMap->SetSolidLayers({ "Tree", "Rock" });

    context.mazeMap = new TileMap(device, "Assets/TileMap/Maze.tmx", "Assets/TileMap/");
    context.mazeMap->SetSolidLayers({ "Maze" });

    context.ruinsExteriorMap = new TileMap(device, "Assets/TileMap/Ruined_Temple_Exterior.tmx", "Assets/TileMap/");
    context.ruinsExteriorMap->SetSolidLayers({ "Tree", "House", "Bricks", "Statues", "Columns" });
    context.ruinsExteriorMap->SetWalkableLayers({ "Ground", "Grass", "Spots", "Grass_details", "Site", "House_platform" });

    context.ruinsInteriorMap = new TileMap(device, "Assets/TileMap/Ruined_Temple_Interior.tmx", "Assets/TileMap/");
    context.ruinsInteriorMap->SetSolidLayers({ "Walls_back", "Walls_top", "Statue" });

    context.tarumtMap = new TileMap(device, "Assets/TileMap/Tarumt.tmx", "Assets/TileMap/");
    context.tarumtMap->SetSolidLayers({ "Tree", "Structure1", "Structure2", "Building" });

    context.pochi = new Sprite(device, "Assets/Characters/Pochi.png", 250, 60, 5, 2, 10, 100.0f, 380.0f);
    if (context.pochi != nullptr) {
        context.pochi->CropToFrame(0);
        context.pochi->SetScale(2.0f);
    }

    context.inventory = new Inventory();
    context.playerStats = new Pochi(1);

    // Sound - Initialize() and every call are safe even with no audio files
    sound = new SoundManager();
    sound->Initialize();
    sound->LoadSound("click", "Assets/Sounds/click.wav");
    sound->LoadSound("gameover", "Assets/Sounds/gameover.wav");
    sound->LoadSound("levelcomplete", "Assets/Sounds/levelcomplete.wav");
    sound->LoadSound("background", "Assets/Sounds/background.wav", true);
    sound->LoadSound("battle", "Assets/Sounds/battle.wav", true);
    sound->LoadSound("attack", "Assets/Sounds/attack.wav");
    sound->LoadSound("hurt", "Assets/Sounds/hurt.ogg");

    {
        save::Settings st = save::LoadSettings();
        sound->SetMasterVolume(st.master);
        sound->SetMusicVolume(st.music);
        sound->SetSFXVolume(st.sfx);
        sound->SetMute(st.muted);
    }

    sound->PlayMusic("background", 0.6f);

    context.sound = sound;
}

void GameStateManager::DrawCheatOverlay()
{
    if (!Cheats::enabled || cheatFont == nullptr) return;

    LPD3DXSPRITE brush = d3d.GetSpriteBrush();
    const char* txt = "CHEAT MODE";
    const float pw = 118.0f, ph = 24.0f;
    const float px = 1280.0f - pw - 12.0f, py = 12.0f;
    if (cheatPlateTex != nullptr) {
        ui::FillRect(brush, cheatPlateTex, px - 1.0f, py - 1.0f, pw + 2.0f, ph + 2.0f, ui::kPlateEdge);
        ui::FillRect(brush, cheatPlateTex, px, py, pw, ph, ui::kPlate);
    }
    cheatFont->Draw(txt, px + 15.0f, py + 4.0f, ui::kShadow, brush);
    cheatFont->Draw(txt, px + 14.0f, py + 3.0f, D3DCOLOR_XRGB(255, 120, 120), brush);
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
