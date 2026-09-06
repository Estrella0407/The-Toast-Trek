#include "TarumtScene.h"
#include "ForestScene.h"

static OverworldConfig BuildTarumtConfig() {
    OverworldConfig config;
    config.mapId = MapId::Tarumt;
    config.foregroundLayers = { "Tree_leaf" };
    config.bosses = { { BossId::MrAndrew, 640.0f, 470.0f } };
    config.ComputeSpawnPosition = [](const D3DXVECTOR2&) {
        return D3DXVECTOR2(1120.0f, 600.0f);
    };
    config.OnReachRightEdge = [] { return std::unique_ptr<GameScene>(std::make_unique<ForestScene>()); };
    config.OnAllCleared     = [] { return std::unique_ptr<GameScene>(std::make_unique<ForestScene>()); };
    config.rightEdgeSpawn   = D3DXVECTOR2(110.0f, 150.0f);
    return config;
}

TarumtScene::TarumtScene() : OverworldScene(BuildTarumtConfig()) {}
