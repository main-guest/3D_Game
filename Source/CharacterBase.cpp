#include <cmath>
#include "CharacterBase.h"

CharacterBase::CharacterBase()
	:handle(-1),
	pos(VGet(0.0f, 0.0f, 0.0f)),
	velocity(VGet(0,0,0)),
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
}

void CharacterBase::ChangeAnimation(int animIndex, bool loop)
{

	if (animIndex == currentAnimIndex &&
		currentState != AnimState::Hit)
		return;

	currentAnimIndex = animIndex;

	// ===== ブレンド中なら強制終了 =====
	if (isBlending)
	{
		if (prevAnimAttach != -1)
		{
			MV1DetachAnim(handle, prevAnimAttach);
			prevAnimAttach = -1;
		}

		isBlending = false;
	}

	bool useBlend = true;

	// ===== ブレンドしないアニメ =====
	/*switch (currentState)
	{
	case AnimState::JumpEnd:
		useBlend = false;

		break;
	}*/

	// ===== 即切り替え =====
	if (!useBlend)
	{
		if (currentAnimAttach != -1)
		{
			MV1DetachAnim(handle, currentAnimAttach);
		}

		currentAnimAttach = MV1AttachAnim(handle, animIndex, -1, TRUE);

		animTime = 0.0f;

		MV1SetAttachAnimTime(handle, currentAnimAttach, 0.0f);

		MV1SetAttachAnimBlendRate(handle, currentAnimAttach, 1.0f);

		animSpeed = 1.0f;
		loopAnim = loop;

		prevAnimAttach = -1;
		isBlending = false;

		return;
	}

	// ===== ブレンド切り替え =====
	prevAnimAttach = currentAnimAttach;

	currentAnimAttach = MV1AttachAnim(handle, animIndex, -1, TRUE);

	animTime = 0.0f;

	MV1SetAttachAnimTime(handle, currentAnimAttach, 0.0f);

	MV1SetAttachAnimBlendRate(handle, currentAnimAttach, 0.0f);

	if (prevAnimAttach != -1)
	{
		MV1SetAttachAnimBlendRate(handle, prevAnimAttach, 1.0f);

		blendTime = 0.0f;
		isBlending = true;
	}
	else
	{
		MV1SetAttachAnimBlendRate(handle, currentAnimAttach, 1.0f);

		isBlending = false;
	}

	// ===== 初期化 =====
	// デフォルト再生速度
	animSpeed = 1.0f;

	// ループ設定保存
	loopAnim = loop;
}

void CharacterBase::UpdateAnimation(float dt)
{
	if (currentAnimAttach == -1) 
		return;

	// ===== アニメ時間更新 =====
	animTime += 60.0f * dt * animSpeed;

	float totalTime =MV1GetAttachAnimTotalTime(handle, currentAnimAttach);

	if (totalTime <= 0.0f)
		return;

	// ===== ブレンド更新 =====
	if (isBlending)
	{
		blendTime += dt;

		float rate = blendTime / blendDuration;

		if (rate > 1.0f)
		{
			rate = 1.0f;
		}

		// 新アニメ
		MV1SetAttachAnimBlendRate(handle, currentAnimAttach, rate);

		// 旧アニメ
		if (prevAnimAttach != -1)
		{
			MV1SetAttachAnimBlendRate(handle, prevAnimAttach, 1.0f - rate);
		}

		// ブレンド終了
		if (rate >= 1.0f)
		{
			if (prevAnimAttach != -1)
			{
				MV1DetachAnim(handle, prevAnimAttach);

				prevAnimAttach = false;
			}

			isBlending = false;
		}
	}

	// ループ
	if (animTime >= totalTime)
	{
		if (loopAnim)
		{
			animTime = fmodf(animTime, totalTime);
		}
		else
		{
			animTime = totalTime;
		}
	}

	MV1SetAttachAnimTime(handle, currentAnimAttach, animTime);
}

VECTOR CharacterBase::GetForward() const
{
	VECTOR forward;

	forward.x = -sinf(characterAngle);
	forward.y = 0.0f;
	forward.z = -cosf(characterAngle);

	return forward;
}