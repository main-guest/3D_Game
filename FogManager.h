#pragma once
#include "DxLib.h"

class FogManager
{
public:
	void Init();

	void Update(VECTOR camPos, VECTOR playerPos);

	void SetWeather(int type);

private:
	float fogStart;
	float fogEnd;

	float targetStart;
	float targetEnd;

	int weatherType;
};