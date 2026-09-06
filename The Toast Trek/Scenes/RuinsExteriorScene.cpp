#include "RuinsExteriorScene.h"
#include "MazeScene.h"
#include "RuinsInteriorScene.h"
#include <algorithm>

static OverworldConfig BuildRuinsExteriorConfig() {
    OverworldConfig config;
    config.mapId = MapId::RuinsExterior;
    config.foregroundLayers = { "Tree_leaf" };
    config.ComputeSpawnPosition = [](const D3DXVECTOR2& current) {
        return D3DXVECTOR2(55.0f, std::clamp(current.y, 360.0f, 630.0f));
    };
    config.doorwayPosition = D3DXVECTOR2(780.0f, 190.0f);
    config.doorwayRadius = 60.0f;
    config.OnEnterDoorway = [] { return std::unique_ptr<GameScene>(std::make_unique<RuinsInteriorScene>()); };
    config.OnReachLeftEdge = [] { return std::unique_ptr<GameScene>(std::make_unique<MazeScene>()); };
    config.leftEdgeSpawn = D3DXVECTOR2(1160.0f, OverworldConfig::kCarryY);

    config.items = {
        { ItemType::Bone, "Assets/Item/bone.png", 32, 32, 380.0f, 550.0f, 2.0f },
        { ItemType::Toast, "Assets/Item/toast.png", 16, 16, 780.0f, 460.0f, 2.0f }
    };
    return config;
}

RuinsExteriorScene::RuinsExteriorScene() : OverworldScene(BuildRuinsExteriorConfig()) {}
