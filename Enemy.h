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
	void Render();

	void Damage(int power);

	void DebugDraw();

	VECTOR GetPos() const { return pos; }

	float GetHeight() const { return height; }

	bool IsDead() const { return isDead; }

private:
	void UpdateState();

private:
	// ==== ステータス ====
	int hp = 100;

	VECTOR pos;
	VECTOR velocity;

	float characterAngle = 0.0f;

	// ===== 状態 =====
	bool prevGround = true;

	bool jumpRequest = false;

	bool isHit = false;

	bool isDead = false;

	// ==== 怯み（ヒットストップ）====
	float hitStopTimer = 0.0f;
	const float hitStopDuration = 2.0f;

	// ==== AI ====
	float searchRange = 500.0f;

	const float jumpStartFrame = 50.0f;
};