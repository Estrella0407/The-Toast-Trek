#include "GameScene.h"

// GameScene is an abstract interface - the concrete behaviour lives in the
// scene subclasses. Defining the (virtual) destructor out of line here gives
// the class a single translation unit to anchor its vtable and type info,
// instead of emitting them in every file that includes GameScene.h.
GameScene::~GameScene() = default;
