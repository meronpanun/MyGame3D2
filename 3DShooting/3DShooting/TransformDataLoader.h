#pragma once
#include "DxLib.h" 
#include <string>
#include <vector>

struct ObjectTransformData
{
	std::string name;  // �I�u�W�F�N�g��
	VECTOR pos;		   // �ʒu
	VECTOR rot;		   // ��]
	VECTOR scale;	   // �X�P�[��
};

/// <summary>
/// �O���t�@�C������ϊ��f�[�^��ǂݍ��ރN���X
/// </summary>
class TransformDataLoader
{
public:

	/// <summary>
	/// CSV�t�@�C������ϊ��f�[�^��ǂݍ���
	/// </summary>
	/// <param name="fileName">�t�@�C����</param>
	/// <returns>�ǂݍ��񂾕ϊ��f�[�^�̃��X�g</returns>
	static std::vector< ObjectTransformData> LoadDataCSV(const char* fileName);
};

