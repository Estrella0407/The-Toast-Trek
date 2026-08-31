//#pragma once
//#include "GameState.h"
//#include "Font.h"
//#include "SoundManage.h"
//#include <vector>
//#include <memory>
//
//struct LevelInfo {
//    std::string name;
//    std::string description;
//    bool unlocked;
//    int stars; // 0-3
//    std::string thumbnailPath;
//};
//
//class LevelSelectState : public GameState {
//private:
//    Font* titleFont;
//    Font* levelNameFont;
//    Font* levelDescFont;
//    Font* statusFont;
//    Font* promptFont;
//
//    std::vector<LevelInfo> levels;
//    int selectedLevel;
//    int maxLevels;
//
//    bool upWasDown;
//    bool downWasDown;
//    bool enterWasDown;
//    bool escapeWasDown;
//    bool leftWasDown;
//    bool rightWasDown;
//
//    SoundManage* soundManage;
//
//    void LoadLevelData();
//
//public:
//    LevelSelectState(SoundManage* soundMgr);
//    ~LevelSelectState();
//
//    void Initialize(GameContext& context) override;
//    void HandleInput(GameContext& context, GameStateManager& manager) override;
//    void Update(GameContext& context, GameStateManager& manager) override;
//    void Render(GameContext& context) override;
//    D3DCOLOR ClearColor() const override;
//};