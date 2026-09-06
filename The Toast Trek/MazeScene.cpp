#include "MazeScene.h"
#include "ForestScene.h"
#include "RuinsExteriorScene.h"

static OverworldConfig BuildMazeConfig() {
    OverworldConfig config;
    config.mapId = MapId::Maze;
    config.bosses = {
        { BossId::SkullBones, 440.0f, 100.0f },
        { BossId::Goblin, 780.0f, 317.0f }
    };
    // The forest and maze art line up at the shared edge
    config.ComputeSpawnPosition = [](const D3DXVECTOR2& current) {
        return D3DXVECTOR2(40.0f, current.y);
    };
    // Walk back into the left edge to return to the forest
    config.OnReachLeftEdge = [] { return std::unique_ptr<GameScene>(std::make_unique<ForestScene>()); };
    config.leftEdgeSpawn = D3DXVECTOR2(1160.0f, OverworldConfig::kCarryY);
    // Fence Pochi's feet inside so he can't skip the whole maze
    config.fenceTop = 40.0f;
    config.fenceBottom = 680.0f;
    // Pochi has to clear the maze - SkullBones then the Goblin
    // before the right-edge exit to the ruins will open
    config.requireBossesCleared = true;
    config.bossesInOrder = true;
    // A barred gate across the right-edge opening while the maze is uncleared
    config.gateX = 1232.0f;
    config.gateY = 40.0f;
    config.gateWidth = 34.0f;
    config.gateHeight = 640.0f;
    config.OnReachRightEdge = [] { return std::unique_ptr<GameScene>(std::make_unique<RuinsExteriorScene>()); };

    config.items = {
        { ItemType::HealthPotion, "Assets/Item/heathPotion.png", 18, 20, 380.0f, 392.0f, 2.0f },
        { ItemType::Bone, "Assets/Item/bone.png", 32, 32, 180.0f, 75.0f, 2.0f },
        { ItemType::Toast, "Assets/Item/toast.png", 16, 16, 1090.0f, 620.0f, 2.0f }
    };

    return config;
}

MazeScene::MazeScene() : OverworldScene(BuildMazeConfig()) {}
