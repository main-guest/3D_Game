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

	// ===== 初期位置 =====
	pos = startPos;

	MV1SetPosition(handle, pos);

	// ===== アニメ番号 =====
	idleAnim = 0;
	walkAnim = 1;
	jumpStartAnim = 2;
	jumpLoopAnim = 3;
	jumpEndAnim = 4;
	attack01Anim = 5;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim);
}

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics)
{
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
	physics.MoveAndCheckCollision(pos, velocity, radius, isGround, dt);

	//　=====　状態更新　=====
	if (currentState != AnimState::Attack01 && currentState != AnimState::JumpStart && currentState != AnimState::JumpEnd)
	{
		AnimState nextState = AnimState::Idle;

		if (!isGround)
		{
			nextState = AnimState::JumpLoop;
		}
		else if (isMove)
		{
			nextState = AnimState::Walk;
		}

		// ===== 状態変更 =====
		if (nextState != currentState)
		{
			currentState = nextState;

			switch (currentState)
			{
			case AnimState::Idle:
				ChangeAnimation(idleAnim);
				break;

			case AnimState::Walk:
				ChangeAnimation(walkAnim);
				break;

			case AnimState::JumpLoop:
				ChangeAnimation(jumpLoopAnim);
				break;
			}
		}
	}

	// ===== アニメーション更新 =====
	UpdateAnimation(dt);

	// ===== モデル反映 =====
	MV1SetPosition(handle, pos);

	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
}