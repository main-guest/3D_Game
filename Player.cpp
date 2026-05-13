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

	// ===== アニメーション番号 =====
	idleAnim = 0;
	walkAnim = 1;
	jumpStartAnim = 2;
	jumpLoopAnim = 3;
	jumpEndAnim = 4;
	attack01Anim = 5;

	// ===== 初期状態 =====
	currentState = AnimState::Idle;
	
	ChangeAnimation(idleAnim, true);
}

void Player::Update(float dt, float cameraAngle, PhysicsManager& physics)
{
	//　=====　入力処理　=====
	UpdateInput(dt, cameraAngle);

	//　=====　物理　=====
	physics.MoveAndCheckCollision(pos, velocity, radius, isGround, dt);

	//　=====　状態更新　=====
	UpdateState();

	//　=====　アニメーション更新　=====
	UpdateAnimation(dt);

	//　=====　接地保存　=====
    prevGround = isGround;

	//　=====　反映　=====
	MV1SetPosition(handle, pos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));
}

void Player::CheckAttackHit(std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// Attack01中のみ
	if (currentState != AnimState::Attack01)
	{
		attackHit = false;
		return;
	}

	// 攻撃判定フレーム
	if (animTime < 20.0f || animTime>35.0f)
	{
		return;
	}

	// 既にヒット済み
	if (attackHit)
	{
		return;
	}

	// 前方向
	VECTOR forward;

	forward.x = sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = cosf(characterAngle);

	// 攻撃中心
	VECTOR attackPos;

	attackPos.x = pos.x + forward.x * 50.0f;
	attackPos.y = 0.0f;
	attackPos.z = pos.z + forward.z * 50.0f;

	float attackRadius = 40.0f;

	// Enemyチェック
	for (auto& enemy : enemies)
	{
		if (enemy->IsDead())
		{
			continue;
		}

		VECTOR epos = enemy->GetPos();

		float dx = epos.x - attackPos.x;
		float dy = epos.y - attackPos.y;
		float dz = epos.z - attackPos.z;

		float distanceSq =
			dx * dx +
			dy * dy +
			dz * dz;

		if (distanceSq <= attackRadius * attackRadius)
		{
			enemy->Damage(20);

			attackHit = true;

			break;
		}
	}

	// デバッグ表示
	DrawSphere3D(
		attackPos,
		attackRadius,
		16,
		GetColor(255, 0, 0),
		GetColor(255, 0, 0),
		FALSE
	);
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