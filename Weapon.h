#pragma once
#include "DxLib.h"
#include "WeaponData.h"

class Weapon
{
public:
	void Init(const TCHAR* path);

	void Update(int parentHandle, const TCHAR* boneName, float characterAngle);

	void Draw();

	void SetData(const WeaponData& d);

private:
	int handle = -1;

	VECTOR pos = VGet(0, 0, 0);

	VECTOR rotOffset;

	WeaponData data;
};