#include "GameState.h"
#include "Pochi.h"
#include "Inventory.h"

GameStateManager::GameStateManager(GameContext& gameContext)
    : context(gameContext), pendingPopCount(0), clearRequested(false) {
}

// Resets everything a fresh playthrough starts from (main menu, retry)
void ResetRunProgress(GameContext& context) {
    if (context.playerStats != NULL) context.playerStats->SetLevel(1); // Fresh Pochi, stats refilled
    if (context.inventory != NULL) context.inventory->Reset();         // Empty the pack
    context.clearedMaps.clear();                                       // Every map locked again
    context.lastBattleOutcome = BattleOutcome::None;                   // Drop any stale battle result
    context.hasPendingSpawn = false;                                   // Drop any pending spawn hand-off
}

void GameStateManager::Push(std::unique_ptr<GameState> state) {
    if (state != NULL) pendingPushes.push_back(std::move(state));
}

void GameStateManager::Pop() {
    ++pendingPopCount;
}

void GameStateManager::ClearAndPush(std::unique_ptr<GameState> state) {
    clearRequested = true;
    pendingPopCount = 0;
    pendingPushes.clear();
    if (state != NULL) pendingPushes.push_back(std::move(state));
}

void GameStateManager::ApplyPendingChanges() {
    if (clearRequested) stateStack.clear();
    clearRequested = false;
    while (pendingPopCount > 0 && !stateStack.empty()) {
        stateStack.pop_back();
        --pendingPopCount;
    }
    pendingPopCount = 0;
    for (size_t i = 0; i < pendingPushes.size(); ++i) {
        pendingPushes[i]->Initialize(context);
        stateStack.push_back(std::move(pendingPushes[i]));
    }
    pendingPushes.clear();
}

void GameStateManager::HandleInput() {
    if (!stateStack.empty()) stateStack.back()->HandleInput(context, *this);
}

void GameStateManager::Update() {
    if (!stateStack.empty()) stateStack.back()->Update(context, *this);
}

void GameStateManager::Render() {
    if (!stateStack.empty()) stateStack.back()->Render(context);
}

D3DCOLOR GameStateManager::ClearColor() const {
    return stateStack.empty() ? D3DCOLOR_XRGB(0, 0, 0) : stateStack.back()->ClearColor();
}
