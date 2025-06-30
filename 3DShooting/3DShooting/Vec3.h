#pragma once

#pragma once
#include <cmath>

/// <summary>
/// 3�����x�N�g���N���X
/// </summary>
class Vec3
{
public:
	float x;
	float y;
	float z; // Z���W��ǉ�

public:
	// �f�t�H���g�R���X�g���N�^
	Vec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f; // Z���W��������
	}

	// �����t���R���X�g���N�^
	Vec3(float posX, float posY, float posZ)
	{
		x = posX;
		y = posY;
		z = posZ;
	}

	// ���W�ݒ�֐�
	void SetPos(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	// ���Z������Z�q(Vec3 = (Vec3 += Vec3))
	Vec3 operator+=(const Vec3& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	// ���Z���Z�q(Vec3 = Vec3 + Vec3)
	Vec3 operator+(const Vec3& vec) const
	{
		Vec3 temp{ x + vec.x, y + vec.y, z + vec.z };
		return temp;
	}

	// ���Z������Z�q(Vec3 = (Vec3 -= Vec3))
	Vec3 operator-=(const Vec3& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	// ���Z���Z�q(Vec3 = Vec3 - Vec3)
	Vec3 operator-(const Vec3& vec) const
	{
		Vec3 temp{ x - vec.x, y - vec.y, z - vec.z };
		return temp;
	}

	// �X�J����Z������Z�q(Vec3 = (Vec3 *= float))
	Vec3 operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	// �X�J����Z���Z�q(Vec3 = Vec3 * float)
	Vec3 operator*(float scale) const
	{
		Vec3 temp{ x * scale, y * scale, z * scale };
		return temp;
	}

	// �X�J�����Z������Z�q(Vec3 = (Vec3 /= float))
	Vec3 operator/=(float scale)
	{
		x /= scale;
		y /= scale;
		z /= scale; 
		return *this;
	}

	// �X�J�����Z���Z�q(Vec3 = Vec3 / float)
	Vec3 operator/(float scale) const
	{
		Vec3 temp{ x / scale, y / scale, z / scale };
		return temp;
	}

	// �����̎擾
	float Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// ���K��
	Vec3 Normalize() const
	{
		float len = Length(); // �������擾
		if (len == 0)
		{
			return *this; // ������0�̏ꍇ�͂��̂܂ܕԂ�
		}
		return (*this) / len; // ���g�̐����𒷂��Ŋ����Đ��K��
	}
};

