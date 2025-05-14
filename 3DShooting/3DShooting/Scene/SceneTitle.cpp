#include "SceneTitle.h"
#include "DxLib.h"
#include "Game.h"
#include "SceneMain.h"
#include "Mouse.h"
#include <cassert>

namespace
{
    // �^�C�g�����S�̕��ƍ���
	constexpr int kLogoWidth  = 1920;
    constexpr int kLogoHeight = 1080;

	// �^�C�g�����S�̕\���ʒu
	constexpr int kLogoX = Game::kScreenWidth * -0.5f;
	constexpr int kLogoY = (Game::kScreenHeigth - kLogoHeight) * 0.5f;

	constexpr int kFadeDuration = 60; // �t�F�[�h�C���E�t�F�[�h�A�E�g�̃t���[����
	constexpr int kWaitDuration = 60; // �t�F�[�h�C����̑ҋ@���ԁi�t���[�����j

    // �p�l���̕��ƍ���
    constexpr int kPanelWidth  = 350; // �p�l���̕�
    constexpr int kPanelHeight = 240; // �p�l���̍���

    // �X�^�[�g�{�^���͈̔͂�萔��
    constexpr int kStartButtonX1 = (Game::kScreenWidth - kPanelWidth) * 0.5f; // �X�^�[�g�{�^���̍���X���W
    constexpr int kStartButtonY1 = 10;                                        // �X�^�[�g�{�^���̍���Y���W
    constexpr int kStartButtonX2 = kStartButtonX1 + kPanelWidth;              // �X�^�[�g�{�^���̉E��X���W
    constexpr int kStartButtonY2 = 300;                                       // �X�^�[�g�{�^���̉E��Y���W
}

SceneTitle::SceneTitle(bool skipLogo):
    m_logoHandle(-1),
	m_fadeAlpha(0),
	m_fadeFrame(0),
	m_sceneFadeAlpha(0),
	m_waitFrame(0),
	m_isFadeComplete(false),
	m_isFadeOut(false),
	m_skipLogo(skipLogo),
	m_isSceneFadeIn(false)
{
    // �^�C�g�����S�摜��ǂݍ���
    m_logoHandle = LoadGraph("data/image/TitleLogo.png");
    assert(m_logoHandle != -1);

    // ���S���X�L�b�v����ꍇ�A�t�F�[�h�����Ƒҋ@���Ԃ��X�L�b�v���A�{�^�������L����
    if (m_skipLogo)
    {
        m_isFadeComplete = true;
        m_isFadeOut      = true;
        m_waitFrame      = kWaitDuration; // �ҋ@���Ԃ��X�L�b�v
        m_fadeAlpha      = 0;             // �t�F�[�h�A�E�g�ς݂̏�Ԃɐݒ�
        m_isSceneFadeIn  = true;          // �^�C�g���V�[���̃t�F�[�h�C�����J�n
        m_sceneFadeAlpha = 255;           // �t�F�[�h�C�������S�ɕ\��
    }
}

SceneTitle::~SceneTitle()
{
    // �^�C�g�����S�摜���������
	DeleteGraph(m_logoHandle);
}

void SceneTitle::Init()
{
    // �}�E�X�J�[�\����\������
    SetMouseDispFlag(true);
}

SceneBase* SceneTitle::Update()
{
    // �^�C�g�����S�̃t�F�[�h�C������
    if (!m_isFadeComplete)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (m_fadeFrame / static_cast<float>(kFadeDuration)));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha      = 255;
            m_isFadeComplete = true; // �t�F�[�h�C��������
            m_fadeFrame      = 0;    // �t�F�[�h�A�E�g�p�Ƀ��Z�b�g
        }
        return this;
    }

    // �t�F�[�h�C����̑ҋ@���Ԃ��J�E���g
    if (m_waitFrame < kWaitDuration)
    {
        m_waitFrame++;
        return this; // �ҋ@���Ԓ��̓V�[���J�ڂ��Ȃ�
    }

    // �^�C�g�����S�̃t�F�[�h�A�E�g����
    if (!m_isFadeOut)
    {
        if (m_fadeFrame < kFadeDuration)
        {
            m_fadeAlpha = static_cast<int>(255.0f * (1.0f - (m_fadeFrame / static_cast<float>(kFadeDuration))));
            m_fadeFrame++;
        }
        else
        {
            m_fadeAlpha = 0;
            m_isFadeOut = true; // �t�F�[�h�A�E�g������
            m_fadeFrame = 0;    // �t�F�[�h�C���p�Ƀ��Z�b�g
        }
        return this;
    }

    // �^�C�g���V�[���̃t�F�[�h�C������
    if (!m_isSceneFadeIn)
    {
        if (m_sceneFadeAlpha < 255)
        {
            m_sceneFadeAlpha += 5; // �t�F�[�h�C���̑��x�𒲐�
        }
        else
        {
            m_sceneFadeAlpha = 255; // �t�F�[�h�C������
            m_isSceneFadeIn  = true;
        }
        return this;
    }


    // �}�E�X�̍��N���b�N���`�F�b�N
    if (Mouse::IsTriggerLeft())
    {
        // �}�E�X�̈ʒu���擾
        Vec2 mousePos = Mouse::GetPos();

        if (mousePos.x >= kStartButtonX1 && mousePos.x <= kStartButtonX2 &&
            mousePos.y >= kStartButtonY1 && mousePos.y <= kStartButtonY2)
        {
            return new SceneMain();
        }
        //// �}�E�X���I�v�V�����{�^���ƃp�l�����͂ޔw�i�͈͓̔��ɂ��邩�`�F�b�N
        //if (mousePos.x >= kBackgroundX1 && mousePos.x <= kBackgroundX2 &&
        //    mousePos.y >= kBackgroundY1 && mousePos.y <= kBackgroundY2)
        //{
        //    return new SceneOption(this, m_currentReticleType);
        //}
    }
    // �������Ȃ���΃V�[���J�ڂ��Ȃ�(�^�C�g����ʂ̂܂�)
    return this;
}

void SceneTitle::Draw()
{
    // �}�E�X�̈ʒu���擾
    Vec2 mousePos = Mouse::GetPos();

    // �^�C�g�����S�̃t�F�[�h�C���E�t�F�[�h�A�E�g�`��
    if (!m_isFadeOut)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawRectExtendGraph(
            0, 0,                       
            Game::kScreenWidth, Game::kScreenHeigth,
            0, 0,                       
            kLogoWidth, kLogoHeight,    
            m_logoHandle, true          
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // �t�F�[�h�������I����Ă��Ȃ��ꍇ�͂����ŕ`����I��
        return;
    }

    // �{�^���̕`��
    if (m_isFadeOut || m_skipLogo)
    {
        // �X�^�[�g�{�^���̕`��
        unsigned int buttonColor = 0xadadad; // �ʏ펞�̃{�^���F�i���j
        if (mousePos.x >= kStartButtonX1 && mousePos.x <= kStartButtonX2 &&
            mousePos.y >= kStartButtonY1 && mousePos.y <= kStartButtonY2)
        {
            buttonColor = 0xffffff; // �z�o�[���̃{�^���F�i�O���[�j
        }

        // �{�^���̔w�i��`��
        DrawBox(kStartButtonX1, kStartButtonY1, kStartButtonX2, kStartButtonY2, buttonColor, true);

        // �{�^���̃e�L�X�g��`��
        const char* buttonText = "START";
        int textWidth = GetDrawStringWidth(buttonText, strlen(buttonText));
        int textX     = (kStartButtonX1 + kStartButtonX2) * 0.5f - textWidth * 0.5f;
        int textY     = (kStartButtonY1 + kStartButtonY2) * 0.5f - 10; // �e�L�X�g�̍����𒲐�
        DrawString(textX, textY, buttonText, GetColor(0, 0, 0)); // �e�L�X�g�F�͍�
    }
}
