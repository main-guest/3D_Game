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
	ChangeWeapon,
	Cooldown
};

class Enemy :public CharacterBase
{
public:
	void Init(VECTOR startPos);
	void Update(float dt, VECTOR playerPos, PhysicsManager& physics);
	void Render();

	void ChangeWeapon();

	void StartChangeWeapon();

	void CheckAttackHit(Player* player);

	void Damage(int power);

	bool CanSeePlayer(VECTOR playerPos) const;

	VECTOR GetCenterPos() const;

	void DebugDraw();

	VECTOR GetPos() const { return pos; }

	bool IsDead() const { return isDead; }

private:
	void GeneratePatrolTarget();
	void ResetAttackState();

	void UpdateAI(float dt, VECTOR playerPos);

	void UpdateIdle(float dt, float distance);
	void UpdatePatrol(float dt, float distance);
	void UpdateChase(float dt, VECTOR dir, float distance);
	void UpdateAttack(float dt, VECTOR dir);
	void UpdateCooldown(float dt, float distance);

	void UpdateState();

private:
	// ==== ステータス ====
	int hp = 100;

	VECTOR pos;
	VECTOR velocity = VGet(0, 0, 0);

	float characterAngle = 0.0f;

	// ===== 状態 =====
	bool prevGround = true;

	//bool jumpRequest = false;

	bool attackActive = false;
	bool attackHit = false;

	bool isHit = false;

	bool isDead = false;
	bool isDying = false;

	EnemyAIState aiState = EnemyAIState::Idle;

	VECTOR attackPos = VGet(0, 0, 0);
	float attackOffset = 140.0f;
	float attackRadius = 80.0f;

	VECTOR attackCenter;

	// ==== 怯み（ヒットストップ）====
	float hitStopTimer = 0.0f;
	const float hitStopDuration = 1.5f;

	// ==== AI ====
	float stateTimer = 2.0f;

	VECTOR patrolTarget = VGet(0, 0, 0);

	// ===== 視界 =====
	float viewDistance = 500.0f;	// 感知距離
	float viewAngle = 90.0f;		// 視野角(度)

	float chaseRange = 800.0f;      // 最大追尾距離
	float attackRange = 150.0f;     // 攻撃距離

	float lostTimer = 0.0f;
	const float lostTime = 5.0f;

	float attackStartFrame = 35.0f;
	float attackEndFrame = 53.0f;

	bool attackStarted = false;

	float attackTimer = 0.0f;
	const float attackCooldown = 1.5f;

	// PLAYER位置保存
	VECTOR targetPlayerPos;

	// ===== 武器 =====
	EnemyWeaponType weaponType = EnemyWeaponType::Sword;

	// ===== 武器データ =====
	EnemyWeaponData swordData;
	EnemyWeaponData gunData;

	// 現在装備
	EnemyWeaponData* currentWeapon = nullptr;

	// ===== 攻撃コンボ =====
	int comboStep = 0;          // 現在のコンボ段数
	bool comboNext = false;     // 次段予約
};