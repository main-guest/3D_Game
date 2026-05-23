#pragma once
#include <vector>
#include <memory>
#include "CharacterBase.h"
#include "Weapon.h"

class PhysicsManager;
class CollisionWorld;
class Enemy;

enum class EquipState
{
	Unarmed,
	Weapon1,
	Weapon2
};

class Player : public CharacterBase
{
public:
	void Init(CollisionWorld* w);
	void Update(float dt, float cameraAngle, PhysicsManager& physics);
	void Draw();

	void EquipWeapon1();
	void EquipWeapon2();
	void Unequip();

	void CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies);

	void SetPos(const VECTOR& p) { pos = p; }

	const VECTOR& GetVelocity() const;
	VECTOR& GetVelocity();

	void SetVelocity(const VECTOR& v);

	VECTOR GetForward() const;

	void DebugDraw();
	void DrawCapsuleDebug(std::vector<std::unique_ptr<Enemy>>& enemies);

private:
	// “à•”ˆ—
	void UpdateInput(float dt, float cameraAngle);
	void UpdateState();

private:
	VECTOR velocity = VGet(0, 0, 0);

	// ===== “ü—Í =====
	int oldMouse = 0;

	// ===== ó‘Ô =====
	bool isDash = false;

	bool prevGround = true;

	bool jumpRequest=false;

	bool attackActive = false;
	bool attackHit = false;

	// ===== ˆÚ“®‘¬“x =====
	float walkSpeed = 220.0f;
	float dashSpeed = 400.0f;

	// ===== UŒ‚“–‚½‚è”»’è =====
	VECTOR attackPos;

	float attackRadius = 40.0f;

	const float jumpStartFrame = 55.0f;
	const float attackStartFrame = 50.0f;
	const float attackEndFrame = 100.0f;

	// ŠO•”QÆ
	CollisionWorld* world = nullptr;
	
	// ===== •Ší =====
	Weapon weapon1;
	Weapon weapon2;

	// ===== Œ»İ‘•”õ =====
	EquipState equipState;

	int weponHandle;
};
