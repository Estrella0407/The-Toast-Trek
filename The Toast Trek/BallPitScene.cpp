#include "BallPitScene.h"
#include "GameStateManager.h"
#include "Ball.h"
#include "PhysicsManager.h"
#include "Font.h"
#include "UiFill.h"
#include "SoundManager.h"
#include <dinput.h>
#include <cmath>
#include <cstdio>
#include <memory>

namespace {

// Arena rectangle (leaves a strip at the top for the HUD text)
constexpr float kL = 70.0f, kT = 150.0f, kR = 1210.0f, kB = 690.0f;

// One fixed simulation tick (the game loop is frame-locked ~60 fps, so
// speeds/forces here are "per tick", not per second)
constexpr float kDt          = 1.0f;

constexpr float kThrust      = 0.85f;   // steering force while a key is held (a = F / mass)
constexpr float kMaxSpeed    = 12.0f;   // velocity cap, px per tick
constexpr float kDrag        = 0.990f;  // per-tick velocity damping (friction)
constexpr float kWallBounce  = 0.92f;   // restitution against the walls
constexpr float kBallBounce  = 0.98f;   // restitution ball-to-ball

constexpr float kMassHeavy   = 3.0f;
constexpr float kMassLight   = 1.0f;

// A slowly-rotating rectangular bumper in the arena centre. Both balls test
// against it with the Separating Axis Theorem, so they bounce off its tilted
// faces at the surface angle - not along the screen axes.
constexpr float kBumperHalfW  = 135.0f;
constexpr float kBumperHalfH  = 15.0f;
constexpr float kBumperSpin   = 0.0075f;  // radians per tick
constexpr float kBumperBounce = 0.85f;
const D3DCOLOR  kBumperColour = D3DCOLOR_ARGB(255, 200, 90, 90);

const D3DCOLOR kTintA = D3DCOLOR_XRGB(150, 200, 255);   // heavy ball (WASD)
const D3DCOLOR kTintB = D3DCOLOR_XRGB(255, 190, 120);   // light ball (arrows)
const D3DCOLOR kArenaFill = D3DCOLOR_ARGB(255, 18, 20, 28);
const D3DCOLOR kArenaEdge = D3DCOLOR_ARGB(255, 120, 130, 160);

float Length(const D3DXVECTOR2& v) { return sqrtf(v.x * v.x + v.y * v.y); }

D3DXVECTOR2 ReadDir(const BYTE* keys, int up, int down, int left, int right) {
    D3DXVECTOR2 d(0.0f, 0.0f);
    if (GameScene::IsKeyDown(keys, left))  d.x -= 1.0f;
    if (GameScene::IsKeyDown(keys, right)) d.x += 1.0f;
    if (GameScene::IsKeyDown(keys, up))    d.y -= 1.0f;
    if (GameScene::IsKeyDown(keys, down))  d.y += 1.0f;
    const float len = Length(d);
    if (len > 0.0f) d /= len;               // keep diagonals the same speed
    return d;
}

class BallPitScene : public GameScene {
public:
    ~BallPitScene() override {
        if (ballTex)  ballTex->Release();
        delete hudFont;
    }

    void Initialize(GameContext& context) override {
        ballTex  = ui::LoadTexture(context.device, "Assets/Characters/football.png", 1330, 1183);
        hudFont  = new Font(context.device, 0.0f, 0.0f, 1280, 40, 20, "Arial");

        // Heavy + big vs light + small, so the mass term shows in both the
        // steering (F = m a) and the collision response
        a = std::make_unique<Ball>(ballTex, 360.0f, 420.0f, 54.0f, kMassHeavy, kMaxSpeed, kDrag);
        b = std::make_unique<Ball>(ballTex, 900.0f, 420.0f, 32.0f, kMassLight, kMaxSpeed, kDrag);

        bumperCentre = D3DXVECTOR2((kL + kR) * 0.5f, (kT + kB) * 0.5f);
        bumperAngle  = 0.35f;

        // Whatever opened this screen may still be held
        escWasDown = GameScene::IsKeyDown(context.keys, DIK_ESCAPE);
        eWasDown   = GameScene::IsKeyDown(context.keys, DIK_E);
    }

    void HandleInput(GameContext& context, GameStateManager& manager) override {
        if (JustPressed(context.keys, DIK_ESCAPE, escWasDown) ||
            JustPressed(context.keys, DIK_E, eWasDown)) {
            manager.Pop();               // back to the menu
        }
    }

    void Update(GameContext& context, GameStateManager&) override {
        const BYTE* k = context.keys;

        // Input -> force, then integrate the rigid body one tick
        a->ApplyThrust(ReadDir(k, DIK_W, DIK_S, DIK_A, DIK_D),            kThrust);
        b->ApplyThrust(ReadDir(k, DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT), kThrust);

        a->Step(kDt);
        b->Step(kDt);

        BounceWalls(*a);
        BounceWalls(*b);

        // Rotating bumper: SAT (circle vs oriented box) for each ball
        bumperAngle += kBumperSpin;
        BounceBumper(*a);
        BounceBumper(*b);

        // Ball-to-ball: non-axis-aligned elastic resolution
        const bool overlapping = PhysicsManager::CirclesOverlap(
            a->GetPosition(), a->Radius(), b->GetPosition(), b->Radius());
        if (overlapping) {
            D3DXVECTOR2 pa = a->GetPosition(), pb = b->GetPosition();
            D3DXVECTOR2 va = a->Body().velocity, vb = b->Body().velocity;
            PhysicsManager::ResolveCircleCollision(
                pa, va, a->Mass(), a->Radius(),
                pb, vb, b->Mass(), b->Radius(), kBallBounce);
            a->SetPosition(pa.x, pa.y); a->Body().velocity = va;
            b->SetPosition(pb.x, pb.y); b->Body().velocity = vb;

            if (!wasOverlapping && context.sound) context.sound->PlaySfx("click");
        }
        wasOverlapping = overlapping;
    }

    void Render(GameContext& context) override {
        LPD3DXSPRITE brush = context.spriteBrush;

        // Arena
        ui::FillRect(brush, kL, kT, kR - kL, kB - kT, kArenaFill);
        const float bw = 3.0f;
        ui::FillRect(brush, kL, kT, kR - kL, bw, kArenaEdge);
        ui::FillRect(brush, kL, kB - bw, kR - kL, bw, kArenaEdge);
        ui::FillRect(brush, kL, kT, bw, kB - kT, kArenaEdge);
        ui::FillRect(brush, kR - bw, kT, bw, kB - kT, kArenaEdge);

        // Rotating bumper (behind the balls) - a thick line along its long axis
        const float ca = cosf(bumperAngle), sa = sinf(bumperAngle);
        ui::FillLine(brush,
                     bumperCentre.x - ca * kBumperHalfW, bumperCentre.y - sa * kBumperHalfW,
                     bumperCentre.x + ca * kBumperHalfW, bumperCentre.y + sa * kBumperHalfW,
                     kBumperHalfH * 2.0f, kBumperColour);

        a->Render(brush, kTintA);
        b->Render(brush, kTintB);

        hudFont->Draw("PHYSICS DEMO - ball/ball uses impulse along the contact normal; ball/bumper uses SAT",
                      80.0f, 34.0f, D3DCOLOR_XRGB(230, 230, 235), brush);
        hudFont->Draw("WASD: heavy ball      Arrow keys: light ball      Esc / E: back to menu",
                      80.0f, 70.0f, D3DCOLOR_XRGB(170, 175, 185), brush);

        char buf[96];
        sprintf_s(buf, "heavy  mass %.0f   speed %.1f", a->Mass(), Length(a->Body().velocity));
        hudFont->Draw(buf, 80.0f, 104.0f, kTintA, brush);
        sprintf_s(buf, "light  mass %.0f   speed %.1f", b->Mass(), Length(b->Body().velocity));
        hudFont->Draw(buf, 470.0f, 104.0f, kTintB, brush);
    }

    D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(24, 26, 34); }

private:
    void BounceWalls(Ball& ball) {
        D3DXVECTOR2 p = ball.GetPosition();
        D3DXVECTOR2 v = ball.Body().velocity;
        const float r = ball.Radius();
        if (p.x - r < kL) { p.x = kL + r; v.x = -v.x * kWallBounce; }
        if (p.x + r > kR) { p.x = kR - r; v.x = -v.x * kWallBounce; }
        if (p.y - r < kT) { p.y = kT + r; v.y = -v.y * kWallBounce; }
        if (p.y + r > kB) { p.y = kB - r; v.y = -v.y * kWallBounce; }
        ball.SetPosition(p.x, p.y);
        ball.Body().velocity = v;
    }

    // SAT vs the oriented bumper: detect, push out along the minimum
    // translation vector, then reflect the velocity about that same normal.
    void BounceBumper(Ball& ball) {
        D3DXVECTOR2 corners[4];
        PhysicsManager::BoxCorners(bumperCentre, kBumperHalfW, kBumperHalfH,
                                   bumperAngle, corners);

        const D3DXVECTOR2 c = ball.GetPosition();
        D3DXVECTOR2 axis;
        float depth = 0.0f;
        if (!PhysicsManager::SatCircleVsPolygon(c, ball.Radius(), corners, 4, &axis, &depth))
            return;

        // Point the MTV axis from the bumper toward the ball
        const D3DXVECTOR2 away = c - bumperCentre;
        if (away.x * axis.x + away.y * axis.y < 0.0f) axis = -axis;

        ball.SetPosition(c.x + axis.x * depth, c.y + axis.y * depth);

        D3DXVECTOR2 v = ball.Body().velocity;
        const float vn = v.x * axis.x + v.y * axis.y;
        if (vn < 0.0f) {                       // moving into the bumper
            v -= axis * ((1.0f + kBumperBounce) * vn);
            ball.Body().velocity = v;
        }
    }

    std::unique_ptr<Ball> a;   // WASD, heavy
    std::unique_ptr<Ball> b;   // arrows, light
    D3DXVECTOR2 bumperCentre { 0.0f, 0.0f };
    float bumperAngle = 0.0f;
    IDirect3DTexture9* ballTex = nullptr;
    Font* hudFont = nullptr;
    bool escWasDown = false;
    bool eWasDown = false;
    bool wasOverlapping = false;
};

} // namespace

std::unique_ptr<GameScene> CreateBallPitScene() {
    return std::make_unique<BallPitScene>();
}
