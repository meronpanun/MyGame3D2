#include "DebugUtil.h"
#include "DxLib.h"
#include <cstdarg>

bool DebugUtil::s_isVisible = false;

// 3D�J�v�Z���̃f�o�b�O�`��֐�
void DebugUtil::DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill)
{
    DrawCapsule3D(a, b, radius, div, color, color, fill);
}

// 3D���̃f�o�b�O�`��֐�
void DebugUtil::DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill)
{
    DrawSphere3D(center, radius, div, color, color, fill);
}

// 2D���b�Z�[�W��`�悷��֐�
void DebugUtil::DrawMessage(int x, int y, unsigned int color, const std::string& msg)
{
    DrawString(x, y, msg.c_str(), color);
}

// 2D�t�H�[�}�b�g�������`�悷��֐�
void DebugUtil::DrawFormat(int x, int y, unsigned int color, const char* format, ...)
{
    char buf[256];
	va_list args;                              // �ψ������X�g
	va_start(args, format);                    // �ψ����̏�����
	vsnprintf(buf, sizeof(buf), format, args); // �t�H�[�}�b�g��������o�b�t�@�ɏ�������
	va_end(args);                              // �ψ����̏I��
	DrawString(x, y, buf, color); // �`��
}

// ���S�X�L�b�v�L�[�������ꂽ���ǂ������`�F�b�N����֐�
bool DebugUtil::IsSkipLogoKeyPressed()
{
    return CheckHitKey(KEY_INPUT_S) != 0;
}

void DebugUtil::ShowDebugWindow()
{
    static bool isVisible = false;

    // F1�L�[�������ꂽ�u�Ԃɕ\��/��\����؂�ւ�
    static int prevF1 = 0;
    int nowF1 = CheckHitKey(KEY_INPUT_F1);
    if (nowF1 && !prevF1)
    {
        s_isVisible = !s_isVisible;
    }
    prevF1 = nowF1;

    if (!s_isVisible) return;

    // �f�o�b�O�E�B���h�E�̔w�i�𔼓����ŕ`��
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, NULL);
    unsigned int bgAlphaColor = GetColor(0, 0, 0);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); 
    DrawBox(0, 0, screenW, screenH, bgAlphaColor, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // �f�o�b�O�E�B���h�E�̓��e
    const int x = 40;
    const int y = 40;
    const int w = 400;
    const int h = 200;
    unsigned int bgColor = GetColor(0, 0, 0);
    unsigned int borderColor = GetColor(0, 0, 0);
    unsigned int textColor = GetColor(255, 255, 255);

    // �w�i
    DrawBox(x, y, x + w, y + h, bgColor, true);
    // �g
    DrawBox(x, y, x + w, y + h, borderColor, true);

    // �e�L�X�g
    DrawString(x + 16, y + 16, "�f�o�b�O�E�B���h�E (F1�Őؑ�)", textColor);
    DrawString(x + 16, y + 48, "�E�����Ƀf�o�b�O����\���ł��܂�", textColor);
    // �K�v�ɉ����Ēǉ����������ɕ`��
}

// �f�o�b�O�E�B���h�E���\������Ă��邩�ǂ�����Ԃ�
bool DebugUtil::IsDebugWindowVisible() 
{
    return s_isVisible;
}
