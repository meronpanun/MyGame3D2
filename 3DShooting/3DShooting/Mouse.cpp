#include "Mouse.h"
#include "DxLib.h"
#include "Game.h"

namespace
{
    // �}�E�X�̓��̓��O�̐�
    constexpr int kLogNum = 16;
    // ���̓��O0���ŐV�̏��
    int mouseLog[kLogNum];
    int mouseRightLog[kLogNum];
}

namespace Mouse
{
    // �}�E�X�̓��͏�Ԃ��擾
    void Mouse::Update()
    {
        // ���O�̍X�V
        for (int i = kLogNum - 1; i >= 1; i--)
        {
            mouseLog[i] = mouseLog[i - 1];
            mouseRightLog[i] = mouseRightLog[i - 1]; // �E�{�^����
        }
        // �ŐV�̏�Ԃ��擾
        mouseLog[0]      = (GetMouseInput() & MOUSE_INPUT_LEFT) ? 1 : 0;
        mouseRightLog[0] = (GetMouseInput() & MOUSE_INPUT_RIGHT) ? 1 : 0; 
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
        bool isNow  = mouseLog[0]; // ���݂̏��
        bool isLast = mouseLog[1]; // 1�t���[���O�̏��
        return (isNow && !isLast);
    }
    // �������u�Ԕ���
    bool IsReleaseLeft()
    {
        bool isNow  = mouseLog[0]; // ���݂̏��
        bool isLast = mouseLog[1]; // 1�t���[���O�̏��
        return (!isNow && isLast);
    }

	// �E�N���b�N�̃g���K�[����
    bool IsTriggerRight()
    {
        bool isNow = mouseRightLog[0];
        bool isLast = mouseRightLog[1];
        return (isNow && !isLast);
    }

	// �E�N���b�N�̉�����������
    bool IsPressRight()
    {
        return (mouseRightLog[0]);
    }

    // �J�����̉�]�p�x���X�V
    void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity)
    {
        // �}�E�X�̈ړ��ʂ��擾
        Vec2 mousePos = GetPos();

        // �}�E�X�̈ړ��ʂɊ�Â��ăJ�����̉�]�p�x���X�V
        cameraYaw   += (mousePos.x - Game::kScreenWidth  * 0.5f) * sensitivity;
        cameraPitch -= (mousePos.y - Game::kScreenHeigth * 0.5f) * sensitivity;

        // �J�����̃s�b�`�p�x�𐧌�
        if (cameraPitch >  DX_PI_F * 0.5f) cameraPitch = DX_PI_F * 0.5f;
        if (cameraPitch < -DX_PI_F * 0.5f) cameraPitch = -DX_PI_F * 0.5f;

        // �}�E�X�̈ʒu�𒆉��ɖ߂�
        SetMousePoint(static_cast<int>(Game::kScreenWidth * 0.5f), static_cast<int>(Game::kScreenHeigth * 0.5f));
    }
}
