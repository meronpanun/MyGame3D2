#include "Mouse.h"
#include "DxLib.h"
#include "Game.h"

namespace
{
    // �}�E�X�̓��̓��O�̐�
    constexpr int kLogNum = 16;
    // ���̓��O0���ŐV�̏��
    int mouseLog[kLogNum];
}

namespace Mouse
{
    // �}�E�X�̓��͏�Ԃ��擾
    void Update()
    {
        // ���O�̍X�V
        for (int i = kLogNum - 1; i >= 1; i--)
        {
            mouseLog[i] = mouseLog[i - 1];
        }
        // �ŐV�̏�Ԃ��擾
        if (GetMouseInput() & MOUSE_INPUT_LEFT)
        {
            mouseLog[0] = 1;
        }
        else
        {
            mouseLog[0] = 0;
        }
    }

    // ���݂̃}�E�X�̈ʒu���擾
    Vec2 GetPos()
    {
        Vec2 mousePos{ 0,0 };
        int mouseX = 0;
        int mouseY = 0;
        if (GetMousePoint(&mouseX, &mouseY) == -1)
        {
            return mousePos;
        }
        mousePos.x = static_cast<float>(mouseX);
        mousePos.y = static_cast<float>(mouseY);
        return mousePos;
    }

    // ������������
    bool IsPressLeft()
    {
        return (mouseLog[0]);
    }
    // �g���K�[����
    bool IsTriggerLeft()
    {
        bool isNow = mouseLog[0]; // ���݂̏��
        bool isLast = mouseLog[1]; // 1�t���[���O�̏��
        return (isNow && !isLast);
    }
    // �������u�Ԕ���
    bool IsReleaseLeft()
    {
        bool isNow = mouseLog[0]; // ���݂̏��
        bool isLast = mouseLog[1]; // 1�t���[���O�̏��
        return (!isNow && isLast);
    }

    // �J�����̉�]�p�x���X�V
    void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity)
    {
        // �}�E�X�̈ړ��ʂ��擾
        Vec2 mousePos = GetPos();

        // �}�E�X�̈ړ��ʂɊ�Â��ăJ�����̉�]�p�x���X�V
        cameraYaw += (mousePos.x - Game::kScreenWidth * 0.5f) * sensitivity;
        cameraPitch -= (mousePos.y - Game::kScreenHeigth * 0.5f) * sensitivity;

        // �J�����̃s�b�`�p�x�𐧌�
        if (cameraPitch > DX_PI_F * 0.5f) cameraPitch = DX_PI_F * 0.5f;
        if (cameraPitch < -DX_PI_F * 0.5f) cameraPitch = -DX_PI_F * 0.5f;

        // �}�E�X�̈ʒu�𒆉��ɖ߂�
        SetMousePoint(Game::kScreenWidth * 0.5f, Game::kScreenHeigth * 0.5f);
    }
}
