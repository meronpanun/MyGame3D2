//#include "Effect.h"
//#include "EffekseerForDXLib.h"
//#include "DxLib.h"
//#include <assert.h>
//
//Effect::Effect() :
//	muzzleFlashEffectHandle(-1)
//{
//	// �G�t�F�N�g�̃n���h�����擾
//	muzzleFlashEffectHandle = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc", 0.7f);
//	assert(muzzleFlashEffectHandle != -1);
//}
//
//Effect::~Effect()
//{
//	// �G�t�F�N�g�̃n���h�����폜
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
//	// 3D�G�t�F�N�g�̍X�V
//	UpdateEffekseer3D();
//}
//
//void Effect::Draw()
//{
//	// 3D�G�t�F�N�g�̕`��
//	DrawEffekseer3D();
//}
//
//// �}�Y���t���b�V�����Đ�����
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