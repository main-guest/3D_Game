#pragma once
#include <vector>
#include "DxLib.h"
#include "CharacterBase.h"

class CollisionWorld;

class PhysicsManager
{
public:
	void Init(CollisionWorld* world);

	bool MoveCharacter(VECTOR& pos, VECTOR& velocity, CharacterState& state, float radius, float dt);

	// ===== Capsule Collision ====
	void ResolveCharacterCollision(VECTOR& pos, VECTOR& velocity, float radius, float height, const std::vector<VECTOR>& others, float otherHeight);

private:
	float Dot(VECTOR a, VECTOR b);
	VECTOR Sub(VECTOR a, VECTOR b);
	VECTOR Add(VECTOR a, VECTOR b);
	VECTOR Mul(VECTOR v, float f);

	VECTOR ClosestPointOnSegment(VECTOR a, VECTOR b, VECTOR p);

private:
	CollisionWorld* world = nullptr;

	float gravity = 1200.0f;
};