#pragma once
#include "CharacterBase.h"

class PhysicsManager;
class CollisionWorld;

class Player : public CharacterBase
{
public:
	void Init(CollisionWorld* w);
	void Update(float dt, float cameraAngle, PhysicsManager& physics);

private:
	// “à•”ˆ—
	void UpdateInput(float dt, float cameraAngle);
	void UpdateState();

private:
	VECTOR velocity = VGet(0, 0, 0);

	// ===== “ü—Í =====
	int oldMouse = 0;

	// ===== ó‘Ô =====
	bool prevGround = true;

	bool jumpRequest=false;

	const float jumpStartFrame = 55.0f;

	// ŠO•”QÆ
	CollisionWorld* world = nullptr;
};
