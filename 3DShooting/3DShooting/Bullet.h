#pragma once
#include "DxLib.h"
#include <vector>

/// <summary>
/// �e�N���X
/// </summary>
class Bullet
{
public:
	Bullet(VECTOR position, VECTOR direction, float damage = 10.0f);
	virtual ~Bullet();

	void Init();
	void Update();
	void Draw() const;

	// �e�̈ʒu���擾
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// �e�̔��a���擾
	/// </summary>
	/// <returns>�e�̔��a</returns>
	float GetRadius() const { return m_radius; }

	/// <summary>
	/// �e���L�����ǂ���
	/// </summary>
	/// <returns>�L���Ȃ�true</returns>
	bool IsActive() const { return m_isActive; }

	/// <summary>
	/// �e�̍X�V
	/// </summary>
	/// <param name="bullets">�e�̔z��</param>
	static void UpdateBullets(std::vector<Bullet>& bullets);

	/// <summary>
	/// �e�̕`��
	/// </summary>
	/// <param name="bullets">�e�̔z��</param>
	static void DrawBullets(const std::vector<Bullet>& bullets);

	/// <summary>
	/// �e���A�N�e�B�u��
	/// </summary>
	void Deactivate();

	float GetDamage() const { return m_damage; }

private:
	VECTOR m_pos; // �e�̈ʒu
	VECTOR m_dir; // �e�̕���

	float m_speed;    // �e�̑��x
	float m_radius;   // �e�̔��a
	float m_damage;   // �e�̃_���[�W
	bool  m_isActive; // �e�̗L����
};

