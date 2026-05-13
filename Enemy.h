#pragma once
#include "CharacterBase.h"

class PhysicsManager;

class Enemy :public CharacterBase
{
public:
	Enemy();
	virtual ~Enemy();

	void Init(VECTOR startPos);

	void Update(float dt, VECTOR playerPos, PhysicsManager& physics);

private:
	VECTOR velocity;

	// ==== AI ====
	float searchRange = 500.0f;

	// ==== ƒWƒƒƒ“ƒv ====
	bool jumpRequest = false;

	const float jumpStartFrame = 50.0f;
};