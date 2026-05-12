#pragma once
#include "CharacterBase.h"

class Player : public CharacterBase
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update(float cameraAngle, Object& object);

private:
	// ===== “ü—Í =====
	int oldMouse;

	// ===== ó‘Ô =====

	bool jumpRequest;

	const float jumpStartFrame = 56.0f;
};
