#pragma once
#include <vector>
#include "DxLib.h"

class CollisionWorld;
class Player;
class Enemy;

class PhysicsManager
{
public:
	void Init(CollisionWorld* world);

	void MoveCharacter(VECTOR& pos, VECTOR& velocity, float radius, bool& isGround, float dt);

	void ResolveCharacterCollision(VECTOR& pos, VECTOR& velocity, float radius, const std::vector<VECTOR>& others);

private:
	CollisionWorld* world = nullptr;

	float gravity = 1200.0f;
	float groundHeight = 0.0f;
};