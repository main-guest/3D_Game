#include <cmath>
#include "Player.h"
#include "PhysicsManager.h"
#include "CollisionWorld.h"
#include "Enemy.h"

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
	jumpLoopAnim = 5;
	jumpEndAnim = 6;
	attack01Anim = 7;
	attack02Anim = 8;
	attack03Anim = 9;
	hitAnim = 10;

	// ===== 武器データ設定 =====
	// 素手
	unarmedData.attackAnim = attack01Anim;
	unarmedData.dashAnim = dash01Anim;

	unarmedData.attackRadius = 40.0f;
	unarmedData.attackDistance = 120.0f;
	unarmedData.attackOffset = VGet(0, 115, 0);

	unarmedData.followAttack = true;

	// weapon1
	weapon1Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon1Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon1Data.attackAnim = attack02Anim;
	weapon1Data.dashAnim = dash02Anim;

	weapon1Data.attackRadius = 20.0f;
	weapon1Data.attackDistance = 160.0f;
	weapon1Data.attackOffset = VGet(0, 115, 0);

	weapon1Data.followAttack = true;

	weapon1.SetData(weapon1Data);
	
	// weapon2
	weapon2Data.posOffset = VGet(0.0f, 0.0f, 0.0f);

	weapon2Data.rotOffset = VGet(DX_PI_F / -2.0f, DX_PI_F / 2.0f, DX_PI_F);

	weapon2Data.attackAnim = attack03Anim;
	weapon2Data.dashAnim = dash01Anim;

	weapon2Data.attackRadius = 10.0f;
	weapon2Data.attackDistance = 220.0f;
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

	int frameNum = MV1GetFrameNum(handle);

	for (int i = 0; i < frameNum; i++)
	{
		printfDx(_T("%d : %s\n"),
			i,
			MV1GetFrameName(handle, i));
	}
}

void Player::Update(float dt, float cameraAngle, PhysicsManager& physics)
{
	//　=====　入力処理　=====
	UpdateInput(dt, cameraAngle);

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

	//　=====　接地保存　=====
    prevGround = isGround;
}

void Player::Draw()
{
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
		isGround
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

	// =========================================
	// 剣ボーン座標デバッグ
	// =========================================

	// 右手ボーン番号取得
	int frameIndex = MV1SearchFrame(handle, _T("mixamorig:RightHand"));

	if (frameIndex >= 0)
	{
		// ボーン行列取得
		MATRIX frameMat = MV1GetFrameLocalWorldMatrix(handle, frameIndex);

		// 座標抽出
		VECTOR handPos = MV1GetFramePosition(handle, frameIndex);

		DrawFormatString(
			20, 320,
			GetColor(0, 255, 255),
			_T("RightHand Pos : X=%.2f Y=%.2f Z=%.2f"),
			handPos.x,
			handPos.y,
			handPos.z
		);
	}

	// =========================================
	// weapon1モデル座標
	// =========================================

	if (equipState == EquipState::Weapon1)
	{
		VECTOR weaponPos = MV1GetPosition(weapon1.GetHandle());

		DrawFormatString(
			20, 340,
			GetColor(255, 128, 0),
			_T("Weapon1 Pos : X=%.2f Y=%.2f Z=%.2f"),
			weaponPos.x,
			weaponPos.y,
			weaponPos.z
		);
	}

	// =========================================
	// weapon2モデル座標
	// =========================================

	if (equipState == EquipState::Weapon2)
	{
		VECTOR weaponPos = MV1GetPosition(weapon2.GetHandle());

		DrawFormatString(
			20, 360,
			GetColor(128, 255, 0),
			_T("Weapon2 Pos : X=%.2f Y=%.2f Z=%.2f"),
			weaponPos.x,
			weaponPos.y,
			weaponPos.z
		);
	}
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

	DrawSphere3D(
		weapon1.GetDebugPos(),
		5.0f,
		8,
		GetColor(0, 0, 255),
		GetColor(0, 0, 255),
		TRUE
	);

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

	// ===== ダッシュ入力 =====
	isDash = CheckHitKey(KEY_INPUT_LSHIFT) != 0;

	// 速度切り替え
	speed = isDash ? dashSpeed : walkSpeed;

	// ===== マウスクリック =====
	int mouse = GetMouseInput();

	bool leftClick = (mouse & MOUSE_INPUT_LEFT) && !(oldMouse & MOUSE_INPUT_LEFT);

	oldMouse = mouse;

	// ===== 攻撃 =====
	if (leftClick && currentState != AnimState::Attack && isGround)
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
	if (CheckHitKey(KEY_INPUT_SPACE) && isGround && !jumpRequest)
	{
		jumpRequest = true;

		currentState = AnimState::JumpStart;
		ChangeAnimation(jumpStartAnim, false);
	}

	// ==== 移動 ====
	velocity.x = 0.0f;
	velocity.z = 0.0f;

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

    //　=====　Attack中　=====
    if (currentState == AnimState::Attack)
    {
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		// 攻撃判定ON区間
		if (animTime >= attackStartFrame && animTime <= attackEndFrame)
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

	//　=====　通常状態　=====
    AnimState nextState = AnimState::Idle;

    if (!isGround)
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