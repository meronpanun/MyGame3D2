#pragma once
#include "EnemyBase.h"

class Bullet;

/// <summary>
/// �ʏ�̓G�N���X
/// </summary>
class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(); 
    virtual ~EnemyNormal() = default;

    void Init() override;
    void Update(const std::vector<Bullet>& bullets) override; // �e���X�g���󂯎��
    void Draw() override;

    /// <summary>
	/// �����蔻����s���֐�
    /// </summary>
	/// <param name="bullet">�e�̏��</param>
	/// /// <returns>�����������ǂ���</returns>
    virtual bool IsHit(const Bullet& bullet) const override;

    /// <summary>
	/// �f�o�b�O�p�̓����蔻���`�悷��֐�
    /// </summary>
    virtual void DrawCollisionDebug() const override;

    /// <summary>
	/// �ǂ��ɓ����������𔻒肷��֐�
    /// </summary>
	/// <param name="bullet">�e�̏��</param>
	/// /// <returns>������������</returns>
    HitPart CheckHitPart(const Bullet& bullet) const override;

	/// <summary>
	/// �G���e�ɓ����������ǂ������`�F�b�N���A�_���[�W���󂯂鏈��
	/// </summary>
	/// /// <param name="bullets">�e�̃��X�g</param>
	virtual void CheckHitAndDamage(const std::vector<Bullet>& bullets) override;

    /// <summary>
	/// �G���_���[�W���󂯂鏈��
    /// </summary>
	/// <param name="damage">�󂯂�_���[�W��</param>
    void TakeDamage(float damage) override;

protected:

private:
	VECTOR m_pos;     // �G�̈ʒu
    VECTOR m_aabbMin; // AABB�ŏ����W
    VECTOR m_aabbMax; // AABB�ő���W
    VECTOR m_headPos; // �w�b�h�V���b�g����p���S���W

    HitPart m_lastHitPart = HitPart::None; // ���߂̃q�b�g����

    int m_hitDisplayTimer = 0; // �f�o�b�O�\���p�^�C�}�[
	float m_colRadius;         // �����蔻��p���a
    float m_headRadius;        // �w�b�h�V���b�g����p���a
	float m_hp;                // �G�̗̑�
};

