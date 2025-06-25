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

protected:
};

