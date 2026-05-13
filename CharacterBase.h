#pragma once
#include "Dxlib.h"

enum class AnimState
{
	Idle,
	Walk,
	JumpStart,
	JumpLoop,
	JumpEnd,
	Attack01
};

class CharacterBase
{
public:
	CharacterBase();
	virtual ~CharacterBase();

	virtual void Init(const TCHAR* modelPath);
	virtual void Draw();

	VECTOR GetPos() const { return pos; }

protected:
	// ===== モデル =====
	int handle;

	// ===== 座標 =====
	VECTOR pos;
	float characterAngle;

	// ===== アニメーション =====
	AnimState currentState;

	int currentAnimAttach;
	float animTime;

	int idleAnim;
	int walkAnim;
	int jumpStartAnim;
	int jumpLoopAnim;
	int jumpEndAnim;
	int attack01Anim;

	// ===== 共通関数 ====
	void ChangeAnimation(int animIndex);

	void UpdateAnimation(float dt);

	// ===== 定数 ====
	const float speed = 220.0f;
	const float gravity = -1200.0f;
	const float radius = 10.0f;		// 当たり判定半径
	const float jumpPower = 500.0f;
	const float groundHeight = 0.0f;

	bool isGround = true;
};
