//#include "Effect.h"
//#include "EffekseerForDXLib.h"
//#include "DxLib.h"
//#include <assert.h>
//
//Effect::Effect() :
//	muzzleFlashEffectHandle(-1)
//{
//	// エフェクトのハンドルを取得
//	muzzleFlashEffectHandle = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc", 0.7f);
//	assert(muzzleFlashEffectHandle != -1);
//}
//
//Effect::~Effect()
//{
//	// エフェクトのハンドルを削除
//	DeleteEffekseerEffect(muzzleFlashEffectHandle);
//}
//
//void Effect::Init()
//{
//
//}
//
//void Effect::Update()
//{
//	// 3Dエフェクトの更新
//	UpdateEffekseer3D();
//}
//
//void Effect::Draw()
//{
//	// 3Dエフェクトの描画
//	DrawEffekseer3D();
//}
//
//// マズルフラッシュを再生する
//void Effect::PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ)
//{
//	if (muzzleFlashEffectHandle != -1)
//	{
//		int handle = PlayEffekseer3DEffect(muzzleFlashEffectHandle);
//		if (handle != -1)
//		{
//			SetPosPlayingEffekseer3DEffect(handle, x, y, z);
//			SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
//		}
//	}
//}