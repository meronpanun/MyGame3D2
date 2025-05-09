#include "SceneMain.h"
#include "SceneTitle.h"
#include "SceneResult.h"
#include "SceneOption.h"
#include "DxLib.h"
#include "Player.h"
#include "Mouse.h"
#include "Game.h"
#include "Camera.h"
#include <cassert>

namespace
{
    constexpr int	kInitialCountdownValue = 3;	   // �����J�E���g�_�E���̒l
    constexpr int	kMainCountdownValue = 10;   // ���C���J�E���g�_�E���̒l
    constexpr int	kButtonWidth = 200;  // �{�^���̕�
    constexpr int	kButtonHeight = 50;   // �{�^���̕�
    constexpr int	kFontSize = 48;   // �t�H���g�T�C�Y
    constexpr int   kReticleSize = 2;    // ���e�B�N���̃T�C�Y
    constexpr float kScreenCenterOffset = 0.5f; // ��ʒ����̃I�t�Z�b�g
    constexpr int   kButtonYOffset = 70;   // �{�^����Y���I�t�Z�b�g
    constexpr int   kButtonSpacing = 20;   // �{�^���Ԃ̃X�y�[�X

    // �{�^���̈ʒu���v�Z
    constexpr int kReturnButtonX = 210; // �߂�{�^���̔w�i�̍���X���W
    constexpr int kReturnButtonY = 290; // �߂�{�^���̔w�i�̍���Y���W
    constexpr int kOptionButtonX = 210; // �I�v�V�����{�^���̔w�i�̍���X���W
    constexpr int kOptionButtonY = 120; // �I�v�V�����{�^���̔w�i�̍���Y���W

    //�J�������񂷑��x
    constexpr float kCameraRotaSpeed = 0.001f;
}

SceneMain::SceneMain() :
    m_isPaused(false),
    m_isEscapePressed(false),
    m_isReturningFromOption(false),
    m_cameraSensitivity(Game::g_cameraSensitivity),
    m_pCamera(std::make_unique<Camera>()),
    m_skyDomeHandle(-1),
    m_skyDomeTextureHandle(-1)
{
    m_skyDomeHandle = MV1LoadModel("data/image/Dome.mv1");
    assert(m_skyDomeHandle != -1);
}

SceneMain::~SceneMain()
{
    MV1DeleteModel(m_skyDomeHandle); // �X�J�C�h�[���̃��f�����폜
}

void SceneMain::Init()
{
    // �v���C���[�̏�����
    m_pPlayer = std::make_unique<Player>();
    m_pPlayer->Init();

    // �O���[�o���ϐ�����J�������x���擾���Đݒ�
    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    // �|�[�Y��Ԃɉ����ă}�E�X�J�[�\���̕\����؂�ւ���
    SetMouseDispFlag(m_isPaused);

    //�X�J�C�h�[���̐ݒ�
    MV1SetPosition(m_skyDomeHandle, VGet(0, 200, 0));

    // �X�J�C�h�[���̃X�P�[����ݒ�
    MV1SetScale(m_skyDomeHandle, VGet(10.0f, 10.0f, 10.0f));

    // �t���O�����Z�b�g
    m_isReturningFromOption = false;
}

SceneBase* SceneMain::Update()
{
    // �X�J�C�h�[���̉�]���X�V
    MV1SetRotationXYZ(m_skyDomeHandle, VGet(0, MV1GetRotationXYZ(m_skyDomeHandle).y + kCameraRotaSpeed, 0));

    // �G�X�P�[�v�L�[�������ꂽ��|�[�Y��Ԃ�؂�ւ���
    if (CheckHitKey(KEY_INPUT_ESCAPE))
    {
        if (!m_isEscapePressed)
        {
            m_isPaused = !m_isPaused;
            SetMouseDispFlag(m_isPaused); // �|�[�Y��Ԃɉ����ă}�E�X�J�[�\���̕\����؂�ւ���
            m_isEscapePressed = true;

            if (m_isPaused)
            {
                // �|�[�Y�ɓ������^�C�~���O�Ō��݂̎��Ԃ��L�^
                m_pauseStartTime = std::chrono::steady_clock::now();
            }
            else
            {
                // �|�[�Y����߂����^�C�~���O�Ōo�ߎ��Ԃ��v�Z���A�J�E���g�_�E���̊J�n���Ԃ𒲐�
                auto now = std::chrono::steady_clock::now();
                auto pauseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pauseStartTime).count();
            }
        }
    }
    else
    {
        m_isEscapePressed = false;
    }

    // �|�[�Y���͍X�V�������s��Ȃ�
    if (m_isPaused)
    {
        // �}�E�X�̍��N���b�N���`�F�b�N
        if (Mouse::IsTriggerLeft())
        {
            // �}�E�X�̈ʒu���擾
            Vec2 mousePos = Mouse::GetPos();

            // �}�E�X���^�C�g���ɖ߂�{�^���͈͓̔��ɂ��邩�`�F�b�N
            if (mousePos.x >= kReturnButtonX && mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY && mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                return new SceneTitle(true); // ���S�X�L�b�v�t���O��L���ɂ��ă^�C�g���V�[���ɖ߂�
            }

            // �}�E�X���I�v�V�����{�^���͈͓̔��ɂ��邩�`�F�b�N
            if (mousePos.x >= kOptionButtonX && mousePos.x <= kOptionButtonX + kButtonWidth &&
                mousePos.y >= kOptionButtonY && mousePos.y <= kOptionButtonY + kButtonHeight)
            {
                m_isReturningFromOption = true; // �I�v�V�����V�[������߂�t���O��ݒ�
                return new SceneOption(this);
            }
        }
        return this;
    }

    // �v���C���[�̍X�V
    m_pPlayer->Update();

    // �������Ȃ���΃V�[���J�ڂ��Ȃ�(�Q�[����ʂ̂܂�)
    return this;
}

void SceneMain::Draw()
{
    MV1DrawModel(m_skyDomeHandle);
    MV1SetTextureGraphHandle(m_skyDomeHandle, 0, m_skyDomeTextureHandle, false);

    // �v���C���[�̕`��
    m_pPlayer->Draw();

    // �|�[�Y���̓|�[�Y���j���[��`�悷��
    if (m_isPaused)
    {
        DrawPauseMenu();
    }
}

void SceneMain::DrawPauseMenu()
{
    // �}�E�X�̈ʒu���擾
    Vec2 mousePos = Mouse::GetPos();

    // �������Ȕ�`��
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // �����x��ݒ�
    DrawBox(50, 50, Game::kScreenWidth - 50, Game::kScreenHeigth - 50, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // �����x�����Z�b�g
}

// �|�[�Y��Ԃ�ݒ肷��
void SceneMain::SetPaused(bool paused)
{
    m_isPaused = paused;
    SetMouseDispFlag(m_isPaused); // �|�[�Y��Ԃɉ����ă}�E�X�J�[�\���̕\����؂�ւ���
}

// �J�����̊��x��ݒ肷��
void SceneMain::SetCameraSensitivity(float sensitivity)
{
    m_cameraSensitivity = sensitivity;
    if (m_pCamera)
    {
        m_pCamera->SetSensitivity(sensitivity);
    }
}
