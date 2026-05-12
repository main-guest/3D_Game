#pragma once
#include "Dxlib.h"
#include "Object.h"

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

	virtual void Init(const CHAR* modelPath);
	virtual void Update(Object& object);
	virtual void Draw();

	VECTOR GetPos() const { return pos; }

protected:
	// ===== モデル =====
	int handle;

	// ===== 座標 =====
	VECTOR pos;
	float characterAngle;

	float velocityY;
	bool isGround;

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

	void UpdateGravity(Object& object);

	void UpdateAnimation();

	// ===== 定数 ====
	const float speed = 2.5f;
	const float gravity = -0.2f;
	const float radius = 10.0f;		// 当たり判定半径
	const float jumpPower = 7.0f;
	const float groundHeight = 0.0f;
};
