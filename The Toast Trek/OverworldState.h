#pragma once
#include "GameState.h"
#include <functional>
#include <string>
#include <vector>

// Which loaded map an OverworldState renders/collides against - resolved
// against GameContext (context.forestMap / context.mazeMap / ...) at
// Initialize() time, not baked in at construction.
enum class MapId {
    Forest,
    Maze,
    RuinsExterior,
    RuinsInterior
};

// One boss standing in the map, waiting to be walked up to (F to fight).
struct BossSpawn {
    BossId id;
    float x, y;
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

    // Where Pochi appears when this state starts, given wherever she
    // currently is - e.g. carry her Y across a map seam, or ignore the
    // argument and return a fixed spawn point.
    std::function<D3DXVECTOR2(const D3DXVECTOR2& currentPosition)> computeSpawnPosition;

    // Called when Pochi's box touches the map's right edge; return the
    // next state to push, or nullptr for "no exit on this edge".
    std::function<std::unique_ptr<GameState>()> onReachRightEdge;

    // A specific spot on the map (e.g. a temple doorway) rather than an
    // edge - walking within doorwayRadius of doorwayPosition calls
    // onEnterDoorway the same way onReachRightEdge works. Ignored unless
    // onEnterDoorway is set.
    D3DXVECTOR2 doorwayPosition = D3DXVECTOR2(0.0f, 0.0f);
    float doorwayRadius = 40.0f;
    std::function<std::unique_ptr<GameState>()> onEnterDoorway;
};

std::unique_ptr<GameState> CreateOverworldState(OverworldConfig config);

// The game's first overworld screen, reached from the main menu.
std::unique_ptr<GameState> CreateForestState();
