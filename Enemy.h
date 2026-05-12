#pragma once
#include "CharacterBase.h"

class Enemy :public CharacterBase
{
public:
	void Init(VECTOR startPos);

	void Update(float deltaTime, Object& object, VECTOR playerPos);
};