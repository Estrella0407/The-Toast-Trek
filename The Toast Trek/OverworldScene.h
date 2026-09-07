#pragma once
#include "GameScene.h"
#include "Inventory.h" // ItemType
#include <functional>
#include <memory>
#include <string>
#include <vector>

class TileMap;
class Enemy;
class Font;
class Item;
class Pochi;
class PochiBadge;
class Sprite;
class GameStateManager;

// Which loaded map an OverworldScene draws / collides against
enum class MapId : int {   // fixed underlying type: forward-declared in GameContext.h
    Forest,
    Maze,
    RuinsExterior,
    RuinsInterior,
    Tarumt          // Secret boss (Mr Andrew), off the forest's top-left
};

// A boss standing in the map - walk up and press F to fight
struct BossSpawn {
    BossId id;
    float x, y;
};

// An item lying in the map - walk over it and press F to pick it up
// texWidth/texHeight are the PNG's exact pixel size (the loader needs them)
struct ItemSpawn {
    ItemType type;
    std::string texture;
    int texWidth, texHeight;
    float x, y;
    float scale;
};

// Everything that differs between one overworld map and the next, so the
// shared walk / collide / render loop is written once and fed data. Each map
// subclass fills one of these in its constructor.
struct OverworldConfig {
    MapId mapId = MapId::Forest;

    // Layers drawn in front of Pochi (a leaf canopy); empty = none
    std::vector<std::string> foregroundLayers;

    std::vector<BossSpawn> bosses;
    std::vector<ItemSpawn> items;

    // Where Pochi spawns, given his current position (carry Y across a seam)
    std::function<D3DXVECTOR2(const D3DXVECTOR2& currentPosition)> ComputeSpawnPosition;

    // Pochi reaches the right edge -> next scene to push (null = no exit)
    std::function<std::unique_ptr<GameScene>()> OnReachRightEdge;

    // Left edge -> backtrack to the previous map, allowed even while bosses
    // are uncleared (null = no exit)
    std::function<std::unique_ptr<GameScene>()> OnReachLeftEdge;

    // Forced spawn in the DESTINATION map per exit, so Pochi lands on the seam
    // kNoSpawn = let the destination decide; kCarryY = keep current y
    static constexpr float kNoSpawn = -1000000.0f;
    static constexpr float kCarryY  = -1.0f;
    D3DXVECTOR2 rightEdgeSpawn = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 leftEdgeSpawn  = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 doorwaySpawn   = D3DXVECTOR2(kNoSpawn, kNoSpawn);

    // Fired once, the frame every boss is cleared (roll the ending)
    std::function<std::unique_ptr<GameScene>()> OnAllCleared;

    // requireBossesCleared: seal the forward exit until every boss is beaten.
    // bossesInOrder: boss i can't be fought until 0..i-1 are down
    bool requireBossesCleared = false;
    bool bossesInOrder = false;

    // Locked gate across the exit while requireBossesCleared is unmet
    // Active when gateWidth and gateHeight are both > 0; uses gateTexture if set, else a drawn barred gate
    // Also physically stops Pochi at gateX
    float gateX = 0.0f;
    float gateY = 0.0f;
    float gateWidth = 0.0f;
    float gateHeight = 0.0f;
    std::string gateTexture;
    int gateTexWidth = 0;
    int gateTexHeight = 0;

    // A point, not an edge: within doorwayRadius of doorwayPosition calls
    // OnEnterDoorway. Ignored unless OnEnterDoorway is set
    D3DXVECTOR2 doorwayPosition = D3DXVECTOR2(0.0f, 0.0f);
    float doorwayRadius = 40.0f;
    std::function<std::unique_ptr<GameScene>()> OnEnterDoorway;

    // Invisible fence: Pochi's feet kept between these Y values, on top of
    // tile collision. fenceBottom <= fenceTop disables it
    float fenceTop = 0.0f;
    float fenceBottom = 0.0f;
};

// Shared behaviour for every overworld map: the walk / collide / interact /
// render loop, written once. Each map is a subclass whose constructor hands
// up its own OverworldConfig. Construct via std::make_unique<ForestScene>()
// etc. - there is no factory function.
class OverworldScene : public GameScene {
protected:
    explicit OverworldScene(OverworldConfig cfg);

public:
    ~OverworldScene() override;

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;

private:
    void LeaveBoostedMap(GameContext& context);
    void StashSpawn(GameContext& context, const D3DXVECTOR2& s);
    // Drop any stat boost, stash the destination spawn, push the scene the
    // hook builds. Shared by every map exit (right edge / doorway / left edge
    // / all-cleared).
    void TakeExit(GameContext& context, GameStateManager& manager,
                  const D3DXVECTOR2& spawn,
                  const std::function<std::unique_ptr<GameScene>()>& makeNext);
    bool HasGate() const;
    void DrawGate(GameContext& context);
    bool AllBossesCleared() const;
    bool ExitLocked() const;

    OverworldConfig config;
    TileMap* map;

    bool interactWasDown;
    bool menuWasDown;
    bool cheatClearWasDown;   // K - clear nearest boss
    bool cheatWarpWasDown;    // L - jump to this map's exit
    bool allClearedFired;
    bool exitsArmed;          // False until Pochi has stood clear of every exit trigger
    std::vector<Enemy*> bossEnemies;
    std::vector<bool> bossCleared;
    Font* interactPrompt;

    std::vector<Item*> itemSprites;
    std::vector<bool> itemCollected;
    IDirect3DTexture9* exclaimTex;   // "!" bubble, floated over Pochi's head

    PochiBadge* hud;   // Top-left HP / DEF / ATK readout

    Font* levelUpFont;      // Floating text over Pochi's head (win / stat-boost)
    int levelUpFrames;      // Frames left to show it (~60/sec)
    const char* floatText;  // what floating text says

    Pochi* boostedStats;    // Tarumt map: Pochi's stats force-boosted, restored on the way out

    Sprite* gateSprite;            // Exit gate art, if config.gateTexture is set
};

// "Continue" - rebuild the scene for the map a save was taken in
std::unique_ptr<GameScene> CreateOverworldSceneForMap(MapId id);
