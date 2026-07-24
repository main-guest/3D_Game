#pragma once
#include <vector>
#include <memory>

#include "CharacterBase.h"
#include "Weapon.h"
#include "MoveDirection.h"

class PhysicsManager;
class CollisionWorld;
class Enemy;
class Stage;
class Camera;

enum class EquipState
{
	Unarmed,
	Sword,
	Gun01
};

enum class MoveAnimState
{
	None = -1,

	Idle,

	WalkFront,
	WalkBack,
	WalkLeft,
	WalkRight,

	DashFront,
	DashBack,
	DashLeft,
	DashRight
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

	void StartDodge(float cameraAngle);

	void ConsumeStamina(float cost, float recoveryDelay);

	void LockOn(std::vector<std::unique_ptr<Enemy>>& enemies);
	void SwitchLockTarget(std::vector<std::unique_ptr<Enemy>>& enemies, bool right);

	Enemy* GetLockOnTarget() const
	{
		return lockOnTarget;
	}

	void SetLockOnTarget(Enemy* enemy)
	{
		lockOnTarget = enemy;
	}

	bool IsLockOn() const
	{
		return isLockOn;
	}

	MoveDirection GetDodgeDir() const
	{
		return dodgeDirection;
	}

	bool IsDodging() const;

	bool IsInvincible() const;

	bool IsAttackActive() const { return attackActive; }

	const WeaponData* GetCurrentWeaponData() const;

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

	// Enemy基準の前・右ベクトルを更新
	void UpdateEnemyBasis();

	VECTOR GetLockMoveVector() const;

	VECTOR GetFreeMoveVector(float cameraAngle) const;

	MoveDirection GetLockMoveDirection() const;

	// ロックオン／ロックオフに応じて回転処理を振り分け
	void UpdateRotation(float dt);

	// ロックオン時はEnemy基準、ロックオフ時はカメラ基準の移動ベクトルを更新
	void UpdateMoveVector(float cameraAngle);

	// ロックオン中の向きを制御（通常はEnemy方向、後ろダッシュ時のみ移動方向）
	void UpdateLockOnMove(float dt);

	// ロックオフ中は移動方向へ向きを補間
	void UpdateFreeMove(float dt);

	// ロックオン中の歩き・ダッシュアニメーションを方向に応じて更新
	void UpdateLockOnAnimation();

	// ロックオフ中のIdle／Walk／Dashアニメーションを更新
	void UpdateFreeAnimation();

	bool UpdateDead();

	bool UpdateDodge();

	bool UpdateJump();

	bool UpdateAttack();

	bool UpdateHit();

	std::vector<Enemy*> FindGunTargets(std::vector<std::unique_ptr<Enemy>>& enemies);

	void UpdateInput(float dt, float cameraAngle, std::vector<std::unique_ptr<Enemy>>& enemies);
	void UpdateState();

	MoveAnimState GetNextMoveAnim() const;

private:
	// ===== ステータス =====
	int hp = 1000;

	// --スタミナ--
	float stamina = 1000.0f;
	const float maxStamina = 1000.0f;

	// 回復速度
	float staminaRecovery = 25.0f;

	// 回復開始までの待機時間
	float staminaRecoveryDelay = 1.0f;

	// 消費してからの経過時間
	float staminaRecoveryTimer = 0.0f;

	// ダッシュ消費
	float dashStaminaCost = 20.0f;

	// 行動消費
	float dodgeCost = 25.0f;
	float attackCost = 15.0f;
	float jumpCost = 10.0f;

	// 行動ごとの回復待機時間
	float dashDelay = 2.0f;
	float dodgeDelay = 2.0f;
	float attackDelay = 3.5f;
	float jumpDelay = 3.0f;

	bool staminaBreak = false;

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

	bool prevDash = false;

	bool prevGround = true;

	bool jumpRequest = false;

	bool dashJump = false;

	bool attackActive = false;
	bool attackHit = false;

	bool isDead = false;

	bool deadFinished = false;

	MoveAnimState currentMoveAnim = MoveAnimState::Idle;

	// ===== 移動速度 =====
	float walkSpeed = 220.0f;
	float dashSpeed = 400.0f;

	VECTOR dashJumpVelocity = VGet(0, 0, 0);

	// ===== 攻撃当たり判定 =====
	VECTOR attackPos;

	// ===== 攻撃コンボ =====
	int comboStep = 0;          // 現在のコンボ段数
	bool comboNext = false;     // 次段予約

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

	// 無敵
	bool isInvincible = false;

	// 外部参照
	CollisionWorld* world = nullptr;
	Stage* stage = nullptr;
	Camera* camera;

	// ===== 武器 =====
	Weapon sword;
	Weapon gun01;

	// ===== 武器データ =====
	WeaponData unarmedData;
	WeaponData swordData;
	WeaponData gun01Data;

	WeaponRenderData unarmedRenderData;
	WeaponRenderData swordRenderData;
	WeaponRenderData gun01RenderData;

	// ===== 現在装備 =====
	EquipState equipState;

	// ===== 現在装備中データ =====
	const WeaponData* currentWeaponData = &unarmedData;

	//ガンターゲット
	std::vector<Enemy*> gunTargets;

	float gunTimer = 0.0f;
	bool gunWaitingDamage = false;

	// ===== ロックオン =====
	// 方向
	MoveDirection currentDirection = MoveDirection::None;
	MoveDirection dodgeDirection = MoveDirection::None;

	Enemy* lockOnTarget = nullptr;
	bool isLockOn = false;

	int oldQ = 0;

	// 回避開始時保存
	VECTOR dodgeMoveDir = VGet(0, 0, 0);

	// 毎フレーム移動方向
	VECTOR moveVector = VGet(0, 0, 0);

	// Enemy基準ベクトル
	VECTOR enemyForward = VGet(0, 0, 1);
	VECTOR enemyRight = VGet(1, 0, 0);

	// マウス位置
	int prevMouseX = 0;
	int prevMouseY = 0;

	// ターゲット切替
	bool canSwitchTarget = true;
	const int switchThreshold = 80;

	// ロックオン距離
	float lockOnDistance = 800.0f;
};
