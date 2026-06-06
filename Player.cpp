#include <cmath>
#include <algorithm>
#include "Player.h"
#include "PhysicsManager.h"
#include "CollisionWorld.h"
#include "Enemy.h"

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

void Player::Init(CollisionWorld* w)
{
	world = w;

	// ===== モデル読み込み =====
	CharacterBase::Init(_T("mv1model/Player.mv1"));

	// ===== 武器モデル読み込み =====
	weapon1.Init(_T("mv1model/Blade.mv1"));
	weapon2.Init(_T("mv1model/Gun01.mv1"));

	// ===== アニメーション番号 =====
	idleAnim = 0;
	walkAnim = 1;
	dash01Anim = 2;
	dash02Anim = 3;
	jumpStartAnim = 4;
	dashJumpStartAnim = 5;
	jumpLoopAnim = 6;
	jumpEndAnim = 7;
	attack01Anim = 8;
	attack02Anim = 9;
	attack03Anim = 10;
	hitAnim = 11;
	dodgeAnim = 12;

	// ===== 武器データ設定 =====
	// 素手
	unarmedData.attackAnim = attack01Anim;
	unarmedData.dashAnim = dash01Anim;

	unarmedData.attackShape = AttackShape::Sphere;
	unarmedData.attackRadius = 40.0f;
	unarmedData.attackDistance = 120.0f;
	unarmedData.attackOffset = VGet(0, 115, 0);

	unarmedData.followAttack = true;

	unarmedData.attackStartFrame = 50.0f;
	unarmedData.attackEndFrame = 100.0f;

	// weapon1
	weapon1Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon1Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon1Data.attackAnim = attack02Anim;
	weapon1Data.dashAnim = dash02Anim;

	weapon1Data.attackShape = AttackShape::Line;
	weapon1Data.attackRadius = 20.0f;
	weapon1Data.attackDistance = 160.0f;
	weapon1Data.attackOffset = VGet(0, 115, 0);

	weapon1Data.followAttack = true;

	weapon1Data.attackStartFrame = 30.0f;
	weapon1Data.attackEndFrame = 80.0f;

	weapon1.SetData(weapon1Data);

	// weapon2
	weapon2Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon2Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon2Data.attackAnim = attack03Anim;
	weapon2Data.dashAnim = dash01Anim;
	weapon2Data.attackShape = AttackShape::Gun;

	weapon2Data.attackRadius = 10.0f;
	weapon2Data.attackDistance = 600.0f;
	weapon2Data.attackOffset = VGet(0, 115, 0);

	weapon2Data.followAttack = false;

	weapon2Data.attackStartFrame = 10.0f;
	weapon2Data.attackEndFrame = 20.0f;

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
	if (isDodging)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 回避移動区間
		if (animTime >= dodgeStartFrame && animTime <= dodgeEndFrame)
		{
			velocity.x = dodgeDir.x * dodgeSpeed;
			velocity.z = dodgeDir.z * dodgeSpeed;
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

		physics.MoveCharacter(pos, velocity, state, radius, dt);

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

		physics.MoveCharacter(pos, velocity, state, radius, dt);

		UpdateState();
		UpdateAnimation(dt);

		return;
	}

	//　=====　入力処理　=====
	UpdateInput(dt, cameraAngle);

	//　=====　物理　=====
	physics.MoveCharacter(pos, velocity, state, radius, dt);

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

	//　=====　武器更新　=====
	switch (equipState)
	{
	case EquipState::Weapon1:
		weapon1.Update(handle, _T("mixamorig:RightHand"), characterAngle);
		break;

	case EquipState::Weapon2:
		weapon2.Update(handle, _T("mixamorig:RightHand"), characterAngle);
		break;

	case EquipState::Unarmed:
		break;
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
	state.wasGround = state.isGround;
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

				VECTOR center = enemy->GetPos();
				center.y += enemy->GetHeight() * 0.5f;

				float radius = 10.0f;

				bool hit =
					LineSphereHit(prevRoot, prevTip, center, radius) ||	// 前フレームの剣
					LineSphereHit(root, tip, center, radius) ||			// 現在フレームの剣
					LineSphereHit(prevTip, tip, center, radius) ||		// 剣先の移動軌跡
					LineSphereHit(prevRoot, root, center, radius);		// 剣根本の移動軌跡

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

	hp -= power;

	isKnockback = true;
	knockbackTimer = knockbackDuration;

	VECTOR back = GetForward();

	velocity.x = -back.x * 350.0f;
	velocity.z = -back.z * 350.0f;

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

void Player::StartDodge()
{
	if (isDodging)
		return;

	isDash = false;

	if (!state.isGround)
		return;

	isDodging = true;

	dodgeDir = GetForward();

	currentState = AnimState::Dodge;

	ChangeAnimation(dodgeAnim, false);

	SetAnimSpeed(1.5f);
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
		_T("Player Pos : X=%.2f Y=%.2f Z=%.2f"),
		pos.x, pos.y, pos.z
	);

	// ===== 速度 =====
	DrawFormatString(
		20, 200,
		GetColor(255, 255, 0),
		_T("Velocity   : X=%.2f Y=%.2f Z=%.2f"),
		velocity.x, velocity.y, velocity.z
	);

	// ===== 接地状態 =====
	DrawFormatString(
		20, 220,
		GetColor(0, 255, 0),
		_T("IsGround : %d"),
		state.isGround
	);

	DrawFormatString(
		20, 240,
		GetColor(255, 255, 255),
		_T("AttackRadius : %.2f"),
		currentWeaponData->attackRadius
	);

	DrawFormatString(
		20, 260,
		GetColor(255, 255, 255),
		_T("AttackPos : %.2f %.2f %.2f"),
		attackPos.x,
		attackPos.y,
		attackPos.z
	);

	DrawFormatString(
		20, 280,
		GetColor(255, 255, 255),
		_T("AttackActive : %d"),
		attackActive
	);

	// ガン
	DrawFormatString(
		20,
		300,
		GetColor(255, 255, 0),
		_T("GunTimer : %.2f"),
		gunTimer
	);

	DrawFormatString(
		20,
		320,
		GetColor(255, 255, 0),
		_T("GunWaiting : %d"),
		gunWaitingDamage
	);

	// ダッシュジャンプ
	DrawFormatString(
		20,
		380,
		GetColor(255, 255, 255),
		_T("dashJump : %d"),
		dashJump
	);

	DrawFormatString(
		20,
		400,
		GetColor(255, 255, 255),
		_T("AnimState=%d"),
		(int)currentState
	);
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

void Player::UpdateInput(float dt, float cameraAngle)
{
	float moveX = 0.0f;
	float moveZ = 0.0f;

	//　=====　入力　=====	
	if (CheckHitKey(KEY_INPUT_W)) moveZ += 1.0f;
	if (CheckHitKey(KEY_INPUT_S)) moveZ -= 1.0f;
	if (CheckHitKey(KEY_INPUT_D)) moveX += 1.0f;
	if (CheckHitKey(KEY_INPUT_A)) moveX -= 1.0f;

	//　=====　移動判定　=====
	bool isMove = (moveX != 0.0f || moveZ != 0.0f);

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
		if (shiftHoldTimer >= dashHoldTime)
		{
			isDash = true;
		}
	}

	// 離した瞬間
	if (!shiftNow && shiftPressed)
	{
		// 短押しなら回避
		if (shiftHoldTimer < dashHoldTime)
		{
			// ===== 回避 =====
			StartDodge();
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
	if (leftClick && currentState != AnimState::Attack && state.isGround)
	{
		currentState = AnimState::Attack;

		// ===== 武器ごとの攻撃アニメ =====
		ChangeAnimation(currentWeaponData->attackAnim, false);

		// ===== 固定型当たり判定 =====
		if (!currentWeaponData->followAttack)
		{
			UpdateAttackPos();
		}
	}

	// ===== ジャンプ =====
	if (CheckHitKey(KEY_INPUT_SPACE) && state.isGround)
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
			state.isGround = false;

			ChangeAnimation(dashJumpStartAnim, false);
		}
		else
		{
			jumpRequest = true;

			ChangeAnimation(jumpStartAnim, false);
		}
	}

	// ==== 移動 ====
	if (!(currentState == AnimState::JumpStart && dashJump) && !(currentState == AnimState::JumpLoop && dashJump))
	{
		velocity.x = 0.0f;
		velocity.z = 0.0f;
	}

	if (currentState != AnimState::Attack && currentState != AnimState::JumpStart && currentState != AnimState::JumpEnd && isMove)
	{
		//　正規化
		float len = sqrtf(moveX * moveX + moveZ * moveZ);
		if (len > 0.001f)
		{
			moveX /= len;
			moveZ /= len;
		}

		//　=====　カメラ基準変換　=====
		float sinY = sinf(cameraAngle);
		float cosY = cosf(cameraAngle);

		VECTOR dir;

		dir.x = moveX * cosY + moveZ * sinY;
		dir.z = moveZ * cosY - moveX * sinY;

		velocity.x = dir.x * speed;
		velocity.z = dir.z * speed;

		//　=====　回転　=====
		float targetAngle = atan2f(dir.x, dir.z) + DX_PI;

		float diff = targetAngle - characterAngle;

		while (diff > DX_PI)diff -= DX_TWO_PI;
		while (diff < -DX_PI)diff += DX_TWO_PI;

		characterAngle += diff * 10.0f * dt;
	}
}

void Player::UpdateState()
{
	//　=====　JumpStart → JumpLoop　=====
	if (currentState == AnimState::JumpStart)
	{
		// 通常ジャンプのみ待機
		if (!dashJump)
		{
			if (jumpRequest && animTime >= jumpStartFrame)
			{
				velocity.y = jumpPower;

				state.isGround = false;

				jumpRequest = false;

				currentState = AnimState::JumpLoop;

				ChangeAnimation(jumpLoopAnim, true);
			}
		}
		else
		{
			float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

			if (animTime >= totalTime)
			{
				currentState = AnimState::JumpLoop;

				ChangeAnimation(jumpLoopAnim, true);
			}
		}

		return;
	}

	//　=====　Attack中　=====
	if (currentState == AnimState::Attack)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 攻撃判定ON区間
		if (animTime >= currentWeaponData->attackStartFrame && animTime <= currentWeaponData->attackEndFrame)
		{
			attackActive = true;
		}
		else
		{
			attackActive = false;
		}

		// アニメ終了
		if (animTime >= totalTime)
		{
			attackHit = false;

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

	//　=====　着地瞬間検知　=====
	bool landed = (!state.wasGround && state.isGround);

	if (landed && currentState != AnimState::JumpEnd)
	{
		dashJump = false;

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
				if (isDash)
				{
					currentState = AnimState::Dash;

					// ===== 武器ごとのダッシュ =====
					ChangeAnimation(currentWeaponData->dashAnim, true);
				}
				else
				{
					currentState = AnimState::Walk;
					ChangeAnimation(walkAnim, true);
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

	// 接地状態を保存
	state.wasGround = state.isGround;

	//　=====　通常状態　=====
	AnimState nextState = AnimState::Idle;

	if (!state.isGround)
	{
		nextState = AnimState::JumpLoop;
	}
	else if (fabsf(velocity.x) > 0.1f ||
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

		case AnimState::Dash:
			// ===== 武器ごとのダッシュ =====
			ChangeAnimation(currentWeaponData->dashAnim, true);
			break;

		case AnimState::JumpLoop:
			ChangeAnimation(jumpLoopAnim, true);
			break;
		}
	}
}