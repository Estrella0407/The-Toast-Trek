#include "BattleState.h"

void BattleState::Initialize(GameContext& context) {
	battlefield = std::make_unique<Battlefield>(context.device);
}

void BattleState::HandleInput(GameContext& context, GameStateManager& manager) {

}

void BattleState::Update(GameContext& context, GameStateManager& manager) {
	battlefield->Update(context.keys);
}

void BattleState::Render(GameContext& context) {
	battlefield->Render(context.spriteBrush);
}

D3DCOLOR BattleState::ClearColor() const {
	return D3DCOLOR_XRGB(255, 255, 255);
}