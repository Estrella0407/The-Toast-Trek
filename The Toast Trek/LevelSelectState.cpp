//#include "LevelSelectState.h"
//#include "GameState.h"
//#include <dinput.h>
//#include <string>
//
//namespace {
//    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
//        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
//        bool pressed = isDown && !wasDown;
//        wasDown = isDown;
//        return pressed;
//    }
//}
//
//LevelSelectState::LevelSelectState(SoundManage* soundMgr)
//    : titleFont(nullptr)
//    , levelNameFont(nullptr)
//    , levelDescFont(nullptr)
//    , statusFont(nullptr)
//    , promptFont(nullptr)
//    , selectedLevel(0)
//    , maxLevels(0)
//    , upWasDown(false)
//    , downWasDown(false)
//    , enterWasDown(false)
//    , escapeWasDown(false)
//    , leftWasDown(false)
//    , rightWasDown(false)
//    , soundManage(soundMgr) {
//
//    LoadLevelData();
//}
//
//LevelSelectState::~LevelSelectState() {
//    delete titleFont;
//    delete levelNameFont;
//    delete levelDescFont;
//    delete statusFont;
//    delete promptFont;
//}
//
//void LevelSelectState::LoadLevelData() {
//    // Example level data - you can load this from a file later
//    levels.push_back({ "Tutorial", "Learn the basics of movement", true, 0, "" });
//    levels.push_back({ "Level 1", "The Forest of Beginnings", true, 0, "" });
//    levels.push_back({ "Level 2", "The Dark Cave", false, 0, "" });
//    levels.push_back({ "Level 3", "The Frozen Peak", false, 0, "" });
//    levels.push_back({ "Level 4", "The Final Battle", false, 0, "" });
//
//    maxLevels = (int)levels.size();
//}
//
//void LevelSelectState::Initialize(GameContext& context) {
//    titleFont = new Font(context.device, 0.0f, 80.0f, 1280, 70, 48, "Arial");
//    levelNameFont = new Font(context.device, 200.0f, 200.0f, 500, 50, 32, "Arial");
//    levelDescFont = new Font(context.device, 200.0f, 260.0f, 600, 40, 22, "Arial");
//    statusFont = new Font(context.device, 200.0f, 320.0f, 300, 40, 22, "Arial");
//    promptFont = new Font(context.device, 0.0f, 650.0f, 1280, 30, 20, "Arial");
//}
//
//void LevelSelectState::HandleInput(GameContext& context, GameStateManager& manager) {
//    BYTE* keys = context.keys;
//
//    if (JustPressed(keys, DIK_ESCAPE, escapeWasDown)) {
//        manager.Pop();
//        return;
//    }
//
//    if (JustPressed(keys, DIK_UP, upWasDown)) {
//        selectedLevel = (selectedLevel - 1 + maxLevels) % maxLevels;
//    }
//
//    if (JustPressed(keys, DIK_DOWN, downWasDown)) {
//        selectedLevel = (selectedLevel + 1) % maxLevels;
//    }
//
//    if (JustPressed(keys, DIK_RETURN, enterWasDown)) {
//        if (levels[selectedLevel].unlocked) {
//            // Start the selected level
//            // TODO: Push the appropriate level state
//            // For now, just show a message
//            OutputDebugStringA(("Starting level: " + levels[selectedLevel].name + "\n").c_str());
//        }
//    }
//}
//
//void LevelSelectState::Update(GameContext& context, GameStateManager& manager) {
//    if (soundManage) {
//        soundManage->Update();
//    }
//}
//
//void LevelSelectState::Render(GameContext& context) {
//    if (titleFont) {
//        titleFont->Draw("SELECT LEVEL", D3DCOLOR_XRGB(255, 255, 255));
//    }
//
//    float yPos = 180.0f;
//    for (int i = 0; i < maxLevels; i++) {
//        D3DCOLOR color = (i == selectedLevel) ? D3DCOLOR_XRGB(255, 255, 100) : D3DCOLOR_XRGB(200, 200, 200);
//        std::string displayName = levels[i].name;
//
//        if (!levels[i].unlocked) {
//            displayName += " (LOCKED)";
//            color = D3DCOLOR_XRGB(100, 100, 100);
//        }
//
//        if (levelNameFont) {
//            levelNameFont->Draw(displayName.c_str(), 300.0f, yPos, color);
//        }
//
//        // Draw stars if unlocked
//        if (levels[i].unlocked && levels[i].stars > 0) {
//            std::string stars;
//            for (int s = 0; s < levels[i].stars; s++) {
//                stars += "★";
//            }
//            for (int s = levels[i].stars; s < 3; s++) {
//                stars += "☆";
//            }
//            if (statusFont) {
//                statusFont->Draw(stars.c_str(), 750.0f, yPos, D3DCOLOR_XRGB(255, 215, 0));
//            }
//        }
//
//        yPos += 60.0f;
//    }
//
//    // Show selected level description
//    if (levelDescFont) {
//        levelDescFont->Draw(levels[selectedLevel].description.c_str(),
//            300.0f, 480.0f, D3DCOLOR_XRGB(180, 180, 180));
//    }
//
//    if (promptFont) {
//        promptFont->Draw("UP/DOWN: Select  |  ENTER: Start  |  ESC: Back",
//            D3DCOLOR_XRGB(150, 150, 150));
//    }
//}
//
//D3DCOLOR LevelSelectState::ClearColor() const {
//    return D3DCOLOR_XRGB(20, 30, 50);
//}