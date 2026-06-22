#pragma once
#include <vector>
#include <memory>
#include "CharacterBase.h"
#include "Weapon.h"

class PhysicsManager;
class CollisionWorld;
class Enemy;
class Stage;
class Camera;

enum class EquipState
{
	Unarmed,
	Weapon1,
	Weapon2
};

class Player : public CharacterBase
{
public:
	void Init(CollisionWorld* w, Stage* s, Camera* c);
	void Update(float dt, float cameraAngle, PhysicsManager& physics);
	void Draw();

	void EquipWeapon1();
	void EquipWeapon2();
	void Unequip();

	void UpdateAttackPos();

	void CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies);

	void Damage(int power);

	void StartDodge();

	void LockOn(std::vector<std::unique_ptr<Enemy>>& enemies);
	void SwitchLockTarget(std::vector<std::unique_ptr<Enemy>>& enemies, bool right);

	Enemy* GetLockOnTarget() const
	{
		return lockOnTarget;
	}

	bool IsLockOn() const
	{
		return isLockOn;
	}

	VECTOR GetCenterPos() const;

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

	MoveDirection GetLockMoveDirection() const;

	VECTOR GetLockMoveVector() const;
	void UpdateInput(float dt, float cameraAngle, std::vector<std::unique_ptr<Enemy>>& enemies);
	void UpdateState();

private:
	// ===== ステータス =====
	int hp = 100;

	// ===== 入力 =====
	int oldMouse = 0;

	bool shiftPressed = false;

	float shiftHoldTimer = 0.0f;

	// 入力方向
	float inputMoveX = 0.0f;
	float inputMoveZ = 0.0f;

	// 長押し判定時間]
	const float dashHoldTime = 0.2f;

	// ===== 状態 =====
	bool isDash = false;

	bool prevGround = true;

	bool jumpRequest = false;

	bool dashJump = false;

	bool attackActive = false;
	bool attackHit = false;

	bool isHit = false;

	bool isDead = false;
	bool isDying = false; 

	// ===== 移動速度 =====
	float walkSpeed = 220.0f;
	float dashSpeed = 400.0f;

	VECTOR dashJumpVelocity = VGet(0, 0, 0);

	// ===== 攻撃当たり判定 =====
	VECTOR attackPos;

	// ===== 攻撃コンボ =====
	int comboStep = 0;          // 現在のコンボ段数
	bool comboNext = false;     // 次段予約
	float comboTimer = 0.0f;    // 現在の攻撃時間

	// ===== ジャンプ開始フレーム =====
	const float jumpStartFrame = 54.0f;

	// ==== 怯み（ヒットストップ）====
	float hitStopTimer = 0.0f;
	const float hitStopDuration = 2.0f;

	// ===== ノックバック =====
	bool isKnockback = false;
	float knockbackTimer = 0.0f;
	const float knockbackDuration = 0.25f;

	// ===== 回避 =====
	bool isDodging = false;
	
	float dodgeSpeed = 600.0f;

	float dodgeStartFrame = 0.0f;
	float dodgeEndFrame = 47.0f;

	VECTOR dodgeDir = VGet(0, 0, 0);

	// 無敵
	bool isInvincible = false;

	// 外部参照
	CollisionWorld* world = nullptr;
	Stage* stage = nullptr;
	Camera* camera;

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

	// ===== ロックオン =====
	Enemy* lockOnTarget = nullptr;
	bool isLockOn = false;

	int oldQ = 0;

	// マウス位置
	int prevMouseX = 0;
	int prevMouseY = 0;

	// ターゲット切替
	bool canSwitchTarget = true;
	const int switchThreshold = 80;

	// ロックオン距離
	float lockOnDistance = 800.0f;
};
