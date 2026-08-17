#pragma once
#include <unordered_map>
#include <string>
#include "DxLib.h"
#include "EffekseerforDxLib.h"

class EffectManager
{
public:
	static EffectManager& Instance()
	{
		static EffectManager instance;
		return instance;
	}

	// 初期化
	bool Init();

	// 更新
	void Update();

	// 描画
	void Draw();

	// 解放
	void Release();

	// エフェクト読み込み
	void Load(const std::string& name, 
		const std::string& path,
		float scale);

	// 再生
	int Play(const std::string& name, 
		VECTOR pos, 
		VECTOR rot,
		float scale);

private:
	EffectManager() = default;
	~EffectManager() = default;

	// コピー禁止
	EffectManager(const EffectManager&) = delete;
	EffectManager& operator=(const EffectManager&) = delete;

private:
	// 読み込み済みエフェクト
	std::unordered_map<std::string, int> effects;
};