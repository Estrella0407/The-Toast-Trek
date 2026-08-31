#include "GameState.h"

// This translation unit now owns only GameStateManager.
//
// It used to also carry hand-written MainMenuState / TutorialState /
// MazeState / GameOverState / Level1State classes. Those were superseded by
// the data-driven OverworldState (see OverworldState.cpp/.h) and the
// standalone MainMenuState.cpp / BattleState.cpp, and left two copies of
// CreateMainMenuState() / CreateBattleState() in the link. They've been
// removed; the concrete screens each live in their own file now:
//   - MainMenuState.cpp        CreateMainMenuState()
//   - OverworldState.cpp       CreateForestState() / CreateOverworldState()
//   - BattleState.cpp          CreateBattleState()
//   - TutorialPopupState.cpp   CreateForestIntroPopup()
//
// A Game Over screen is planned again but not yet reimplemented.

GameStateManager::GameStateManager(GameContext& gameContext)
    : context(gameContext), pendingPopCount(0), clearRequested(false) {
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
