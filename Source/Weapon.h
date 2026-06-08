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

	const VECTOR& GetRootPosition() const
	{
		return rootPos;
	}

	const VECTOR& GetTipPosition() const
	{
		return tipPos;
	}

	const VECTOR& GetPrevRootPosition() const
	{
		return prevRootPos;
	}

	const VECTOR& GetPrevTipPosition() const
	{
		return prevTipPos;
	}

	const VECTOR& GetDebugPos()const
	{
		return debugPos;
	}

	int GetHandle()const
	{
		return handle;
	}

private:
	int handle = -1;

	VECTOR pos = VGet(0, 0, 0);

	MATRIX worldMatrix = MGetIdent();

	VECTOR rootPos{};
	VECTOR tipPos{};

	VECTOR prevRootPos{};
	VECTOR prevTipPos{};

	// åïêÊà íu
	const float BLADE_LENGTH = 150.0f;

	WeaponData data;

	VECTOR debugPos;
};