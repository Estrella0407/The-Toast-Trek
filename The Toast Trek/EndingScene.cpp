#include "EndingScene.h"
#include "Pochi.h"
#include "Sprite.h"
#include "MainMenuScene.h"
#include "FrameTimer.h"
#include "Font.h"
#include "Sprite.h"
#include "SoundManager.h"
#include "SaveGame.h"
#include "TileMap.h"
#include "MapLibrary.h"
#include "UiFill.h"
#include <d3dx9.h>
#include <dinput.h>
#include <cstring>

// The ending scene
namespace {

    // True on the frame `key` goes from up to down

    constexpr int kUpdateFps = 60;   // Fixed updates per second

    // --- Reunion cutscene -----
    // Denji walks up from just below the screen to a stop below Pochi
    constexpr float kMeetX        = 640.0f;     // Denji's x - centred in the corridor gap
    constexpr float kDenjiInY     = 810.0f;     // Feet anchor: start off-screen, emerge from the corridor
    constexpr float kDenjiMeetY   = 430.0f;     // Feet anchor: stopped just below Pochi
    constexpr int   kWalkInUpdates = 130;       // A touch longer, now that the walk is longer
    constexpr float kPochiCutsceneX = 610.0f;   // Sprite pos (top-left), not feet - near where Maki fell
    constexpr float kPochiCutsceneY = 300.0f;
    constexpr int   kDenjiTexW = 1122, kDenjiTexH = 1402;
    constexpr float kDenjiDrawH = 120.0f;       // Match Makima's on-screen size
    constexpr float kDenjiDrawW = kDenjiDrawH * (float)kDenjiTexW / (float)kDenjiTexH;

    const char* const kReunionLines[] = {
        "Pochi... I finally found you!",
        "Let's get you home.",
    };
    constexpr int kReunionLineCount = 2;

    // Bottom dialogue panel
    constexpr float kBoxL = 110.0f, kBoxT = 548.0f, kBoxW = 1060.0f, kBoxH = 138.0f;

    // --- Credit roll -----------------------------------------------------
    constexpr float kCreditsScrollSpeed = 0.55f;   // px per 1/60 s update
    constexpr float kCreditsStartY      = 720.0f;
    constexpr float kCreditsLineGap     = 40.0f;
    constexpr float kCreditsCenterX     = 640.0f;

    // Edit freely - one entry per line, "" for a spacer
    const char* const kCreditsLines[] = {
        "THE  TOAST  TREK",
        "",
        "BMCS2224  -  COMPUTER GAME PROGRAMMING",
        "",
        "",
        "-  Contributors  -",
        "Chong Wei Xin",
        "Khor Pheng Xuan",
        "Melaine Yang Mei",  
        "",
        "",
        "Pochi, Denji, Makima and friends belong",
        "to their respective creators.",
        "",
        "",
        "Thanks for playing!",
        "",
        "",
        "",
    };
    constexpr int kCreditsLineCount = (int)(sizeof(kCreditsLines) / sizeof(kCreditsLines[0]));

    enum class Phase { WalkIn, Dialogue, Credits };

    class EndingScene : public GameScene {
    private:
        Font* creditsFont;
        Font* hintFont;
        Font* nameFont;
        Font* dialogFont;
        IDirect3DTexture9* whiteTex;    // 1x1, for the dim band + dialogue panel
        IDirect3DTexture9* denjiTex;    // Assets/Characters/denji.png

        FrameTimer timer;              // Keeps the sim speed constant on any PC

        bool fWasDown, enterWasDown, escWasDown;
        int pochiAnimAccum;

        Phase phase;
        int dialogIndex;
        int walkInAccum;
        float denjiY;
        float creditsScroll;

        void EnterCredits(GameContext& context) {
            phase = Phase::Credits;
            creditsScroll = 0.0f;
            if (context.sound != NULL) context.sound->PlaySfx("levelcomplete");
        }

        void StepOnce(GameContext& context) {
            if (phase == Phase::WalkIn) {
                ++walkInAccum;
                float t = (float)walkInAccum / (float)kWalkInUpdates;
                if (t > 1.0f) t = 1.0f;
                denjiY = kDenjiInY + (kDenjiMeetY - kDenjiInY) * t;   // Linear interpolation
                if (t >= 1.0f) phase = Phase::Dialogue;
            }
            else if (phase == Phase::Credits) {
                creditsScroll += kCreditsScrollSpeed;
                const float rollHeight = kCreditsStartY + kCreditsLineCount * kCreditsLineGap + 80.0f;
                if (creditsScroll > rollHeight) creditsScroll = 0.0f;   // Loop
            }

            // Advance Pochi's walk animation during the cutscene
            if (phase != Phase::Credits && context.pochi != NULL && ++pochiAnimAccum >= 12) {
                pochiAnimAccum = 0;
                context.pochi->GetSprite()->NextFrame();
            }
        }

    public:
        EndingScene()
            : creditsFont(NULL), hintFont(NULL), nameFont(NULL), dialogFont(NULL),
              whiteTex(NULL), denjiTex(NULL),
              fWasDown(true), enterWasDown(true), escWasDown(true), pochiAnimAccum(0),
              phase(Phase::WalkIn), dialogIndex(0), walkInAccum(0), denjiY(kDenjiInY),
              creditsScroll(0.0f) {
        }

        ~EndingScene() override {
            delete creditsFont;
            delete hintFont;
            delete nameFont;
            delete dialogFont;
            if (whiteTex != NULL) whiteTex->Release();
            if (denjiTex != NULL) denjiTex->Release();
        }

        void Initialize(GameContext& context) override {
            save::ClearProgress();   // game finished - no "Continue" from here

            denjiTex = ui::LoadTexture(context.device, "Assets/Characters/denji.png",
                                       kDenjiTexW, kDenjiTexH);
            whiteTex = ui::MakeWhiteTexture(context.device);

            creditsFont = new Font(context.device, 0.0f, 0.0f, 1200, 40, 24, "Arial");
            hintFont = new Font(context.device, 0.0f, 0.0f, 800, 30, 18, "Arial");
            nameFont = new Font(context.device, 0.0f, 0.0f, 300, 34, 22, "Arial");
            dialogFont = new Font(context.device, 0.0f, 0.0f, 1000, 44, 26, "Arial");

            if (context.pochi != NULL) {
                context.pochi->GetSprite()->SetPosition(kPochiCutsceneX, kPochiCutsceneY);
                context.pochi->GetSprite()->SetScale(2.0f);
                context.pochi->GetSprite()->CropToFrame(0);
            }

            timer.Init(kUpdateFps);

            phase = Phase::WalkIn;
            dialogIndex = 0;
            walkInAccum = 0;
            denjiY = kDenjiInY;
            pochiAnimAccum = 0;
            creditsScroll = 0.0f;

            fWasDown = enterWasDown = escWasDown = true;
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (phase == Phase::WalkIn) {
                if (JustPressed(k, DIK_F, fWasDown) || JustPressed(k, DIK_RETURN, enterWasDown)) {
                    walkInAccum = kWalkInUpdates;   // Snap Denji to the meeting spot
                    denjiY = kDenjiMeetY;
                    phase = Phase::Dialogue;
                }
                return;
            }

            if (phase == Phase::Dialogue) {
                if (JustPressed(k, DIK_F, fWasDown) || JustPressed(k, DIK_RETURN, enterWasDown)) {
                    ++dialogIndex;
                    if (context.sound != NULL) context.sound->PlaySfx("click");
                    if (dialogIndex >= kReunionLineCount) EnterCredits(context);
                }
                return;
            }

            // Phase::Credits - Enter / Esc returns to the main menu
            if (JustPressed(k, DIK_RETURN, enterWasDown) || JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.ClearAndPush(CreateMainMenuScene());
            }
        }

        void Update(GameContext& context, GameStateManager&) override {
            int frames = timer.FramesToUpdate();
            if (frames > 4) frames = 4;   // Don't let one long hitch spiral
            for (int i = 0; i < frames; ++i) StepOnce(context);
        }

        void RenderCutscene(GameContext& context) {
            LPD3DXSPRITE b = context.spriteBrush;

            if (context.maps->RuinsInterior() != NULL) context.maps->RuinsInterior()->Draw(b);
            else ui::FillRect(b, whiteTex, 0.0f, 0.0f, 1280.0f, 720.0f, D3DCOLOR_XRGB(14, 12, 18));

            if (context.pochi != NULL) context.pochi->GetSprite()->Draw(b);
            if (denjiTex != NULL) {
                ui::DrawTexture(b, denjiTex, kDenjiTexW, kDenjiTexH,
                                kMeetX - kDenjiDrawW * 0.5f, denjiY - kDenjiDrawH,
                                kDenjiDrawW / kDenjiTexW, kDenjiDrawH / kDenjiTexH);
            }

            if (phase != Phase::Dialogue) return;

            ui::FillRect(b, whiteTex, kBoxL, kBoxT, kBoxW, kBoxH, D3DCOLOR_ARGB(238, 20, 16, 24));
            const D3DCOLOR gold = D3DCOLOR_ARGB(255, 216, 184, 128);
            const float bw = 3.0f;
            ui::FillRect(b, whiteTex, kBoxL, kBoxT, kBoxW, bw, gold);
            ui::FillRect(b, whiteTex, kBoxL, kBoxT + kBoxH - bw, kBoxW, bw, gold);
            ui::FillRect(b, whiteTex, kBoxL, kBoxT, bw, kBoxH, gold);
            ui::FillRect(b, whiteTex, kBoxL + kBoxW - bw, kBoxT, bw, kBoxH, gold);

            if (nameFont != NULL)
                nameFont->Draw("Denji", kBoxL + 34.0f, kBoxT + 20.0f, gold, b);
            if (dialogFont != NULL)
                dialogFont->Draw(kReunionLines[dialogIndex], kBoxL + 34.0f, kBoxT + 64.0f,
                                 D3DCOLOR_XRGB(236, 232, 226), b);
            if (hintFont != NULL)
                hintFont->Draw("Press Enter to continue",
                               kBoxL + kBoxW - 214.0f, kBoxT + kBoxH - 30.0f,
                               D3DCOLOR_XRGB(168, 166, 162), b);
        }

        void RenderCredits(GameContext& context) {
            LPD3DXSPRITE b = context.spriteBrush;

            ui::FillRect(b, whiteTex, 0.0f, 0.0f, 1280.0f, 720.0f, D3DCOLOR_XRGB(0, 0, 0));

            // Auto-scrolling credit roll up the middle
            if (creditsFont != NULL) {
                for (int i = 0; i < kCreditsLineCount; ++i) {
                    const char* line = kCreditsLines[i];
                    if (line == NULL || line[0] == '\0') continue;
                    const float y = kCreditsStartY + i * kCreditsLineGap - creditsScroll;
                    if (y < -kCreditsLineGap || y > 720.0f) continue;
                    const float x = kCreditsCenterX - (float)strlen(line) * 6.3f;   // Rough centre
                    creditsFont->Draw(line, x + 1.0f, y + 1.0f, D3DCOLOR_XRGB(0, 0, 0), b);   // shadow
                    creditsFont->Draw(line, x, y, D3DCOLOR_XRGB(232, 228, 220), b);
                }
            }

            if (hintFont != NULL)
                hintFont->Draw("Press Enter / Esc to return to main menu",
                               500.0f, 26.0f, D3DCOLOR_XRGB(110, 110, 118), b);
        }

        void Render(GameContext& context) override {
            if (phase == Phase::Credits) RenderCredits(context);
            else RenderCutscene(context);
            // The "CHEAT MODE" indicator is drawn globally in Main.cpp
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

} // Namespace

std::unique_ptr<GameScene> CreateEndingScene() {
    return std::make_unique<EndingScene>();
}
