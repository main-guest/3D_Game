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
	CharacterBase::Init("Assets/mv1model/Enemy/Enemy.mv1");

	sword.Init("Assets/mv1model/Enemy/Sword.mv1");

	// ===== 当たり判定サイズ =====
	radius = 10.0f;
	height = 140.0f;

	// ===== 初期位置 =====
	pos = startPos;

	MV1SetPosition(handle, pos);

	// ===== アニメ番号 =====
	idleAnim = 0;
	walk_fAnim = 1;

	chase_fAnim = 2;
	chase_bAnim = 3;
	chase_rAnim = 4;
	chase_lAnim = 5;

	dash_f01Anim = 6;
	dash_f02Anim = 7;

	swordAttack01Anim = 8;
	swordAttack02Anim = 9;
	swordAttack03Anim = 10;
	swordAttack04Anim = 11;
	gunAttackAnim = 12;

	dodge_b01Anim = 13;
	dodge_rAnim = 14;
	dodge_lAnim = 15;

	changeWeaponAnim = 16;

	hitAnim = 17;
	deadAnim = 18;

	// ===== 武器データ設定 =====
	// ----- Sword -----
	swordData.comboCount = 4;

	swordData.attackAnim[0] = swordAttack01Anim;
	swordData.attackAnim[1] = swordAttack02Anim;
	swordData.attackAnim[2] = swordAttack03Anim;
	swordData.attackAnim[3] = swordAttack04Anim;

	swordData.dashAnim = dash_f01Anim;

	swordData.attackStartFrame[0] = 34.0f;
	swordData.attackEndFrame[0] = 50.0f;

	swordData.attackStartFrame[1] = 1.0f;
	swordData.attackEndFrame[1] = 35.0f;

	swordData.attackStartFrame[2] = 20.0f;
	swordData.attackEndFrame[2] = 55.0f;

	swordData.attackStartFrame[3] = 45.0f;
	swordData.attackEndFrame[3] = 70.0f;

	swordData.comboNextFrame[0] = 52.0f;

	swordData.comboNextFrame[1] = 35.0f;

	swordData.comboNextFrame[2] = 0.0f;

	swordData.comboNextFrame[3] = 0.0f;

	swordData.attackShape = AttackShape::Sphere;

	swordData.attackDistance = 150.0f;
	swordData.attackRadius = 80.0f;

	swordData.moveSpeed = 230.0f;
	swordData.attackCooldown = 1.2f;

	swordData.attackOffset = VGet(0, 115, 0);

	swordRenderData.posOffset = VGet(0.0f, 10.0f, -5.0f);

	swordRenderData.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 1.25f, DX_PI_F);

	sword.SetRenderData(swordRenderData);

	// ----- Gun -----
	gunData.comboCount = 1;

	gunData.attackAnim[0] = gunAttackAnim;

	gunData.dashAnim = dash_f02Anim;

	gunData.attackStartFrame[0] = 14.0f;

	gunData.attackEndFrame[0] = 20.0f;

	gunData.attackShape = AttackShape::Gun;

	gunData.attackDistance = 650.0f;
	gunData.attackRadius = 20.0f;

	gunData.moveSpeed = 200.0f;
	gunData.attackCooldown = 1.8f;

	gunData.attackOffset = VGet(0, 115, 0);

	gunRenderData.posOffset = VGet(0.0f, 0.0f, 0.0f);

	gunRenderData.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	gun.SetRenderData(gunRenderData);

	// 初期装備
	weaponType = EnemyWeaponType::Sword;
	currentWeapon = &swordData;

	// ===== 初期状態 =====
	speed = currentWeapon->moveSpeed;

	aiState = EnemyAIState::Idle;

	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim, true);
}

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics)
{
	switch (weaponType)
	{
	case EnemyWeaponType::Sword:
		sword.Update(handle, "mixamorig:RightHand", characterAngle, true);
		break;

	case EnemyWeaponType::Gun:
		gun.Update(handle, "mixamorig:RightHand", characterAngle, true);
		break;
	}

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

	targetPlayerPos = playerPos;

	if (CanSeePlayer(targetPlayerPos))
	{
		// プレイヤーを見えている間はタイマーをリセット
		lostTimer = lostTime;
	}
	else
	{
		lostTimer -= dt;

		if (lostTimer < 0.0f)
		{
			lostTimer = 0.0f;
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

	// ==== クールダウン更新 ====
	if (attackTimer > 0.0f)
	{
		attackTimer -= dt;

		if (attackTimer < 0.0f)
		{
			attackTimer = 0.0f;
		}
	}

	UpdateAI(dt, playerPos);

	//　=====　物理処理　=====
	physics.MoveCharacter(pos, velocity, radius, isGround, dt);

	UpdateState();

	// ===== アニメーション更新 =====
	UpdateAnimation(dt);

	VECTOR drawPos = pos;
	drawPos.y -= 30; // 足元補正

	// ===== 描画 =====
	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
}

void Enemy::Render()
{
	VECTOR drawPos = pos;
	drawPos.y -= 30; // 足元補正

	// ===== 描画 =====
	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
	MV1DrawModel(handle);

	switch (weaponType)
	{
	case EnemyWeaponType::Sword:
		sword.Draw();
		break;

	case EnemyWeaponType::Gun:
		gun.Draw();
		break;
	}

	// ===== Debug =====
	DebugDraw();
}

void Enemy::Damage(int power)
{
	if (isDead)
		return;

	hp -= power;

	if (useJumpAttack)
		return;

	attackStarted = false;
	attackHit = false;
	attackActive = false;

	comboStep = 0;

	attackTimer = currentWeapon->attackCooldown;

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

	// ===== 攻撃されたらプレイヤーを認識 =====
	if (aiState != EnemyAIState::Attack)
	{
		aiState = EnemyAIState::Chase;
	}
}

void Enemy::CheckAttackHit(Player* player)
{
	if (!attackActive)
		return;

	if (attackHit)
		return;

	VECTOR center = GetCenterPos();

	VECTOR forward =
	{
		-sinf(characterAngle),
		0.0f,
		-cosf(characterAngle)
	};

	VECTOR attackCenter =
	{
		center.x + forward.x * currentWeapon->attackDistance,
		center.y,
		center.z + forward.z * currentWeapon->attackDistance
	};

	float dist = VSize(VSub(player->GetCenterPos(), attackCenter));

	if (dist <= currentWeapon->attackRadius)
	{
		player->Damage(currentWeapon->damage);

		attackHit = true;
	}
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
			currentWeapon->attackRadius,
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
		pos.x - sinf(left) * searchRange,
		pos.y + 20.0f,
		pos.z - cosf(left) * searchRange
	};

	VECTOR rightPos =
	{
		pos.x - sinf(right) * searchRange,
		pos.y + 20.0f,
		pos.z - cosf(right) * searchRange
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
		pos.x - sinf(characterAngle) * searchRange,
		pos.y + 20.0f,
		pos.z - cosf(characterAngle) * searchRange
	};

	DrawLine3D(center, front, GetColor(255, 255, 0));

	DrawFormatString(
		1000,
		200,
		GetColor(255, 255, 255),
		"lostTimer : %.2f",
		lostTimer);

	DrawFormatString(
		20,
		60,
		GetColor(255, 255, 255),
		"comboGoal = %d",
		comboGoal);

	DrawFormatString(
		20,
		80,
		GetColor(255, 255, 0),
		"Enemy HP : %d",
		hp);
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

	speed = currentWeapon->moveSpeed;

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
		UpdateAttack(dt, dir, distance);
		break;

	case EnemyAIState::Dash:
		UpdateDash(dt);
		break;

	case EnemyAIState::Dodge:
		UpdateDodge(dt);
		break;

	case EnemyAIState::ChangeWeapon:
		UpdateChangeWeapon(dt);
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

	velocity = VScale(moveDir, currentWeapon->moveSpeed * 0.5f);

	float targetAngle = atan2f(moveDir.x, moveDir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 5.0f * dt;
}

void Enemy::UpdateChase(float dt, VECTOR dir, float distance)
{
	if (distance > chaseRange)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;

		return;
	}

	if (distance <= jumpAttackDistance &&
		distance > currentWeapon->attackDistance)
	{
		if (!jumpAttackChecked)
		{
			jumpAttackChecked = true;

			float r = GetRand(999) / 999.0f;

			if (r < jumpAttackProbability)
			{
				useJumpAttack = true;
				aiState = EnemyAIState::Attack;
				return;
			}
		}
	}

	if (distance <= currentWeapon->attackDistance)
	{
		jumpAttackChecked = false;
		useJumpAttack = false;

		aiState = EnemyAIState::Attack;

		return;
	}

	velocity = VScale(dir, currentWeapon->moveSpeed);

	float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;

	if (lostTimer <= 0.0f)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;

		return;
	}
}

void Enemy::UpdateAttack(float dt, VECTOR dir, float distance)
{
	velocity = VGet(0, 0, 0);

	// プレイヤー方向へ向く
	float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;

	// 攻撃開始
	if (!attackStarted)
	{
		attackStarted = true;

		comboStep = 0;
		comboGoal = 0;

		attackHit = false;
		attackActive = false;

		currentState = AnimState::Attack;

		if (useJumpAttack)
		{
			ChangeAnimation(currentWeapon->attackAnim[3], false);
		}
		else
		{
			// コンボ数決定
			float r = GetRand(999) / 999.0f;

			if (r < currentWeapon->combo1Probability)
			{
				comboGoal = 0;
			}
			else if (r < currentWeapon->combo1Probability +
						 currentWeapon->combo2Probability)
			{
				comboGoal = 1;
			}
			else
			{
				comboGoal = 2;
			}

			if (comboGoal >= currentWeapon->comboCount)
			{
				comboGoal = currentWeapon->comboCount - 1;
			}

			ChangeAnimation(currentWeapon->attackAnim[0], false);
		}
	}

	if (useJumpAttack)
	{
		attackActive =
			animTime >= currentWeapon->attackStartFrame[3] &&
			animTime <= currentWeapon->attackEndFrame[3];
	}
	else
	{
		attackActive =
			animTime >= currentWeapon->attackStartFrame[comboStep] &&
			animTime <= currentWeapon->attackEndFrame[comboStep];
	}

	if (useJumpAttack &&
		animTime <= jumpAttackMoveFrame)
	{
		velocity = VScale(dir, currentWeapon->moveSpeed * 2.0f);
	}
	else
	{
		velocity = VGet(0, 0, 0);
	}

	// 次段へ
	if (!useJumpAttack)
	{
		if (comboStep < comboGoal &&
			comboStep + 1 <currentWeapon->comboCount &&
			animTime >= currentWeapon->comboNextFrame[comboStep])
		{
			comboStep++;

			attackHit = false;

			ChangeAnimation(currentWeapon->attackAnim[comboStep], false);

			return;
		}
	}

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (animTime >= totalTime)
	{
		attackStarted = false;
		attackActive = false;
		attackHit = false;

		comboStep = 0;
		comboGoal = 0;

		useJumpAttack = false;

		attackTimer = currentWeapon->attackCooldown;

		lostTimer = lostTime;

		currentState = AnimState::Idle;
		aiState = EnemyAIState::Cooldown;

		ChangeAnimation(idleAnim, true);

		return;
	}
}

void Enemy::UpdateDash(float dt)
{
	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (!dashStarted)
	{
		dashStarted = true;

		currentState = AnimState::Dash;

		ChangeAnimation(currentWeapon->dashAnim, false);
	}

	if (animTime >= totalTime)
	{
		dashStarted = false;

		aiState = EnemyAIState::Chase;
	}
}

void Enemy::UpdateDodge(float dt)
{
	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (!dodgeStarted)
	{
		dodgeStarted = true;

		currentState = AnimState::Dodge;

		int r = GetRand(2);

		switch (r)
		{
		case 0:
			ChangeAnimation(dodge_b01Anim, false);
			break;

		case 1:
			ChangeAnimation(dodge_rAnim, false);
			break;

		case 2:
			ChangeAnimation(dodge_lAnim, false);
			break;
		}
	}

	if (animTime >= totalTime)
	{
		dodgeStarted = false;

		aiState = EnemyAIState::Chase;
	}
}

void Enemy::UpdateChangeWeapon(float dt)
{
	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (!weaponChanging)
	{
		weaponChanging = true;

		currentState = AnimState::ChangeWeapon;

		ChangeAnimation(changeWeaponAnim, false);
	}

	if (animTime >= totalTime)
	{
		weaponChanging = false;

		ChangeWeapon();

		aiState = EnemyAIState::Chase;
	}
}

void Enemy::UpdateCooldown(float dt, float distance)
{
	velocity = VGet(0, 0, 0);

	if (attackTimer > 0.0f)
		return;

	if (lostTimer <= 0.0f)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;

		return;
	}

	if (attackTimer <= 0.0f)
	{
		aiState = EnemyAIState::Chase;
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
			currentState = AnimState::Idle;

			ChangeAnimation(idleAnim, true);
		}

		return;
	}

	if (currentState == AnimState::Attack ||
		currentState == AnimState::Dash ||
		currentState == AnimState::Dodge ||
		currentState == AnimState::ChangeWeapon)
	{
		return;
	}

	AnimState next = AnimState::Idle;

	switch (aiState)
	{
	case EnemyAIState::Idle:
		next = AnimState::Idle;
		break;

	case EnemyAIState::Patrol:
		next = (velocity.x != 0.0f || velocity.z != 0.0f)
			? AnimState::Walk
			: AnimState::Idle;
		break;

	case EnemyAIState::Chase:
		next = (velocity.x != 0.0f || velocity.z != 0.0f)
			? AnimState::Chase
			: AnimState::Idle;
		break;

	case EnemyAIState::Cooldown:
		next = AnimState::Idle;
		break;

	default:
		next = currentState;
		break;
	}

	if (next == currentState)
		return;

	currentState = next;

	switch (currentState)
	{
	case AnimState::Idle:
		ChangeAnimation(idleAnim, true);
		break;

	case AnimState::Walk:
		ChangeAnimation(walk_fAnim, true);
		break;

	case AnimState::Chase:
		ChangeAnimation(chase_fAnim, true);
		break;
	}
}

void Enemy::ChangeWeapon()
{
	if (weaponType == EnemyWeaponType::Sword)
	{
		weaponType = EnemyWeaponType::Gun;
		currentWeapon = &gunData;
	}
	else
	{
		weaponType = EnemyWeaponType::Sword;
		currentWeapon = &swordData;
	}

	speed = currentWeapon->moveSpeed;
}

void Enemy::ResetAttackState()
{
	attackStarted = false;
	attackHit = false;
	attackActive = false;

	attackTimer = currentWeapon->attackCooldown;
}

bool Enemy::CanSeePlayer(VECTOR playerPos) const
{
	VECTOR toPlayer = VSub(playerPos, pos);

	toPlayer.y = 0.0f;

	float distance = VSize(toPlayer);

	// 距離外
	if (distance > searchRange)
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

void Enemy::GeneratePatrolTarget()
{
	float range = 300.0f;

	patrolTarget.x = pos.x + (float)(GetRand((int)range * 2) - range);
	patrolTarget.y = pos.y;
	patrolTarget.z = pos.z + (float)(GetRand((int)range * 2) - range);
}