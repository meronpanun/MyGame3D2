#pragma once

/// <summary>
/// �����蔻��̊��N���X
/// </summary>
class Collider abstract
{
public:
	Collider() = default;
	virtual ~Collider() = default;

	virtual void Init()   abstract;
	virtual void Update() abstract;

	// �������z�֐��Ƃ��ē����蔻�胁�\�b�h��ǉ�
	virtual bool Intersects(const Collider* other) const abstract;

	// Ray�Ƃ̌������胁�\�b�h��ǉ�
	// out_hitPos: ���������ꍇ�̃q�b�g���W
	// out_hitDistSq: ���������ꍇ�́ARay�n�_����̋�����2��
	virtual bool IntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& out_hitPos, float& out_hitDistSq) const abstract;


protected:
};

