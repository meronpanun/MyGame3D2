#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include <cmath>
#include <cassert>

namespace
{
	// �A�j���[�V������
	const char* const kIdleAnimName = "Pistol_IDLE"; // �ҋ@
	const char* const kShotAnimName = "Pistol_FIRE"; // �e������
}

Player::Player() :
	m_modelHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_animBlendRate(0.0f)

{
	// ���f���̓ǂݍ���
	m_modelHandle = MV1LoadModel("data/image/player.mv1");

	// �e������SE��ǂݍ���
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);

	// �v���C���[���f���̏�����]��ݒ� (Z-����������)
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));
}

Player::~Player()
{
	// ���f���̍폜
	MV1DeleteModel(m_modelHandle);
	// SE�̍폜
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	// �J�����̏�����
	m_pCamera->Init();

	// �A�j���[�V�����̃A�^�b�`
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = 1.0f; // �A�j���[�V�����̃u�����h����1.0�ɐݒ�
}

void Player::Update()
{
	// �J�����̈ʒu���v���C���[�̈ʒu�Ɋ�Â��čX�V
	m_pCamera->SetPlayerPos(m_modelPos);

	// �J�����̍X�V
	m_pCamera->Update();

	UpdateAnime(m_prevAnimData);
	UpdateAnime(m_nextAnimData);
	UpdateAnimeBlend();

	// ���f���̈ʒu���J�����̑O���ɐݒ�
	VECTOR modelOffset = VGet(4.0f, 30.0f, 40.0f);
	MATRIX rotYaw = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPosition = VAdd(m_modelPos, rotatedModelOffset);

	// ���f���̈ʒu���X�V
	MV1SetPosition(m_modelHandle, modelPosition);

	// �v���C���[���f���̉�]���X�V
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	// �e�̔���
	if (Mouse::IsTriggerLeft())
	{
		Shoot();
		// �V���b�g�A�j���[�V�������Đ�
		ChangeAnime(kShotAnimName, false);
	}
	// ���ݍĐ����̃A�j���[�V�������I���ҋ@�A�j���[�V�����ɖ߂�
	if (m_nextAnimData.isEnd)
	{
		// �ҋ@�A�j���[�V�������Đ�
		ChangeAnime(kIdleAnimName, true);
	}
}

void Player::Draw()
{
	// ���f���̕`��
	MV1DrawModel(m_modelHandle);

	// �t�B�[���h�̕`��
	DrawField();
}

void Player::DrawField()
{
	// ���C�e�B���O�𖳌���
	SetUseLighting(false);

	// �O���b�h�T�C�Y�ƃt�B�[���h�͈͂̐ݒ�
	const int gridSize = 100;  // �O���b�h1�}�X�̃T�C�Y
	const int fieldSize = 800; // �t�B�[���h�͈̔́i-400�`400�j

	// �n���ȐF2�F��ݒ�
	unsigned int color1 = GetColor(80, 80, 80);  // �n���ȐF1
	unsigned int color2 = GetColor(100, 100, 100); // �n���ȐF2

	// �n�ʂ̕`��
	for (int z = -fieldSize * 0.5f; z < fieldSize * 0.5f; z += gridSize)
	{
		for (int x = -fieldSize * 0.5f; x < fieldSize * 0.5f; x += gridSize)
		{
			// �����̂̒��S���W
			VECTOR cubeCenter = VGet(x + gridSize * 0.5f, 0, z + gridSize * 0.5f);

			// �����̂̃T�C�Y
			VECTOR cubeSize = VGet(gridSize, 10, gridSize);

			// �F�����݂ɐ؂�ւ�
			unsigned int color = ((x / gridSize + z / gridSize) % 2 == 0) ? color1 : color2;

			// �����̂�`��
			DrawCube3D(
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y, cubeCenter.z - cubeSize.z * 0.5f), // �ŏ����W
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f), // �ő���W
				color,
				color,
				true // �h��Ԃ�
			);
		}
	}

	// ���C�e�B���O���ēx�L����
	SetUseLighting(true);
}

void Player::Shoot()
{
	// �e�̈ʒu���擾
	VECTOR gunPos = GetGunPos();
	VECTOR gunDir = GetGunRot();

	// �e������SE���Đ�
	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);
}

void Player::AttachAnime(AnimData& data, const char* animName, bool isLoop)
{
	// �A�^�b�`�������A�j���[�V�����ԍ����擾
	int index = MV1GetAnimIndex(m_modelHandle, animName);

	// ���f���A�j���[�V�������A�^�b�`
	data.attachNo = MV1AttachAnim(m_modelHandle, index, -1, false);
	// �A�j���[�V�����J�E���^������
	data.count = 0.0f;
	// �A�j���[�V�����̃��[�v�ݒ�
	data.isLoop = isLoop;
	// �A�j���[�V�������I��������
	data.isEnd = false;
}

void Player::UpdateAnime(AnimData& data)
{
	// �A�j���[�V�������A�^�b�`����Ă��Ȃ��ꍇ�͉������Ȃ�
	if (data.attachNo == -1) return;

	// �A�j���[�V�����̍X�V
	data.count += 1.0f;

	// ���ݍĐ����̃A�j���[�V�����̑����Ԃ��擾
	const float totalTime = MV1GetAttachAnimTotalTime(m_modelHandle, data.attachNo);

	// �A�j���[�V�����̐ݒ�ɂ���ă��[�v�����邩�Ō�̃t���[���Œ�~���邩�𔻒�
	if (data.isLoop)
	{
		// �A�j���[�V���������[�v������
		while (data.count > totalTime)
		{
			data.count -= totalTime;
		}
	}
	else
	{
		// �Ō�̃t���[���Œ�~����
		if (data.count > totalTime)
		{
			data.count = totalTime;
			data.isEnd = true;
		}
	}
	// �i�s���������A�j���[�V���������f���ɓK�p����
	MV1SetAttachAnimTime(m_modelHandle, data.attachNo, data.count);
}

// �A�j���[�V�����u�����h���̍X�V
void Player::UpdateAnimeBlend()
{
	//�����ɃA�j�����ݒ肳��Ă��Ȃ��ꍇ�͕ω������Ȃ�
	if (m_nextAnimData.attachNo == -1 && m_prevAnimData.attachNo == -1) return;

	m_animBlendRate += 1.0f / 8.0f;
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f;
	}

	// m_animBlendRate���A�j���[�V�����ɓK�p����
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

void Player::ChangeAnime(const char* animName, bool isLoop)
{
	//��O�̃f�[�^�͍���Ǘ��ł��Ȃ��Ȃ�̂ł��炩���߃f�^�b�`���Ă���
	MV1DetachAnim(m_modelHandle, m_prevAnimData.attachNo);

	// ���ݍĐ����̃A�j���[�V�����͈�Â��f�[�^�����ɂȂ�
	m_prevAnimData = m_nextAnimData;

	// �V���ɃV���b�g�̃A�j���[�V�������A�^�b�`����
	AttachAnime(m_nextAnimData, animName, isLoop);

	// �A�j���[�V�����u�����h����������
	m_animBlendRate = 0.0f;

	// m_animBlendRate���A�j���[�V�����ɓK�p����
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

VECTOR Player::GetGunPos() const
{
	// �e�̈ʒu���v���C���[���f���̈ʒu�Ɋ�Â��Đݒ�
	VECTOR gunOffset = VGet(10.0f, 88.0f, 200.0f);
	MATRIX rotYaw    = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX gunRot    = MMult(rotPitch, rotYaw);
	return VAdd(m_modelPos, VTransform(gunOffset, gunRot));
}

VECTOR Player::GetGunRot() const
{
	// �e�̕������v�Z
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}
