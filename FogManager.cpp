#include "FogManager.h"

void FogManager::Init()
{
	fogStart = 500.0f;
	fogEnd = 3000.0f;

	targetStart = fogStart;
	targetEnd = fogEnd;

	weatherType = 0;

	SetFogEnable(TRUE);
	SetFogColor(180, 210, 240);
}

void FogManager::Update(VECTOR camPos, VECTOR playerPos)
{
	float height = (playerPos.y + camPos.y) * 0.5f;

	// ===== 天候ベースのターゲット =====
	switch (weatherType)
	{
	case 0: // 晴れ
		targetStart = 2000.0f;
		targetEnd = 16000.0f;
		break;

	case 1: // 霧
		targetStart = 500.0f;
		targetEnd = 2500.0f;
		break;

	case 2: // ダンジョン
		targetStart = 200.0f;
		targetEnd = 1200.0f;
		break;
	}

	// ===== 高度で微調整 =====
	float t = height / 500.0f;
	if (t < 0)t = 0;
	if (t > 1)t = 1;

	targetStart += 300.0f * t;
	targetEnd += 600.0f * t;

	// ===== なめらか補間 =====
	fogStart += (targetStart - fogStart) * 0.08f;
	fogEnd += (targetEnd - fogEnd) * 0.08f;

	// ===== 適用 =====
	SetFogStartEnd(fogStart, fogEnd);
}

void FogManager::SetWeather(int type)
{
	weatherType = type;
}