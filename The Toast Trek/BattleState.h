#pragma once

#include "GameState.h"
#include "BattleUI.h"
#include "Battlefield.h"
#include "Enemy.h"
#include <memory>

class BattleState : public GameState {
private:
	std::unique_ptr<Battlefield> battlefield;
	std::unique_ptr<BattleUI> battleUI;
	BossId bossId;

	enum BattlePhase {
		ENCOUNTER,
		PLAYER_TURN,
		ENEMY_HIT,
		ENEMY_ATTACK
	};

	//You have encounter Skullie!
	bool showEncounterMessage;
	//Player hit enemy sprite turn red color
	float enemyFlashTimer;
	int enemyHitFrames;

public:
	explicit BattleState(BossId bossId) : bossId(bossId), battlefield(nullptr), battleUI(nullptr), phase(ENCOUNTER), 
	showEncounterMessage(true), enemyFlashTimer(0.0f), enemyHitFrames(0){}
	~BattleState();
	BattlePhase phase;

	void Initialize(GameContext& context) override;
	void HandleInput(GameContext& context, GameStateManager& manager) override;
	void Update(GameContext& context, GameStateManager& manager) override;
	void Render(GameContext& context) override;
	D3DCOLOR ClearColor() const override;
};