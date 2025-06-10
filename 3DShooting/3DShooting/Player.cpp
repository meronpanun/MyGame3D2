#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include "Bullet.h"
#include "SceneMain.h"
#include "EnemyBase.h"
#include <cmath>
#include <cassert>

namespace
{
	// �A�j���[�V������
	const char* const kIdleAnimName = "Pistol_IDLE"; // �ҋ@
	const char* const kShotAnimName = "Pistol_FIRE"; // ����
	const char* const kWalkAnimName = "Pistol_WALK"; // ���s
	const char* const kRunAnimName  = "Pistol_RUN";  // ���s
	const char* const kJumpAnimName = "Pistol_JUMP"; // �W�����v

	constexpr float kMoveSpeed = 2.0f; // �ړ����x
	constexpr float kRunSpeed  = 5.0f; // ���鑬�x

	// ���f���̃I�t�Z�b�g
	constexpr float kModelOffsetX = 2.0f; 
	constexpr float kModelOffsetY = 30.0f;
	constexpr float kModelOffsetZ = 25.0f;

	// �A�j���[�V�����̃u�����h��
	constexpr float kAnimBlendRate = 1.0f; 

	// �e�̃I�t�Z�b�g
	constexpr float kGunOffsetX = 10.0f;
	constexpr float kGunOffsetY = 58.0f;
	constexpr float kGunOffsetZ = 12.0f;

	// �X�^�~�i�֘A
	constexpr float kStaminaMax        = 100.0f;
	constexpr float kStaminaRunCost	   = 0.8f;
	constexpr float kStaminaRecover    = 0.5f;
	constexpr float kStaminaRunRecover = 30.0f;

	// �����e��
	constexpr int kInitialAmmo = 700;

	// UI�֘A
	constexpr int kMarginX    = 20; 
	constexpr int kMarginY    = 20;
	constexpr int kFontHeight = 20;

	// �d�͂ƃW�����v�֘A
	constexpr float kGravity   = 0.35f; // �d�͂̋���
	constexpr float kJumpPower = 7.0f;  // �W�����v�̏���
	constexpr float kGroundY   = 0.0f;  // �n�ʂ�Y���W
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
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
	m_shotCooldown(0),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_isJumping(false),
	m_jumpVelocity(0.0f),
	m_hasShot(false),
	m_lockOnTargetId(-1),
	m_isLockOn(false)
{
	// �v���C���[���f���̓ǂݍ���
	m_modelHandle = MV1LoadModel("data/model/Player.mv1");
	assert(m_modelHandle != -1);

	// �V�[���h�̓ǂݍ���
	m_swordHandle = MV1LoadModel("data/model/Sword.mv1");
	assert(m_swordHandle != -1);

	// SE�̓ǂݍ���
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);
}

Player::~Player()
{
	// ���f���̉��
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_swordHandle);

	// SE�̉��
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	m_pCamera->Init(); // �J�����̏�����
	m_pEffect->Init(); // �G�t�F�N�g�̏�����

	// ���f���̏����ʒu�Ɖ�]
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));

	// �A�j���[�V�����f�[�^�̏�����
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate; // �A�j���[�V�����̃u�����h����ݒ�
}

void Player::Update()
{
	// �v���C���[�̈ʒu���J�����ɐݒ�
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update(); // �J�����̍X�V
	m_pEffect->Update(); // �G�t�F�N�g�̍X�V

	UpdateAnime(m_prevAnimData); // �O�̃A�j���[�V�����f�[�^���X�V
	UpdateAnime(m_nextAnimData); //	���̃A�j���[�V�����f�[�^���X�V
	UpdateAnimeBlend();			 // �A�j���[�V�����̃u�����h���X�V

	// ���f���̈ʒu�Ɖ�]���X�V
	VECTOR modelOffset    = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw         = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch	      = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot       = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// ���f���̈ʒu��ݒ�
	MV1SetPosition(m_modelHandle, modelPos); 

	// ���f���̉�]��ݒ�
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f)); 

	// �V���b�g�N�[���_�E��������ꍇ
	if (m_shotCooldown > 0) 
	{
		m_shotCooldown--;
	}

	// �}�E�X�̍��N���b�N�Ŏˌ�
	if (Mouse::IsTriggerLeft() && m_ammo > 0 && m_shotCooldown == 0)  
	{
		Shoot();  // �ˌ�����
		m_ammo--; // �e������炷
		m_shotCooldown = 10; // �V���b�g�N�[���_�E����ݒ�
		ChangeAnime(kShotAnimName, false); // ���˃A�j���[�V�����ɕύX
	}

	// �}�E�X�̉E�N���b�N�Ń��b�N�I���̐؂�ւ�
	//if (Mouse::IsPressRight()) 
	//{
	//	m_isLockOn = true;
	//}
	//else
	//{
	//	m_isLockOn = false;
	//}

	// �e�̍X�V
	Bullet::UpdateBullets(m_bullets);

	// ����L�[����
	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT); 
	bool isRunning = false; // �����Ă��邩�ǂ����̃t���O

	// �������
	if (wantRun && m_isCanRun && m_stamina > 0.0f) 
	{
		isRunning = true;
	}
	float moveSpeed = isRunning ? kRunSpeed : kMoveSpeed; // �ړ����x�̐ݒ�
	bool isMoving = false; // �ړ������ǂ����̃t���O

	// �ړ������̏�����s
	VECTOR moveDir = VGet(0, 0, 0); 

	// �L�[���͂ɂ��ړ������̐ݒ�
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

	// �n�ʂɂ��邩����
	bool isOnGround = (m_modelPos.y <= kGroundY + 0.01f);

	// �X�y�[�X�L�[�ŃW�����v
	if (CheckHitKey(KEY_INPUT_SPACE) && isOnGround && !m_isJumping)
	{
		m_jumpVelocity = kJumpPower;
		m_isJumping = true;
		//ChangeAnime(kJumpAnimName, false);
	}

	// �W�����v���܂��͋󒆂Ȃ�d�͓K�p
	if (m_isJumping || !isOnGround)
	{
		m_modelPos.y += m_jumpVelocity; // �W�����v�̑��x��K�p
		m_jumpVelocity -= kGravity;     // �d�͂�K�p

		// ���n����
		if (m_modelPos.y <= kGroundY)
		{
			m_modelPos.y = kGroundY; // �n�ʂɒ��n
			m_jumpVelocity = 0.0f;   // �W�����v���x�����Z�b�g
			m_isJumping = false;     // �W�����v��Ԃ�����
		}
	}

	// �ړ�����������ꍇ
	if (moveDir.x != 0.0f || moveDir.z != 0.0f) 
	{
		// �ړ������̒������v�Z
		float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z); 
		moveDir.x /= len;
		moveDir.z /= len;
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
		isMoving = true;
	}

	// �����Ă��Ĉړ����Ȃ�
	if (isRunning && isMoving) 
	{
		m_stamina -= kStaminaRunCost; // �X�^�~�i�����炷

		// �X�^�~�i��0�����ɂȂ�Ȃ��悤�ɂ���
		if (m_stamina < 0.0f) m_stamina = 0.0f; 
	}
	else
	{
		m_stamina += kStaminaRecover; // �X�^�~�i����

		// �X�^�~�i�̏����ݒ�
		if (m_stamina > kStaminaMax) m_stamina = kStaminaMax; 
	}

	// �X�^�~�i��0�ȉ��Ȃ瑖��Ȃ�
	if (m_stamina <= 0.0f) 
	{
		m_isCanRun = false; // ����Ȃ���Ԃɂ���
	}
	else if (!m_isCanRun && m_stamina >= kStaminaRunRecover) // �X�^�~�i���񕜂�����
	{
		m_isCanRun = true; // ������Ԃɖ߂�
	}

	// �A�j���[�V�������I��������
	if (m_nextAnimData.isEnd)  
	{
		if (m_isMoving)
		{
			// �ړ��A�j���[�V�����ɕύX
			ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true); 
		}
		else
		{
			// �ҋ@�A�j���[�V�����ɕύX
			ChangeAnime(kIdleAnimName, true);
		}
	}

	// �ړ����őO��͈ړ����Ă��Ȃ������ꍇ
	if (isMoving && !m_isMoving)
	{
		ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
	}
	else if (!isMoving && m_isMoving) 
	{
		ChangeAnime(kIdleAnimName, true);
	}
	else if (isMoving && m_isMoving && (isRunning != m_isWasRunning)) 
	{
		ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true); 
	}

	m_isMoving	   = isMoving;  // �ړ����̏�Ԃ��X�V
	m_isWasRunning = isRunning; // �����Ă����Ԃ��X�V
}
void Player::Draw()
{
	// �v���C���[���f���̕`��
	MV1DrawModel(m_modelHandle); 

	// �e�̕`��
	Bullet::DrawBullets(m_bullets);

	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;

	float shieldScreenX = -25.0f;
	float shieldScreenY = -30;
	float shieldScreenZ = 55.0f;

	VECTOR camPos = VGet(0, 0, -shieldScreenZ); // �J�����̈ʒu
	VECTOR camTgt = VGet(0, 0, 0);			    // �J�����̃^�[�Q�b�g�ʒu
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt); 

	// ���̈ʒu
	MV1SetPosition(m_swordHandle, VGet(shieldScreenX, shieldScreenY, 0.0f));

	MV1SetRotationXYZ(m_swordHandle, VGet(0.0f, 200.0f, 0.0f)); // ���̉�]
	MV1SetScale(m_swordHandle, VGet(0.5f, 0.5f, 0.5f));		    // ���̃X�P�[��

	// ���̕`��
	MV1DrawModel(m_swordHandle);

	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw(); // �G�t�F�N�g�̕`��

	DrawField(); // �t�B�[���h�̕`��

	const int bgWidth = 160;
	const int bgHeight = 48;
	int bgX = screenW - kMarginX - bgWidth;
	int bgY = screenH - kMarginY - bgHeight;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(bgX, bgY, bgX + bgWidth, bgY + bgHeight, GetColor(0, 0, 0), false);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int ammoX = bgX + 12;
	int ammoY = bgY + 8;
	DrawFormatString(ammoX, ammoY, 0xFFFFFF, "Ammo: %d", m_ammo); 

	DrawFormatString(10, 30, 0x000000, "Stamina: %.1f", m_stamina);

	const int gaugeX = 10;
	const int gaugeY = 50;
	const int gaugeWidth = 200;
	const int gaugeHeight = 16;

	DrawBox(gaugeX - 1, gaugeY - 1, gaugeX + gaugeWidth + 1, gaugeY + gaugeHeight + 1, GetColor(0x80, 0x80, 0x80), false);

	float staminaRate = m_stamina / kStaminaMax;
	int filledWidth = static_cast<int>(gaugeWidth * staminaRate);

	int r = static_cast<int>((1.0f - staminaRate) * 255);
	int g = static_cast<int>(staminaRate * 255);
	int gaugeColor = GetColor(r, g, 0);

	DrawBox(gaugeX, gaugeY, gaugeX + filledWidth, gaugeY + gaugeHeight, gaugeColor, false);

	// ���b�N�I�����Ȃ��ʒ����ɐԂ��~��`��
	if (m_isLockOn)
	{
		// ��ʒ����ɐԂ��~��`��
		int centerX = Game::kScreenWidth * 0.5f;
		int centerY = Game::kScreenHeigth * 0.5f;
		int radius = 40; // �~�̔��a
		int color = GetColor(255, 64, 64);
		int thickness = 4; // �~�̑���
		DrawCircle(centerX, centerY, radius, color, false, thickness);
	}
}

void Player::DrawField()
{
	SetUseLighting(false);

	const int gridSize = 100;
	const int fieldSize = 800;

	unsigned int color1 = GetColor(80, 80, 80);
	unsigned int color2 = GetColor(100, 100, 100);

	for (int z = static_cast<int>(-fieldSize * 0.5f); z < static_cast<int>(fieldSize * 0.5f); z += gridSize)
	{
		for (int x = static_cast<int>(-fieldSize * 0.5f); x < static_cast<int>(fieldSize * 0.5f); x += gridSize)
		{
			VECTOR cubeCenter = VGet(x + gridSize * 0.5f, 0, z + gridSize * 0.5f);

			VECTOR cubeSize = VGet(gridSize, 10, gridSize);

			unsigned int color = ((x / gridSize + z / gridSize) % 2 == 0) ? color1 : color2;

			DrawCube3D(
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y - 80, cubeCenter.z - cubeSize.z * 0.5f),
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y - 80 + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f),
				color,
				color,
				true
			);
		}
	}

	SetUseLighting(true);
}

void Player::TakeDamage(float damage)
{
	m_health -= damage; // �_���[�W��K�p
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // �̗͂����ɂȂ�Ȃ��悤�ɐ���
	}
}

// �e�̎擾
std::vector<Bullet>& Player::GetBullets()
{
	return m_bullets;
}

bool Player::HasShot()
{
	bool shot = m_hasShot;
	m_hasShot = false; // ��Ԃ����Z�b�g
	return shot; // ���������ǂ�����Ԃ�
}

void Player::Shoot()
{
	// �e�̈ʒu���擾
	VECTOR gunPos = GetGunPos();
	VECTOR gunDir = GetGunRot();

	// �e�𐶐�
	m_bullets.emplace_back(gunPos, gunDir, 10.0f);

	float rotX = -m_pCamera->GetPitch();
	float rotY = m_pCamera->GetYaw();
	float rotZ = 0.0f;

	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	// �e������SE���Đ�
	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);

	m_hasShot = true; // �e���������t���O�𗧂Ă�
}

// �A�j���[�V�����̃A�^�b�`
void Player::AttachAnime(AnimData& data, const char* animName, bool isLoop) 
{
	// �A�j���[�V�����̃C���f�b�N�X���擾
	int index = MV1GetAnimIndex(m_modelHandle, animName); 

	// ���f���A�j���[�V�������A�^�b�`
	data.attachNo = MV1AttachAnim(m_modelHandle, index, -1, false); 
	data.count  = 0.0f;	  // �A�j���[�V�����̃J�E���g��������
	data.isLoop = isLoop; // ���[�v�t���O��ݒ�
	data.isEnd  = false;  // �A�j���[�V�������I��������
}

// �A�j���[�V�����̍X�V
void Player::UpdateAnime(AnimData& data) 
{
	if (data.attachNo == -1) return; // �A�^�b�`����Ă��Ȃ��ꍇ�͉������Ȃ�

	float animSpeed = 1.0f;

	data.count += animSpeed;

	// �A�j���[�V�����̑����Ԃ��擾
	const float totalTime = MV1GetAttachAnimTotalTime(m_modelHandle, data.attachNo); 
	
	// ���[�v�A�j���[�V�����̏ꍇ
	if (data.isLoop)
	{
		// �A�j���[�V�����̃J�E���g�������Ԃ𒴂����ꍇ�A���[�v������
		while (data.count > totalTime)  
		{
			data.count -= totalTime; // �����Ԃ������ă��[�v
		}
	}
	else
	{
		if (data.count > totalTime)
		{
			data.count = totalTime; // �A�j���[�V�����̃J�E���g�𑍎��Ԃɐ���
			data.isEnd = true;      // �A�j���[�V�������I�������t���O�𗧂Ă�
		}
	}

	// �A�j���[�V�����̎��Ԃ�ݒ�
	MV1SetAttachAnimTime(m_modelHandle, data.attachNo, data.count); 
}

// �A�j���[�V�����̃u�����h���X�V
void Player::UpdateAnimeBlend() 
{
	// �A�^�b�`����Ă��Ȃ��ꍇ�͉������Ȃ�
	if (m_nextAnimData.attachNo == -1 && m_prevAnimData.attachNo == -1) return; 

	// �A�j���[�V�����̃u�����h�����X�V
	m_animBlendRate += 1.0f / 8.0f; 

	// �u�����h����1.0�𒴂�����
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f; // 1.0�ɐ���
	}

	// �O�̃A�j���[�V�����̃u�����h����ݒ�
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate); 
	// ���̃A�j���[�V�����̃u�����h����ݒ�
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate); 
}

// �A�j���[�V�����̕ύX
void Player::ChangeAnime(const char* animName, bool isLoop) 
{
	// �O�̃A�j���[�V����������
	MV1DetachAnim(m_modelHandle, m_prevAnimData.attachNo);

	// �O�̃A�j���[�V�����f�[�^��ۑ�
	m_prevAnimData = m_nextAnimData; 

	// ���̃A�j���[�V�������A�^�b�`
	AttachAnime(m_nextAnimData, animName, isLoop); 

	// �A�j���[�V�����̃u�����h�������Z�b�g
	m_animBlendRate = 0.0f; 

	// �O�̃A�j���[�V�����̃u�����h����ݒ�
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate); 
	// ���̃A�j���[�V�����̃u�����h����ݒ�
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

// �e�̈ʒu���擾
VECTOR Player::GetGunPos() const 
{
	// ���f���̃I�t�Z�b�g�Ɖ�]���v�Z
	VECTOR modelOffset		  = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ); // ���f���̃I�t�Z�b�g
	MATRIX rotYaw			  = MGetRotY(m_pCamera->GetYaw());                     // �J�����̃��[��]
	MATRIX rotPitch			  = MGetRotX(-m_pCamera->GetPitch());	               // �J�����̃s�b�`��]
	MATRIX modelRot			  = MMult(rotPitch, rotYaw);						   // ���f���̉�]�s����v�Z
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);				   // �I�t�Z�b�g����]
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset);			   // ���f���̈ʒu�ƃI�t�Z�b�g��g�ݍ��킹�ďe�̈ʒu���v�Z

	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ); // �e�̃I�t�Z�b�g
	VECTOR gunPos    = VTransform(gunOffset, modelRot);			    // �e�̃I�t�Z�b�g����]

	// �e�̈ʒu���v�Z���ĕԂ�
	return VAdd(modelPosition, gunPos); 
}

// �e�̉�]���擾
VECTOR Player::GetGunRot() const 
{
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}
