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

	bool IsDead() const { return isDead; }

	void Damage(int power);
private:
	VECTOR velocity;

	// ==== ステータス ====
	int hp = 100;

	bool isDead = false;

	// ==== AI ====
	float searchRange = 500.0f;

	// ==== ジャンプ ====
	bool jumpRequest = false;

	const float jumpStartFrame = 50.0f;
};