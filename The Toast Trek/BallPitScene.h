#include <memory>
#include "GameScene.h"

// Physics sandbox reached from the main menu. Two balls of different mass
// and size - one driven with WASD, one with the arrow keys - bounce off the
// walls and off each other. The ball-to-ball bounce uses the non-axis-aligned
// elastic resolution in PhysicsManager. Esc returns to the menu.
// The concrete class is private to BallPitScene.cpp.
std::unique_ptr<GameScene> CreateBallPitScene();
