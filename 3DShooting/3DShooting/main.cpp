#include "DxLib.h"
#include "game.h"
#include "SceneManager.h"
#include "EffekseerForDXLib.h"

// �v���O������ WinMain ����n�܂�܂�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// �t���X�N���[���ł͂Ȃ��A�E�C���h�E���[�h�ŊJ���悤�ɂ���
	ChangeWindowMode(Game::kDefaultWindowMode);
	// ��ʂ̃T�C�Y��ύX����
	SetGraphMode(Game::kScreenWidth, Game::kScreenHeigth, Game:: kColorBitNum);

	if (DxLib_Init() == -1)		// �c�w���C�u��������������
	{
		return -1;			// �G���[���N�����璼���ɏI��
	}
	// �`���𗠉�ʂɂ���
	SetDrawScreen(DX_SCREEN_BACK);

	// 3D�֘A�̐ݒ�
	SetUseZBuffer3D(true);	 // 3D�`���ZBuffer���g�p����
	SetWriteZBuffer3D(true); // 3D�`���ZBuffer�ɏ�������
	SetUseBackCulling(true); // ���ʃJ�����O��L���ɂ���

	//Effekseer�֌W������
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	// �����ɂ͉�ʂɕ\������ő�p�[�e�B�N������ݒ肷��B
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		return -1;
	}
	// �t���X�N���[���E�C���h�E�̐؂�ւ��Ń��\�[�X��������̂�h��
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// DX���C�u�����̃f�o�C�X���X�g�������̃R�[���o�b�N��ݒ肷��
	Effekseer_SetGraphicsDeviceLostCallbackFunctions();

	SceneManager* pScene = new SceneManager();
	pScene->Init();

	// �Q�[�����[�v
	while (ProcessMessage() == 0)	// Windows���s��������҂K�v������
	{
		// DX���C�u�����̃J������Effekseer�̃J�����𓯊�����B
		Effekseer_Sync3DSetting();

		// �G�X�P�[�v�L�[�������ꂽ�烋�[�v�𔲂���
		if (CheckHitKey(KEY_INPUT_RETURN))
		{
			break;
		}

		// ����̃��[�v���n�܂������Ԃ��o���Ă���
		LONGLONG time = GetNowHiPerformanceCount();

		// ��ʑS�̂��N���A����
		ClearDrawScreen();


		// �Q�[���̏���
		pScene->Update();
		pScene->Draw();

		// ��ʂ̐؂�ւ���҂K�v������
		ScreenFlip();	// 1/60�b�o�߂���܂ő҂�

		// FPS(Frame Per Second)60�ɌŒ�
		while (GetNowHiPerformanceCount() - time < 16667)
		{
		}
	}

	// Effekseer���I������B
	Effkseer_End();

	DxLib_End();				// �c�w���C�u�����g�p�̏I������

	return 0;				// �\�t�g�̏I�� 
}