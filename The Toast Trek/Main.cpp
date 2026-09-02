//	Ask the compiler to include minimal header files for our program
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <fstream>

//	Include the Direct3D 9 library
#include <d3d9.h>
#include <d3dx9.h>
#include<string>
#include <dinput.h>
#include "Sprite.h"
#include "Font.h"
#include "Line.h"
#include "FrameTimer.h"
#include "Physics.h"
#include "TileMap.h"
#include "GameState.h"
#include "Battlefield.h"
#include "Heart.h"
#include "SoundManage.h"
#include "Inventory.h"
#include "Pochi.h"
#include "Cheats.h"
#include "UiFill.h"

//	--------------------------------------------------------------------
//	Window handle
HWND g_hWnd = NULL;
WNDCLASS wndClass;
MSG msg;
IDirect3DDevice9* d3dDevice;
D3DPRESENT_PARAMETERS d3dPP;

LPDIRECT3DTEXTURE9 texture = NULL;
LPD3DXSPRITE spriteBrush = NULL;
LPDIRECTINPUT8 dInput;
LPDIRECTINPUTDEVICE8  dInputKeyboardDevice;
LPDIRECTINPUTDEVICE8 dInputMouseDevice;
DIMOUSESTATE mouseState;
D3DXMATRIX spriteMatrix;

FrameTimer* gameTimer = new FrameTimer();

//	Key input buffer
BYTE  diKeys[256];

TileMap* forestMap = NULL;
TileMap* mazeMap = NULL;
TileMap* ruinsExteriorMap = NULL;
TileMap* ruinsInteriorMap = NULL;
TileMap* tarumtMap = NULL;
Sprite* pochi = NULL;
GameContext gameContext = {};
GameStateManager* gameStates = NULL;
Inventory* playerInventory = NULL;
Pochi* playerStats = NULL;
Battlefield* battlefield;
SoundManage* g_soundManager = nullptr;

// Global "CHEAT MODE" overlay
Font* g_cheatFont = NULL;
IDirect3DTexture9* g_cheatPlateTex = NULL;

int spriteVelocity = 5;

using namespace std;

//	--------------------------------------------------------------------
//	Window Procedure, for event handling
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//	The message is post when we destroy the window
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// When a key is pressed
	case WM_KEYDOWN:
		switch (wParam)
		{

		case 'F':
			// Toggle fullscreen
			break;

		case VK_SPACE:

			break;

		// ESC is a normal gameplay key (the tab menu uses it to close);
		// quitting is via the window's close button
		}
		break;

		//	Default handling for other messages
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//--------------------------------------------------------------------

void CreateMyWindow()
{
	/*
		Step 1
		Define and Register a Window.
	*/

	//	Set all members in wndClass to 0
	ZeroMemory(&wndClass, sizeof(wndClass));

	//	Filling wndClass. You are to refer to MSDN for each of the members details
	//	these are the fundamental structure members to be specify, in order to create your window
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hInstance = GetModuleHandle(NULL); // Replaced hInstance to reduced the passing of hInstance as a parameter
	wndClass.lpfnWndProc = WindowProcedure;
	wndClass.lpszClassName = "My Window";
	wndClass.style = CS_HREDRAW | CS_VREDRAW;

	//	Register the window
	RegisterClass(&wndClass);

	/*
		Step 2
		Create the Window.
	*/

	g_hWnd = CreateWindowEx(0, wndClass.lpszClassName, "Let's Gooooo", WS_OVERLAPPEDWINDOW, 0, 100, 1280, 720, NULL, NULL, wndClass.hInstance, NULL);
	ShowWindow(g_hWnd, 1);

	ZeroMemory(&msg, sizeof(msg));
}

void DestroyMyWindow()
{
	// Step 1: Destroy the active window instance
	if (g_hWnd != NULL)
	{
		DestroyWindow(g_hWnd);
		g_hWnd = NULL; // Prevent using a dangling handle
	}

	// Step 2: Unregister the class so the name "My Window" can be reused
	// use the same instance handle used during registration
	HINSTANCE hInstance = GetModuleHandle(NULL);
	UnregisterClass("My Window", hInstance);

	// Step 3: Optional clear of your global structure
	ZeroMemory(&wndClass, sizeof(wndClass));
}

void CreateDirectX()
{
	// Define Direct3D 9
	// instantiate the Direct3D 9 object
	IDirect3D9* direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!direct3D9) {
		MessageBox(NULL, "Direct3DCreate9 failed", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Define how the screen presents
	ZeroMemory(&d3dPP, sizeof(d3dPP));

	// Refer to Direct3D 9 documentation for the meaning of the members
	d3dPP.Windowed = true;						// run in windowed view mode
	d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;	// discard the back buffer contents after presenting
	d3dPP.BackBufferFormat = D3DFMT_X8R8G8B8;	// Back buffer format
	d3dPP.BackBufferCount = 1;					// number of back buffers
	d3dPP.BackBufferWidth = 1280;
	d3dPP.BackBufferHeight = 720;
	d3dPP.hDeviceWindow = g_hWnd;				// Handle to the window associated with the device

	// Use the Direct3D 9 object to call the CreateDevice() funcition
	// create a Direct3D 9 device (returns either it succeeded or failed).									How it should look (declared above)
	HRESULT hr = direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dPP, &d3dDevice); // the address of the pointer

	// To Do: Cout out the message to indicate the failure
	// read why CreateDevice() failed
	if (FAILED(hr)) {
		MessageBox(NULL, "CreateDevice failed", "Error", MB_OK | MB_ICONERROR);
		d3dDevice = NULL;
	}
}

void CreateDirectInput()
{
	// Create the Direct Input object
	HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), 0x0800, IID_IDirectInput8, (void**)&dInput, NULL);

	// Create the keyboard device
	hr = dInput->CreateDevice(GUID_SysKeyboard, &dInputKeyboardDevice, NULL);

	// Set the input data format
	dInputKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);

	// Set the cooperative level
	dInputKeyboardDevice->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	/*---
		For buffered data
		// Define buffer for input
		const int DEVICE_BUFFER_SIZE = 4;
		DIDEVICEOBJECTDATA deviceBuffer[DEVICE_BUFFER_SIZE];

		// Set the event buffer / properties
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = DEVICE_BUFFER_SIZE;

		hr = dInputKeyboardDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	--*/

	// Acquire the device
	dInputKeyboardDevice->Acquire();

	// Create the mouse device
	hr = dInput->CreateDevice(GUID_SysMouse, &dInputMouseDevice, NULL);

	// Set the input data format
	dInputMouseDevice->SetDataFormat(&c_dfDIMouse);

	// Set the cooperative level
	dInputMouseDevice->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	// Acquire the device
	dInputMouseDevice->Acquire();
}

void CreateSprite()
{
	HRESULT hr = D3DXCreateSprite(d3dDevice, &spriteBrush);

	// Wide rect so the banner still renders when drawn far to the right
	// (Font::Draw(x,y,...) inverts its rect once x passes the width)
	g_cheatFont = new Font(d3dDevice, 0.0f, 0.0f, 1600, 30, 18, "Arial");
	g_cheatPlateTex = ui::MakeWhiteTexture(d3dDevice);

	forestMap = new TileMap(d3dDevice, "Assets/TileMap/Forest.tmx", "Assets/TileMap/");
	forestMap->SetSolidLayers({ "Tree", "Rock" });

	mazeMap = new TileMap(d3dDevice, "Assets/TileMap/Maze.tmx", "Assets/TileMap/");
	mazeMap->SetSolidLayers({ "Maze" });

	ruinsExteriorMap = new TileMap(d3dDevice, "Assets/TileMap/Ruined_Temple_Exterior.tmx", "Assets/TileMap/");
	ruinsExteriorMap->SetSolidLayers({ "Tree", "House", "Bricks", "Statues", "Columns" });
	ruinsExteriorMap->SetWalkableLayers({ "Ground", "Grass", "Spots", "Grass_details", "Site", "House_platform" });

	ruinsInteriorMap = new TileMap(d3dDevice, "Assets/TileMap/Ruined_Temple_Interior.tmx", "Assets/TileMap/");
	// Decorative_objects1/2 are floor clutter (pots, rubble, banner poles) -
	// they were blocking the middle of the path up to Maki, so only the real
	// walls + the dragon statue are solid
	ruinsInteriorMap->SetSolidLayers({ "Walls_back", "Walls_top", "Statue" });

	// Secret-boss area, reached from the forest's top-left
	tarumtMap = new TileMap(d3dDevice, "Assets/TileMap/Tarumt.tmx", "Assets/TileMap/");
	tarumtMap->SetSolidLayers({ "Tree", "Structure1", "Structure2", "Building" });

	pochi = new Sprite(d3dDevice, "Assets/Characters/Pochi.png", 250, 60, 5, 2, 10, 100.0f, 380.0f);
	if (pochi != nullptr) {
		pochi->CropToFrame(0);
		pochi->SetScale(2.0f);
	}

	// Sound\
	// Initialize() and every call are safe even with no audio files - missing sounds just no-op
	g_soundManager = new SoundManage();
	g_soundManager->Initialize();

	g_soundManager->LoadSound("click", "Assets/Sounds/click.wav");
	g_soundManager->LoadSound("gameover", "Assets/Sounds/gameover.wav");
	g_soundManager->LoadSound("levelcomplete", "Assets/Sounds/levelcomplete.wav");
	g_soundManager->LoadSound("background", "Assets/Sounds/background.wav", true);
	g_soundManager->LoadSound("battle", "Assets/Sounds/battle.wav", true);
	g_soundManager->LoadSound("attack", "Assets/Sounds/attack.wav");   // Pochi's FIGHT swing
	g_soundManager->LoadSound("hurt", "Assets/Sounds/hurt.ogg");       // Pochi takes damage

	g_soundManager->PlayMusic("background", 0.6f);
}

bool WindowIsRunning()
{
	/*
		Step 3
		Handling window messages
	*/
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		//	Receive a quit message
		if (msg.message == WM_QUIT)
		{
			return false;
			break;
		}
		// Translate the message 
		TranslateMessage(&msg);
		// Send message to your window procedure
		DispatchMessage(&msg);
	}

	return true;
}

void GetInput()
{
	// Get immediate Keyboard Data
	HRESULT hr = dInputKeyboardDevice->GetDeviceState(256, diKeys);
	if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
		dInputKeyboardDevice->Acquire();
		hr = dInputKeyboardDevice->GetDeviceState(256, diKeys);
	}
	if (FAILED(hr)) ZeroMemory(diKeys, sizeof(diKeys));

	// Get immediate Mouse Data
	hr = dInputMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

	if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
		dInputMouseDevice->Acquire();
		hr = dInputMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
	}

	// DIMOUSESTATE above is relative motion (deltas), not an absolute
	// position, so button hit-testing reads the cursor directly instead
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(g_hWnd, &cursor);
	RECT clientRect = {};
	GetClientRect(g_hWnd, &clientRect);
	const float clientWidth = (float)(clientRect.right - clientRect.left);
	const float clientHeight = (float)(clientRect.bottom - clientRect.top);
	gameContext.mouseX = clientWidth > 0.0f ? cursor.x * d3dPP.BackBufferWidth / clientWidth : 0.0f;
	gameContext.mouseY = clientHeight > 0.0f ? cursor.y * d3dPP.BackBufferHeight / clientHeight : 0.0f;
	gameContext.mouseLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void Render()
{
	// Update
	if (!d3dDevice) return;

	// Clear the back buffer (into a colour)
	D3DCOLOR clearColor = gameStates ? gameStates->ClearColor() : D3DCOLOR_XRGB(0, 0, 0);
	d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.0f, 0);

	// Begin the scene -> Unlocks the buffer for drawing
	d3dDevice->BeginScene();

	// Drawing
	// Clear and begin scene
	// Specify alpha blend will ensure that the sprite will render the background with alpha
	spriteBrush->Begin(D3DXSPRITE_ALPHABLEND);

	if (gameStates) gameStates->Render();

	// Global CHEAT MODE banner, upper-right, over every state
	if (Cheats::enabled && g_cheatFont != NULL) {
		const char* txt = "CHEAT MODE";
		const float pw = 118.0f, ph = 24.0f;
		const float px = 1280.0f - pw - 12.0f, py = 12.0f;
		if (g_cheatPlateTex != NULL) {
			ui::FillRect(spriteBrush, g_cheatPlateTex, px - 1.0f, py - 1.0f,
				pw + 2.0f, ph + 2.0f, ui::kPlateEdge);
			ui::FillRect(spriteBrush, g_cheatPlateTex, px, py, pw, ph, ui::kPlate);
		}
		g_cheatFont->Draw(txt, px + 15.0f, py + 4.0f, ui::kShadow, spriteBrush);
		g_cheatFont->Draw(txt, px + 14.0f, py + 3.0f, D3DCOLOR_XRGB(255, 120, 120), spriteBrush);
	}

	spriteBrush->End();

	// End the scene -> Locks the buffer for presenting
	d3dDevice->EndScene();

	// Present the back buffer to screen -> Swap the back buffer to the front buffer
	d3dDevice->Present(NULL, NULL, NULL, NULL);
}

void CleanupSprite()
{
	if (forestMap) { delete forestMap; forestMap = nullptr; }
	if (mazeMap) { delete mazeMap; mazeMap = nullptr; }
	if (ruinsExteriorMap) { delete ruinsExteriorMap; ruinsExteriorMap = nullptr; }
	if (ruinsInteriorMap) { delete ruinsInteriorMap; ruinsInteriorMap = nullptr; }
	if (tarumtMap) { delete tarumtMap; tarumtMap = nullptr; }
	if (pochi) { delete pochi;     pochi = nullptr; }

	if (g_cheatFont) { delete g_cheatFont; g_cheatFont = nullptr; }
	if (g_cheatPlateTex) { g_cheatPlateTex->Release(); g_cheatPlateTex = nullptr; }

	// Clean up sound manager
	if (g_soundManager) {
		g_soundManager->Shutdown();
		delete g_soundManager;
		g_soundManager = nullptr;
	}


	if (spriteBrush) {
		spriteBrush->Release();
		spriteBrush = NULL;
	}
}

void CleanupDirectX()
{
	// Release the device when exiting
	d3dDevice->Release();

	// Reset pointer to NULL, a good practice
	d3dDevice = NULL;
}

void CleanupDirectInput()
{
	// Release keyboard device
	dInputKeyboardDevice->Unacquire();
	dInputKeyboardDevice->Release();
	dInputKeyboardDevice = NULL;

	// Release mouse device
	dInputMouseDevice->Unacquire();
	dInputMouseDevice->Release();
	dInputMouseDevice = NULL;

	// Release DirectInput
	dInput->Release();
	dInput = NULL;
}

void CleanupWindow()
{
	// Free up the memory
	UnregisterClass(wndClass.lpszClassName, GetModuleHandle(NULL));
}

//--------------------------------------------------------------------
//	Use WinMain if you don't want the console
//  int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//	Window's structure
	CreateMyWindow();
	CreateDirectX();
	CreateDirectInput();
	CreateSprite();

	gameContext.device = d3dDevice;
	gameContext.spriteBrush = spriteBrush;
	gameContext.pochi = pochi;
	gameContext.forestMap = forestMap;
	gameContext.mazeMap = mazeMap;
	playerInventory = new Inventory();
	playerStats = new Pochi(1);
	gameContext.inventory = playerInventory;
	gameContext.playerStats = playerStats;
	gameContext.sound = g_soundManager;
	gameContext.ruinsExteriorMap = ruinsExteriorMap;
	gameContext.ruinsInteriorMap = ruinsInteriorMap;
	gameContext.tarumtMap = tarumtMap;
	gameContext.keys = diKeys;
	gameContext.moveSpeed = spriteVelocity;
	gameStates = new GameStateManager(gameContext);
	gameStates->Push(CreateMainMenuState());
	gameStates->ApplyPendingChanges();

	gameTimer->Init(10);

	while (WindowIsRunning())
	{
		// Deleting the outer loop of WIndowIsRunning() will cause the Game loop to be non-executable, 
		// as the window will not be able to process messages and will not be able to close properly
		// the outer loop is necessary to keep the window running and responsive to user input and system messages

		// GAME LOOP
		// One application loop delegates work to the state on top of the stack
		GetInput();

		// Developer cheat switch (F5)
		// Reads the same key buffer GetInput()
		Cheats::Update(diKeys);

		// Update Sound system
		if (g_soundManager) {
			g_soundManager->Update();
		}

		// Update Game States
		if (gameStates) {
			gameStates->HandleInput();
			gameStates->ApplyPendingChanges();
			gameStates->Update();
			gameStates->ApplyPendingChanges();
		}

		Render();
	}

	delete gameStates;
	gameStates = NULL;
	delete playerInventory;
	playerInventory = NULL;
	delete playerStats;
	playerStats = NULL;
	CleanupSprite();
	CleanupDirectInput();
	CleanupDirectX();
	CleanupWindow();

	return 0;
}

//--------------------------------------------------------------------
