#include "ForestScene.h"
#include "MazeScene.h"
#include "TarumtScene.h"

static OverworldConfig BuildForestConfig() {
    OverworldConfig config;
    config.mapId = MapId::Forest;
    config.foregroundLayers = { "Tree_Leaf" }; // Only the leaf canopy draws in front of Pochi
    config.ComputeSpawnPosition = [](const D3DXVECTOR2&) {
        return D3DXVECTOR2(100.0f, 380.0f);
    };
    // A couple of pickups along the walk to the maze
    config.items = {
        { ItemType::HealthPotion, "Assets/Item/heathPotion.png", 18, 20, 380.0f, 392.0f, 2.0f },
        { ItemType::Bone,         "Assets/Item/bone.png",        32, 32, 680.0f, 360.0f, 1.5f },
        { ItemType::Toast,        "Assets/Item/toast.png",       16, 16, 380.0f, 75.0f, 2.0f },
    };
    config.OnReachRightEdge = [] { return std::unique_ptr<GameScene>(std::make_unique<MazeScene>()); };

    // The path leading off the forest's TOP-LEFT corner goes to the secret Tarumt area where Mr Andrew is
    config.doorwayPosition = D3DXVECTOR2(110.0f, 30.0f);
    config.doorwayRadius = 90.0f;
    config.OnEnterDoorway = [] { return std::unique_ptr<GameScene>(std::make_unique<TarumtScene>()); };
    return config;
}

ForestScene::ForestScene() : OverworldScene(BuildForestConfig()) {}
