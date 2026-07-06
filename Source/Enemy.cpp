#include <cmath>
#include "Enemy.h"
#include "Player.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"

const TCHAR* ToString(EnemyAIState state)
{
	switch (state)
	{
	case EnemyAIState::Idle:   return "Idle";
	case EnemyAIState::Patrol: return "Patrol";
	case EnemyAIState::Chase:  return "Chase";
	case EnemyAIState::Attack: return "Attack";
	case EnemyAIState::Cooldown: return "Cooldown";
	default:                   return "Unknown";
	}
}

void Enemy::Init(VECTOR startPos)
{
	// ===== 共通初期化 =====
	CharacterBase::Init("Assets/mv1model/Enemy.mv1");

	// ===== 当たり判定サイズ =====
	radius = 10.0f;
	height = 140.0f;

	// ===== 移動速度 =====
	speed = 200.0f;

	// ===== 初期位置 =====
	pos = startPos;

	MV1SetPosition(handle, pos);

	// ===== アニメ番号 =====
	idleAnim = 0;
	walk_fAnim = 1;
	chaseAnim = 2;
	jumpStartAnim = 3;
	jumpRiseAnim = 4;
	jumpEndAnim = 5;
	handAttackAnim = 6;
	hitAnim = 7;
	deadAnim = 8;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim, true);

	aiState = EnemyAIState::Idle;
	stateTimer = 2.0f;
}

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics)
{
	targetPlayerPos = playerPos;

	if (isDead)
	{
		// ===== モデル非表示 =====
		MV1SetVisible(handle, FALSE);

		return;
	}

	// ===== 死亡アニメ中はAI停止 =====
	if (currentState == AnimState::Dead)
	{
		velocity = VGet(0, 0, 0);

		UpdateState();
		UpdateAnimation(dt);

		return;
	}

	// ==== クールダウン更新 ====
	if (attackTimer > 0.0f)
	{
		attackTimer -= dt;

		if (attackTimer < 0.0f)
		{
			attackTimer = 0.0f;
		}
	}

	// ==== ヒットストップ中は停止 ====
	if (hitStopTimer > 0.0f)
	{
		hitStopTimer -= dt;

		velocity = VGet(0, 0, 0);

		UpdateAnimation(dt);

		return;
	}

	UpdateAI(dt, playerPos);

	//　=====　物理処理　=====
	physics.MoveCharacter(pos, velocity, radius, isGround, dt);

	UpdateState();

	// ===== アニメーション更新 =====
	UpdateAnimation(dt);
}

void Enemy::Render()
{
	VECTOR drawPos = pos;
	drawPos.y -= 30; // 足元補正

	// ===== 描画 =====
	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
	MV1DrawModel(handle);

	// ===== Debug =====
	DebugDraw();
}

void Enemy::CheckAttackHit(Player* player)
{
	if (!attackActive)
		return;

	if (attackHit)
		return;

	VECTOR playerCenter = player->GetCenterPos();

	VECTOR diff = VSub(playerCenter, attackCenter);

	float dist = VSize(diff);


	if (dist <= attackRadius)
	{
		player->Damage(20);

		attackHit = true;
	}
}

void Enemy::Damage(int power)
{
	if (isDead)
		return;

	ResetAttackState();

	// ===== 攻撃されたらプレイヤーを認識 =====
	if (aiState != EnemyAIState::Attack)
	{
		aiState = EnemyAIState::Chase;
	}

	hp -= power;

	isHit = true;

	if (hp <= 0)
	{
		isDying = true;

		currentState = AnimState::Dead;
		ChangeAnimation(deadAnim, false);

		return;
	}

	// ==== 怯み開始 ====
	hitStopTimer = hitStopDuration;

	// ===== アニメーション切り替え =====
	currentState = AnimState::Hit;
	ChangeAnimation(hitAnim, false);
}

bool Enemy::CanSeePlayer(VECTOR playerPos) const
{
	VECTOR toPlayer = VSub(playerPos, pos);

	toPlayer.y = 0.0f;

	float distance = VSize(toPlayer);

	// 距離外
	if (distance > viewDistance)
		return false;

	toPlayer = VNorm(toPlayer);

	// Enemy前方向
	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	// 内積
	float dot =
		forward.x * toPlayer.x +
		forward.y * toPlayer.y +
		forward.z * toPlayer.z;

	// cos(45°)
	float limit = cosf((viewAngle * 0.5f) * DX_PI_F / 180.0f);

	return dot >= limit;
}

VECTOR Enemy::GetCenterPos() const
{
	return VGet(pos.x, pos.y + height * 0.75f, pos.z);
}

void Enemy::DebugDraw()
{
	float h = height;

	VECTOR bottom = pos;
	VECTOR top = VGet(pos.x, pos.y + h, pos.z);

	DrawSphere3D(bottom, radius, 12, GetColor(255, 0, 0), GetColor(255, 0, 0), FALSE);
	DrawSphere3D(top, radius, 12, GetColor(255, 0, 0), GetColor(255, 0, 0), FALSE);
	DrawLine3D(bottom, top, GetColor(0, 255, 0));

	DrawFormatString(20, 20, GetColor(255, 255, 255),
		"AI State: %s", ToString(aiState));

	DrawSphere3D(
		bottom,
		radius,
		12,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		FALSE);

	DrawSphere3D(
		top,
		radius,
		12,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		FALSE);

	DrawLine3D(
		bottom,
		top,
		GetColor(0, 255, 0));

	// 攻撃判定
	if (attackActive)
	{
		DrawSphere3D(
			attackCenter,
			attackRadius,
			16,
			GetColor(255, 255, 0),
			GetColor(255, 255, 0),
			FALSE);
	}
	
	float half = viewAngle * 0.5f * DX_PI_F / 180.0f;

	float left = characterAngle - half;
	float right = characterAngle + half;

	VECTOR leftPos =
	{
		pos.x - sinf(left) * viewDistance,
		pos.y + 20.0f,
		pos.z - cosf(left) * viewDistance
	};

	VECTOR rightPos =
	{
		pos.x - sinf(right) * viewDistance,
		pos.y + 20.0f,
		pos.z - cosf(right) * viewDistance
	};

	VECTOR center =
	{
		pos.x,
		pos.y + 20.0f,
		pos.z
	};

	DrawLine3D(center, leftPos, GetColor(0, 255, 255));
	DrawLine3D(center, rightPos, GetColor(0, 255, 255));

	VECTOR front =
	{
		pos.x - sinf(characterAngle) * viewDistance,
		pos.y + 20.0f,
		pos.z - cosf(characterAngle) * viewDistance
	};

	DrawLine3D(center, front, GetColor(255, 255, 0));
}

void Enemy::GeneratePatrolTarget()
{
	float range = 300.0f;

	patrolTarget.x = pos.x + (float)(GetRand((int)range * 2) - range);
	patrolTarget.y = pos.y;
	patrolTarget.z = pos.z + (float)(GetRand((int)range * 2) - range);
}

void Enemy::ResetAttackState()
{
	attackStarted = false;
	attackHit = false;
	attackActive = false;

	attackTimer = attackCooldown;
}

void Enemy::UpdateAI(float dt, VECTOR playerPos)
{
	VECTOR dir = VSub(playerPos, pos);
	dir.y = 0.0f;

	float distance = VSize(dir);

	if (distance > 0.001f)
	{
		dir = VNorm(dir);
	}

	velocity = VGet(0, 0, 0);

	switch (aiState)
	{
	case EnemyAIState::Idle:
		UpdateIdle(dt, distance);
		break;

	case EnemyAIState::Patrol:
		UpdatePatrol(dt, distance);
		break;

	case EnemyAIState::Chase:
		UpdateChase(dt, dir, distance);
		break;

	case EnemyAIState::Attack:
		UpdateAttack(dt, dir);
		break;

	case EnemyAIState::Cooldown:
		UpdateCooldown(dt, distance);
		break;
	}
}

void Enemy::UpdateIdle(float dt, float distance)
{
	velocity = VGet(0, 0, 0);

	stateTimer -= dt;

	if (CanSeePlayer(targetPlayerPos))
	{
		aiState = EnemyAIState::Chase;
		return;
	}

	if (stateTimer <= 0.0f)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;
	}
}

void Enemy::UpdatePatrol(float dt, float distance)
{
	if (CanSeePlayer(targetPlayerPos))
	{
		aiState = EnemyAIState::Chase;
		return;
	}

	VECTOR moveDir = VSub(patrolTarget, pos);
	moveDir.y = 0.0f;

	float dist = VSize(moveDir);

	if (dist <= 20.0f)
	{
		aiState = EnemyAIState::Idle;

		stateTimer = 2.0f;

		return;
	}

	moveDir = VNorm(moveDir);

	velocity = VScale(moveDir, speed * 0.5f);

	float targetAngle = atan2f(moveDir.x, moveDir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 5.0f * dt;
}

void Enemy::UpdateChase(float dt, VECTOR dir, float distance)
{
	if (distance > viewDistance)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;

		return;
	}

	if (distance <= attackRange)
	{
		aiState = EnemyAIState::Attack;

		return;
	}

	velocity = VScale(dir, speed);

	float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;
}

void Enemy::UpdateAttack(float dt, VECTOR dir)
{
	velocity = VGet(0, 0, 0);

	float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;

	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	attackCenter =
	{
		pos.x + forward.x * attackOffset,
		pos.y + 120.0f,
		pos.z + forward.z * attackOffset
	};

	if (!attackStarted)
	{
		attackStarted = true;

		currentState = AnimState::Attack;

		ChangeAnimation(handAttackAnim, false);
	}

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	attackActive =
		animTime >= attackStartFrame &&
		animTime <= attackEndFrame;

	if (animTime >= totalTime)
	{
		ResetAttackState();

		aiState = EnemyAIState::Cooldown;

		currentState = AnimState::Idle;

		ChangeAnimation(idleAnim, true);
	}
}

void Enemy::UpdateCooldown(float dt, float distance)
{
	velocity = VGet(0, 0, 0);

	if (attackTimer > 0.0f)
		return;

	if (distance <= attackRange)
	{
		aiState = EnemyAIState::Attack;
	}
	else if (distance <= viewDistance)
	{
		aiState = EnemyAIState::Chase;
	}
	else
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;
	}
}

void Enemy::UpdateState()
{
	// ===== 死亡 =====
	if (currentState == AnimState::Dead)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// アニメ終了
		if (animTime >= totalTime)
		{
			animTime = totalTime;

			isDead = true;
		}

		return;
	}

	// ===== ヒットストップ中は完全固定 =====
	if (currentState == AnimState::Hit)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// アニメ終了
		if (animTime >= totalTime)
		{
			switch (aiState)
			{
			case EnemyAIState::Idle:
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
				break;

			case EnemyAIState::Patrol:
				currentState = AnimState::Walk;
				ChangeAnimation(walk_fAnim, true);
				break;

			case EnemyAIState::Chase:
				currentState = AnimState::Chase;
				ChangeAnimation(chaseAnim, true);
				break;

			default:
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
				break;
			}
		}

		return;
	}

	//　=====　Attack中　=====
	if (currentState == AnimState::Attack)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// アニメ終了
		if (animTime >= attackStartFrame && animTime <= attackEndFrame)
		{
			attackActive = true;
		}
		else
		{
			attackActive = false;
		}

		if (animTime >= totalTime)
		{
			attackHit = false;

			aiState = EnemyAIState::Chase;

			currentState = AnimState::Walk;

			ChangeAnimation(chaseAnim, true);
		}

		return;
	}

	//　=====　通常状態　=====
	AnimState nextState = AnimState::Idle;

	switch (aiState)
	{
	case EnemyAIState::Idle:
		nextState = AnimState::Idle;
		break;

	case EnemyAIState::Patrol:
		nextState = AnimState::Walk;
		break;

	case EnemyAIState::Chase:
		nextState = AnimState::Chase;
		break;

	case EnemyAIState::Attack:
		nextState = AnimState::Attack;
		break;
	}

	if (nextState != currentState)
	{
		currentState = nextState;

		switch (currentState)
		{
		case AnimState::Idle:
			ChangeAnimation(idleAnim, true);
			break;

		case AnimState::Walk:
			ChangeAnimation(walk_fAnim, true);
			break;

		case AnimState::Chase:
			ChangeAnimation(chaseAnim, true);
			break;

		case AnimState::Attack:
			ChangeAnimation(handAttackAnim, false);
			break;

		case AnimState::JumpRise:
			ChangeAnimation(jumpRiseAnim, true);
			break;
		}
	}
}