#pragma once
#include "Dxlib.h"

enum class AnimState
{
	Idle,
	Walk,
	Chase,

	Dash,

	JumpStart,
	JumpRise,
	JumpFall,
	JumpEnd,

	Attack,

	Hit,

	Dodge,

	Dead
};

class CharacterBase
{
public:
	CharacterBase();
	virtual ~CharacterBase();

	virtual void Init(const TCHAR* modelPath);

	void SetAnimSpeed(float speed)
	{
		animSpeed = speed;
	}

	float GetAnimSpeed() const
	{
		return animSpeed;
	}

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

	int currentAnimIndex = -1;

	float animTime = 0.0f;

	// 現在のアニメ再生速度
	float animSpeed = 1.0f;

	// アニメーションブレンド
	int currentAnimAttach = -1;
	int prevAnimAttach = -1;

	float blendTime = 0.0f;
	float blendDuration = 0.2;

	bool isBlending = false;
	bool loopAnim = true;

	// アニメーションハンドル
	int idleAnim;

	int walk_fAnim;
	int walk_bAnim;
	int walk_rAnim;
	int walk_lAnim;

	int chaseAnim;

	int dash_f01Anim;
	int dash_f02Anim;
	int dash_bAnim;
	int dash_rAnim;
	int dash_lAnim;

	int jumpStartAnim;
	int dashJumpStartAnim;
	int jumpRiseAnim;
	int jumpFallAnim;
	int jumpEndAnim;

	int handAttackAnim;
	int gunAttackAnim;
	int swordAttack01Anim;
	int swordAttack02Anim;
	int swordAttack03Anim;
	int swordAttack04Anim;

	int hitAnim;

	int dodge_fAnim;
	int dodge_b01Anim;
	int dodge_b02Anim;

	int deadAnim;

	// ===== 定数 ====
	float speed;						// 移動速度
	const float gravity = -1200.0f;
	const float jumpPower = 500.0f;
	const float groundHeight = 0.0f;

	bool isGround = true;
};
