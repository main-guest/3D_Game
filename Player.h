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

class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update(float cameraAngle, Object& object);

	void ChangeAnimation(int animIndex);

	void Draw();

	VECTOR GetPos() const { return pos; }

private:
	// ===== モデル =====
	int handle;

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

	bool jumpRequest;

	int oldMouse;

	const float jumpStartFrame = 52.0f;

	const float speed = 2.5f;
	const float gravity = -0.2f;
	const float radius = 10.0f;			//　PLAYER当たり判定半径
	const float jumpPower = 7.0f;

	const float groundHeight = 0.0f;
};
