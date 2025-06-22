#pragma once
#include "DxLib.h"  
#include <vector>
#include <string>

/// <summary>
/// �f�o�b�O���[�e�B���e�B�N���X
/// </summary>
class DebugUtil
{
public:
    /// <summary>
    /// 3D�J�v�Z���̃f�o�b�O�`�� 
    /// </summary>
	/// <param name="a">�J�v�Z���̎n�_</param>
	/// <param name="b">�J�v�Z���̏I�_</param>
	/// <param name="radius">�J�v�Z���̔��a</param>
	/// <param name="div">������</param>
	/// <param name="color">�J�v�Z���̐F</param>
	/// <param name="fill">true�Ȃ�J�v�Z����h��Ԃ�</param>
    static void DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 3D���̃f�o�b�O�`��  
    /// </summary>
	/// <param name="center">���̒��S���W</param>
	/// <param name="radius">���̔��a</param>
	/// <param name="div">������</param>
	/// <param name="color">���̐F</param>
	/// <param name="fill">true�Ȃ狅��h��Ԃ�</param>
    static void DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill = false);

    /// <summary>
	/// 2D�f�o�b�N���b�Z�[�W��`�悷��
    /// </summary>
	/// <param name="x">X���W</param>
	/// <param name="y">Y���W</param>
	/// <param name="color">���b�Z�[�W�̐F</param>
	/// <param name="msg">���b�Z�[�W���e</param>
    static void DrawMessage(int x, int y, unsigned int color, const std::string& msg);

    /// <summary>
	/// 2D�f�o�b�N�t�H�[�}�b�g�������`�悷��
    /// </summary>
	/// <param name="x">X���W</param>
	/// <param name="y">Y���W</param>
	/// <param name="color">���b�Z�[�W�̐F</param>
	/// <param name="format">�t�H�[�}�b�g������</param>
	/// <param name="">�ψ���</param>
    static void DrawFormat(int x, int y, unsigned int color, const char* format, ...);

    /// <summary>
	/// ���S�X�L�b�v�L�[�������ꂽ���ǂ������`�F�b�N����
    /// </summary>
	/// <returns>true�Ȃ�X�L�b�v�L�[�������ꂽ</returns>
    static bool IsSkipLogoKeyPressed();

    /// <summary>
	/// �f�o�b�O�E�B���h�E��\������
    /// </summary>
    static void ShowDebugWindow();

	static bool IsDebugWindowVisible();

private:
	static bool s_isVisible;
};

