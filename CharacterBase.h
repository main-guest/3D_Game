#pragma once
#include "Dxlib.h"

enum class AnimState
{
	Idle,
	Walk,
	JumpStart,
	JumpLoop,
	JumpEnd,
	Attack01,
	Hit
};

class CharacterBase
{
public:
	CharacterBase();
	virtual ~CharacterBase();

	virtual void Init(const TCHAR* modelPath);

	void UpdateAnimation(float dt);

	// ===== Position =====
	const VECTOR& GetPos() const { return pos; }
	void SetPos(const VECTOR& p) { pos = p; }

	// ===== Velocity =====
	const VECTOR& GetVelocity() const { return velocity; }
	VECTOR& GetVelocity() { return velocity; }

	void SetVelocity(const VECTOR& v)
	{
		velocity = v;
	}

	// ===== Capsule =====
	float GetRadius() const { return radius; }
	float GetHeight() const { return height; }

	// ===== Forward =====
	VECTOR GetForward() const;

protected:
	
	void ChangeAnimation(int animIndex, bool loop);

protected:
	// ===== モデル =====
	int handle;

	// ===== 座標 =====
	VECTOR pos;
	VECTOR velocity;

	float characterAngle;

	// ===== Capsule =====
	float radius;
	float height;

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
	int hitAnim;

	// ===== 定数 ====
	float speed;						// 移動速度
	const float gravity = -1200.0f;		// 当たり判定半径
	const float jumpPower = 500.0f;
	const float groundHeight = 0.0f;

	bool isGround = true;
	bool loopAnim = true;
};
