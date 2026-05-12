#include "CharacterBase.h"

CharacterBase::CharacterBase()
	:handle(-1),
	pos(VGet(0.0f, 0.0f, 0.0f)),
	characterAngle(0.0f),
	velocityY(0.0f),
	isGround(false),
	currentState(AnimState::Idle),
	currentAnimAttach(-1),
	animTime(0.0f)
{

}

CharacterBase::~CharacterBase()
{
	if (handle != -1)
	{
		MV1DeleteModel(handle);
	}
}

void CharacterBase::Init(const TCHAR* modelPath)
{
	handle = MV1LoadModel(modelPath);

	MV1SetPosition(handle, pos);
}

void CharacterBase::Update(Object& object)
{

}

void CharacterBase::Draw()
{
	MV1DrawModel(handle);
}

void CharacterBase::ChangeAnimation(int animIndex)
{
	if (currentAnimAttach != -1)
	{
		// 前アニメ削除
		MV1DetachAnim(handle, currentAnimAttach);
	}

	// 新アニメ設定
	currentAnimAttach = MV1AttachAnim(handle, animIndex);

	// 再生時間リセット
	animTime = 0.0f;
}

void CharacterBase::UpdateGravity(float deltaTime, Object& object)
{
	velocityY += gravity * deltaTime;

	VECTOR newPos = pos;

	newPos.y += velocityY * deltaTime;

	// Box衝突
	if (!object.CheckCollision(newPos, radius))
	{
		pos.y = newPos.y;

		isGround = false;
	}
	else
	{
		if (velocityY <= 0.0f)
		{
			isGround = true;
		}
	}

	// 地面
	if (pos.y < groundHeight)
	{
		pos.y = groundHeight;

		velocityY = 0.0f;

		isGround = true;
	}
}

void CharacterBase::UpdateAnimation(float deltaTime)
{
	animTime += 60.0f * deltaTime;

	float totalTime = MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	bool isLoopAnim =
		currentState == AnimState::Idle ||
		currentState == AnimState::Walk ||
		currentState == AnimState::JumpLoop;

	// ループ
	if (isLoopAnim && animTime >= totalTime)
	{
		animTime = 0.0f;
	}

	MV1SetAttachAnimTime(handle, currentAnimAttach, animTime);
}