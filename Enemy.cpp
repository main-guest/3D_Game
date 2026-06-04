#include <cmath>
#include "Enemy.h"
#include "Player.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"

const TCHAR* ToString(EnemyAIState state)
{
	switch (state)
	{
	case EnemyAIState::Idle:   return _T("Idle");
	case EnemyAIState::Patrol: return _T("Patrol");
	case EnemyAIState::Chase:  return _T("Chase");
	case EnemyAIState::Attack: return _T("Attack");
	default:                   return _T("Unknown");
	}
}

Enemy::Enemy()
	:jumpRequest(false)
{

}

Enemy::~Enemy()
{

}

void Enemy::Init(VECTOR startPos)
{
	// ===== 共通初期化 =====
	CharacterBase::Init(_T("mv1model/Enemy2.mv1"));

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
	walkAnim = 1;
	chaseAnim = 2;
	jumpStartAnim = 3;
	jumpLoopAnim = 4;
	jumpEndAnim = 5;
	attack01Anim = 6;
	hitAnim = 7;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim, true);

	aiState = EnemyAIState::Idle;
	stateTimer = 2.0f;
}

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics)
{
	// ==== クールダウン更新 ====
	if (attackTimer > 0.0f)
	{
		attackTimer -= dt;
	}

	bool isCooldown = (attackTimer > 0.0f);

	// ==== ヒットストップ中は停止 ====
	if (hitStopTimer > 0.0f)
	{
		hitStopTimer -= dt;

		velocity = VGet(0, 0, 0);

		UpdateAnimation(dt);

		MV1SetPosition(handle, pos);
		MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
		return;
	}

	VECTOR dir = VGet(0, 0, 0);

	dir.x = playerPos.x - pos.x;
	dir.z = playerPos.z - pos.z;

	float distance = sqrtf(dir.x * dir.x + dir.z * dir.z);

	// ==== プレイヤー追跡 ====
	if (aiState != EnemyAIState::Attack)
	{
		if (distance <= searchRange)
		{
			aiState = EnemyAIState::Chase;
		}
	}

	// ==== AI ====
	switch (aiState)
	{
		case EnemyAIState::Idle:
		{
			velocity = VGet(0, 0, 0);

			stateTimer -= dt;

			if (stateTimer <= 0.0f)
			{
				GeneratePatrolTarget();

				aiState = EnemyAIState::Patrol;
			}

			break;
		}

		case EnemyAIState::Patrol:
		{
			VECTOR moveDir = VSub(patrolTarget, pos);

			float dist = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

			if (dist <= 20.0f)
			{
				aiState = EnemyAIState::Idle;

				stateTimer = 2.0f;
			}
			else
			{
				moveDir.x /= dist;
				moveDir.z /= dist;

				velocity.x = moveDir.x * speed * 0.5f;
				velocity.z = moveDir.z * speed * 0.5f;

				float targetAngle = atan2f(moveDir.x, moveDir.z) + DX_PI;

				float diff = targetAngle - characterAngle;

				while (diff > DX_PI)diff -= DX_TWO_PI;
				while (diff < -DX_PI)diff += DX_TWO_PI;

				characterAngle += diff * 5.0f * dt;
			}

			break;
		}
			
		case EnemyAIState::Chase:
		{
			if (distance > searchRange)
			{
				aiState = EnemyAIState::Idle;

				stateTimer = 2.0f;

				break;
			}

			if (distance > 0.001f)
			{
				dir.x /= distance;
				dir.z /= distance;

				velocity.x = dir.x * speed;
				velocity.z = dir.z * speed;

				float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

				float diff = targetAngle - characterAngle;

				while (diff > DX_PI)diff -= DX_TWO_PI;
				while (diff < -DX_PI)diff += DX_TWO_PI;

				characterAngle += diff * 10.0f * dt;
			}

			if (aiState != EnemyAIState::Attack && distance <= attackRange && attackCooldown <= 0.0f)
			{
				aiState = EnemyAIState::Attack;
				break;
			}
			else if(distance <= attackRange && attackCooldown > 0.0f)
			{
				aiState = EnemyAIState::Idle;
				break;
			}

			break;
		}

		case EnemyAIState::Attack:
		{
			if (!attackStarted)
			{
				attackStarted = true;
				attackTimer = attackCooldown;

				velocity = VGet(0, 0, 0);

				currentState = AnimState::Attack;
				ChangeAnimation(attack01Anim, false);
			}

			float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

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
				attackStarted = false;
				attackHit = false;

				stateTimer = 0.5f;

				aiState = EnemyAIState::Chase;
			}

			break;
		}
	}

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

	VECTOR diff = VSub(player->GetPos(), pos);

	float dist = VSize(diff);

	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	VECTOR dir = VNorm(diff);

	float dot = VDot(forward, dir);

	float limit = cosf(60.0f * DX_PI_F / 180.0f);

	if (dist <= attackRange && dot >= limit)
	{
		player->Damage(20);

		attackHit = true;
	}
}

void Enemy::Damage(int power)
{
	hp -= power;

	isHit = true;

	if (hp <= 0)
	{
		isDead = true;
		return;
	}

	// ==== 怯み開始 ====
	hitStopTimer = hitStopDuration;

	// ===== アニメーション切り替え =====
	currentState = AnimState::Hit;
	ChangeAnimation(hitAnim, false);
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
		_T("AI State: %s"), ToString(aiState));
}

void Enemy::GeneratePatrolTarget()
{
	float range = 300.0f;

	patrolTarget.x = pos.x + (float)(GetRand((int)range * 2) - range);
	patrolTarget.y = pos.y;
	patrolTarget.z = pos.z + (float)(GetRand((int)range * 2) - range);
}

void Enemy::UpdateState()
{
	// ===== ヒットストップ中は完全固定 =====
	if (currentState == AnimState::Hit)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// アニメ終了
		if (animTime >= totalTime)
		{
			currentState = AnimState::Idle;
			ChangeAnimation(idleAnim, true);
		}

		return;
	}

	//　=====　JumpStart → JumpLoop　=====
	if (currentState == AnimState::JumpStart)
	{
		if (jumpRequest && animTime >= jumpStartFrame)
		{
			velocity.y = jumpPower;

			isGround = false;

			jumpRequest = false;

			currentState = AnimState::JumpLoop;

			ChangeAnimation(jumpLoopAnim, true);
		}

		return;
	}

	//　=====　着地瞬間検知　=====
	bool landed = (!prevGround && isGround);

	if (landed && currentState != AnimState::JumpEnd)
	{
		currentState = AnimState::JumpEnd;

		ChangeAnimation(jumpEndAnim, false);

		return;
	}

	//　=====　JumpEnd終了　=====
	if (currentState == AnimState::JumpEnd)
	{
		// アニメ終了判定
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime - 0.1f)
		{
			currentState = AnimState::Idle;
			ChangeAnimation(idleAnim, true);
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
			ChangeAnimation(walkAnim, true);
			break;

		case AnimState::Chase:
			ChangeAnimation(chaseAnim, true);
			break;

		case AnimState::Attack:
			ChangeAnimation(attack01Anim, false);
			break;

		case AnimState::JumpLoop:
			ChangeAnimation(jumpLoopAnim, true);
			break;
		}
	}
}