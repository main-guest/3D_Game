#pragma once
#include "DxLib.h"

class Weapon
{
public:
	void Init(const TCHAR* path);

	void Update(int parentHandle, const TCHAR* boneName, float characterAngle);

	void SetRotation(VECTOR r);

	void Draw();

private:
	int handle = -1;

	VECTOR pos = VGet(0, 0, 0);
	VECTOR rot = VGet(0, 0, 0);
};