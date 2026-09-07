#pragma once
#include <memory>
#include "GameScene.h"   // GameScene, and BossId via GameContext.h

// Pushed by the maze / overworld when Pochi walks into a boss. The concrete
// BattleScene class is private to BattleScene.cpp.
std::unique_ptr<GameScene> CreateBattleScene(BossId bossId);
