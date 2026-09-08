#pragma once
#include "GameState.h"
#include "Inventory.h" // ItemType
#include <functional>
#include <string>
#include <vector>

// Which loaded map an OverworldState draws / collides against
enum class MapId : int {   // fixed underlying type: forward-declared in GameState.h
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
// shared walk / collide / render loop is written once and fed data
struct OverworldConfig {
    MapId mapId = MapId::Forest;

    // Layers drawn in front of Pochi (a leaf canopy); empty = none
    std::vector<std::string> foregroundLayers;

    std::vector<BossSpawn> bosses;
    std::vector<ItemSpawn> items;

    // Where Pochi spawns, given his current position (carry Y across a seam)
    std::function<D3DXVECTOR2(const D3DXVECTOR2& currentPosition)> ComputeSpawnPosition;

    // Pochi reaches the right edge -> next state to push (null = no exit)
    std::function<std::unique_ptr<GameState>()> OnReachRightEdge;

    // Left edge -> backtrack to the previous map, allowed even while bosses
    // are uncleared (null = no exit)
    std::function<std::unique_ptr<GameState>()> OnReachLeftEdge;

    // Forced spawn in the DESTINATION map per exit, so Pochi lands on the seam
    // kNoSpawn = let the destination decide; kCarryY = keep current y
    static constexpr float kNoSpawn = -1000000.0f;
    static constexpr float kCarryY  = -1.0f;
    D3DXVECTOR2 rightEdgeSpawn = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 leftEdgeSpawn  = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 doorwaySpawn   = D3DXVECTOR2(kNoSpawn, kNoSpawn);

    // Fired once, the frame every boss is cleared (roll the ending)
    std::function<std::unique_ptr<GameState>()> OnAllCleared;

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
    std::function<std::unique_ptr<GameState>()> OnEnterDoorway;

    // Invisible fence: Pochi's feet kept between these Y values, on top of
    // tile collision. fenceBottom <= fenceTop disables it
    float fenceTop = 0.0f;
    float fenceBottom = 0.0f;
};

std::unique_ptr<GameState> CreateOverworldState(OverworldConfig config);

// The first overworld screen, reached from the main menu
std::unique_ptr<GameState> CreateForestState();

// Rebuilds the overworld state for `id` - used by "Continue"
std::unique_ptr<GameState> CreateOverworldStateForMap(MapId id);
