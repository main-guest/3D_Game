#pragma once
#include "CharacterBase.h"

class Enemy :public CharacterBase
{
public:
	void Init(VECTOR startPos);

	void Update(Object& object, VECTOR playerPos);
};