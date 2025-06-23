#pragma once
#include "ItemBase.h"

class Player;

/// <summary>
/// �񕜃A�C�e���N���X
/// </summary>
class FirstAidKitItem : public ItemBase
{
public:
	FirstAidKitItem();
	virtual ~FirstAidKitItem();

	void Init() override;
	void Update() override;
	void Draw() override;

	// �ʒu�Ɣ��a��ǉ�
	void SetPosition(const VECTOR& pos) { m_pos = pos; }

	const VECTOR& GetPosition() const { return m_pos; }

	float GetRadius() const { return m_radius; }

	// �J�v�Z���Ƌ��̍ŒZ�����v�Z�֐��̐錾
	float GetCapsuleSphereDistance(const VECTOR& capA, const VECTOR& capB, float capRadius, const VECTOR& spherePos, float sphereRadius);


private:
	VECTOR m_pos;

	float m_radius; 
	
	bool m_isActive;
};

