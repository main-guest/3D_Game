#include "CharacterBase.h"

CharacterBase::CharacterBase()
	:handle(-1),
	pos(VGet(0.0f, 0.0f, 0.0f)),
	characterAngle(0.0f),
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
	currentAnimAttach = MV1AttachAnim(handle, animIndex, -1, FALSE);

	// 再生時間リセット
	animTime = 0.0f;
}

void CharacterBase::UpdateAnimation(float dt)
{

	if (currentAnimAttach == -1) return;

	animTime += 60.0f * dt;

	MV1SetAttachAnimTime(handle, currentAnimAttach, animTime);
}