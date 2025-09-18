#include "Effect.h"
#include "EffekseerForDXLib.h"
#include <assert.h>

#include <string>
#include <vector>
#include <time.h>

Effect::Effect():
	m_lossOfBloodEffectHandle(-1),
	m_concentrationLineEffectHandle(-1),
	m_muzzleFlashEffectHandles{ -1, -1, -1, -1, -1 }
{
	// 乱数のシードを設定
	srand(time(NULL));

	// エフェクトハンドルの読み込み
	m_muzzleFlashEffectHandles[0] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc", 2.0f);
	m_muzzleFlashEffectHandles[1] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash2.efkefc", 2.0f);
	m_muzzleFlashEffectHandles[2] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash3.efkefc", 2.0f);
	m_muzzleFlashEffectHandles[3] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash4.efkefc", 3.5f);
	m_muzzleFlashEffectHandles[4] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash5.efkefc", 3.0f);
	for (int i = 0; i < 5; ++i)
	{
		assert(m_muzzleFlashEffectHandles[i] != -1);
	}

	// 出血エフェクトハンドルの読み込み
	m_lossOfBloodEffectHandle = LoadEffekseerEffect("data/Effekseer/LossOfBlood.efkefc", 2.5f);
	assert(m_lossOfBloodEffectHandle != -1);
	// 集中線エフェクトハンドルの読み込み
	m_concentrationLineEffectHandle = LoadEffekseerEffect("data/Effekseer/ConcentrationLine.efkefc", 1.0f);
	assert(m_concentrationLineEffectHandle != -1);
}

Effect::~Effect()
{
	// エフェクトのハンドルを削除
	for (int i = 0; i < 5; ++i)
	{
		DeleteEffekseerEffect(m_muzzleFlashEffectHandles[i]);
	}
	DeleteEffekseerEffect(m_lossOfBloodEffectHandle);
	DeleteEffekseerEffect(m_concentrationLineEffectHandle);
}

void Effect::Init()
{
}

void Effect::Update()
{
	// 3Dエフェクトの更新
	UpdateEffekseer3D();
}

void Effect::Draw()
{
	// 3Dエフェクトの描画
	DrawEffekseer3D();
}

// マズルフラッシュを再生する
void Effect::PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ)
{
	int index = rand() % 5;
	if (m_muzzleFlashEffectHandles[index] != -1)
	{
		int handle = PlayEffekseer3DEffect(m_muzzleFlashEffectHandles[index]);
		if (handle != -1)
		{
			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
			SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
		}
	}
}

// 出血エフェクトを再生する
void Effect::PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ)
{
	if (m_lossOfBloodEffectHandle != -1)
	{
		int handle = PlayEffekseer3DEffect(m_lossOfBloodEffectHandle);
		if (handle != -1)
		{
			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
			SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
			SetSpeedPlayingEffekseer3DEffect(handle, 5.0f); // 再生速度を5倍に
		}
	}
}

// 集中線エフェクトを再生する
int Effect::PlayConcentrationLine(float x, float y, float z, float scale)
{
	if (m_concentrationLineEffectHandle != -1)
	{
		int handle = PlayEffekseer3DEffect(m_concentrationLineEffectHandle);
		if (handle != -1)
		{
			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
			SetScalePlayingEffekseer3DEffect(handle, scale, scale, scale);
		}
		return handle;
	}
	return -1;
}
