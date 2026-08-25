#pragma once

#include "GameState.h"
#include "Battlefield.h"
#include <memory>

class BattleState : public GameState {
private:
	std::unique_ptr<Battlefield> battlefield;

public:
	void Initialize(GameContext& context) override;
	void HandleInput(GameContext& context, GameStateManager& manager) override;
	void Update(GameContext& context, GameStateManager& manager) override;
	void Render(GameContext& context) override;
	D3DCOLOR ClearColor() const override;
};