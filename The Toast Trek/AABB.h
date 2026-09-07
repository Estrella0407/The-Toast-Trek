#pragma once

// Axis-aligned bounding box in screen space. Shared by PhysicsManager,
// GameObject / RigidBody and the collision helpers.
struct AABB {
    float left, top, right, bottom;
};
