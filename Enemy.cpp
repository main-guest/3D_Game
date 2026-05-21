#include <cmath>
#include "Enemy.h"
#include "CollisionWorld.h"
#include "PhysicsManager.h"

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
	CharacterBase::Init(_T("mv1model/Enemy.mv1"));

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
	jumpStartAnim = 2;
	jumpLoopAnim = 3;
	jumpEndAnim = 4;
	hitAnim = 5;
	attack01Anim = 6;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim, true);
}

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics)
{
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

	bool isMove = (velocity.x != 0 || velocity.z != 0);

	velocity = VGet(0, 0, 0);

	// ==== プレイヤー追跡 ====
	if (distance < searchRange)
	{
		if (distance > 0.001f)
		{
			dir.x /= distance;
			dir.z /= distance;

			float sinY = sinf(characterAngle);
			float cosY = cosf(characterAngle);

			// ワールド方向
			velocity.x = dir.x * speed;
			velocity.z = dir.z * speed;

			isMove = true;

			// ===== 回転 =====
			float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

			float diff = targetAngle - characterAngle;

			while (diff > DX_PI)
			{
				diff -= DX_TWO_PI;
			}

			while (diff < -DX_PI)
			{
				diff += DX_TWO_PI;
			}

			characterAngle += diff * 10.0f * dt;
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
			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				currentState = AnimState::Walk;
				ChangeAnimation(walkAnim, true);
			}
			else
			{
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
			}

			return;
		}
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
			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				currentState = AnimState::Walk;
				ChangeAnimation(walkAnim, true);
			}
			else
			{
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
			}
		}

		return;
	}

	//　=====　Attack中　=====
	if (currentState == AnimState::Attack01)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// アニメ終了
		if (animTime >= totalTime)
		{

			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				currentState = AnimState::Walk;
				ChangeAnimation(walkAnim, true);
			}
			else
			{
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
			}
		}

		return;
	}

	//　=====　通常状態　=====
	AnimState nextState = AnimState::Idle;

	if (!isGround)
	{
		nextState = AnimState::JumpLoop;
	}
	else if (fabsf(velocity.x) > 0.1f ||
		fabsf(velocity.z) > 0.1f)
	{
		nextState = AnimState::Walk;
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

		case AnimState::JumpLoop:
			ChangeAnimation(jumpLoopAnim, true);
			break;
		}
	}
}