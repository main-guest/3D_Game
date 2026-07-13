#pragma once
#include "CharacterBase.h"
#include "Weapon.h"

class PhysicsManager;
class Player;

enum class EnemyWeaponType
{
	Sword,
	Gun
};

enum class EnemyAIState
{
	Idle,
	Patrol,
	Chase,
	Attack,
	Dodge,
	Dash,
	ChangeWeapon,
	Cooldown
};

class Enemy :public CharacterBase
{
public:
	void Init(VECTOR startPos);
	void Update(float dt, VECTOR playerPos, PhysicsManager& physics);
	void Render();

	void Damage(int power);

	void CheckAttackHit(Player* player);

	VECTOR GetCenterPos() const;

	VECTOR GetPos() const { return pos; }

	bool IsDead() const { return isDead; }

	void DebugDraw();

private:
	// ===== AI ======
	void UpdateAI(float dt, VECTOR playerPos);

	void UpdateIdle(float dt, float distance);
	void UpdatePatrol(float dt, float distance);
	void UpdateChase(float dt, VECTOR dir, float distance);
	void UpdateAttack(float dt, VECTOR dir, float distance);
	void UpdateDash(float dt);
	void UpdateDodge(float dt);
	void UpdateChangeWeapon(float dt);
	void UpdateCooldown(float dt, float distance);

	bool CanSeePlayer(VECTOR playerPos) const;

	void GeneratePatrolTarget();

	// ===== Attack =====
	void StartAttacK();

	void UpdateAttackAnimation();

	void EndAttack();

	void ChangeWeapon();

	void ResetAttackState();

	// アニメーション
	void UpdateState();

private:
	// ==== ステータス ====
	int hp = 100;

	float stamina = 100.0f;

	float maxStamina = 100.0f;

	VECTOR pos = VGet(0, 0, 0);
	VECTOR velocity = VGet(0, 0, 0);

	float characterAngle = 0.0f;

	// ===== 状態 =====
	bool prevGround = true;

	bool isDead = false;
	bool isDying = false;

	// ===== AI =====
	EnemyAIState aiState = EnemyAIState::Idle;

	float stateTimer = 2.0f;

	VECTOR patrolTarget = VGet(0, 0, 0);

	// PLAYER位置保存
	VECTOR targetPlayerPos = VGet(0, 0, 0);

	// ===== 視界 =====
	float searchRange = 500.0f;	// 感知距離
	float chaseRange = 800.0f;      // 最大追尾距離
	float attackRange = 150.0f;     // 攻撃距離

	float viewAngle = 90.0f;		// 視野角(度)

	float lostTimer = 0.0f;
	const float lostTime = 5.0f;

	// ===== Weapon =====
	// 武器
	Weapon sword;
	Weapon gun;

	EnemyWeaponType weaponType = EnemyWeaponType::Sword;

	// 武器データ
	EnemyWeaponData swordData;
	EnemyWeaponData gunData;

	WeaponRenderData swordRenderData;
	WeaponRenderData gunRenderData;

	// 現在装備
	EnemyWeaponData* currentWeapon = nullptr;

	// ChangeWeapon
	bool weaponChanging = false;

	// ===== Attack =====
	bool attackStarted = false;

	bool attackActive = false;
	bool attackHit = false;

	// 攻撃コンボ
	int comboStep = 0;          // 現在のコンボ段数
	int comboGoal = 0;			//	最終コンボ数

	bool useJumpAttack = false;

	bool jumpAttackChecked = false;

	float jumpAttackDistance = 400.0f;

	float comboTimer = 0.0f;

	float attackTimer = 0.0f;

	VECTOR attackCenter = VGet(0, 0, 0);

	float jumpAttackProbability = 0.35f;

	float jumpAttackMoveFrame = 40.0f;

	// ===== Dash =====
	bool dashStarted = false;

	VECTOR dashDirection = VGet(0, 0, 0);

	float dashTimer = 0.0;

	float dashTime = 0.35f;

	// ===== Dodge =====
	bool dodgeStarted = false;

	VECTOR dodgeDirection = VGet(0, 0, 0);

	float dodgeTimer = 0.0f;

	float dodgeTime = 0.45f;

	// ==== Hit （ヒットストップ）====
	float hitStopTimer = 0.0f;
	const float hitStopDuration = 1.5f;
};