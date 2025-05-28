#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include <cmath>
#include <cassert>

namespace
{
	// �A�j���[�V������
	const char* const kIdleAnimName   = "Pistol_IDLE";   // �ҋ@
	const char* const kShotAnimName   = "Pistol_FIRE";   // �e������
	const char* const kWalkAnimName   = "Pistol_WALK";   // ����
	const char* const kRunAnimName    = "Pistol_RUN";    // ����
	//const char* const kReloadAnimName = "Pistol_RELOAD"; // �����[�h

	constexpr float kMoveSpeed = 2.0f; // �������x
	constexpr float kRunSpeed  = 5.0f; // ���鑬�x

	// �v���C���[���f���̃I�t�Z�b�g
	constexpr float kModelOffsetX = 2.0f;  // X���I�t�Z�b�g
	constexpr float kModelOffsetY = 30.0f; // Y���I�t�Z�b�g
	constexpr float kModelOffsetZ = 25.0f; // Z���I�t�Z�b�g

	// �A�j���[�V�����̃u�����h��
	constexpr float kAnimBlendRate = 1.0f; 

	// �e�̃I�t�Z�b�g
	constexpr float kGunOffsetX = 10.0f; // X���I�t�Z�b�g
	constexpr float kGunOffsetY = 58.0f; // Y���I�t�Z�b�g
	constexpr float kGunOffsetZ = 12.0f;  // Z���I�t�Z�b�g

	// �X�^�~�i�֘A�萔
	constexpr float kStaminaMax        = 100.0f; // �ő�X�^�~�i
	constexpr float kStaminaRunCost    = 0.8f;   // ���鎞��1�t���[������������
	constexpr float kStaminaRecover    = 0.5f;   // ����/��~����1�t���[��������񕜗�
	constexpr float kStaminaRunRecover = 30.0f;  // �����悤�ɂȂ�X�^�~�i�l

	// �����[�h�֘A�萔
	constexpr int kInitialAmmo = 700; // �����e��
//	constexpr int kMaxAmmo     = 10;  // �ő�e��
//	constexpr int kReloadTime  = 80;  // �����[�h����

	// �E���\���p�̃I�t�Z�b�g
	constexpr int kMarginX    = 20; // X���I�t�Z�b�g
	constexpr int kMarginY    = 20; // Y���I�t�Z�b�g
	constexpr int kFontHeight = 20; // �t�H���g����

	// ���̃I�t�Z�b�g
	constexpr float kShieldScreenOffsetX = -30.0f; 
	constexpr float kShieldScreenOffsetY = 10.0f; 
	constexpr float kShieldScreenOffsetZ = 40.0f; 

}

Player::Player() :
	m_modelHandle(-1),
	m_shieldHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_pEffect(std::make_shared<Effect>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
	m_stamina(kStaminaMax),
	m_isCanRun(true),
	m_ammo(kInitialAmmo),
//	m_maxAmmo(kMaxAmmo),
//	m_isReloading(false),
//	m_reloadTimer(0),
	m_shotCooldown(0)
{
	// ���f���̓ǂݍ���
	m_modelHandle = MV1LoadModel("data/image/Player.mv1");
	assert(m_modelHandle != -1);

	// ���̃��f����ǂݍ���
	m_shieldHandle = MV1LoadModel("data/image/Shield.mv1");
	assert(m_shieldHandle != -1);

	// �e������SE��ǂݍ���
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);

	// �v���C���[���f���̏�����]��ݒ�
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));
}

Player::~Player()
{
	// ���f���̍폜
	MV1DeleteModel(m_modelHandle);
	// ���̃��f���̍폜
	MV1DeleteModel(m_shieldHandle);

	// SE�̍폜
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	// �J�����̏�����
	m_pCamera->Init();

	// �G�t�F�N�g�̏�����
	m_pEffect->Init();

	// �A�j���[�V�����̃A�^�b�`
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate;
}

void Player::Update()
{
	// �J�����̈ʒu���v���C���[�̈ʒu�Ɋ�Â��čX�V
	m_pCamera->SetPlayerPos(m_modelPos);

	// �J�����̍X�V
	m_pCamera->Update();

	// �G�t�F�N�g�̍X�V
	m_pEffect->Update();

	UpdateAnime(m_prevAnimData);
	UpdateAnime(m_nextAnimData);
	UpdateAnimeBlend();

	// ���f���̈ʒu���J�����̑O���ɐݒ�
	VECTOR modelOffset    = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw         = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch       = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot		  = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// ���f���̈ʒu���X�V
	MV1SetPosition(m_modelHandle, modelPos);

	// �v���C���[���f���̉�]���X�V
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	// �N�[���^�C�����Z
	if (m_shotCooldown > 0)
	{
		m_shotCooldown--;
	}

	// �����[�h����
	//if (m_isReloading) 
	//{
	//	m_reloadTimer--;
	//	if (m_reloadTimer <= 0)
	//	{
	//		// ��[�ł���e�����v�Z
	//		int need = kInitialAmmo - m_ammo;
	//		if (need > 0)
	//		{
	//			// ��[�ł���e��
	//			int reloadAmount = (m_maxAmmo < need) ? m_maxAmmo : need; 
	//			m_ammo += reloadAmount;    // �e���𑝂₷
	//			m_maxAmmo -= reloadAmount; // �c�e�������炷

	//			// �ő�e���𒴂��Ȃ��悤�ɂ���
	//			if (m_maxAmmo < 0) m_maxAmmo = 0;
	//		}
	//		m_isReloading = false;
	//	}
	//}
//	else
//	{
		// �e����
		if (Mouse::IsTriggerLeft() && m_ammo > 0 && m_shotCooldown == 0)
		{
			Shoot();
			m_ammo--;
			m_shotCooldown = 10; // 10�t���[���Ԋu�Ŕ��ˉ\
			ChangeAnime(kShotAnimName, false);
		}
		//// R�L�[�Ń����[�h
		//if (CheckHitKey(KEY_INPUT_R) && m_ammo < kInitialAmmo && m_maxAmmo > 0)
		//{
		//	Reload();
		//	ChangeAnime(kReloadAnimName, false);
		//}
//	}

	// �ړ���ԁE�����Ԃ̔���
	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
	bool isRunning = false;
	if (wantRun && m_isCanRun && m_stamina > 0.0f) 
	{
		isRunning = true;
	}
	float moveSpeed = isRunning ? kRunSpeed : kMoveSpeed;
	
	bool isMoving = false; 

	// �ړ��x�N�g��������
	VECTOR moveDir = VGet(0, 0, 0);

	if (CheckHitKey(KEY_INPUT_W))
	{
		moveDir.x += sinf(m_pCamera->GetYaw());
		moveDir.z += cosf(m_pCamera->GetYaw());
	}
	if (CheckHitKey(KEY_INPUT_S))
	{
		moveDir.x -= sinf(m_pCamera->GetYaw());
		moveDir.z -= cosf(m_pCamera->GetYaw());
	}
	if (CheckHitKey(KEY_INPUT_A)) 
	{
		moveDir.x += sinf(m_pCamera->GetYaw() - DX_PI_F * 0.5f);
		moveDir.z += cosf(m_pCamera->GetYaw() - DX_PI_F * 0.5f);
	}
	if (CheckHitKey(KEY_INPUT_D))
	{
		moveDir.x += sinf(m_pCamera->GetYaw() + DX_PI_F * 0.5f);
		moveDir.z += cosf(m_pCamera->GetYaw() + DX_PI_F * 0.5f);
	}

	// DxLib��VNorm�֐����g���ƁA�x�N�g����0�̎��ɃG���[�ɂȂ�̂ŁA�蓮�Ń`�F�b�N
	// �x�N�g����0�łȂ���ΐ��K�����Ĉړ�
	if (moveDir.x != 0.0f || moveDir.z != 0.0f) 
	{
		float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		moveDir.x /= len;
		moveDir.z /= len;
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
		isMoving = true;
	}

	// �X�^�~�i����
	if (isRunning && isMoving) 
	{
		m_stamina -= kStaminaRunCost;
		if (m_stamina < 0.0f) m_stamina = 0.0f;
	}
	else 
	{
		m_stamina += kStaminaRecover;
		if (m_stamina > kStaminaMax) m_stamina = kStaminaMax;
	}

	// ����邩�ǂ����̔���
	if (m_stamina <= 0.0f) 
	{
		m_isCanRun = false;
	}
	else if (!m_isCanRun && m_stamina >= kStaminaRunRecover) 
	{
		m_isCanRun = true;
	}

	// �����[�h���̓A�j���[�V�����؂�ւ����s��Ȃ�
//	if (!m_isReloading)
	{
		if (m_nextAnimData.isEnd)
		{
			if (m_isMoving)
			{
				ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
			}
			else
			{
				ChangeAnime(kIdleAnimName, true);
			}
		}

		// �A�j���[�V�����؂�ւ�
		// �ړ��J�n
		if (isMoving && !m_isMoving)
		{
			ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		}
		// �ړ��I��
		else if (!isMoving && m_isMoving)
		{
			ChangeAnime(kIdleAnimName, true);
		}
		// �ړ����ɑ���̕������؂�ւ�����ꍇ
		else if (isMoving && m_isMoving && (isRunning != m_isWasRunning))
		{
			ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		}
	}

	m_isMoving = isMoving;     
	m_isWasRunning = isRunning; 
}
void Player::Draw()
{
	// ���f���̕`��
	MV1DrawModel(m_modelHandle);
	
	// �����X�N���[�����W�ŕ`��
	// ��ʃT�C�Y�擾
	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;

	// ����\���������X�N���[�����W
	float shieldScreenX = -25.0f;
	float shieldScreenY = -30;
	float shieldScreenZ = 42.0f; // ���s���i�傫���قǏ���������������j

	// ���p�̃J�������ꎞ�I�ɐݒ�
	VECTOR camPos = VGet(0, 0, -shieldScreenZ);
	VECTOR camTgt = VGet(0, 0, 0);
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt);

	// �����f���̈ʒu��X/Y�œ�����
	MV1SetPosition(m_shieldHandle, VGet(shieldScreenX, shieldScreenY, 0.0f));

	MV1SetRotationXYZ(m_shieldHandle, VGet(0.0f, DX_PI_F, 0.0f));
	MV1SetScale(m_shieldHandle, VGet(0.5f, 0.5f, 0.5f));

	// �����f���̕`��
	MV1DrawModel(m_shieldHandle);

	// ���̃J�����ɖ߂�
	m_pCamera->SetCameraToDxLib();

	// �G�t�F�N�g�̕`��
	m_pEffect->Draw();

	// �t�B�[���h�̕`��
	DrawField();

	const int bgWidth = 160;
	const int bgHeight = 48;
	int bgX = screenW - kMarginX - bgWidth;
	int bgY = screenH - kMarginY - bgHeight;

	// �������̔w�i��
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // 0�`255�œ����x�w��
	DrawBox(bgX, bgY, bgX + bgWidth, bgY + bgHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// �e���\��
	int ammoX = bgX + 12;
	int ammoY = bgY + 8;
	DrawFormatString(ammoX, ammoY, 0xFFFFFF, "Ammo: %d", m_ammo/*, m_maxAmmo*/);

	// �X�^�~�i�\��
	DrawFormatString(10, 30, 0x000000, "Stamina: %.1f", m_stamina);
	
	// �X�^�~�i�Q�[�W�`��
	const int gaugeX = 10;
	const int gaugeY = 50;
	const int gaugeWidth = 200;
	const int gaugeHeight = 16;

	// �Q�[�W�̘g
	DrawBox(gaugeX - 1, gaugeY - 1, gaugeX + gaugeWidth + 1, gaugeY + gaugeHeight + 1, GetColor(0x80, 0x80, 0x80), FALSE);

	// �Q�[�W�̒��g�i�X�^�~�i�����ŕ�������j
	float staminaRate = m_stamina / kStaminaMax;
	int filledWidth = static_cast<int>(gaugeWidth * staminaRate);

	// �Q�[�W�{�́i�΁��Ԃɕω�������j
	int r = static_cast<int>((1.0f - staminaRate) * 255);
	int g = static_cast<int>(staminaRate * 255);
	int gaugeColor = GetColor(r, g, 0);

	DrawBox(gaugeX, gaugeY, gaugeX + filledWidth, gaugeY + gaugeHeight, gaugeColor, TRUE);
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
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y - 80, cubeCenter.z - cubeSize.z * 0.5f), // �ŏ����W
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y - 80 + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f), // �ő���W
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

	// �e�̌������I�C���[�p�i���W�A���j�ŎZ�o
	float rotX = -m_pCamera->GetPitch(); // X���i�㉺�j
	float rotY = m_pCamera->GetYaw();    // Y���i���E�j
	float rotZ = 0.0f;                   // Z���i���[���͕s�v�j

	// �}�Y���t���b�V���G�t�F�N�g�Đ�
	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

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
	float animSpeed = 1.0f;
	//// �����[�h�A�j���[�V��������2�{��
	//if (m_isReloading && data.attachNo == m_nextAnimData.attachNo) 
	//{
	//	animSpeed = 2.0f; // 2�{��
	//}
	data.count += animSpeed;
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
   // ���f���̎��ۂ̕`��ʒu���Z�o
	VECTOR modelOffset = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw      = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch    = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot    = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset);

	// �e�̃I�t�Z�b�g�����f���̕`��ʒu����Z�o
	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
	VECTOR gunPos    = VTransform(gunOffset, modelRot);
	return VAdd(modelPosition, gunPos);
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

//void Player::Reload()
//{
//	m_isReloading = true;
//	m_reloadTimer = kReloadTime;
//}
