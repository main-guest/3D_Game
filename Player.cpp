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


	// ===== 当たり判定サイズ =====
	radius = 10.0f;
	height = 140.0f;

	// ===== アニメーション番号 =====
	idleAnim = 0;
	walkAnim = 1;
	dashAnim = 2;
	jumpStartAnim = 3;
	jumpLoopAnim = 4;
	jumpEndAnim = 5;
	hitAnim = 6;
	attack01Anim = 7;


	// ===== 初期状態 =====
	currentState = AnimState::Idle;
	
	ChangeAnimation(idleAnim, true);

	// 初期装備
	equipState = EquipState::Unarmed;
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
	//weapon.Update(handle, _T("ArmRight_012"), characterAngle);
	//weapon.SetRotation(VGet(0, characterAngle, 0));
	switch (equipState)
	{
	case EquipState::Weapon1:
		weapon1.Update(handle, _T("ArmRight_012"), characterAngle);
		break;

	case EquipState::Weapon2:
		weapon2.Update(handle, _T("ArmRight_012"), characterAngle);
		break;

	case EquipState::Unarmed:
		break;
	}

	//　=====　接地保存　=====
    prevGround = isGround;
}

void Player::Draw()
{
	VECTOR drawPos = pos;
	drawPos.y -= 30; // 足元補正

	MV1SetPosition(handle, drawPos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
	MV1DrawModel(handle);

	//武器表示
	//weapon.Draw();
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
}

void Player::EquipWeapon2()
{
	equipState = EquipState::Weapon2;
}

void Player::Unequip()
{
	equipState = EquipState::Unarmed;
}

void Player::CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// 攻撃判定フレーム
	if (!attackActive) return;

	// 既にヒット済み
	if (attackHit) return;

	// 前方向
	VECTOR forward = GetForward();

	attackPos.x = pos.x + forward.x * 120.0f;
	attackPos.y = pos.y + height * 0.75f;
	attackPos.z = pos.z + forward.z * 120.0f;

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

		if (distanceSq <= attackRadius * attackRadius)
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
		attackRadius
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
		attackRadius,
		16,
		GetColor(0, 255, 0),
		GetColor(0, 255, 0),
		FALSE
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
	if (leftClick && currentState != AnimState::Attack01 && isGround)
	{
		currentState = AnimState::Attack01;

		ChangeAnimation(attack01Anim, false);
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

	if (currentState != AnimState::Attack01 && currentState != AnimState::JumpStart && currentState != AnimState::JumpEnd && isMove)
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
    if (currentState == AnimState::Attack01)
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
					ChangeAnimation(dashAnim, true);
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
			ChangeAnimation(dashAnim, true);
			break;

        case AnimState::JumpLoop:
            ChangeAnimation(jumpLoopAnim, true);
            break;
        }
    }
}