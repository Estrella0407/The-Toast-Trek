#include "BattleState.h"
#include "Battlefield.h"
#include "BattleUI.h"
#include "Enemy.h"


void BattleState::Initialize(GameContext& context) {
	battleUI = std::make_unique<BattleUI>(context.device);
	Enemy* enemy = CreateBossEnemy(context.device, bossId, 600.0f, 50.0f);
	battlefield = std::make_unique<Battlefield>(context.device, battleUI.get(), enemy);
}

void BattleState::HandleInput(GameContext& context, GameStateManager& manager) {	
	
	if (phase == ENCOUNTER) {
		//wait for player to continue
		phase = PLAYER_TURN;
		return;
	}

	if (phase == PLAYER_TURN) {
		OutputDebugStringA("PLAYER TURN ACTIVE!\n");
		battleUI->UpdateMenuButtons(context);
		OutputDebugStringA("CHECKING BATTLE	BUTTONS!\n");
		int clickedButton = battleUI->GetSelectButton(context);

		OutputDebugStringA(("Clicked button: " + std::to_string(clickedButton) + "\n").c_str());

		if (clickedButton == 0) {
			//FIGHT was clicked
			phase = ENEMY_HIT;
			battleUI->SetShowEncounterMessage(false);
			battlefield->PerformFight();
			enemyHitFrames = 12;

			OutputDebugStringA("FIGHT CLICKED!\n");
			OutputDebugStringA("Enemy takes damage!\n");
		}
	}
	else if (phase == ENEMY_HIT) {
		enemyHitFrames--;

		if (enemyHitFrames <= 0) {
			phase = ENEMY_ATTACK;
			battlefield->SetShowProjectiles(true);
			OutputDebugStringA("ENEMY ATTACK START!\n");
		}
	}
	else if (phase == ENEMY_ATTACK) {
		battlefield->Update(context);
		if (battlefield->IsProjectileAttackFinished()) {
			battlefield->SetShowProjectiles(false);
			phase = PLAYER_TURN;
			OutputDebugStringA("BACK TO PLAYER TURN\n");
		}
	}

	
}

void BattleState::Update(GameContext& context, GameStateManager& manager) {
	battlefield->Update(context);

	// Win, lose, or flee, return to wherever Pochi was in the maze -
	// BattleState sits on top of that state on the stack, so popping
	// reveals it as-is. Record the outcome so the maze can mark this boss
	// cleared once it's back on top.
	if (battlefield->IsPlayerDefeated()) {
		context.lastBattleOutcome = BattleOutcome::Defeat;
		context.lastBattleBoss = bossId;
		manager.Pop();
	}
	else if (battlefield->IsEnemyDefeated()) {
		context.lastBattleOutcome = BattleOutcome::Victory;
		context.lastBattleBoss = bossId;
		manager.Pop();
	}
	else if (battlefield->HasFled()) {
		context.lastBattleOutcome = BattleOutcome::Fled;
		context.lastBattleBoss = bossId;
		manager.Pop();
	}
}

void BattleState::Render(GameContext& context) {
	battlefield->Render(context.spriteBrush);
	battleUI->Render(context.spriteBrush);
}

D3DCOLOR BattleState::ClearColor() const {
	return D3DCOLOR_XRGB(255, 255, 255);
}

BattleState::~BattleState() = default;