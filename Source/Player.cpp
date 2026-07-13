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
	CharacterBase::Init("Assets/mv1model/Player.mv1");

	// ===== 武器モデル読み込み =====
	sword.Init("Assets/mv1model/Sword.mv1");
	gun01.Init("Assets/mv1model/Gun01.mv1");

	// ===== アニメーション番号 =====
	idleAnim = 0;
	walk_fAnim = 1;
	walk_bAnim = 2;
	walk_rAnim = 3;
	walk_lAnim = 4;

	dash_f01Anim = 5;
	dash_f02Anim = 6;
	dash_bAnim = 7;
	dash_rAnim = 8;
	dash_lAnim = 9;

	jumpStartAnim = 10;
	dashJumpStartAnim = 11;
	jumpRiseAnim = 12;
	jumpFallAnim = 13;
	jumpEndAnim = 14;
	handAttackAnim = 15; 
	swordAttack01Anim = 16; 
	swordAttack02Anim = 17; 
	swordAttack03Anim = 18; 
	swordAttack04Anim = 19;
	gunAttackAnim = 20; 
	hitAnim = 21;

	dodge_fAnim = 22;
	dodge_b01Anim = 23;
	dodge_b02Anim = 24;

	deadAnim = 25;

	// ===== 武器データ設定 =====
	// 素手
	unarmedData.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	unarmedData.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	unarmedData.lockDashFrontAnim = dash_f01Anim;
	unarmedData.lockDashBackAnim = dash_bAnim;
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

	// SwordData（剣）
	swordData.attackAnim[0] = swordAttack01Anim;
	swordData.attackAnim[1] = swordAttack02Anim;
	swordData.attackAnim[2] = swordAttack03Anim;
	swordData.attackAnim[3] = swordAttack04Anim;

	swordData.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	swordData.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	swordData.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	swordData.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	swordData.lockDashFrontAnim = dash_f02Anim;
	swordData.lockDashBackAnim = dash_bAnim;
	swordData.lockDashRightAnim = dash_rAnim;
	swordData.lockDashLeftAnim = dash_lAnim;

	swordData.lockDodgeAnim[(int)MoveDirection::Front] = dodge_fAnim;
	swordData.lockDodgeAnim[(int)MoveDirection::Back] = dodge_b01Anim;
	swordData.lockDodgeAnim[(int)MoveDirection::Right] = dodge_fAnim;
	swordData.lockDodgeAnim[(int)MoveDirection::Left] = dodge_fAnim;

	swordData.comboCount = 4;

	swordData.dashAnim = dash_f02Anim;

	swordData.attackShape = AttackShape::Line;
	swordData.attackRadius = 20.0f;
	swordData.attackDistance = 160.0f;

	// 攻撃コンボ　攻撃開始・終了フレーム
	swordData.attackStartFrame[0] = 30.0f;
	swordData.attackEndFrame[0] = 57.0f;

	swordData.attackStartFrame[1] = 3.0f;
	swordData.attackEndFrame[1] = 29.0f;

	swordData.attackStartFrame[2] = 41.0f;
	swordData.attackEndFrame[2] = 73.0f;

	swordData.attackStartFrame[3] = 46.0f;
	swordData.attackEndFrame[3] = 85.0f;

	// 攻撃コンボ　受付フレーム
	swordData.comboAcceptStartFrame[0] = 43.0f;
	swordData.comboAcceptEndFrame[0] = 65.0f;

	swordData.comboAcceptStartFrame[1] = 20.0f;
	swordData.comboAcceptEndFrame[1] = 44.0f;

	swordData.comboAcceptStartFrame[2] = 47.0f;
	swordData.comboAcceptEndFrame[2] = 97.0f;

	swordData.comboAcceptStartFrame[3] = 0.0f;
	swordData.comboAcceptEndFrame[3] = 0.0f;

	// 攻撃コンボ　コンボキャンセルフレーム
	swordData.comboCancelFrame[0] = 53.0f;
	swordData.comboCancelFrame[1] = 36.0f;
	swordData.comboCancelFrame[2] = 73.0f;
	swordData.comboCancelFrame[3] = 999.0f;

	swordData.attackOffset = VGet(0, 115, 0);

	swordData.followAttack = true;

	swordRenderData.posOffset = VGet(0.0f, 0.0f, 0.0f);

	swordRenderData.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	sword.SetRenderData(swordRenderData);

	// Gun01Data（銃）
	gun01Data.lockWalkAnim[(int)MoveDirection::Front] = walk_fAnim;
	gun01Data.lockWalkAnim[(int)MoveDirection::Back] = walk_bAnim;
	gun01Data.lockWalkAnim[(int)MoveDirection::Right] = walk_rAnim;
	gun01Data.lockWalkAnim[(int)MoveDirection::Left] = walk_lAnim;

	gun01Data.lockDashFrontAnim = dash_f01Anim;
	gun01Data.lockDashBackAnim = dash_bAnim;
	gun01Data.lockDashRightAnim = dash_rAnim;
	gun01Data.lockDashLeftAnim = dash_lAnim;

	gun01Data.lockDodgeAnim[(int)MoveDirection::Front] = dodge_fAnim;
	gun01Data.lockDodgeAnim[(int)MoveDirection::Back] = dodge_b01Anim;
	gun01Data.lockDodgeAnim[(int)MoveDirection::Right] = dodge_fAnim;
	gun01Data.lockDodgeAnim[(int)MoveDirection::Left] = dodge_fAnim;

	for (int i = 0; i < WeaponData::MaxCombo; i++)
	{
		gun01Data.attackAnim[i] = gunAttackAnim;

		gun01Data.attackStartFrame[i] = 10.0f;
		gun01Data.attackEndFrame[i] = 20.0f;

		gun01Data.comboAcceptStartFrame[i] = 0.0f;
		gun01Data.comboAcceptEndFrame[i] = 0.0f;

		gun01Data.comboCancelFrame[i] = 0.0f;
	}

	gun01Data.comboCount = 1;

	gun01Data.dashAnim = dash_f01Anim;

	gun01Data.attackShape = AttackShape::Gun;

	gun01Data.attackRadius = 10.0f;
	gun01Data.attackDistance = 600.0f;

	gun01Data.attackOffset = VGet(0, 115, 0);

	gun01Data.followAttack = false;

	gun01RenderData.posOffset = VGet(0.0f, 0.0f, 0.0f);

	gun01RenderData.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	gun01.SetRenderData(gun01RenderData);

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
	// ===== 死亡判定 =====
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

	//　=====　武器更新　=====
	switch (equipState)
	{
	case EquipState::Sword:
		sword.Update(handle, "mixamorig:RightHand", characterAngle);
		break;

	case EquipState::Gun01:
		gun01.Update(handle, "mixamorig:RightHand", characterAngle);
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

			currentDirection = MoveDirection::None;
		}

		VECTOR diff = VSub(lockOnTarget->GetCenterPos(), GetCenterPos());

		diff.y = 0.0f;

		if (VSize(diff) > lockOnDistance)
		{
			isLockOn = false;
			lockOnTarget = nullptr;

			currentDirection = MoveDirection::None;
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

	// ===== スタミナ回復処理 =====
	// 回復タイマー更新
	staminaRecoveryTimer += dt;

	// 一定時間経過したら回復
	if (staminaRecoveryTimer >= staminaRecoveryDelay)
	{
		stamina += staminaRecovery * dt;

		if (stamina > maxStamina)
		{
			stamina = maxStamina;
		}
	}
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
	case EquipState::Sword:
		sword.Draw();
		break;

	case EquipState::Gun01:
		gun01.Draw();
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
	equipState = EquipState::Sword;

	currentWeaponData = &swordData;
}

void Player::EquipWeapon2()
{
	equipState = EquipState::Gun01;

	currentWeaponData = &gun01Data;
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
			VECTOR prevRoot = sword.GetPrevRootPosition();
			VECTOR root = sword.GetRootPosition();

			VECTOR prevTip = sword.GetPrevTipPosition();
			VECTOR tip = sword.GetTipPosition();

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
}

void Player::StartDodge(float cameraAngle)
{
	if (stamina < dodgeCost)
		return;

	if (isDodging)
		return;

	if (!isGround)
		return;

	isDash = false;

	isDodging = true;

	ConsumeStamina(dodgeCost, dodgeDelay);

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

void Player::ConsumeStamina(float cost, float recoveryDelay)
{
	stamina -= cost;

	if (stamina <= 0.0f)
	{
		stamina = 0.0f;
		isDash = false;

		staminaBreak = true;
	}

	staminaRecoveryTimer = 0.0f;
	staminaRecoveryDelay = recoveryDelay;
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
		1000,
		80,
		GetColor(255, 255, 255),
		"DodgeDir X = % .2f Z = % .2f",
		dodgeMoveDir.x,
		dodgeMoveDir.z);

	DrawFormatString(
		1000, 100,
		GetColor(255, 255, 255),
		"State=%d MoveAnim=%d",
		(int)currentState,
		(int)currentMoveAnim);

	DrawFormatString(
		1000,
		120,
		GetColor(255, 255, 255),
		"isLockOn : %p",
		isLockOn);

	DrawFormatString(
		1000,
		140,
		GetColor(255, 255, 255),
		"lockOnTarget : %p",
		lockOnTarget);

	// ===== ステータス =====
	DrawFormatString(
		1000,
		400,
		GetColor(0, 255, 0),
		"HP : %d",
		hp);

	DrawFormatString(
		1000,
		420,
		GetColor(0, 255, 0),
		"ST : %.2f",
		stamina);

	const MATRIX& mat = sword.GetWorldMatrix();

	DrawFormatString(
		1000,
		440,
		GetColor(255, 255, 255),
		"Player Pos %.1f, %.1f, %.1f",
		mat.m[3][0],
		mat.m[3][1],
		mat.m[3][2]);

	DrawFormatString(
		1000,
		460,
		GetColor(255, 255, 255),
		"Player ScaleX %.2f, %.2f, %.2f",
		VSize(VGet(mat.m[0][0], mat.m[0][1], mat.m[0][2])),
		VSize(VGet(mat.m[1][0], mat.m[1][1], mat.m[1][2])),
		VSize(VGet(mat.m[2][0], mat.m[2][1], mat.m[2][2])));
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
		sword.GetPrevRootPosition(),
		sword.GetPrevTipPosition(),
		hitRadius,
		8,
		GetColor(255, 255, 0),
		GetColor(255, 255, 0), FALSE
	);

	// 現在フレームの剣
	DrawCapsule3D(
		sword.GetRootPosition(),
		sword.GetTipPosition(),
		hitRadius,
		8,
		GetColor(0, 255, 0),
		GetColor(0, 255, 0),
		FALSE
	);

	// 剣先の移動軌跡
	DrawCapsule3D(
		sword.GetPrevRootPosition(),
		sword.GetRootPosition(),
		hitRadius,
		8,
		GetColor(0, 255, 255),
		GetColor(0, 255, 255),
		FALSE
	);

	// 根元の移動軌跡
	DrawCapsule3D(
		sword.GetPrevTipPosition(),
		sword.GetTipPosition(),
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

void Player::UpdateEnemyBasis()
{
	if (!isLockOn || lockOnTarget == nullptr)
		return;

	enemyForward = VSub(lockOnTarget->GetCenterPos(), GetCenterPos());
	enemyForward.y = 0.0f;

	if (VSize(enemyForward) < 0.001f)
		return;

	enemyForward = VNorm(enemyForward);

	// Enemy基準右方向
	enemyRight.x = enemyForward.z;
	enemyRight.y = 0.0f;
	enemyRight.z = -enemyForward.x;
}

VECTOR Player::GetLockMoveVector() const
{
	VECTOR move = VGet(0, 0, 0);

	if (CheckHitKey(KEY_INPUT_W))
		move = VAdd(move, enemyForward);

	if (CheckHitKey(KEY_INPUT_S))
		move = VSub(move, enemyForward);

	if (CheckHitKey(KEY_INPUT_D))
		move = VAdd(move, enemyRight);

	if (CheckHitKey(KEY_INPUT_A))
		move = VSub(move, enemyRight);

	if (VSize(move) > 0.001f)
		move = VNorm(move);

	return move;
}

VECTOR Player::GetFreeMoveVector(float cameraAngle) const
{
	VECTOR move = VGet(0, 0, 0);

	float sinY = sinf(cameraAngle);
	float cosY = cosf(cameraAngle);

	if (CheckHitKey(KEY_INPUT_W))
	{
		move.x += sinY;
		move.z += cosY;
	}

	if (CheckHitKey(KEY_INPUT_S))
	{
		move.x -= sinY;
		move.z -= cosY;
	}

	if (CheckHitKey(KEY_INPUT_D))
	{
		move.x += cosY;
		move.z -= sinY;
	}

	if (CheckHitKey(KEY_INPUT_A))
	{
		move.x -= cosY;
		move.z += sinY;
	}

	if (VSize(move) > 0.001f)
	{
		move = VNorm(move);
	}

	return move;
}

MoveDirection Player::GetLockMoveDirection() const
{
	bool w = CheckHitKey(KEY_INPUT_W);
	bool s = CheckHitKey(KEY_INPUT_S);
	bool a = CheckHitKey(KEY_INPUT_A);
	bool d = CheckHitKey(KEY_INPUT_D);

	if (!w && !s && !a && !d)
		return MoveDirection::None;

	// 前後優先
	if (w)
		return MoveDirection::Front;

	if (s)
		return MoveDirection::Back;

	if (a)
		return MoveDirection::Left;

	if (d)
		return MoveDirection::Right;

	return MoveDirection::None;
}

void Player::UpdateRotation(float dt)
{
	if (isLockOn && lockOnTarget)
	{
		UpdateLockOnMove(dt);
	}
	else
	{
		UpdateFreeMove(dt);
	}
}

void Player::UpdateMoveVector(float cameraAngle)
{
	moveVector = VGet(0, 0, 0);

	if (isLockOn)
	{
		moveVector = GetLockMoveVector();
	}
	else
	{
		moveVector = GetFreeMoveVector(cameraAngle);
	}
}

void Player::UpdateLockOnMove(float dt)
{
	if (lockOnTarget == nullptr)
		return;

	// ===== Enemy方向を向く =====
	VECTOR dir = VSub(lockOnTarget->GetCenterPos(), GetCenterPos());

	dir.y = 0;

	if (VSize(dir) < 0.001f)
		return;

	dir = VNorm(dir);

	float target = atan2f(dir.x, dir.z) + DX_PI;

	float diff = target - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;
}

void Player::UpdateFreeMove(float dt)
{
	if (VSize(moveVector) < 0.001f)
		return;

	float target = atan2f(moveVector.x, moveVector.z) + DX_PI;

	float diff = target - characterAngle;

	while (diff > DX_PI)diff -= DX_TWO_PI;
	while (diff < -DX_PI)diff += DX_TWO_PI;

	characterAngle += diff * 10.0f * dt;
}

void Player::UpdateLockOnAnimation()
{
	MoveAnimState next = GetNextMoveAnim();

	if (next == currentMoveAnim)
		return;

	currentMoveAnim = next;

	switch (next)
	{
	case MoveAnimState::Idle:
		ChangeAnimation(idleAnim, true);
		break;

	case MoveAnimState::WalkFront:
		ChangeAnimation(walk_fAnim, true);
		break;

	case MoveAnimState::WalkBack:
		ChangeAnimation(walk_bAnim, true);
		break;

	case MoveAnimState::WalkLeft:
		ChangeAnimation(walk_lAnim, true);
		break;

	case MoveAnimState::WalkRight:
		ChangeAnimation(walk_rAnim, true);
		break;

	case MoveAnimState::DashFront:
		ChangeAnimation(currentWeaponData->lockDashFrontAnim, true);
		break;

	case MoveAnimState::DashBack:
		ChangeAnimation(currentWeaponData->lockDashBackAnim, true);
		break;

	case MoveAnimState::DashLeft:
		ChangeAnimation(currentWeaponData->lockDashLeftAnim, true);
		break;

	case MoveAnimState::DashRight:
		ChangeAnimation(currentWeaponData->lockDashRightAnim, true);
		break;
	}
}

void Player::UpdateFreeAnimation()
{
	MoveAnimState next = GetNextMoveAnim();

	if (next == currentMoveAnim)
		return;

	currentMoveAnim = next;

	switch (next)
	{
	case MoveAnimState::Idle:
		ChangeAnimation(idleAnim, true);
		break;

	case MoveAnimState::WalkFront:
		ChangeAnimation(walk_fAnim, true);
		break;

	case MoveAnimState::DashFront:
		ChangeAnimation(currentWeaponData->dashAnim, true);
		break;

	default:
		break;
	}
}

bool Player::UpdateDead()
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

bool Player::UpdateDodge()
{
	if (currentState != AnimState::Dodge)
		return false;

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (animTime >= totalTime)
	{
		isDodging = false;

		SetAnimSpeed(1.0f);

		bool isMove =
			fabsf(moveVector.x) > 0.1f ||
			fabsf(moveVector.z) > 0.1f;

		if (isMove)
		{
			currentState = isDash ?
				AnimState::Dash : AnimState::Walk;
		}
		else
		{
			currentState = AnimState::Idle;
		}

		currentMoveAnim = MoveAnimState::None;
	}

	return true;
}

bool Player::UpdateJump()
{
	// ===== 共通：落下開始 =====
	if (!isGround &&
		velocity.y < 0.0f &&
		currentState != AnimState::JumpStart &&
		currentState != AnimState::JumpFall &&
		currentState != AnimState::JumpEnd)
	{
		currentState = AnimState::JumpFall;
		ChangeAnimation(jumpFallAnim, false);

		return true;
	}

	switch (currentState)
	{
	case AnimState::JumpStart:
	{
		// 通常ジャンプのみ待機
		if (!dashJump)
		{
			if (jumpRequest &&
				animTime >= jumpStartFrame)
			{
				velocity.y = jumpPower;

				isGround = false;

				jumpRequest = false;

				currentState = AnimState::JumpRise;

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

		return true;
	}

	case AnimState::JumpRise:
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (velocity.y <= 0)
		{
			currentState = AnimState::JumpFall;

			ChangeAnimation(jumpFallAnim, false);

			return true;
		}

		if (animTime >= totalTime)
		{
			animTime = totalTime;
		}

		return true;
	}

	case AnimState::JumpFall:
	{
		if (!prevGround && isGround)
		{
			// ===== ダッシュジャンプ =====
			if (dashJump)
			{
				dashJump = false;

				bool isMove =
					fabsf(velocity.x) > 0.1f ||
					fabsf(velocity.z) > 0.1f;

				if (isMove)
				{
					if (isDash)
					{
						currentState = AnimState::Dash;

						if (isLockOn)
						{
							UpdateLockOnAnimation();
						}
						else
						{
							ChangeAnimation(currentWeaponData->dashAnim, true);
						}
					}
					else
					{
						currentState = AnimState::Walk;

						if (isLockOn)
						{
							UpdateLockOnAnimation();
						}
						else
						{
							ChangeAnimation(walk_fAnim, true);
						}
					}
				}
				else
				{
					currentState = AnimState::Idle;

					ChangeAnimation(idleAnim, true);
				}

				return true;
			}

			currentState = AnimState::JumpEnd;

			ChangeAnimation(jumpEndAnim, false);

			return true;
		}

		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime)
		{
			animTime = totalTime;

			currentMoveAnim=MoveAnimState::None;
		}

		return true;
	}

	case AnimState::JumpEnd:
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime)
		{
			if (fabsf(velocity.x) > 0.1f ||
				fabsf(velocity.z) > 0.1f)
			{
				currentState = isDash ? AnimState::Dash : AnimState::Walk;
			}
			else
			{
				currentState = AnimState::Idle;
			}
		}

		return true;
	}

	default:
		return false;
	}
}

bool Player::UpdateAttack()
{
	if (currentState != AnimState::Attack)
		return false;

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	// 攻撃判定ON区間
	attackActive =
		animTime >= currentWeaponData->attackStartFrame[comboStep] &&
		animTime <= currentWeaponData->attackEndFrame[comboStep];

	// ===== 次段コンボ =====
		// キャンセル可能になったら
	if (comboNext &&
		comboStep + 1 < currentWeaponData->comboCount &&
		animTime >= currentWeaponData->comboCancelFrame[comboStep])
	{
		// スタミナ不足ならコンボ終了
		if (stamina < attackCost)
		{
			comboNext = false;
			return true;
		}

		ConsumeStamina(attackCost, attackDelay);

		comboStep++;

		comboNext = false;

		attackHit = false;

		ChangeAnimation(currentWeaponData->attackAnim[comboStep], false);

		return true;
	}

	// コンボ終了
	if (animTime >= totalTime)
	{
		comboStep = 0;
		comboNext = false;

		attackHit = false;
		attackActive = false;

		bool moving =
			fabsf(moveVector.x) > 0.01f ||
			fabsf(moveVector.z) > 0.01f;

		if (moving)
		{
			currentMoveAnim = MoveAnimState::None;

			currentState = isDash ?
				AnimState::Dash : AnimState::Walk;

			if (isLockOn)
			{
				UpdateLockOnAnimation();
			}
			else
			{
				UpdateFreeAnimation();
			}
		}
		else
		{
			currentState = AnimState::Idle;

			currentMoveAnim = MoveAnimState::None;
			ChangeAnimation(idleAnim, true);
		}
	}

	return true;
}

bool Player::UpdateHit()
{
	if (currentState != AnimState::Hit)
		return false;

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (animTime >= totalTime)
	{
		currentMoveAnim = MoveAnimState::None;

		currentState = AnimState::Idle;

		ChangeAnimation(idleAnim, true);
	}

	return true;
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

void Player::UpdateInput(float dt, float cameraAngle, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	//　=====　入力取得　=====
	inputMoveX = 0.0f;
	inputMoveZ = 0.0f;

	if (CheckHitKey(KEY_INPUT_W)) inputMoveZ += 1.0f;
	if (CheckHitKey(KEY_INPUT_S)) inputMoveZ -= 1.0f;
	if (CheckHitKey(KEY_INPUT_D)) inputMoveX += 1.0f;
	if (CheckHitKey(KEY_INPUT_A)) inputMoveX -= 1.0f;

	//　=====　移動判定　=====
	bool isMove =
		fabsf(inputMoveX) > 0.01f ||
		fabsf(inputMoveZ) > 0.01f;

	// ===== ロックオン =====
	bool qNow = CheckHitKey(KEY_INPUT_Q);

	if (qNow && !oldQ)
	{
		if (isLockOn)
		{
			isLockOn = false;
			lockOnTarget = nullptr;

			currentDirection = MoveDirection::None;
		}
		else
		{
			LockOn(enemies);
		}
	}

	oldQ = qNow;

	// Enemy基準更新
	UpdateEnemyBasis();

	// 移動方向更新
	UpdateMoveVector(cameraAngle);

	// ===== ロックオンターゲット切り替え =====
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
	bool shiftNow = CheckHitKey(KEY_INPUT_LSHIFT);

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
		if (shiftHoldTimer >= dashHoldTime && 
			!isDash &&
			!staminaBreak &&
			stamina > 0.0f)
		{
			isDash = true;
		}
	}

	if (isDash)
	{
		ConsumeStamina(dashStaminaCost * dt, dashDelay);
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

		staminaBreak = false;
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
		if (currentState != AnimState::Attack && 
			isGround)
		{
			if (stamina < attackCost)
				return;

			ConsumeStamina(attackCost, attackDelay);

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
	if (CheckHitKey(KEY_INPUT_SPACE) && 
		isGround)
	{
		if (stamina < jumpCost)
			return;

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

			ConsumeStamina(jumpCost, jumpDelay);

			ChangeAnimation(dashJumpStartAnim, false);
		}
		else
		{
			jumpRequest = true;

			ConsumeStamina(jumpCost, jumpDelay);

			ChangeAnimation(jumpStartAnim, false);
		}
	}

	// ==== 移動 ====
	velocity.x = 0.0f;
	velocity.z = 0.0f;

	if (!isDodging &&
		currentState != AnimState::Attack &&
		currentState != AnimState::JumpStart &&
		currentState != AnimState::JumpEnd)
	{
		velocity.x = moveVector.x * speed;
		velocity.z = moveVector.z * speed;
	}

	//　=====　回転　=====
	if (currentState != AnimState::Attack &&
		currentState != AnimState::JumpStart &&
		currentState != AnimState::JumpEnd)
	{
		UpdateRotation(dt);
	}
}

void Player::UpdateState()
{
	if (UpdateDead()) return;

	if (UpdateDodge()) return;

	if (UpdateJump()) return;

	if (UpdateAttack()) return;

	if (UpdateHit()) return;

	// ===== 通常状態更新 =====
	if (!isDodging &&
		currentState != AnimState::Attack &&
		currentState != AnimState::JumpStart &&
		currentState != AnimState::JumpRise &&
		currentState != AnimState::JumpFall &&
		currentState != AnimState::JumpEnd &&
		currentState != AnimState::Dead)
	{
		bool moving =
			fabsf(moveVector.x) > 0.01f ||
			fabsf(moveVector.z) > 0.01f;

		if (moving)
		{
			currentState = isDash ?
				AnimState::Dash : AnimState::Walk;
		}
		else
		{
			currentState = AnimState::Idle;
		}
	}

	if (isLockOn)
	{
		UpdateLockOnAnimation();
	}
	else
	{
		UpdateFreeAnimation();
	}
}

MoveAnimState Player::GetNextMoveAnim() const
{
	bool moving =
		fabsf(moveVector.x) > 0.01f ||
		fabsf(moveVector.z) > 0.01f;

	if (!moving)
		return MoveAnimState::Idle;

	if (isLockOn)
	{
		MoveDirection dir = GetLockMoveDirection();

		if (isDash)
		{
			switch (dir)
			{
			case MoveDirection::Front:	return  MoveAnimState::DashFront;
			case MoveDirection::Back:	return MoveAnimState::DashBack;
			case MoveDirection::Left:	return MoveAnimState::DashLeft;
			case MoveDirection::Right:	return MoveAnimState::DashRight;
			default:					return MoveAnimState::Idle;
			}
		}
		else
		{
			switch (dir)
			{
			case MoveDirection::Front:	return MoveAnimState::WalkFront;
			case MoveDirection::Back:	return MoveAnimState::WalkBack;
			case MoveDirection::Left:	return MoveAnimState::WalkLeft;
			case MoveDirection::Right:	return MoveAnimState::WalkRight;
			default:					return MoveAnimState::Idle;
			}
		}
	}

	return isDash ?
		MoveAnimState::DashFront : MoveAnimState::WalkFront;

}