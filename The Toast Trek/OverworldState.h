#pragma once
#include "GameState.h"
#include "Inventory.h" // ItemType
#include <functional>
#include <string>
#include <vector>

// Which loaded map an OverworldState renders/collides against - resolved
// against GameContext (context.forestMap / context.mazeMap / ...) at
// Initialize() time, not baked in at construction.
enum class MapId : int {   // fixed underlying type: forward-declared in GameState.h
    Forest,
    Maze,
    RuinsExterior,
    RuinsInterior,
    Tarumt          // secret-boss (Mr Andrew) area off the forest's top-left
};

// One boss standing in the map, waiting to be walked up to (F to fight).
struct BossSpawn {
    BossId id;
    float x, y;
};

// One item lying in the map. Walk over it and press F to add it to the
// inventory. texWidth/texHeight are the PNG's exact pixel dimensions (the
// Sprite loader needs them so non-power-of-two art isn't silently resized).
struct ItemSpawn {
    ItemType type;
    std::string texture;
    int texWidth, texHeight;
    float x, y;
    float scale;
};

// Everything that differs between one overworld map and the next. Pochi's
// forest walk and her maze walk used to be two near-identical copies of
// the same movement/collision/render loop (TutorialState and MazeState);
// this is that loop written once, with the differences supplied as data -
// so the next map (e.g. Maki's, once its art exists) is a new
// OverworldConfig, not a third copy-pasted state class.
struct OverworldConfig {
    MapId mapId = MapId::Forest;

    // Drawn in front of Pochi and any bosses (e.g. the forest's leaf
    // canopy) - everything else on the map draws behind them. Leave empty
    // if nothing in this map needs to layer in front of the player.
    std::vector<std::string> foregroundLayers;

    std::vector<BossSpawn> bosses;

    std::vector<ItemSpawn> items;

    // Where Pochi appears when this state starts, given wherever she
    // currently is - e.g. carry her Y across a map seam, or ignore the
    // argument and return a fixed spawn point.
    std::function<D3DXVECTOR2(const D3DXVECTOR2& currentPosition)> computeSpawnPosition;

    // Called when Pochi's box touches the map's right edge; return the
    // next state to push, or nullptr for "no exit on this edge".
    std::function<std::unique_ptr<GameState>()> onReachRightEdge;

    // The reverse: walking into the map's LEFT edge, to backtrack to the
    // previous map. Retreat is allowed even while this map's bosses are
    // uncleared (only the forward exits seal). nullptr = no exit here.
    std::function<std::unique_ptr<GameState>()> onReachLeftEdge;

    // Optional forced spawn point in the DESTINATION map for each of this
    // map's exits, so Pochi lands on the connecting seam rather than the
    // destination's own default spawn. Leave as kNoSpawn to let the
    // destination's computeSpawnPosition decide. A y of kCarryY keeps
    // Pochi's current y (seamless edge crossings).
    static constexpr float kNoSpawn = -1000000.0f;
    static constexpr float kCarryY  = -1.0f;
    D3DXVECTOR2 rightEdgeSpawn = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 leftEdgeSpawn  = D3DXVECTOR2(kNoSpawn, kNoSpawn);
    D3DXVECTOR2 doorwaySpawn   = D3DXVECTOR2(kNoSpawn, kNoSpawn);

    // Called the frame every boss in `bosses` has been cleared (e.g. the
    // final map pushing the ending). Fired once.
    std::function<std::unique_ptr<GameState>()> onAllCleared;

    // Sequence gating. When requireBossesCleared is true the map's exit
    // (onReachRightEdge / onEnterDoorway) stays locked until every boss in
    // `bosses` is beaten - Pochi is walled in until the room is cleared, and
    // a prompt shows at the blocked exit. When bossesInOrder is true the
    // bosses must be fought in list order: boss i can't be engaged until
    // bosses 0..i-1 are down. Both default off. The F8 dev-warp ignores this.
    bool requireBossesCleared = false;
    bool bossesInOrder = false;

    // Optional locked gate drawn across the exit while requireBossesCleared
    // is still unmet - "beat them, then cross". Enabled when gateWidth and
    // gateHeight are both > 0. It's drawn from gateTexture (a PNG at its
    // real texWidth/texHeight, positioned at gateX/gateY) if one is given,
    // otherwise a plain barred gate is drawn to fill the gate rect. While
    // the gate is up Pochi is also physically stopped at gateX (she can't
    // walk past it), so it blocks a right-edge exit.
    float gateX = 0.0f;
    float gateY = 0.0f;
    float gateWidth = 0.0f;
    float gateHeight = 0.0f;
    std::string gateTexture;
    int gateTexWidth = 0;
    int gateTexHeight = 0;

    // A specific spot on the map (e.g. a temple doorway) rather than an
    // edge - walking within doorwayRadius of doorwayPosition calls
    // onEnterDoorway the same way onReachRightEdge works. Ignored unless
    // onEnterDoorway is set.
    D3DXVECTOR2 doorwayPosition = D3DXVECTOR2(0.0f, 0.0f);
    float doorwayRadius = 40.0f;
    std::function<std::unique_ptr<GameState>()> onEnterDoorway;

    // Invisible top/bottom fence in pixels - Pochi's feet are kept between
    // these Y values, on top of normal tile collision. Use it to stop the
    // player walking around a maze along the map's open edges. fenceBottom
    // <= fenceTop disables it (the default).
    float fenceTop = 0.0f;
    float fenceBottom = 0.0f;
};

std::unique_ptr<GameState> CreateOverworldState(OverworldConfig config);

// The game's first overworld screen, reached from the main menu.
std::unique_ptr<GameState> CreateForestState();
