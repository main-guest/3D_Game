#include <cmath>
#include "Player.h"
#include "PhysicsManager.h"
#include "CollisionWorld.h"

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
	
	ChangeAnimation(idleAnim);
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
	if (leftClick && currentState != AnimState::Attack01)
	{
		currentState = AnimState::Attack01;

		ChangeAnimation(attack01Anim);
	}

	// ===== ジャンプ =====
	if (CheckHitKey(KEY_INPUT_SPACE) && isGround && !jumpRequest)
	{
		jumpRequest = true;

		currentState = AnimState::JumpStart;
		ChangeAnimation(jumpStartAnim);
	}

	// ==== 移動 ====
	velocity.x = 0.0f;
	velocity.z = 0.0f;

	if (currentState != AnimState::JumpStart && currentState != AnimState::JumpEnd && isMove)
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

            ChangeAnimation(jumpLoopAnim);
        }

        return;
    }

    //　=====　Attack中　=====
    if (currentState == AnimState::Attack01)
    {
        return;
    }

    //　=====　着地瞬間検知　=====
    if (!prevGround && isGround)
    {
        currentState = AnimState::JumpEnd;

        ChangeAnimation(jumpEndAnim);

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
                currentState = AnimState::Walk;
                ChangeAnimation(walkAnim);
            }
            else
            {
                currentState = AnimState::Idle;
                ChangeAnimation(idleAnim);
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