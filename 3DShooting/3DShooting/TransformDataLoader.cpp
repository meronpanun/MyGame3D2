#include "TransformDataLoader.h"
#include <fstream>
#include <sstream>

namespace
{
	// ���O�A�ʒu�A��]�A�X�P�[���̗v�f��
	constexpr int kElementNum = 10; 

	// Unity�̍��W�n����DxLib�̍��W�n�ւ̕ϊ��W��
	constexpr float kUnityToDxPos = 100.0f; 
}

std::vector<ObjectTransformData> TransformDataLoader::LoadDataCSV(const char* fileName)
{
	// �f�[�^���i�[����z��
	std::vector<ObjectTransformData> Object;

	// �t�@�C�����J��
	std::ifstream file(fileName);

	// �������t�@�C�����J���Ȃ�������
	if (!file.is_open())
	{
		return Object; // ��̔z���Ԃ�
	}

	// 1�s���ǂݍ���
	std::string line;
	//�ŏ��̃w�b�_�[�̓X�L�b�v
	bool isHeader = true;

	// CSV�t�@�C���̊e�s��ǂݍ���
	while (std::getline(file, line))
	{
		if (isHeader)
		{
			isHeader = false; // �w�b�_�[�s���X�L�b�v
			continue;
		}

		std::stringstream ss(line);
		ObjectTransformData data;

		// �J���}�ŕ������Ċe�v�f��ǂݍ���
		std::string element;
		int index = 0;

		while (std::getline(ss, element, ',') && index < kElementNum)
		{
			switch (index)
			{
			case 0: // ���O
				data.name = element;
				break;
			case 1: // �ʒuX
				data.pos.x = std::stof(element) * kUnityToDxPos;
				break;
			case 2: // �ʒuY
				data.pos.y = std::stof(element) * kUnityToDxPos;
				break;
			case 3: // �ʒuZ
				data.pos.z = std::stof(element) * kUnityToDxPos;
				break;
			case 4: // ��]X
				data.rot.x = std::stof(element);
				break;
			case 5: // ��]Y
				data.rot.y = std::stof(element);
				break;
			case 6: // ��]Z
				data.rot.z = std::stof(element);
				break;
			case 7: // �X�P�[��X
				data.scale.x = std::stof(element);
				break;
			case 8: // �X�P�[��Y
				data.scale.y = std::stof(element);
				break;
			case 9: // �X�P�[��Z
				data.scale.z = std::stof(element);
				break;
			default:
				break; // �s���ȗv�f�͖���
			}
			index++;
		}
		Object.push_back(data); // �ǂݍ��񂾃f�[�^��z��ɒǉ�
	}

	// �t�@�C�������
	file.close();
	return Object; // �ǂݍ��񂾃f�[�^�̔z���Ԃ�
}
