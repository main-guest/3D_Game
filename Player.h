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

	void UpdateAttackPos();

	void CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies);

	void SetPos(const VECTOR& p) { pos = p; }

	const VECTOR& GetVelocity() const;
	VECTOR& GetVelocity();

	void SetVelocity(const VECTOR& v);

	VECTOR GetForward() const;

	void DebugDraw();
	void DrawCapsuleDebug(std::vector<std::unique_ptr<Enemy>>& enemies);

private:
	// 内部処理
	std::vector<Enemy*> FindGunTargets(std::vector<std::unique_ptr<Enemy>>& enemies);

	void UpdateInput(float dt, float cameraAngle);
	void UpdateState();

private:
	// ===== 入力 =====
	int oldMouse = 0;

	// ===== 状態 =====
	bool isDash = false;

	bool prevGround = true;

	bool jumpRequest = false;

	bool dashJump = false;

	bool attackActive = false;
	bool attackHit = false;

	// ===== 移動速度 =====
	float walkSpeed = 220.0f;
	float dashSpeed = 400.0f;

	VECTOR dashJumpVelocity = VGet(0, 0, 0);

	// ===== 攻撃当たり判定 =====
	VECTOR attackPos;

	const float jumpStartFrame = 55.0f;
	const float attackStartFrame = 50.0f;
	const float attackEndFrame = 100.0f;

	// 外部参照
	CollisionWorld* world = nullptr;

	// ===== 武器 =====
	Weapon weapon1;
	Weapon weapon2;

	// ===== 武器データ =====
	WeaponData unarmedData;
	WeaponData weapon1Data;
	WeaponData weapon2Data;

	// ===== 現在装備 =====
	EquipState equipState;

	// ===== 現在装備中データ =====
	const WeaponData* currentWeaponData = &unarmedData;

	//ガンターゲット
	std::vector<Enemy*> gunTargets;

	float gunTimer = 0.0f;
	bool gunWaitingDamage = false;
};
