#pragma once
#include <memory>
#include "GameScene.h"

class SoundManager;

// Pushed by BattleScene when Pochi loses a fight. R retries from the start,
// M returns to the main menu. The concrete class is private to GameOverScene.cpp.
std::unique_ptr<GameScene> CreateGameOverScene(SoundManager* sound);
