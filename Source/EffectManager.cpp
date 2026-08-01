#include "EffectManager.h"

bool EffectManager::Init()
{
	return true;
}

void EffectManager::Update()
{
	UpdateEffekseer3D();
}

void EffectManager::Draw()
{
	DrawEffekseer3D();
}

void EffectManager::Release()
{
	for (auto& e : effects)
	{
		DeleteEffekseerEffect(e.second);
	}

	effects.clear();
}

void EffectManager::Load(const std::string& name,
	const std::string& path,
	float scale)
{
	int handle = LoadEffekseerEffect(path.c_str(), scale);

	if (handle == -1)
	{
		printfDx("Effect Load Failed : %s\n", path.c_str());

		return;
	}

	effects[name] = handle;
}

int EffectManager::Play(const std::string& name,
	VECTOR pos,
	float scale)
{
	auto itr = effects.find(name);

	if (itr == effects.end())
	{
		printfDx("Effect Not Found : %s\n", name.c_str());

		return -1;
	}

	int playHandle =
		PlayEffekseer3DEffect(itr->second);

	SetPosPlayingEffekseer3DEffect(
		playHandle,
		pos.x,
		pos.y,
		pos.z
	);

	SetScalePlayingEffekseer3DEffect(
		playHandle,
		scale,
		scale,
		scale
	);

	return playHandle;
}