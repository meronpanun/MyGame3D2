#include "Effect.h"
#include "EffekseerForDXLib.h"
#include <assert.h>

Effect::Effect() :
	m_muzzleFlashEffectHandle(-1)
{
	// エフェクトのハンドルを取得
	m_muzzleFlashEffectHandle = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc", 40.7f);
	assert(m_muzzleFlashEffectHandle != -1);
}

Effect::~Effect()
{
	// エフェクトのハンドルを削除
	DeleteEffekseerEffect(m_muzzleFlashEffectHandle);
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
	if (m_muzzleFlashEffectHandle != -1)
	{
		int handle = PlayEffekseer3DEffect(m_muzzleFlashEffectHandle);
		if (handle != -1)
		{
			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
			SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
		}
	}
}