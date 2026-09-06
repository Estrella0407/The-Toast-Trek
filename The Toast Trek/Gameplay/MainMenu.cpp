#include "MainMenu.h"

MainMenu::MainMenu(IDirect3DDevice9* d3dDevice, Sprite* sharedPochi) {
    pochi = sharedPochi;
    animCounter = 0;
    animDelay = 8;

    // Reposition Pochi for the menu center pose
    if (pochi != nullptr) {
        pochi->SetPosition(615.0f, 320.0f);
        pochi->CropToFrame(0);
    }

    titleFont = new Font(d3dDevice, 0.0f, 180.0f, 1280, 80, 48, "Arial");
    promptFont = new Font(d3dDevice, 0.0f, 480.0f, 1280, 60, 24, "Arial");
}

MainMenu::~MainMenu() {
    delete titleFont;
    delete promptFont;
    titleFont = nullptr;
    promptFont = nullptr;
    pochi = nullptr;
}

void MainMenu::Update() {
    if (pochi != nullptr) {
        animCounter++;
        if (animCounter >= animDelay) {
            animCounter = 0;
            pochi->NextFrame();
        }
    }
}

void MainMenu::Draw(LPD3DXSPRITE brush) {
    if (pochi != nullptr && brush != nullptr) {
        pochi->Draw(brush);
    }

    if (brush != nullptr) {
        // Sprite::Draw leaves its own scale/translate matrix on the brush -
        // reset to identity or the text below inherits it and vanishes.
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        brush->SetTransform(&identity);
    }

    // Text goes through the SHARED brush (not a NULL sprite) so it doesn't
    // disturb the open batch for anything drawn afterwards.
    if (titleFont != nullptr) {
        titleFont->Draw("THE TOAST TREK", D3DCOLOR_XRGB(35, 35, 35), brush);
    }
    if (promptFont != nullptr) {
        promptFont->Draw("PRESS ENTER TO CONTINUE", D3DCOLOR_XRGB(70, 70, 70), brush);
    }
}
