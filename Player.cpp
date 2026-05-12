#include "Player.h"
#include <cmath>

Player::Player()
	:oldMouse(0),
	jumpRequest(false)
{

}

Player::~Player()
{

}

void Player::Init()
{
	// ===== モデル読み込み =====
	handle = MV1LoadModel(_T("mv1model/Player.mv1"));

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

	// ===== 初期位置 =====
	//MV1SetPosition(handle, pos);

	//// ===== アニメ確認 =====
	//int animNum = MV1GetAnimNum(handle);

	//for (int i = 0; i < animNum; i++)
	//{
	//	const TCHAR* name = MV1GetAnimName(handle, i);

	//	printfDx(_T("Anim %d : %s\n"), i, name);
	//}
}

void Player::Update(float cameraAngle, Object& object)
{
	float moveX = 0.0f;
	float moveZ = 0.0f;

	bool canMove = currentState != AnimState::Attack01 && currentState != AnimState::JumpEnd && !jumpRequest;

	//　=====　入力　=====
	if (canMove)
	{
		if (CheckHitKey(KEY_INPUT_W)) moveZ += 1;
		if (CheckHitKey(KEY_INPUT_S)) moveZ -= 1;
		if (CheckHitKey(KEY_INPUT_D)) moveX += 1;
		if (CheckHitKey(KEY_INPUT_A)) moveX -= 1;
	}

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

	// ===== 移動方向 =====
	VECTOR move = VGet(0, 0, 0);

	bool isMove = (moveX != 0.0f || moveZ != 0.0f);

	if (isMove)
	{
		//　正規化
		float len = sqrtf(moveX * moveX + moveZ * moveZ);
		moveX /= len;
		moveZ /= len;

		//　=====　カメラ基準変換　=====
		float sinY = sinf(cameraAngle);
		float cosY = cosf(cameraAngle);

		move.x = moveX * cosY + moveZ * sinY;
		move.z = moveZ * cosY - moveX * sinY;

		//　=====　移動処理 ＆ 当たり判定　=====
		// X方向
		VECTOR newPos = pos;

		newPos.x += move.x * speed;

		if (!object.CheckCollision(newPos, radius))
		{
			pos.x = newPos.x;
		}

		// Z方向
		newPos = pos;

		newPos.z += move.z * speed;

		if (!object.CheckCollision(newPos, radius))
		{
			pos.z = newPos.z;
		}

		//　=====　回転　=====
		float targetAngle = atan2f(move.x, move.z) + DX_PI; //向き更新
		float diff = targetAngle - characterAngle;

		// -π～πに収める
		while (diff > DX_PI)diff -= DX_TWO_PI;
		while (diff < -DX_PI)diff += DX_TWO_PI;

		// 滑らか回転
		characterAngle += diff * 0.2f;
	}

	// ===== Y方向移動 =====
	if (jumpRequest && currentState == AnimState::JumpStart && animTime >= jumpStartFrame)
	{
		velocityY = jumpPower;

		isGround = false;

		jumpRequest = false;

		currentState = AnimState::JumpLoop;

		ChangeAnimation(jumpLoopAnim);
	}
	
	//　=====　重力　=====
	UpdateGravity(object);

	//　=====　着地　=====

	if (isGround && currentState == AnimState::JumpLoop)
	{
		currentState = AnimState::JumpEnd;

		ChangeAnimation(jumpEndAnim);
	}

	//　=====　地面判定　=====
	if (pos.y < groundHeight)
	{
		pos.y = groundHeight;

		velocityY = 0.0f;

		if (!isGround && currentState == AnimState::JumpLoop)
		{
			currentState = AnimState::JumpEnd;

			ChangeAnimation(jumpEndAnim);
		}

		isGround = true;
	}

	//　=====　アニメーション状態更新　=====
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

		//　=====　アニメーション切り替え　=====
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

	//　=====　JumpEnd終了　=====
	if (currentState == AnimState::JumpEnd)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime - 1.0f)
		{
			currentState = isMove ? AnimState::Walk : AnimState::Idle;

			ChangeAnimation(isMove ? walkAnim : idleAnim);
		}
	}

	//　=====　Attack終了　=====
	if (currentState == AnimState::Attack01)
	{
		float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

		if (animTime >= totalTime - 1.0f)
		{
			currentState = AnimState::Idle;

			ChangeAnimation(idleAnim);
		}
	}

	//　=====　アニメーション更新　=====
	UpdateAnimation();

	//　=====　反映　=====
	MV1SetPosition(handle, pos);
	MV1SetRotationXYZ(handle, VGet(0, characterAngle, 0));

}

//void Player::ChangeAnimation(int animIndex)
//{
//	// ===== 前のアニメ削除 =====
//	if (currentAnimAttach != -1)
//	{
//		MV1DetachAnim(handle, currentAnimAttach);
//	}
//	// ===== 新しいアニメ設定 =====
//
//	currentAnimAttach = MV1AttachAnim(handle, animIndex);
//
//	// ===== 再生時間リセット =====
//	animTime = 0.0f;
//}
//
//void Player::Draw()
//{
//	MV1DrawModel(handle);
//
//	DrawFormatString(0, 380, GetColor(255, 255, 255), _T("PLAYERの座標:　(%.2f,　%.2f,　%.2f)"), pos.x, pos.y, pos.z);
//	DrawFormatString(0, 400, GetColor(255, 255, 255), _T("velocityY:　(%.2f)"), velocityY);
//	DrawFormatString(0, 420, GetColor(255, 255, 255), _T("isGround:　(%d)"), isGround);
//}