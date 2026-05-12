#pragma once
#include "CharacterBase.h"

class Enemy :public CharacterBase
{
public:
	Enemy();
	virtual ~Enemy();

	void Init(VECTOR startPos);

	void Update(float deltaTime, Object& object, VECTOR playerPos);

private:
	// ==== AI ====
	float searchRange = 500.0f;

	// ==== ƒWƒƒƒ“ƒv ====
	bool jumpRequest;

	const float jumpStartFrame = 50.0f;
};