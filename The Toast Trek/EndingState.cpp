#include "EndingState.h"
#include "FrameTimer.h"
#include "Font.h"
#include "Physics.h"
#include "Sprite.h"
#include "SoundManage.h"
#include "TileMap.h"
#include "UiFill.h"
#include <d3dx9.h>
#include <dinput.h>
#include <cmath>
#include <cstring>

// ---------------------------------------------------------------------------
//  The ending scene. Everything here is built from the unit's lecture notes:
//    * Lecture 4 (Input)     - DirectInput key buffer, `keys[DIK_x] & 0x80`,
//                              edge detection (JustPressed).
//    * Lecture 5 (2D Animation) - FrameTimer (QueryPerformanceCounter) drives
//                              a fixed 60-updates-per-second step.
//    * Lecture 6 (Physics)   - the credit-roll ball is a convex-polygon rigid
//                              body bounced around the screen frame:
//                                1. Collision detection - Separating Axis
//                                   Theorem. Project both polygons onto every
//                                   candidate axis (each polygon's edge
//                                   normals); compare the projections - a gap
//                                   on ANY axis means no collision. The axis
//                                   of least overlap is the contact normal
//                                   (tells us which side, not the incoming
//                                   direction). Convex only.
//                                2. Overlap resolution - once penetrating,
//                                   push the pair apart along that normal by
//                                   the penetration depth (MTV), split by
//                                   inverse mass.
//                                3. Bounce force - an impulse. F = m a, so
//                                   a = F/m; an impulse is F*dt applied along
//                                   the normal. Equal and opposite on both
//                                   bodies (Newton's 3rd law / conservation of
//                                   momentum). j = -(1+e)(vRel . n) / (1/mA +
//                                   1/mB + (rA x n)^2/IA + (rB x n)^2/IB); the
//                                   (r x n) terms give the spin (torque) from
//                                   an off-centre hit. Momentum p = m v.
//    * Lecture 3 (2D Graphics) - one D3DXSprite Begin/End batch; textures and
//                              ID3DXFont text drawn through it; a stretched
//                              1x1 texture for panel fills; the ball texture
//                              rotated via the transformation matrix.
//    * Lecture 7 (Sound)     - a short SFX on each bounce.
// ---------------------------------------------------------------------------

namespace {

    // --- Edge-detect one key on the DirectInput buffer -----------
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        const bool isDown = keys != NULL && (keys[key] & 0x80) != 0;   // & 0x80 -> key held
        const bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    constexpr int kUpdateFps = 60;   // Lecture 5: fixed timestep

    // --- Reunion cutscene (phases 1 & 2, over the ruins-interior map) -----
    // The entrance corridor is the gap in Walls_top at cols 38-41 (x 608-672);
    // Denji walks up it from just below the screen to a stop below Pochi.
    constexpr float kMeetX        = 640.0f;   // Denji's x - centred in the corridor gap
    constexpr float kDenjiInY     = 810.0f;   // feet anchor: start off-screen, emerge from the corridor
    constexpr float kDenjiMeetY   = 430.0f;   // feet anchor: stopped just below Pochi
    constexpr int   kWalkInUpdates = 130;     // a touch longer, now that the walk is longer
    constexpr float kPochiCutsceneX = 610.0f; // Sprite pos (top-left), not feet - near where Maki fell
    constexpr float kPochiCutsceneY = 300.0f;
    constexpr int   kDenjiTexW = 1122, kDenjiTexH = 1402;
    constexpr float kDenjiDrawH = 120.0f;   // match Makima's on-screen size
    constexpr float kDenjiDrawW = kDenjiDrawH * (float)kDenjiTexW / (float)kDenjiTexH;

    const char* const kReunionLines[] = {
        "Pochi... I finally found you!",
        "Let's get you home.",
    };
    constexpr int kReunionLineCount = 2;

    // Bottom dialogue panel.
    constexpr float kBoxL = 110.0f, kBoxT = 548.0f, kBoxW = 1060.0f, kBoxH = 138.0f;

    // --- Credit roll -----------------------------------------------------
    constexpr float kCreditsScrollSpeed = 0.55f;   // px per 1/60 s update
    constexpr float kCreditsStartY      = 720.0f;
    constexpr float kCreditsLineGap     = 40.0f;
    constexpr float kCreditsCenterX     = 640.0f;

    // Edit freely - one entry per line, "" for a spacer.
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

    // --- ball / arena tuning (per 1/60 s update) ------------------------
    constexpr int   kBallTexW = 1330, kBallTexH = 1183;   // football.png
    constexpr float kBallRadius   = 30.0f;    // octagon circumradius
    constexpr float kBallSpeed    = 9.0f;     // target speed - a DVD-logo ball never slows
    constexpr float kRestitution  = 0.98f;    // ~elastic collision
    constexpr float kAngularDamp  = 0.995f;
    constexpr float kEdgeMargin   = 8.0f;     // visible gap between the ball's arena and the window edge

    enum class Phase { WalkIn, Dialogue, Credits };

    inline float Len(float x, float y) { return std::sqrt(x * x + y * y); }

    // The convex-polygon rigid body + SAT + impulse solver lives in
    // Physics (Physics::Body / SatPolyPoly / ResolveBodies) - this scene
    // just sets up the bodies and runs the loop.

    class EndingState : public GameState {
    private:
        Font* creditsFont;
        Font* hintFont;
        Font* nameFont;
        Font* dialogFont;
        IDirect3DTexture9* whiteTex;    // 1x1, for the dim band + dialogue panel
        IDirect3DTexture9* denjiTex;    // Assets/Characters/denji.png
        IDirect3DTexture9* ballTex;     // Assets/Characters/football.png

        FrameTimer timer;              // Lecture 5: consistent timing across PCs

        Physics::Body ball;
        Physics::Body wall[4];         // top, bottom, left, right screen edges

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

        void BuildArena() {
            // The ball lives inside the window minus a small margin; each wall
            // is a fat static box whose inner face sits on that boundary.
            const float L = kEdgeMargin, R = 1280.0f - kEdgeMargin;
            const float T = kEdgeMargin, B = 720.0f - kEdgeMargin;
            const float t = 80.0f;   // wall thickness (off-screen)
            Physics::MakeBox(wall[0], 640.0f, T - t, 720.0f, t);  // top
            Physics::MakeBox(wall[1], 640.0f, B + t, 720.0f, t);  // bottom
            Physics::MakeBox(wall[2], L - t, 360.0f, t, 420.0f);  // left
            Physics::MakeBox(wall[3], R + t, 360.0f, t, 420.0f);  // right
        }

        void EnterCredits(GameContext& context) {
            phase = Phase::Credits;
            creditsScroll = 0.0f;
            BuildArena();
            Physics::MakePolygon(ball, 430.0f, 250.0f, kBallRadius, 8, 1.0f);   // octagon "ball"
            ball.vel = D3DXVECTOR2(kBallSpeed * 0.80f, kBallSpeed * 0.60f);     // DVD-style diagonal
            ball.angVel = 0.02f;
            if (context.sound != NULL) context.sound->PlaySfx("levelcomplete");
        }

        // One 1/60 s update of the bouncing ball.
        void StepBall(GameContext& context) {
            // --- Newtonian integration (Lecture 6: pos += vel) -----------
            ball.pos.x += ball.vel.x;
            ball.pos.y += ball.vel.y;
            ball.angle += ball.angVel;
            ball.angVel *= kAngularDamp;

            D3DXVECTOR2 bw[8];
            Physics::BodyWorldVertices(ball, bw);

            // --- SAT against each screen-edge wall, then resolve ----------
            for (int w = 0; w < 4; ++w) {
                D3DXVECTOR2 ww[8];
                Physics::BodyWorldVertices(wall[w], ww);
                D3DXVECTOR2 n; float depth;
                if (Physics::SatPolyPoly(bw, ball.count, ww, wall[w].count,
                                         ball.pos, wall[w].pos, n, depth)) {
                    Physics::ResolveBodies(ball, wall[w], n, depth, kRestitution, bw, ball.count);
                    Physics::BodyWorldVertices(ball, bw);   // ball moved - refresh for the next wall
                    PlayBounceSfx(context);
                }
            }

            // --- keep the DVD ball lively: spring its speed back to target
            //     so restitution loss (or a lucky corner) can't kill/rocket it.
            const float s = Len(ball.vel.x, ball.vel.y);
            if (s > 0.01f) {
                float k = 1.0f + (kBallSpeed - s) * 0.02f;
                if (k < 0.9f) k = 0.9f; else if (k > 1.1f) k = 1.1f;
                ball.vel.x *= k; ball.vel.y *= k;
            }
        }

        void StepOnce(GameContext& context) {
            if (bounceSfxCooldown > 0.0f) bounceSfxCooldown -= 1.0f / kUpdateFps;

            if (phase == Phase::WalkIn) {
                ++walkInAccum;
                float t = (float)walkInAccum / (float)kWalkInUpdates;
                if (t > 1.0f) t = 1.0f;
                denjiY = kDenjiInY + (kDenjiMeetY - kDenjiInY) * t;   // linear interpolation
                if (t >= 1.0f) phase = Phase::Dialogue;
            }
            else if (phase == Phase::Credits) {
                StepBall(context);
                creditsScroll += kCreditsScrollSpeed;
                const float rollHeight = kCreditsStartY + kCreditsLineCount * kCreditsLineGap + 80.0f;
                if (creditsScroll > rollHeight) creditsScroll = 0.0f;   // loop
            }

            // Pochi's walk sheet ticks over during the cutscene (Lecture 5).
            if (phase != Phase::Credits && context.pochi != NULL && ++pochiAnimAccum >= 12) {
                pochiAnimAccum = 0;
                context.pochi->NextFrame();
            }
        }

    public:
        EndingState()
            : creditsFont(NULL), hintFont(NULL), nameFont(NULL), dialogFont(NULL),
              whiteTex(NULL), denjiTex(NULL), ballTex(NULL),
              fWasDown(true), enterWasDown(true), escWasDown(true), pochiAnimAccum(0),
              phase(Phase::WalkIn), dialogIndex(0), walkInAccum(0), denjiY(kDenjiInY),
              creditsScroll(0.0f), bounceSfxCooldown(0.0f) {
            ball = Physics::Body{};
            for (int i = 0; i < 4; ++i) wall[i] = Physics::Body{};
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

            timer.Init(kUpdateFps);   // Lecture 5

            phase = Phase::WalkIn;
            dialogIndex = 0;
            walkInAccum = 0;
            denjiY = kDenjiInY;
            pochiAnimAccum = 0;
            creditsScroll = 0.0f;

            BuildArena();
            Physics::MakePolygon(ball, 430.0f, 250.0f, kBallRadius, 8, 1.0f);

            fWasDown = enterWasDown = escWasDown = true;
            bounceSfxCooldown = 0.0f;
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (phase == Phase::WalkIn) {
                if (JustPressed(k, DIK_F, fWasDown) || JustPressed(k, DIK_RETURN, enterWasDown)) {
                    walkInAccum = kWalkInUpdates;   // snap Denji to the meeting spot
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

            // Phase::Credits - just the ball; Enter / Esc leaves.
            if (JustPressed(k, DIK_RETURN, enterWasDown) || JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.ClearAndPush(CreateMainMenuState());
            }
        }

        void Update(GameContext& context, GameStateManager&) override {
            int frames = timer.FramesToUpdate();   // Lecture 5
            if (frames > 4) frames = 4;
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

            // Auto-scrolling credit roll up the middle.
            if (creditsFont != NULL) {
                for (int i = 0; i < kCreditsLineCount; ++i) {
                    const char* line = kCreditsLines[i];
                    if (line == NULL || line[0] == '\0') continue;
                    const float y = kCreditsStartY + i * kCreditsLineGap - creditsScroll;
                    if (y < -kCreditsLineGap || y > 720.0f) continue;
                    const float x = kCreditsCenterX - (float)strlen(line) * 6.3f;   // rough centre
                    creditsFont->Draw(line, x + 1.0f, y + 1.0f, D3DCOLOR_XRGB(0, 0, 0), b);   // shadow
                    creditsFont->Draw(line, x, y, D3DCOLOR_XRGB(232, 228, 220), b);
                }
            }

            // The football, spun to ball.angle, over the top of everything.
            if (ballTex != NULL) {
                const float d = 2.0f * kBallRadius;
                ui::DrawTextureRotated(b, ballTex, kBallTexW, kBallTexH,
                                       ball.pos.x, ball.pos.y, d, d, ball.angle);
            }

            if (hintFont != NULL)
                hintFont->Draw("Press Enter / Esc to return to main menu",
                               500.0f, 26.0f, D3DCOLOR_XRGB(110, 110, 118), b);
        }

        void Render(GameContext& context) override {
            if (phase == Phase::Credits) RenderCredits(context);
            else RenderCutscene(context);
            // The "CHEAT MODE" indicator is drawn globally in Main.cpp.
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

} // namespace

std::unique_ptr<GameState> CreateEndingState() {
    return std::make_unique<EndingState>();
}
