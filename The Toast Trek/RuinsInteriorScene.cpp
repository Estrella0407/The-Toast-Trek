#include "RuinsInteriorScene.h"
#include "EndingScene.h"

static OverworldConfig BuildRuinsInteriorConfig() {
    OverworldConfig config;
    config.mapId = MapId::RuinsInterior;
    config.bosses = {
        { BossId::Maki, 610.0f, 200.0f }
    };
    config.OnAllCleared = [] { return CreateEndingScene(); };
    config.ComputeSpawnPosition = [](const D3DXVECTOR2&) {
        return D3DXVECTOR2(614.0f, 540.0f);
    };
    return config;
}

RuinsInteriorScene::RuinsInteriorScene() : OverworldScene(BuildRuinsInteriorConfig()) {}
