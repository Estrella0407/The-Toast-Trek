#pragma once

#include "GameState.h"
#include "BattleUI.h"
#include "Battlefield.h"
#include "Pochi.h"
#include "Enemy.h"
#include <memory>

class BattleState : public GameState {
private:
	std::unique_ptr<Battlefield> battlefield;
	std::unique_ptr<BattleUI> battleUI;
	Pochi* pochi;
	BossId bossId;

	enum BattlePhase {
		ENCOUNTER,
		PLAYER_TURN,
		ACT_MENU,
		ITEM_MENU,
		ACT_ANIMATION,
		ENEMY_HIT,
		ENEMY_ATTACK
	};

	// You have encounter Skullie!
	bool showEncounterMessage;
	// Player hit enemy sprite turn red color
	float enemyFlashTimer;
	int enemyHitFrames;
	bool actionKeyWasDown[4];
	bool actChoiceWasDown[3];
	bool itemChoiceWasDown[3];
	bool actChoiceUsed[3];	// After use one of the choices, the selected button will be gone
	bool cheatWinWasDown;	// Dev cheat: K ends the fight in a win (see Cheats.h)
	int lastPochiHealth;	// Prev-frame heart + armour total -> a drop triggers the "hurt" sfx

public:
	explicit BattleState(BossId bossId) : bossId(bossId), battlefield(nullptr), battleUI(nullptr), phase(ENCOUNTER), 
	showEncounterMessage(true), enemyFlashTimer(0.0f), enemyHitFrames(0),
		actionKeyWasDown{ false, false, false, false }, actChoiceWasDown{ false, false, false },
		itemChoiceWasDown{ false, false, false }, actChoiceUsed{ false, false, false},
		cheatWinWasDown(false), lastPochiHealth(0) {}

	~BattleState();
	BattlePhase phase;

	void Initialize(GameContext& context) override;
	void HandleInput(GameContext& context, GameStateManager& manager) override;
	void Update(GameContext& context, GameStateManager& manager) override;
	void Render(GameContext& context) override;
	D3DCOLOR ClearColor() const override;
};
