#include "BattleState.h"

void BattleState::Initialize(GameContext& context) {
	battlefield = std::make_unique<Battlefield>(context.device, bossId);
}

void BattleState::HandleInput(GameContext& context, GameStateManager& manager) {

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
}

D3DCOLOR BattleState::ClearColor() const {
	return D3DCOLOR_XRGB(255, 255, 255);
}