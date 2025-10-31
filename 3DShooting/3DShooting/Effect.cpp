#include "Effect.h"
#include "EffekseerForDXLib.h"
#include <assert.h>
#include <string>
#include <vector>
#include <time.h>

namespace
{
	// 各エフェクト拡大率
	constexpr float kMuzzleFlashEffectScale       = 2.0f; 
	constexpr float kMuzzleFlashEffectScale2      = 3.0f; 
	constexpr float kMuzzleFlashEffectScale3      = 3.5f; 
	constexpr float kLossOfBloodEffectScale       = 2.5f;
	constexpr float kConcentrationLineEffectScale = 1.0f;
	constexpr float kGuardEffectScale             = 10.5f;
}

Effect::Effect():
	m_lossOfBloodEffectHandle(-1),
	m_concentrationLineEffectHandle(-1),
	m_muzzleFlashEffectHandles{ -1, -1, -1, -1, -1 }
{
	// 乱数のシードを設定
	srand(time(NULL));

	// エフェクトハンドルの読み込み
	m_muzzleFlashEffectHandles[0] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc", kMuzzleFlashEffectScale);
	m_muzzleFlashEffectHandles[1] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash2.efkefc", kMuzzleFlashEffectScale);
	m_muzzleFlashEffectHandles[2] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash3.efkefc", kMuzzleFlashEffectScale);
	m_muzzleFlashEffectHandles[3] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash4.efkefc", kMuzzleFlashEffectScale3);
	m_muzzleFlashEffectHandles[4] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash5.efkefc", kMuzzleFlashEffectScale2);
	for (int i = 0; i < 5; ++i)
	{
		assert(m_muzzleFlashEffectHandles[i] != -1);
	}

	// 出血エフェクトハンドルの読み込み
	m_lossOfBloodEffectHandle = LoadEffekseerEffect("data/Effekseer/LossOfBlood.efkefc", kLossOfBloodEffectScale);
	assert(m_lossOfBloodEffectHandle != -1);
	// 集中線エフェクトハンドルの読み込み
	m_concentrationLineEffectHandle = LoadEffekseerEffect("data/Effekseer/ConcentrationLine.efkefc", kConcentrationLineEffectScale);
	assert(m_concentrationLineEffectHandle != -1);

	// ガードエフェクトハンドルの読み込み
	m_guardEffectHandle = LoadEffekseerEffect("data/Effekseer/Circle.efkefc", kGuardEffectScale);
	assert(m_guardEffectHandle != -1);
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
	DeleteEffekseerEffect(m_guardEffectHandle);
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

// ガードエフェクトを再生する
int Effect::PlayGuardEffect(float x, float y, float z, float rotX, float rotY, float rotZ)
{
	if (m_guardEffectHandle != -1)
	{
		int handle = PlayEffekseer3DEffect(m_guardEffectHandle);
		if (handle != -1)
		{
			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
			SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
		}
		return handle;
	}
	return -1;
}
