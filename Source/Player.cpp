#include <cmath>
#include <algorithm>
#include <limits>
#include <cfloat>
#include "Player.h"
#include "PhysicsManager.h"
#include "CollisionWorld.h"
#include "Enemy.h"
#include "Collision.h"
#include "Stage.h"
#include "Camera.h"

namespace
{
	float Clamp(float v, float min, float max)
	{
		if (v < min)return min;
		if (v > max)return max;
		return v;
	}

	bool LineSphereHit(const VECTOR& start, const VECTOR& end, const VECTOR& center, float radius)
	{
		VECTOR ab = VSub(end, start);
		VECTOR ac = VSub(center, start);

		float abLenSq = VDot(ab, ab);

		if (abLenSq < 0.0001f)
		{
			return false;
		}

		float t = VDot(ac, ab) / abLenSq;

		t = Clamp(t, 0.0f, 1.0f);

		VECTOR nearest = { start.x + ab.x * t,start.y + ab.y * t,start.z + ab.z * t };

		VECTOR diff = VSub(center, nearest);

		float distSq = VDot(diff, diff);

		return distSq <= radius * radius;
	}
}

void Player::Init(CollisionWorld* w, Stage* s, Camera* c)
{
	world = w;
	stage = s;
	camera = c;

	GetMousePoint(&prevMouseX, &prevMouseY);

	// ===== モデル読み込み =====
	CharacterBase::Init("Assets/mv1model/Player2.mv1");

	// ===== 武器モデル読み込み =====
	weapon1.Init("Assets/mv1model/Blade.mv1");
	weapon2.Init("Assets/mv1model/Gun01.mv1");

	// ===== アニメーション番号 =====
	idleAnim = 0;
	walk_fAnim = 1;
	walk_bAnim = 2;
	walk_rAnim = 3;
	walk_lAnim = 4;

	dash_f01Anim = 5;
	dash_f02Anim = 6;
	dash_rAnim = 7;
	dash_lAnim = 8;

	jumpStartAnim = 9;
	dashJumpStartAnim = 10;
	jumpRiseAnim = 11;
	jumpFallAnim = 12;
	jumpEndAnim = 13;
	handAttackAnim = 14; 
	swordAttack01Anim = 15; 
	swordAttack02Anim = 16; 
	swordAttack03Anim = 17; 
	swordAttack04Anim = 18;
	gunAttackAnim = 19; 
	hitAnim = 20;

	dodge_fAnim = 21;
	dodge_b01Anim = 22;
	dodge_b02Anim = 23;

	deadAnim = 24;

	// ===== 武器データ設定 =====
	// 素手
	unarmedData.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	unarmedData.lockDashFrontAnim = dash_f01Anim;
	unarmedData.lockDashRightAnim = dash_rAnim;
	unarmedData.lockDashLeftAnim = dash_lAnim;

	unarmedData.lockDodgeAnim[(int)MoveDirection::Front] = dodge_fAnim;
	unarmedData.lockDodgeAnim[(int)MoveDirection::Back] = dodge_b01Anim;
	unarmedData.lockDodgeAnim[(int)MoveDirection::Right] = dodge_fAnim;
	unarmedData.lockDodgeAnim[(int)MoveDirection::Left] = dodge_fAnim;

	for (int i = 0; i < WeaponData::MaxCombo; i++)
	{
		unarmedData.attackAnim[i] = handAttackAnim;

		unarmedData.attackStartFrame[i] = 50.0f;
		unarmedData.attackEndFrame[i] = 100.0f;

		unarmedData.comboAcceptStartFrame[i] = 0.0f;
		unarmedData.comboAcceptEndFrame[i] = 0.0f;

		unarmedData.comboCancelFrame[i] = 0.0f;
	}

	unarmedData.comboCount = 1;

	unarmedData.dashAnim = dash_f01Anim;

	unarmedData.attackShape = AttackShape::Sphere;
	unarmedData.attackRadius = 40.0f;
	unarmedData.attackDistance = 120.0f;
	unarmedData.attackOffset = VGet(0, 115, 0);

	unarmedData.followAttack = true;

	// weapon1（剣）
	weapon1Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon1Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon1Data.attackAnim[0] = swordAttack01Anim;
	weapon1Data.attackAnim[1] = swordAttack02Anim;
	weapon1Data.attackAnim[2] = swordAttack03Anim;
	weapon1Data.attackAnim[3] = swordAttack04Anim;

	weapon1Data.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	weapon1Data.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	weapon1Data.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	weapon1Data.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	weapon1Data.lockDashFrontAnim = dash_f02Anim;
	weapon1Data.lockDashRightAnim = dash_rAnim;
	weapon1Data.lockDashLeftAnim = dash_lAnim;

	weapon1Data.lockDodgeAnim[(int)MoveDirection::Front] = dodge_fAnim;
	weapon1Data.lockDodgeAnim[(int)MoveDirection::Back] = dodge_b01Anim;
	weapon1Data.lockDodgeAnim[(int)MoveDirection::Right] = dodge_fAnim;
	weapon1Data.lockDodgeAnim[(int)MoveDirection::Left] = dodge_fAnim;

	weapon1Data.comboCount = 4;

	weapon1Data.dashAnim = dash_f02Anim;

	weapon1Data.attackShape = AttackShape::Line;
	weapon1Data.attackRadius = 20.0f;
	weapon1Data.attackDistance = 160.0f;
	weapon1Data.attackOffset = VGet(0, 115, 0);

	weapon1Data.followAttack = true;

	// 攻撃コンボ　攻撃開始・終了フレーム
	weapon1Data.attackStartFrame[0] = 30.0f;
	weapon1Data.attackEndFrame[0] = 57.0f;

	weapon1Data.attackStartFrame[1] = 3.0f;
	weapon1Data.attackEndFrame[1] = 29.0f;

	weapon1Data.attackStartFrame[2] = 41.0f;
	weapon1Data.attackEndFrame[2] = 73.0f;

	weapon1Data.attackStartFrame[3] = 46.0f;
	weapon1Data.attackEndFrame[3] = 85.0f;

	// 攻撃コンボ　受付フレーム
	weapon1Data.comboAcceptStartFrame[0] = 43.0f;
	weapon1Data.comboAcceptEndFrame[0] = 65.0f;

	weapon1Data.comboAcceptStartFrame[1] = 20.0f;
	weapon1Data.comboAcceptEndFrame[1] = 44.0f;

	weapon1Data.comboAcceptStartFrame[2] = 47.0f;
	weapon1Data.comboAcceptEndFrame[2] = 97.0f;

	weapon1Data.comboAcceptStartFrame[3] = 0.0f;
	weapon1Data.comboAcceptEndFrame[3] = 0.0f;

	// 攻撃コンボ　コンボキャンセルフレーム
	weapon1Data.comboCancelFrame[0] = 53.0f;
	weapon1Data.comboCancelFrame[1] = 36.0f;
	weapon1Data.comboCancelFrame[2] = 73.0f;
	weapon1Data.comboCancelFrame[3] = 999.0f;

	weapon1.SetData(weapon1Data);

	// weapon2（銃）
	weapon2Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon2Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon2Data.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	weapon2Data.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	weapon2Data.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	weapon2Data.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	weapon2Data.lockDashFrontAnim = dash_f01Anim;
	weapon2Data.lockDashRightAnim = dash_rAnim;
	weapon2Data.lockDashLeftAnim = dash_lAnim;

	weapon2Data.lockDodgeAnim[(int)MoveDirection::Front] = dodge_fAnim;
	weapon2Data.lockDodgeAnim[(int)MoveDirection::Back] = dodge_b01Anim;
	weapon2Data.lockDodgeAnim[(int)MoveDirection::Right] = dodge_fAnim;
	weapon2Data.lockDodgeAnim[(int)MoveDirection::Left] = dodge_fAnim;

	for (int i = 0; i < WeaponData::MaxCombo; i++)
	{
		weapon2Data.attackAnim[i] = gunAttackAnim;

		weapon2Data.attackStartFrame[i] = 10.0f;
		weapon2Data.attackEndFrame[i] = 20.0f;

		weapon2Data.comboAcceptStartFrame[i] = 0.0f;
		weapon2Data.comboAcceptEndFrame[i] = 0.0f;

		weapon2Data.comboCancelFrame[i] = 0.0f;
	}

	weapon2Data.comboCount = 1;

	weapon2Data.dashAnim = dash_f01Anim;

	weapon2Data.attackShape = AttackShape::Gun;

	weapon2Data.attackRadius = 10.0f;
	weapon2Data.attackDistance = 600.0f;
	weapon2Data.attackOffset = VGet(0, 115, 0);

	weapon2Data.followAttack = false;

	weapon2.SetData(weapon2Data);

	// ===== 当たり判定サイズ =====
	radius = 10.0f;
	height = 140.0f;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;

	ChangeAnimation(idleAnim, true);

	// 初期装備
	equipState = EquipState::Unarmed;

	currentWeaponData = &unarmedData;
}

void Player::Update(float dt, float cameraAngle, PhysicsManager& physics)
{
	//　=====　武器更新　=====
	switch (equipState)
	{
	case EquipState::Weapon1:
		weapon1.Update(handle, "mixamorig:RightHand", characterAngle);
		break;

	case EquipState::Weapon2:
		weapon2.Update(handle, "mixamorig:RightHand", characterAngle);
		break;

	case EquipState::Unarmed:
		break;
	}

	if (isDead)
	{
		// ===== モデル非表示 =====
		MV1SetVisible(handle, FALSE);

		return;
	}

	if (isLockOn && lockOnTarget)
	{
		if (lockOnTarget->IsDead())
		{
			isLockOn = false;
			lockOnTarget = nullptr;
		}

		VECTOR diff = VSub(lockOnTarget->GetCenterPos(), GetCenterPos());

		diff.y = 0.0f;

		if (VSize(diff) > lockOnDistance)
		{
			isLockOn = false;
			lockOnTarget = nullptr;
		}
	}

	if (isDodging)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 回避移動区間
		if (animTime >= dodgeStartFrame && animTime <= dodgeEndFrame)
		{
			velocity.x = dodgeMoveDir.x * dodgeSpeed;
			velocity.z = dodgeMoveDir.z * dodgeSpeed;
		}
		else
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}

		// アニメ終了
		if (animTime >= totalTime)
		{
			isDodging = false;

			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}

		physics.MoveCharacter(pos, velocity, radius, isGround, dt);

		UpdateAnimation(dt);

		return;
	}

	//　=====　ノックバック＆ヒットストップ処理　=====
	if (currentState == AnimState::Hit)
	{
		if (isKnockback)
		{
			knockbackTimer -= dt;

			if (knockbackTimer <= 0.0f)
			{
				isKnockback = false;
				velocity = VGet(0, velocity.y, 0);
			}
		}

		physics.MoveCharacter(pos, velocity, radius, isGround, dt);

		UpdateState();
		UpdateAnimation(dt);

		return;
	}

	//　=====　入力処理　=====
	UpdateInput(dt, cameraAngle, stage->GetEnemies());

	//　=====　物理　=====
	physics.MoveCharacter(pos, velocity, radius, isGround, dt);

	//　=====　状態更新　=====
	UpdateState();

	//　=====　アニメーション更新　=====
	UpdateAnimation(dt);

	VECTOR drawPos = pos;
	drawPos.y -= 30; // 足元補正

	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));

	//　=====　武器切り替え　=====
	if (CheckHitKey(KEY_INPUT_1))
	{
		EquipWeapon1();
	}
	if (CheckHitKey(KEY_INPUT_2))
	{
		EquipWeapon2();
	}
	if (CheckHitKey(KEY_INPUT_3))
	{
		Unequip();
	}

	// ガン遅延ダメージ
	if (gunWaitingDamage)
	{
		gunTimer -= dt;

		if (gunTimer <= 0.0f)
		{
			for (Enemy* enemy : gunTargets)
			{
				if (enemy && !enemy->IsDead())
				{
					enemy->Damage(20);
				}
			}

			gunTargets.clear();
			gunWaitingDamage = false;
		}
	}

	//　=====　接地保存　=====
	prevGround = isGround;
}

void Player::Draw()
{
	VECTOR drawPos = pos;
	drawPos.y -= 30.0f;

	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));

	MV1DrawModel(handle);

	//　=====　武器表示　=====
	switch (equipState)
	{
	case EquipState::Weapon1:
		weapon1.Draw();
		break;

	case EquipState::Weapon2:
		weapon2.Draw();
		break;

	case EquipState::Unarmed:
		break;
	}
}

// ==========================================
// 装備変更
// ==========================================
void Player::EquipWeapon1()
{
	equipState = EquipState::Weapon1;

	currentWeaponData = &weapon1Data;
}

void Player::EquipWeapon2()
{
	equipState = EquipState::Weapon2;

	currentWeaponData = &weapon2Data;
}

void Player::Unequip()
{
	equipState = EquipState::Unarmed;

	currentWeaponData = &unarmedData;
}

void Player::UpdateAttackPos()
{
	// ===== 攻撃当たり判定 =====
		// 前方向
	VECTOR forward = GetForward();

	attackPos.x = pos.x + forward.x * currentWeaponData->attackDistance + currentWeaponData->attackOffset.x;
	attackPos.y = pos.y + currentWeaponData->attackOffset.y;
	attackPos.z = pos.z + forward.z * currentWeaponData->attackDistance + currentWeaponData->attackOffset.z;
}

void Player::CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// 攻撃判定フレーム
	if (!attackActive) return;

	// 既にヒット済み
	if (attackHit) return;

	// ===== 追従型当たり判定 =====
	if (currentWeaponData->followAttack)
	{
		UpdateAttackPos();
	}

	switch (currentWeaponData->attackShape)
	{
		case AttackShape::Sphere:
		{
			UpdateAttackPos();

			// Enemyチェック
			for (auto& enemy : enemies)
			{
				if (enemy->IsDead())
				{
					continue;
				}

				VECTOR epos = enemy->GetPos();

				epos.y += enemy->GetHeight() * 0.75f;

				float dx = epos.x - attackPos.x;
				float dy = epos.y - attackPos.y;
				float dz = epos.z - attackPos.z;

				float distanceSq = dx * dx + dy * dy + dz * dz;

				float radius = currentWeaponData->attackRadius;

				if (distanceSq <= radius * radius)
				{
					enemy->Damage(20);

					attackHit = true;
					break;
				}
			}

			break;
		}

		case AttackShape::Line:
		{
			VECTOR prevRoot = weapon1.GetPrevRootPosition();
			VECTOR root = weapon1.GetRootPosition();

			VECTOR prevTip = weapon1.GetPrevTipPosition();
			VECTOR tip = weapon1.GetTipPosition();

			for (auto& enemy : enemies)
			{
				if (enemy->IsDead())
					continue;

				VECTOR bottom = enemy->GetPos();

				VECTOR top = bottom;
				top.y += enemy->GetHeight();

				float radius = enemy->GetRadius();

				bool hit =
					LineCapsuleHit(prevRoot, prevTip, bottom, top, radius) ||
					LineCapsuleHit(root, tip, bottom, top, radius) ||
					LineCapsuleHit(prevTip, tip, bottom, top, radius) ||
					LineCapsuleHit(prevRoot, root, bottom, top, radius);

				if (hit)
				{
					enemy->Damage(20);

					attackHit = true;
					break;
				}
			}

			break;
		}

		case AttackShape::Gun:
		{
			if (!gunWaitingDamage)
			{
				gunTargets = FindGunTargets(enemies);

				if (!gunTargets.empty())
				{
					gunTimer = 2.0f;
					gunWaitingDamage = true;

					attackHit = true;
				}
			}

			break;
		}
	}
}

void Player::Damage(int power)
{
	if (isDodging)
		return;

	if (isInvincible)
		return;

	if (isDead || isDying)
		return;

	hp -= power;

	isKnockback = true;
	knockbackTimer = knockbackDuration;

	VECTOR back = GetForward();

	velocity.x = -back.x * 350.0f;
	velocity.z = -back.z * 350.0f;

	// ==== 死亡処理 ====
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

void Player::StartDodge(float cameraAngle)
{
	if (isDodging)
		return;

	if (!isGround)
		return;

	isDash = false;

	isDodging = true;

	currentState = AnimState::Dodge;

	// ===== 回避方向保存 =====
	dodgeDirection = GetLockMoveDirection();

	if (isLockOn)
	{
		dodgeMoveDir = GetLockMoveVector();

		switch (dodgeDirection)
		{
		case MoveDirection::None:
			dodgeMoveDir = VScale(GetForward(), -1.0f);
			ChangeAnimation(dodge_b02Anim, false);
			break;

		case MoveDirection::Front:
			ChangeAnimation(dodge_fAnim, false);
			break;

		case MoveDirection::Left:
		{
			VECTOR f = GetForward();

			VECTOR left;
			left.x = -f.z;
			left.y = 0.0f;
			left.z = f.x;

			dodgeMoveDir = VNorm(left);

			ChangeAnimation(dodge_fAnim, false);
		}
		break;

		case MoveDirection::Right:
		{
			VECTOR f = GetForward();

			VECTOR right;
			right.x = f.z;
			right.y = 0.0f;
			right.z = -f.x;

			dodgeMoveDir = VNorm(right);

			ChangeAnimation(dodge_fAnim, false);
		}
		break;

		case MoveDirection::Back:
			ChangeAnimation(dodge_b01Anim, false);
			break;
		}
	}
	else
	{
		bool noInput =
			fabsf(inputMoveX) < 0.01f &&
			fabsf(inputMoveZ) < 0.01f;

		if (noInput)
		{
			dodgeMoveDir = VScale(GetForward(), -1.0f);

			ChangeAnimation(dodge_b02Anim, false);
		}
		else
		{
			dodgeMoveDir = GetFreeMoveVector(cameraAngle);

			float targetAngle = atan2f(dodgeMoveDir.x, dodgeMoveDir.z) + DX_PI;

			characterAngle = targetAngle;

			ChangeAnimation(dodge_fAnim, false);
		}
	}

	SetAnimSpeed(1.5f);
}

void Player::LockOn(std::vector<std::unique_ptr<Enemy>>& enemies)
{
	lockOnTarget = nullptr;

	float nearestDistSq = FLT_MAX;

	for (auto& enemy : enemies)
	{
		if (enemy->IsDead())
			continue;

		VECTOR diff = VSub(enemy->GetCenterPos(), GetCenterPos());

		// Yは無視
		diff.y = 0.0f;

		float distSq = VDot(diff, diff);

		if (distSq > lockOnDistance * lockOnDistance)
			continue;

		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			lockOnTarget = enemy.get();
		}
	}

	isLockOn = (lockOnTarget != nullptr);

	if (camera)
	{
		camera->SetLockTarget(lockOnTarget);
	}
}

void Player::SwitchLockTarget(std::vector<std::unique_ptr<Enemy>>& enemies, bool right)
{
	if (!lockOnTarget)
		return;

	VECTOR playerPos = GetCenterPos();

	VECTOR current = VSub(lockOnTarget->GetCenterPos(), playerPos);

	current.y = 0.0f;

	current = VNorm(current);

	Enemy* best = nullptr;

	float bestAngle = DX_PI;

	for (auto& enemy : enemies)
	{
		if (enemy.get() == lockOnTarget)
			continue;

		if (enemy->IsDead())
			continue;

		VECTOR dir = VSub(enemy->GetCenterPos(), playerPos);

		dir.y = 0.0f;

		float dist = VSize(dir);

		if (dist > lockOnDistance)
			continue;

		dir = VNorm(dir);

		float angleCurrent = atan2f(current.x, current.z);

		float angleNew = atan2f(dir.x, dir.z);

		float diff = angleNew - angleCurrent;

		while (diff > DX_PI)
			diff -= DX_TWO_PI;

		while (diff < -DX_PI)
			diff += DX_TWO_PI;

		if (right)
		{
			if (diff <= 0.0f)
				continue;
		}
		else
		{
			if (diff >= 0.0f)
				continue;
		}

		float absDiff = fabsf(diff);

		if (absDiff < bestAngle)
		{
			bestAngle = absDiff;

			best = enemy.get();
		}
	}

	if (best)
	{
		lockOnTarget = best;

		if (camera)
		{
			camera->SetLockTarget(lockOnTarget);
		}
	}
}

VECTOR Player::GetCenterPos() const
{
	return VGet(pos.x, pos.y + height * 0.75f, pos.z);
}

const VECTOR& Player::GetVelocity() const
{
	return velocity;
}

VECTOR& Player::GetVelocity()
{
	return velocity;
}

void Player::SetVelocity(const VECTOR& v)
{
	velocity = v;
}

VECTOR Player::GetForward() const
{
	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	return forward;
}

void Player::DebugDraw()
{
	// ===== プレイヤー座標 =====
	DrawFormatString(
		20, 180,
		GetColor(255, 255, 255),
		"Player Pos : X=%.2f Y=%.2f Z=%.2f",
		pos.x, pos.y, pos.z
	);

	// ===== 速度 =====
	DrawFormatString(
		20, 200,
		GetColor(255, 255, 0),
		"Velocity   : X=%.2f Y=%.2f Z=%.2f",
		velocity.x, velocity.y, velocity.z
	);

	// ===== 接地状態 =====
	DrawFormatString(
		20, 220,
		GetColor(0, 255, 0),
		"IsGround : %d",
		isGround
	);

	DrawFormatString(
		20, 240,
		GetColor(255, 255, 255),
		"AttackRadius : %.2f",
		currentWeaponData->attackRadius
	);

	DrawFormatString(
		20, 260,
		GetColor(255, 255, 255),
		"AttackPos : %.2f %.2f %.2f",
		attackPos.x,
		attackPos.y,
		attackPos.z
	);

	DrawFormatString(
		20, 280,
		GetColor(255, 255, 255),
		"AttackActive : %d",
		attackActive
	);

	// ガン
	DrawFormatString(
		20,
		300,
		GetColor(255, 255, 0),
		"GunTimer : %.2f",
		gunTimer
	);

	DrawFormatString(
		20,
		320,
		GetColor(255, 255, 0),
		"GunWaiting : %d",
		gunWaitingDamage
	);

	// ダッシュジャンプ
	DrawFormatString(
		20,
		380,
		GetColor(255, 255, 255),
		"dashJump : %d",
		dashJump
	);

	DrawFormatString(
		20,
		400,
		GetColor(255, 255, 255),
		"State : %d",
		(int)currentState);

	DrawFormatString(
		20,
		420,
		GetColor(255, 255, 255),
		"AnimIndex : %d",
		currentAnimIndex);

	DrawFormatString(
		20,
		440,
		GetColor(255, 255, 255),
		"AnimTime : %.2f",
		animTime);

	if (currentAnimAttach != -1)
	{
		DrawFormatString(
			20,
			460,
			GetColor(255, 255, 255),
			"TotalTime : %.2f",
			MV1GetAttachAnimTotalTime(
				handle,
				currentAnimAttach));
	}

	DrawFormatString(
		20,
		480,
		GetColor(255, 255, 255),
		"Ground : %d",
		isGround);

	DrawFormatString(
		20,
		500,
		GetColor(255, 255, 255),
		"PrevGround : %d",
		prevGround);

	DrawFormatString(
		20, 540,
		GetColor(255, 255, 255),
		"CurrentAttach=%d PrevAttach=%d",
		currentAnimAttach,
		prevAnimAttach);

	DrawFormatString(
		20, 560,
		GetColor(255, 255, 255),
		"Blending = %d",
		isBlending);

	DrawFormatString(
		20, 580,
		GetColor(255, 255, 255),
		"Vy = %.2f",
		velocity.y);

	DrawFormatString(
		1100,
		20,
		GetColor(255, 255, 255),
		"Combo : %d",
		comboStep + 1);

	DrawFormatString(
		1100,
		40,
		GetColor(255, 255, 255),
		"ComboNext : %d",
		comboNext);

	DrawFormatString(
		1100,
		60,
		GetColor(255, 255, 255),
		"ComboWindow : %d",
		animTime >= currentWeaponData->comboAcceptStartFrame[comboStep] &&
		animTime <= currentWeaponData->comboAcceptEndFrame[comboStep]);

	DrawFormatString(
		900,
		80,
		GetColor(255, 255, 255),
		"DodgeDir X = % .2f Z = % .2f",
		dodgeMoveDir.x,
		dodgeMoveDir.z);
}

void Player::DrawCapsuleDebug(std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// 足元方式なのでそのまま基準
	VECTOR bottom = pos;

	VECTOR top = pos;
	top.y += height;

	// 下（足元）
	DrawSphere3D(bottom, radius, 12,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		FALSE);

	// 上（頭）
	DrawSphere3D(top, radius, 12,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		FALSE);

	// 中心（見た目用）
	VECTOR center = pos;
	center.y += height * 0.5f;

	DrawSphere3D(center, 3.0f, 8,
		GetColor(255, 255, 0),
		GetColor(255, 255, 0),
		FALSE);

	// 縦ライン
	DrawLine3D(bottom, top, GetColor(0, 255, 0));

	// 攻撃範囲（球）
	// ===== Player attack sphere =====
	DrawSphere3D(
		attackPos,
		currentWeaponData->attackRadius,
		16,
		GetColor(0, 255, 0),
		GetColor(0, 255, 0),
		FALSE
	);

	float hitRadius = 10.0f;

	// 前フレームの剣
	DrawCapsule3D(
		weapon1.GetPrevRootPosition(),
		weapon1.GetPrevTipPosition(),
		hitRadius,
		8,
		GetColor(255, 255, 0),
		GetColor(255, 255, 0), FALSE
	);

	// 現在フレームの剣
	DrawCapsule3D(
		weapon1.GetRootPosition(),
		weapon1.GetTipPosition(),
		hitRadius,
		8,
		GetColor(0, 255, 0),
		GetColor(0, 255, 0),
		FALSE
	);

	// 剣先の移動軌跡
	DrawCapsule3D(
		weapon1.GetPrevRootPosition(),
		weapon1.GetRootPosition(),
		hitRadius,
		8,
		GetColor(0, 255, 255),
		GetColor(0, 255, 255),
		FALSE
	);

	// 根元の移動軌跡
	DrawCapsule3D(
		weapon1.GetPrevTipPosition(),
		weapon1.GetTipPosition(),
		hitRadius,
		8,
		GetColor(255, 0, 255),
		GetColor(255, 0, 255),
		FALSE
	);

	// 銃ターゲットマーカー
	for (Enemy* enemy : gunTargets)
	{
		if (!enemy || enemy->IsDead())
			continue;

		VECTOR pos = enemy->GetPos();
		pos.y += enemy->GetHeight() + 20.0f;

		DrawSphere3D(
			pos,
			8.0f,
			8,
			GetColor(255,0,0),
			GetColor(255,0,0),
			TRUE
		);
	}

	// ===== Enemy positions =====
	for (auto& enemy : enemies)
	{
		if (enemy->IsDead()) continue;

		VECTOR epos = enemy->GetPos();

		// 高さ補正（Enemyと合わせる）
		epos.y += 70.0f;

		DrawSphere3D(
			epos,
			10.0f,
			8,
			GetColor(255, 0, 0),
			GetColor(255, 0, 0),
			TRUE
		);
	}
}

// 敵探索
std::vector<Enemy*> Player::FindGunTargets(std::vector<std::unique_ptr< Enemy >>& enemies)
{
	struct Candidate
	{
		Enemy* enemy;
		float score;
	};

	std::vector<Candidate> candiates;

	VECTOR forward = GetForward();

	float limit = cosf(currentWeaponData->lockAngle * DX_PI_F / 180.0f);

	for (auto& enemy : enemies)
	{
		if (enemy->IsDead())
			continue;

		VECTOR center = enemy->GetPos();
		center.y += enemy->GetHeight() * 0.5f;

		VECTOR toEnemy = VSub(center, pos);

		float dist = VSize(toEnemy);

		// 射程
		float gunRange = currentWeaponData->attackDistance;

		// 射程外
		if (dist > gunRange)
			continue;

		VECTOR dir = VNorm(toEnemy);

		float dot = VDot(forward, dir);

		// 視界外
		if (dot < limit)
			continue;

		Candidate c;

		c.enemy = enemy.get();

		// 画面中央に近いほど高得点
		c.score = dot;

		candiates.push_back(c);
	}

	// 中央優先でソート
	std::sort(candiates.begin(), candiates.end(), [](const Candidate& a, const Candidate& b)
		{
			return a.score > b.score;
		}
	);

	std::vector<Enemy*> result;

	const int MAX_LOCK = 5;

	for (size_t i = 0; i < candiates.size() && i < MAX_LOCK; i++)
	{
		result.push_back(candiates[i].enemy);
	}

	return result;
}

MoveDirection Player::GetLockMoveDirection() const
{
	// 入力なし
	if (fabsf(inputMoveX) < 0.01f &&
		fabsf(inputMoveZ) < 0.01f)
	{
		return MoveDirection::None;
	}

	// 前後優先
	if (fabsf(inputMoveZ) >= fabsf(inputMoveX))
	{
		if (inputMoveZ > 0.0f)
		{
			return MoveDirection::Front;
		}
		else
		{
			return MoveDirection::Back;
		}
	}
	else
	{
		if (inputMoveX > 0.0f)
		{
			return MoveDirection::Right;
		}
		else
		{
			return MoveDirection::Left;
		}
	}
}

VECTOR Player::GetLockMoveVector() const
{
	// Player前方向
	VECTOR forward = GetForward();
	forward.y = 0.0f;

	if (VSize(forward) > 0.001f)
	{
		forward = VNorm(forward);
	}

	// Player右方向
	VECTOR right;
	right.x = forward.z;
	right.y = 0.0f;
	right.z = -forward.x;

	VECTOR moveDir = VGet(0, 0, 0);

	if (CheckHitKey(KEY_INPUT_W))
	{
		moveDir = forward;
	}
	else if (CheckHitKey(KEY_INPUT_S))
	{
		moveDir = VScale(forward, -1.0f);
	}
	else if (CheckHitKey(KEY_INPUT_A))
	{
		moveDir = VScale(right, -1.0f);
	}
	else if (CheckHitKey(KEY_INPUT_D))
	{
		moveDir = right;
	}
	else
	{
		moveDir = VGet(0, 0, 0);
	}

	return moveDir;
}

VECTOR Player::GetFreeMoveVector(float cameraAngle)
{
	VECTOR dir = VGet(0, 0, 0);

	float x = inputMoveX;
	float z = inputMoveZ;

	float len = sqrtf(x * x + z * z);

	if (len > 0.001f)
	{
		x /= len;
		z /= len;
	}

	float sinY = sinf(cameraAngle);
	float cosY = cosf(cameraAngle);

	dir.x = x * cosY + z * sinY;
	dir.z = z * cosY - x * sinY;

	return dir;
}

void Player::UpdateInput(float dt, float cameraAngle, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	inputMoveX = 0.0f;
	inputMoveZ = 0.0f;

	//　=====　入力　=====	
	if (CheckHitKey(KEY_INPUT_W)) inputMoveZ += 1.0f;
	if (CheckHitKey(KEY_INPUT_S)) inputMoveZ -= 1.0f;
	if (CheckHitKey(KEY_INPUT_D)) inputMoveX += 1.0f;
	if (CheckHitKey(KEY_INPUT_A)) inputMoveX -= 1.0f;

	//　=====　移動判定　=====
	bool isMove = (inputMoveX != 0.0f || inputMoveZ != 0.0f);

	// ===== ロックオン =====
	bool qNow = CheckHitKey(KEY_INPUT_Q);

	if (qNow && !oldQ)
	{
		if (!isLockOn)
		{
			LockOn(enemies);
		}
		else
		{
			isLockOn = false;
			lockOnTarget = nullptr;

			if (camera)
			{
				camera->ClearLockTarget();
			}
		}
	}

	oldQ = qNow;

	// マウス移動量取得
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	int dx = mouseX - prevMouseX;
	int dy = mouseY - prevMouseY;

	prevMouseX = mouseX;
	prevMouseY = mouseY;

	// マウス左右でターゲット切替
	if (isLockOn)
	{
		if (dx > switchThreshold && canSwitchTarget)
		{
			// 右のEnemy
			SwitchLockTarget(enemies, true);

			canSwitchTarget = false;
		}
		else if (dx < -switchThreshold && canSwitchTarget)
		{
			// 左のEnemy
			SwitchLockTarget(enemies, false);

			canSwitchTarget = false;
		}

		// マウスが止まったら再度切替可能
		if (abs(dx) < 5)
		{
			canSwitchTarget = true;
		}
	}

	// ===== Shift押下状態取得 =====
	bool shiftNow = CheckHitKey(KEY_INPUT_LSHIFT) != 0;

	// 押した瞬間
	if (shiftNow && !shiftPressed)
	{
		shiftPressed = true;
		shiftHoldTimer = 0.0f;
	}

	// 押している間
	if (shiftNow && shiftPressed)
	{
		shiftHoldTimer += dt;

		// 長押しでダッシュ
		if (shiftHoldTimer >= dashHoldTime && !isDash)
		{
			isDash = true;

			dashDirection = GetLockMoveDirection();
			dashMoveDir = GetLockMoveVector();
		}
	}

	// 離した瞬間
	if (!shiftNow && shiftPressed)
	{
		// 短押しなら回避
		if (shiftHoldTimer < dashHoldTime)
		{
			// ===== 回避 =====
			StartDodge(cameraAngle);
		}

		// ダッシュ解除
		isDash = false;

		// Shift状態リセット
		shiftPressed = false;
		shiftHoldTimer = 0.0f;
	}

	// ===== 速度切り替え =====
	speed = isDash ? dashSpeed : walkSpeed;

	// ===== マウスクリック =====
	int mouse = GetMouseInput();

	bool leftClick = (mouse & MOUSE_INPUT_LEFT) && !(oldMouse & MOUSE_INPUT_LEFT);

	oldMouse = mouse;

	// ===== 攻撃 =====
	if (leftClick)
	{
		// 攻撃開始
		if (currentState != AnimState::Attack && isGround)
		{
			comboStep = 0;
			comboNext = false;

			currentState = AnimState::Attack;

			// ===== 武器ごとの攻撃アニメ =====
			ChangeAnimation(currentWeaponData->attackAnim[comboStep], false);

			// ===== 固定型当たり判定 =====
			if (!currentWeaponData->followAttack)
			{
				UpdateAttackPos();
			}
		}
		else if (currentState == AnimState::Attack)  // コンボ予約
		{
			if (animTime >= currentWeaponData->comboAcceptStartFrame[comboStep] &&
				animTime <= currentWeaponData->comboAcceptEndFrame[comboStep])
			{
				comboNext = true;
			}
		}
	}

	// ===== ジャンプ =====
	if (CheckHitKey(KEY_INPUT_SPACE) && isGround)
	{
		currentState = AnimState::JumpStart;

		dashJump = isDash;

		if (dashJump)
		{
			// ダッシュ速度保存
			dashJumpVelocity.x = velocity.x;
			dashJumpVelocity.y = 0.0f;
			dashJumpVelocity.z = velocity.z;

			velocity.y = jumpPower;
			isGround = false;

			ChangeAnimation(dashJumpStartAnim, false);
		}
		else
		{
			jumpRequest = true;

			ChangeAnimation(jumpStartAnim, false);
		}
	}

	// ==== 移動 ====
	if (!(currentState == AnimState::JumpStart && dashJump) && !(currentState == AnimState::JumpRise && dashJump))
	{
		velocity.x = 0.0f;
		velocity.z = 0.0f;
	}

	if (currentState != AnimState::Attack && currentState != AnimState::JumpStart && currentState != AnimState::JumpEnd && isMove)
	{
		//　正規化
		float len = sqrtf(inputMoveX * inputMoveX + inputMoveZ * inputMoveZ);

		if (len > 0.001f)
		{
			inputMoveX /= len;
			inputMoveZ /= len;
		}

		//　=====　カメラ基準変換　=====
		float sinY = sinf(cameraAngle);
		float cosY = cosf(cameraAngle);

		VECTOR dir;

		dir.x = inputMoveX * cosY + inputMoveZ * sinY;
		dir.z = inputMoveZ * cosY - inputMoveX * sinY;

		velocity.x = dir.x * speed;
		velocity.z = dir.z * speed;
	}

	//　=====　回転　=====
	if (isLockOn && lockOnTarget != nullptr)
	{
		bool backDash =
			isDash && GetLockMoveDirection() == MoveDirection::Back;

		if (backDash)
		{
			VECTOR moveDir = GetLockMoveVector();

			if (VSize(moveDir) > 0.001f)
			{
				float targetAngle = atan2f(moveDir.x, moveDir.z) + DX_PI;

				float diff = targetAngle - characterAngle;

				while (diff > DX_PI)diff -= DX_TWO_PI;
				while (diff < -DX_PI)diff += DX_TWO_PI;

				characterAngle += diff * 12.0f * dt;
			}
		}
		else
		{
			// Enemy方向を向く
			VECTOR dir = VSub(lockOnTarget->GetCenterPos(), GetCenterPos());

			dir.y = 0.0f;

			if (VSize(dir) > 0.001f)
			{
				dir = VNorm(dir);

				float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

				float diff = targetAngle - characterAngle;

				while (diff > DX_PI)diff -= DX_TWO_PI;
				while (diff < -DX_PI)diff += DX_TWO_PI;

				characterAngle += diff * 10.0f * dt;
			}
		}

	}
	else if (isMove)
	{
		// 通常は移動方向を向く
		float len = sqrtf(inputMoveX * inputMoveX + inputMoveZ * inputMoveZ);

		if (len > 0.001f)
		{
			inputMoveX /= len;
			inputMoveZ /= len;
		}

		float sinY = sinf(cameraAngle);
		float cosY = cosf(cameraAngle);

		VECTOR dir;

		dir.x = inputMoveX * cosY + inputMoveZ * sinY;
		dir.z = inputMoveZ * cosY - inputMoveX * sinY;

		float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

		float diff = targetAngle - characterAngle;

		while (diff > DX_PI)diff -= DX_TWO_PI;
		while (diff < -DX_PI)diff += DX_TWO_PI;

		characterAngle += diff * 10.0f * dt;
	}
}

void Player::UpdateState()
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

	// ===== 回避中 =====
	if (isDodging)
	{
		return;
	}

	//　=====　JumpStart → JumpRise　=====
	if (currentState == AnimState::JumpStart)
	{
		// 通常ジャンプのみ待機
		if (!dashJump)
		{
			if (jumpRequest && animTime >= jumpStartFrame)
			{
				velocity.y = jumpPower;

				isGround = false;

				jumpRequest = false;

				currentState = AnimState::JumpRise;

				// 上昇アニメ開始
				ChangeAnimation(jumpRiseAnim, false);
			}
		}
		else
		{
			float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

			if (animTime >= totalTime)
			{
				currentState = AnimState::JumpRise;

				ChangeAnimation(jumpRiseAnim, false);
			}
		}

		return;
	}

	//　=====　JumpRise → JumpFall　=====
	if (currentState == AnimState::JumpRise)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 落下開始
		if (velocity.y <= 0.0f)
		{
			currentState = AnimState::JumpFall;

			ChangeAnimation(jumpFallAnim, false);

			return;
		}

		// 最終フレームで停止
		if (animTime >= totalTime)
		{
			animTime = totalTime;
		}

		return;
	}

	// ===== JumpFall =====
	if (currentState == AnimState::JumpFall)
	{
		// 着地
		if (!prevGround && isGround)
		{
			dashJump = false;
			currentState = AnimState::JumpEnd;

			ChangeAnimation(jumpEndAnim, false);

			return;
		}

		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 最終フレームで停止
		if (animTime >= totalTime)
		{
			animTime = totalTime;
		}

		return;
	}

	//　=====　Attack中　=====
	if (currentState == AnimState::Attack)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 攻撃判定ON区間
		if (animTime >= currentWeaponData->attackStartFrame[comboStep] && 
			animTime <= currentWeaponData->attackEndFrame[comboStep])
		{
			attackActive = true;
		}
		else
		{
			attackActive = false;
		}

		// ===== 次段コンボ =====
		// キャンセル可能になったら
		if (comboNext &&
			comboStep + 1 < currentWeaponData->comboCount &&
			animTime >= currentWeaponData->comboCancelFrame[comboStep])
		{
			comboStep++;
			comboNext = false;

			attackHit = false;

			ChangeAnimation(currentWeaponData->attackAnim[comboStep], false);

			return;
		}

		// ===== コンボ終了 =====
		if (animTime >= totalTime)
		{
			comboStep = 0;
			comboNext = false;

			attackHit = false;
			attackActive = false;

			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				currentState = AnimState::Walk;
				ChangeAnimation(walk_fAnim, true);
			}
			else
			{
				currentState = AnimState::Idle;
				ChangeAnimation(idleAnim, true);
			}
		}

		return;
	}

	// ===== 回避中 =====
	if (currentState == AnimState::Dodge)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime)
		{
			isDodging = false;

			SetAnimSpeed(1.0f);

			currentState = AnimState::Idle;

			ChangeAnimation(idleAnim, true);
		}

		return;
	}

	// ===== ロックオン移動 =====
	if (isLockOn)
	{
		MoveDirection dir = GetLockMoveDirection();

		// 毎フレーム方向更新
		if (dir != currentDirection)
		{
			currentDirection = dir;

			if (isDash)
			{
				switch (dir)
				{
				case MoveDirection::Front:
					ChangeAnimation(dash_f01Anim, true);
					break;

				case MoveDirection::Back:
					ChangeAnimation(dash_f01Anim, true);
					break;

				case MoveDirection::Left:
					ChangeAnimation(dash_lAnim, true);
					break;

				case MoveDirection::Right:
					ChangeAnimation(dash_rAnim, true);
					break;

				case MoveDirection::None:
					ChangeAnimation(idleAnim, true);
					break;
				}
			}
			else
			{
				switch (dir)
				{
				case MoveDirection::Front:
					ChangeAnimation(walk_fAnim, true);
					break;

				case MoveDirection::Back:
					ChangeAnimation(walk_bAnim, true);
					break;

				case MoveDirection::Left:
					ChangeAnimation(walk_lAnim, true);
					break;

				case MoveDirection::Right:
					ChangeAnimation(walk_rAnim, true);
					break;

				case MoveDirection::None:
					ChangeAnimation(idleAnim, true);
					break;
				}
			}
		}

		return;
	}

	// ===== ノックバック中 =====
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

	//　=====　JumpEnd終了　=====
	if (currentState == AnimState::JumpEnd)
	{
		// アニメ終了判定
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime)
		{
			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				if (isDash)
				{
					currentState = AnimState::Dash;

					// ===== 武器ごとのダッシュ =====
					ChangeAnimation(currentWeaponData->dashAnim, true);
				}
				else
				{
					currentState = AnimState::Walk;
					ChangeAnimation(walk_fAnim, true);
				}
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

	if (fabsf(velocity.x) > 0.1f ||
		fabsf(velocity.z) > 0.1f)
	{
		if (isDash)
		{
			nextState = AnimState::Dash;
		}
		else
		{
			nextState = AnimState::Walk;
		}
	}

	if (velocity.y < 0.0f)
	{
		nextState = AnimState::JumpFall;
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
			if (isLockOn)
			{
				MoveDirection dir = GetLockMoveDirection();

				ChangeAnimation(currentWeaponData->lockWalkAnim[(int)dir], true);
			}
			else
			{
				ChangeAnimation(walk_fAnim, true);
			}
			break;

		case AnimState::Dash:
			// ===== 武器ごとのダッシュ =====
			if (isLockOn)
			{
				MoveDirection dir = GetLockMoveDirection();

				switch (dir)
				{
				case MoveDirection::Front:
				case MoveDirection::Back:
					ChangeAnimation(currentWeaponData->lockDashFrontAnim, true);
					break;

				case MoveDirection::Right:
					ChangeAnimation(currentWeaponData->lockDashRightAnim, true);
					break;

				case MoveDirection::Left:
					ChangeAnimation(currentWeaponData->lockDashLeftAnim, true);
				}
			}
			else
			{
				ChangeAnimation(currentWeaponData->dashAnim, true);
			}
			break;

		case AnimState::JumpFall:
			ChangeAnimation(jumpFallAnim, false);
			break;
		}
	}
}
