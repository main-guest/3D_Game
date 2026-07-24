#include <cmath>

#include "Enemy.h"
#include "Player.h"
#include "Collision.h"
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
	CharacterBase::Init("Assets/mv1model/Enemy/Enemy2.mv1");

	sword.Init("Assets/mv1model/Enemy/Sword.mv1");
	gun.Init("Assets/mv1model/Enemy/Gun01.mv1");

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

	swordData.attackDistance = 180.0f;
	swordData.attackRadius = 80.0f;

	swordData.moveSpeed = 230.0f;
	swordData.attackCooldown = 1.2f;

	swordData.attackOffset = VGet(0, 115, 0);

	swordRenderData.posOffset = VGet(0.0f, 10.0f, -5.0f);

	swordRenderData.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 1.25f, DX_PI_F);

	sword.SetRenderData(swordRenderData);

	// ----- Gun -----
	for (int i = 0; i < WeaponData::MaxCombo; i++)
	{
		gunData.attackAnim[i] = gunAttackAnim;

		gunData.attackStartFrame[i] = 22.0f;
		gunData.attackEndFrame[i] = 24.0f;

		gunData.comboAcceptStartFrame[i] = 0.0f;
		gunData.comboAcceptEndFrame[i] = 0.0f;
	}

	gunData.comboCount = 1;

	gunData.dashAnim = dash_f02Anim;

	gunData.attackShape = AttackShape::Gun;

	gunData.attackDistance = 650.0f;
	gunData.attackRadius = 50.0f;

	gunData.moveSpeed = 200.0f;
	gunData.attackCooldown = 3.0f;

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

void Enemy::Update(float dt, VECTOR playerPos, PhysicsManager& physics, Player* player)
{
	if (isDead)
		return;

	switch (weaponType)
	{
	case EnemyWeaponType::Sword:
		sword.Update(handle, "mixamorig:RightHand", characterAngle, true);
		break;

	case EnemyWeaponType::Gun:
		gun.Update(handle, "mixamorig:RightHand", characterAngle, true);
		break;
	}

	// ===== 死亡処理 =====
	if (hp <= 0)
	{
		if (currentState != AnimState::Dead)
		{
			currentState = AnimState::Dead;

			velocity = VGet(0, 0, 0);

			ChangeAnimation(deadAnim, false);
		}

		UpdateAnimation(dt);

		UpdateDead();

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

	playerWeapon = player->GetCurrentWeaponData();

	isGun = playerWeapon->attackShape == AttackShape::Gun;

	VECTOR toPlayer = VSub(playerPos, pos);
	float distance = VSize(toPlayer);

	TryDodge(player, distance);

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

	if (currentState == AnimState::Hit)
		return;

	hp -= power;

	if (useJumpAttack)
		return;

	attackStarted = false;
	attackHit = false;
	attackActive = false;

	comboStep = 0;

	attackTimer = currentWeapon->attackCooldown;

	if (hp < 0)
	{
		hp = 0;
	}

	if (hp > 0)
	{
		// ==== 怯み開始 ====
		hitStopTimer = hitStopDuration;

		// ===== アニメーション切り替え =====
		currentState = AnimState::Hit;
		ChangeAnimation(hitAnim, false);
	}

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

	switch (currentWeapon->attackShape)
	{
	case AttackShape::Sphere:
	{
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
			attackHit = true;

			if (player->IsInvincible())
				return;

			player->Damage(currentWeapon->swordDamage);
		}

		break;
	}

	case AttackShape::Gun:
	{
		VECTOR start = GetCenterPos();
		VECTOR end =
		{
			start.x - sinf(characterAngle) * currentWeapon->attackDistance,
			start.y,
			start.z - cosf(characterAngle) * currentWeapon->attackDistance
		};

		VECTOR bottom = player->GetPos();

		VECTOR top = bottom;
		top.y += player->GetHeight();

		bool hit =
			LineCapsuleHit(start, end, bottom, top, player->GetRadius() + currentWeapon->attackRadius);

		if (hit)
		{
			attackHit = true;

			if (player->IsDodging() &&
				player->IsInvincible())
			{
				auto dir = player->GetDodgeDir();

				if (dir == MoveDirection::Left ||
					dir == MoveDirection::Right)
				{
					return;
				}
			}

			player->Damage(currentWeapon->gunDamage);
		}

		break;
	}
	}
}

bool Enemy::IsDodging() const
{
	return currentState == AnimState::Dodge ||
		aiState == EnemyAIState::Dodge;
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
		switch (currentWeapon->attackShape)
		{
		case AttackShape::Sphere:
		{
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

			DrawSphere3D(
				attackCenter,
				5.0f,
				8,
				GetColor(255, 255, 0),
				GetColor(255, 255, 0),
				TRUE);

			DrawSphere3D(
				attackCenter,
				currentWeapon->attackRadius,
				16,
				GetColor(255, 0, 0),
				0,
				FALSE);

			break;
		}

		case AttackShape::Gun:
		{
			VECTOR start = GetCenterPos();

			VECTOR end =
			{
				start.x - sinf(characterAngle) * currentWeapon->attackDistance,
				start.y,
				start.z - cosf(characterAngle) * currentWeapon->attackDistance
			};

			DrawLine3D(
				start,
				end,
				GetColor(0, 255, 0));

			break;
		}
		}
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

	DrawFormatString(
		20,
		100,
		GetColor(0, 255, 255),
		"State=%d",
		(int)currentState);

	DrawFormatString(
		1000,
		480,
		GetColor(255, 255, 255),
		"ChangeProb : %.2f",
		changeProb);
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
		UpdateDash(dt, dir, distance);
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

	velocity = VScale(moveDir, speed * 0.5f);

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

	if (attackTimer > 0.0f)
		return;

	velocity = VScale(dir, speed);

	float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

	float diff = targetAngle - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;

	if (weaponType == EnemyWeaponType::Sword)
	{
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
	}

	if (distance <= currentWeapon->attackDistance)
	{
		jumpAttackChecked = false;
		useJumpAttack = false;

		aiState = EnemyAIState::Attack;

		return;
	}

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
	if ((!attackStarted))
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
		velocity = VScale(dir, speed * 2.0f);
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

		if (weaponType == EnemyWeaponType::Sword &&
			distance > 350.0f)
		{

			float r = GetRand(999) / 999.0f;

			if (r < 0.6f)
			{
				currentState = AnimState::Dash;
				aiState = EnemyAIState::Dash;

				return;
			}
		}

		changeProb = weaponChangeProbability;

		// 剣を持っていて遠い
		if (weaponType == EnemyWeaponType::Sword &&
			distance >= 550.0f)
		{
			changeProb *= 3.0f;
		}

		// 銃を持っていて近い
		if (weaponType == EnemyWeaponType::Gun &&
			distance <= 180.0f)
		{
			changeProb *= 5.0f;
		}

		float r = GetRand(999) / 999.0f;

		if (r < changeProb)
		{
			currentState = AnimState::ChangeWeapon;
			aiState = EnemyAIState::ChangeWeapon;

			return;
		}

		// ===== Cooldown行動 =====
		if (weaponType == EnemyWeaponType::Gun)
		{
			if (distance <= 150.0f)
			{
				cooldownMove = CooldownMove::Back;
			}
			else
			{
				int r = GetRand(1);

				if (r == 0)
					cooldownMove = CooldownMove::Left;
				else
					cooldownMove = CooldownMove::Right;
			}
		}
		else
		{
			int type = GetRand(99);

			if (distance <= 150.0f)
			{
				cooldownMove = CooldownMove::Back;
			}
			else
			{
				if (type < 10)
					cooldownMove = CooldownMove::Forward;
				else if (type < 40)
					cooldownMove = CooldownMove::Back;
				else if (type < 70)
					cooldownMove = CooldownMove::Left;
				else
					cooldownMove = CooldownMove::Right;
			}
		}

		cooldownMoveTimer = currentWeapon->attackCooldown;

		currentState = AnimState::Idle;
		aiState = EnemyAIState::Cooldown;

		ChangeAnimation(idleAnim, true);

		return;
	}
}

void Enemy::UpdateDash(float dt, VECTOR dir, float distance)
{
	if (!dashStarted)
	{
		dashStarted = true;

		currentState = AnimState::Dash;

		ChangeAnimation(currentWeapon->dashAnim, true);
	}

	velocity = VScale(dir, speed * 1.8f);

	if ((attackTimer <= 0.0f && cooldownMoveTimer <= 0.0f) ||
		distance <= currentWeapon->attackDistance)
	{
		dashStarted = false;

		velocity = VGet(0, 0, 0);

		aiState = EnemyAIState::Attack;
		currentState == AnimState::Attack;
	}
}

void Enemy::UpdateDodge(float dt)
{
	// Enemy前方向
	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	VECTOR right;

	right.x = forward.z;
	right.y = 0.0f;
	right.z = -forward.x;

	if (animTime >= dodgeStartFrame &&
		animTime <= dodgeEndFrame)
	{
		switch (dodgeDir)
		{
		case MoveDirection::Back:
			velocity = VScale(forward, -350);
			break;

		case MoveDirection::Right:
			velocity = VScale(right, 350);
			break;

		case MoveDirection::Left:
			velocity = VScale(right, -350);
			break;
		}
	}
	else
	{
		velocity = VGet(0, 0, 0);
	}

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (animTime >= totalTime)
	{
		dodgeStarted = false;

		velocity = VGet(0, 0, 0);

		aiState = EnemyAIState::Chase;

		currentState = AnimState::Chase;
	}
}

bool Enemy::UpdateDead()
{
	if (currentState != AnimState::Dead)
		return false;

	if (deadFinished)
		return true;

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (animTime >= totalTime)
	{
		animTime = totalTime;

		isDead = true;

		deadFinished = true;

		MV1SetVisible(handle, FALSE);
	}

	return true;
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

		attackTimer = 0.0f;

		aiState = EnemyAIState::Chase;
		currentState = AnimState::Chase;
	}
}

void Enemy::UpdateCooldown(float dt, float distance)
{
	VECTOR dir = VSub(targetPlayerPos, pos);
	dir.y = 0.0f;

	if (VSize(dir) > 0.01f)
	{
		dir = VNorm(dir);

		float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

		float diff = targetAngle - characterAngle;

		while (diff > DX_PI) diff -= DX_TWO_PI;
		while (diff < -DX_PI) diff += DX_TWO_PI;

		characterAngle += diff * 10.0f * dt;
	}

	VECTOR forward =
	{
		-sinf(characterAngle),
		0.0f,
		-cosf(characterAngle)
	};

	VECTOR right =
	{
		forward.z,
		0.0f,
		-forward.x
	};

	velocity = VGet(0, 0, 0);

	if (cooldownMoveTimer > 0.0f)
	{
		cooldownMoveTimer -= dt;

		if (weaponType == EnemyWeaponType::Gun &&
			distance <= 150.0f &&
			cooldownMove == CooldownMove::Forward)
		{
			int r = GetRand(2);

			switch (r)
			{
			case 0:
				cooldownMove = CooldownMove::Back;
				break;

			case 1:
				cooldownMove = CooldownMove::Left;
				break;

			case 2:
				cooldownMove = CooldownMove::Right;
				break;
			}
		}

		switch (cooldownMove)
		{
		case CooldownMove::Forward:
			velocity = VScale(forward, speed);
			ChangeAnimation(chase_fAnim, true);
			break;

		case CooldownMove::Back:
			velocity = VScale(forward, -speed);
			ChangeAnimation(chase_bAnim, true);
			break;

		case CooldownMove::Left:
			velocity = VScale(right, -speed);
			ChangeAnimation(chase_lAnim, true);
			break;

		case CooldownMove::Right:
			velocity = VScale(right, speed);
			ChangeAnimation(chase_rAnim, true);
			break;
		}
	}

	if (attackTimer > 0.0f || cooldownMoveTimer > 0.0f)
		return;

	if (lostTimer <= 0.0f)
	{
		GeneratePatrolTarget();

		aiState = EnemyAIState::Patrol;

		return;
	}

	aiState = EnemyAIState::Chase;
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

void Enemy::TryDodge(Player* player, float distance)
{
	if (currentState == AnimState::Attack ||
		currentState == AnimState::Dodge ||
		currentState == AnimState::Dead ||
		currentState == AnimState::Hit)
		return;

	if (!player->IsAttackActive())
	{
		dodgeStarted = false;
		return;
	}

	if (dodgeStarted)
		return;

	// 視界外なら回避しない
	if (!CanSeePlayer(player->GetPos()))
		return;

	// 距離が遠いなら回避しない
	float dodgeDistance =
		playerWeapon->attackDistance +
		playerWeapon->attackRadius + 50.0f;

	if (distance > dodgeDistance)
		return;

	dodgeStarted = true;

	float dodgeProb = currentWeapon->dodgeProbability;

	if (player->GetCurrentWeaponData()->attackShape == AttackShape::Gun)
	{
		dodgeProb = currentWeapon->gunDodgeProbability;
	}

	float r = GetRand(999) / 999.0f;

	if (r < dodgeProb)
	{
		aiState = EnemyAIState::Dodge;

		currentState = AnimState::Dodge;

		if (isGun)
		{
			int type = GetRand(1);

			if (type == 0)
			{
				ChangeAnimation(dodge_rAnim, false);
				dodgeDir = MoveDirection::Right;
			}
			else
			{
				ChangeAnimation(dodge_lAnim, false);
				dodgeDir = MoveDirection::Left;
			}
		}
		else
		{
			int type = GetRand(2);

			switch (type)
			{
			case 0:
				ChangeAnimation(dodge_b01Anim, false);
				dodgeDir = MoveDirection::Back;
				break;

			case 1:
				ChangeAnimation(dodge_rAnim, false);
				dodgeDir = MoveDirection::Right;
				break;

			case 2:
				ChangeAnimation(dodge_lAnim, false);
				dodgeDir = MoveDirection::Left;
				break;
			}
		}
	}
}