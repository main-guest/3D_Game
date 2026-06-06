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

	// 状態（統一管理）
	CharacterState state;

	// 装備
	void EquipWeapon1();
	void EquipWeapon2();
	void Unequip();

	// 戦闘
	void UpdateAttackPos();
	void CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies);
	void Damage(int power);

	void StartDodge();

	VECTOR GetForward() const;
	VECTOR GetCenterPos() const;

	void SetPos(const VECTOR& p) { pos = p; }

	const VECTOR& GetVelocity() const;
	VECTOR& GetVelocity();
	void SetVelocity(const VECTOR& v);


	void DebugDraw();
	void DrawCapsuleDebug(std::vector<std::unique_ptr<Enemy>>& enemies);

private:
	// 内部処理
	std::vector<Enemy*> FindGunTargets(std::vector<std::unique_ptr<Enemy>>& enemies);

	void UpdateInput(float dt, float cameraAngle);
	void UpdateState();

private:
	// ===== ステータス =====
	int hp = 100;

	// ===== 入力 =====
	int oldMouse = 0;

	bool shiftPressed = false;
	float shiftHoldTimer = 0.0f;

	// 長押し判定時間]
	const float dashHoldTime = 0.2f;

	// ===== 状態 =====
	bool isDash = false;

	// ===== 移動速度 =====
	float walkSpeed = 220.0f;
	float dashSpeed = 400.0f;

	bool jumpRequest = false;
	bool dashJump = false;

	bool attackActive = false;
	bool attackHit = false;

	// ===== 攻撃当たり判定 =====
	VECTOR attackPos;

	bool isHit = false;
	bool isDead = false;

	const float jumpStartFrame = 55.0f;

	// ==== 怯み（ヒットストップ）====
	float hitStopTimer = 0.0f;
	const float hitStopDuration = 2.0f;

	// ===== ノックバック =====
	bool isKnockback = false;
	float knockbackTimer = 0.0f;
	const float knockbackDuration = 0.25f;

	VECTOR dashJumpVelocity = VGet(0, 0, 0);

	// ===== 回避 =====
	bool isDodging = false;
	float dodgeSpeed = 600.0f;
	float dodgeStartFrame = 34.0f;
	float dodgeEndFrame = 71.0f;

	VECTOR dodgeDir = VGet(0, 0, 0);

	// 無敵
	bool isInvincible = false;

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
