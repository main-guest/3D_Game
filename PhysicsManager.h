#pragma once
#include "DxLib.h"

class CollisionWorld;

class PhysicsManager
{
public:
	void Init(CollisionWorld* world);

	bool MoveAndCheckCollision(VECTOR& pos, VECTOR& velocity, float radius, bool& isGround, float dt);

private:
	CollisionWorld* world = nullptr;

	float gravity = 1200.0f;
	float groundHeight = 0.0f;
};