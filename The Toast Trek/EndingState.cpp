#include "EndingState.h"
#include "FrameTimer.h"
#include "Font.h"
#include "Sprite.h"
#include "SoundManage.h"
#include "TileMap.h"
#include "UiFill.h"
#include <d3dx9.h>
#include <dinput.h>
#include <cmath>
#include <cstring>

// The ending scene
namespace {

    // True on the frame `key` goes from up to down
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        const bool isDown = keys != NULL && (keys[key] & 0x80) != 0;   // 0x80 bit = held
        const bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

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

    // --- Ball / arena tuning (per 1/60 s update) ------------------------
    constexpr int   kBallTexW = 1330, kBallTexH = 1183;   // football.png
    constexpr float kBallRadius = 30.0f;
    constexpr float kBallSpeed  = 9.0f;     // Target speed - a DVD-logo ball never slows
    constexpr float kBounce     = 0.98f;    // fraction of speed kept after a wall bounce
    constexpr float kSpinDamp   = 0.99f;
    constexpr float kArenaL = 8.0f, kArenaR = 1280.0f - 8.0f;   // Window minus a small margin
    constexpr float kArenaT = 8.0f, kArenaB = 720.0f - 8.0f;

    enum class Phase { WalkIn, Dialogue, Credits };

    inline float Len(float x, float y) { return std::sqrt(x * x + y * y); }

    class EndingState : public GameState {
    private:
        Font* creditsFont;
        Font* hintFont;
        Font* nameFont;
        Font* dialogFont;
        IDirect3DTexture9* whiteTex;    // 1x1, for the dim band + dialogue panel
        IDirect3DTexture9* denjiTex;    // Assets/Characters/denji.png
        IDirect3DTexture9* ballTex;     // Assets/Characters/football.png

        FrameTimer timer;              // Keeps the sim speed constant on any PC

        // The credit-roll ball: a circle bouncing inside the window rectangle
        // (Lecture 6: pos += vel, then invert a velocity component on a wall hit)
        D3DXVECTOR2 ballPos;
        D3DXVECTOR2 ballVel;
        float ballAngle;
        float ballSpin;

        bool fWasDown, enterWasDown, escWasDown;
        int pochiAnimAccum;

        Phase phase;
        int dialogIndex;
        int walkInAccum;
        float denjiY;
        float creditsScroll;

        float bounceSfxCooldown;

        void PlayBounceSfx(GameContext& context) {
            if (context.sound != NULL && bounceSfxCooldown <= 0.0f) {
                context.sound->PlaySfx("click", 0.30f);
                bounceSfxCooldown = 0.05f;
            }
        }

        void ResetBall() {
            ballPos = D3DXVECTOR2(430.0f, 250.0f);
            ballVel = D3DXVECTOR2(kBallSpeed * 0.80f, kBallSpeed * 0.60f);   // DVD-style diagonal
            ballAngle = 0.0f;
            ballSpin = 0.02f;
        }

        void EnterCredits(GameContext& context) {
            phase = Phase::Credits;
            creditsScroll = 0.0f;
            ResetBall();
            if (context.sound != NULL) context.sound->PlaySfx("levelcomplete");
        }

        // One update of the bouncing ball
        void StepBall(GameContext& context) {
            // Newtonian motion
            ballPos.x += ballVel.x;
            ballPos.y += ballVel.y;
            ballAngle += ballSpin;
            ballSpin *= kSpinDamp;

            // Wall bounce: clamp back inside the arena, flip that velocity
            // component and keep a fraction of the speed (Lecture 6 deflect)
            if (ballPos.x - kBallRadius < kArenaL) {
                ballPos.x = kArenaL + kBallRadius;
                ballVel.x = -ballVel.x * kBounce;
                PlayBounceSfx(context);
            }
            else if (ballPos.x + kBallRadius > kArenaR) {
                ballPos.x = kArenaR - kBallRadius;
                ballVel.x = -ballVel.x * kBounce;
                PlayBounceSfx(context);
            }
            if (ballPos.y - kBallRadius < kArenaT) {
                ballPos.y = kArenaT + kBallRadius;
                ballVel.y = -ballVel.y * kBounce;
                PlayBounceSfx(context);
            }
            else if (ballPos.y + kBallRadius > kArenaB) {
                ballPos.y = kArenaB - kBallRadius;
                ballVel.y = -ballVel.y * kBounce;
                PlayBounceSfx(context);
            }

            // Nudge the speed back toward target so it never stalls or runs away
            const float s = Len(ballVel.x, ballVel.y);
            if (s > 0.01f) {
                float k = 1.0f + (kBallSpeed - s) * 0.02f;
                if (k < 0.9f) k = 0.9f; else if (k > 1.1f) k = 1.1f;
                ballVel.x *= k; ballVel.y *= k;
            }
        }

        void StepOnce(GameContext& context) {
            if (bounceSfxCooldown > 0.0f) bounceSfxCooldown -= 1.0f / kUpdateFps;

            if (phase == Phase::WalkIn) {
                ++walkInAccum;
                float t = (float)walkInAccum / (float)kWalkInUpdates;
                if (t > 1.0f) t = 1.0f;
                denjiY = kDenjiInY + (kDenjiMeetY - kDenjiInY) * t;   // Linear interpolation
                if (t >= 1.0f) phase = Phase::Dialogue;
            }
            else if (phase == Phase::Credits) {
                StepBall(context);
                creditsScroll += kCreditsScrollSpeed;
                const float rollHeight = kCreditsStartY + kCreditsLineCount * kCreditsLineGap + 80.0f;
                if (creditsScroll > rollHeight) creditsScroll = 0.0f;   // Loop
            }

            // Advance Pochi's walk animation during the cutscene
            if (phase != Phase::Credits && context.pochi != NULL && ++pochiAnimAccum >= 12) {
                pochiAnimAccum = 0;
                context.pochi->NextFrame();
            }
        }

    public:
        EndingState()
            : creditsFont(NULL), hintFont(NULL), nameFont(NULL), dialogFont(NULL),
              whiteTex(NULL), denjiTex(NULL), ballTex(NULL),
              ballPos(0.0f, 0.0f), ballVel(0.0f, 0.0f), ballAngle(0.0f), ballSpin(0.0f),
              fWasDown(true), enterWasDown(true), escWasDown(true), pochiAnimAccum(0),
              phase(Phase::WalkIn), dialogIndex(0), walkInAccum(0), denjiY(kDenjiInY),
              creditsScroll(0.0f), bounceSfxCooldown(0.0f) {
        }

        ~EndingState() override {
            delete creditsFont;
            delete hintFont;
            delete nameFont;
            delete dialogFont;
            if (whiteTex != NULL) whiteTex->Release();
            if (denjiTex != NULL) denjiTex->Release();
            if (ballTex != NULL) ballTex->Release();
        }

        void Initialize(GameContext& context) override {
            denjiTex = ui::LoadTexture(context.device, "Assets/Characters/denji.png",
                                       kDenjiTexW, kDenjiTexH);
            ballTex = ui::LoadTexture(context.device, "Assets/Characters/football.png",
                                      kBallTexW, kBallTexH);
            whiteTex = ui::MakeWhiteTexture(context.device);

            creditsFont = new Font(context.device, 0.0f, 0.0f, 1200, 40, 24, "Arial");
            hintFont = new Font(context.device, 0.0f, 0.0f, 800, 30, 18, "Arial");
            nameFont = new Font(context.device, 0.0f, 0.0f, 300, 34, 22, "Arial");
            dialogFont = new Font(context.device, 0.0f, 0.0f, 1000, 44, 26, "Arial");

            if (context.pochi != NULL) {
                context.pochi->SetPosition(kPochiCutsceneX, kPochiCutsceneY);
                context.pochi->SetScale(2.0f);
                context.pochi->CropToFrame(0);
            }

            timer.Init(kUpdateFps);

            phase = Phase::WalkIn;
            dialogIndex = 0;
            walkInAccum = 0;
            denjiY = kDenjiInY;
            pochiAnimAccum = 0;
            creditsScroll = 0.0f;
            ResetBall();

            fWasDown = enterWasDown = escWasDown = true;
            bounceSfxCooldown = 0.0f;
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

            // Phase::Credits - just the ball; Enter / Esc leaves
            if (JustPressed(k, DIK_RETURN, enterWasDown) || JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.ClearAndPush(CreateMainMenuState());
            }
        }

        void Update(GameContext& context, GameStateManager&) override {
            int frames = timer.FramesToUpdate();
            if (frames > 4) frames = 4;   // Don't let one long hitch spiral
            for (int i = 0; i < frames; ++i) StepOnce(context);
        }

        void RenderCutscene(GameContext& context) {
            LPD3DXSPRITE b = context.spriteBrush;

            if (context.ruinsInteriorMap != NULL) context.ruinsInteriorMap->Draw(b);
            else ui::FillRect(b, whiteTex, 0.0f, 0.0f, 1280.0f, 720.0f, D3DCOLOR_XRGB(14, 12, 18));

            if (context.pochi != NULL) context.pochi->Draw(b);
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

            // The football, spun to ballAngle, over the top of everything
            if (ballTex != NULL) {
                const float d = 2.0f * kBallRadius;
                ui::DrawTextureRotated(b, ballTex, kBallTexW, kBallTexH,
                                       ballPos.x, ballPos.y, d, d, ballAngle);
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

std::unique_ptr<GameState> CreateEndingState() {
    return std::make_unique<EndingState>();
}
