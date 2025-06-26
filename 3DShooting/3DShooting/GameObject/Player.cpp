#include "Player.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include "Bullet.h"
#include "SceneManager.h"
#include "SceneMain.h"
#include "SceneGameOver.h"
#include "DebugUtil.h"	
#include <cmath>
#include <cassert>
#include <algorithm>

namespace
{
	constexpr float kMoveSpeed = 3.0f; // �ړ����x
	constexpr float kRunSpeed  = 6.0f; // ���鑬�x

	// ���f���̃I�t�Z�b�g
	constexpr float kModelOffsetX = 80.0f; 
	constexpr float kModelOffsetY = 20.0f;
	constexpr float kModelOffsetZ = 60.0f;

	// �A�j���[�V�����̃u�����h��
	constexpr float kAnimBlendRate = 1.0f; 

	// �e�̃I�t�Z�b�g
	constexpr float kGunOffsetX = -55.0f;
	constexpr float kGunOffsetY = 55.0f;
	constexpr float kGunOffsetZ = 10.0f;

	// �����e��
	constexpr int kInitialAmmo = 7000;

	// UI�֘A
	constexpr int kMarginX    = 20; 
	constexpr int kMarginY    = 20;
	constexpr int kFontHeight = 20;

	// �d�͂ƃW�����v�֘A
	constexpr float kGravity   = 0.35f; // �d�͂̋���
	constexpr float kJumpPower = 7.0f;  // �W�����v�̏���
	constexpr float kGroundY   = 0.0f;  // �n�ʂ�Y���W

	// �^�b�N���֘A
	constexpr int   kTackleDuration    = 20;     // �^�b�N�������t���[����
	constexpr int   kTackleCooldownMax = 120;    // �N�[���^�C���ő�l
	constexpr float kTackleSpeed	   = 25.0f;  // �^�b�N�����̑��x
	constexpr float kTackleHitRange    = 250.0f; // �^�b�N���̑O���L������
	constexpr float kTackleHitRadius   = 250.0f; // �^�b�N���̉���(���a)
	constexpr float kTackleHitHeight   = 100.0f; // �^�b�N���̍���
	constexpr float kTackleDamage      = 10.0f;  // �^�b�N���_���[�W

	// �̗�
	constexpr float kMaxHealth = 100.0f; // �ő�̗�
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_pDebugCamera(std::make_shared<Camera>()),
	m_pEffect(std::make_shared<Effect>()),
	m_pEnemy(std::make_shared<EnemyNormal>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
	m_ammo(kInitialAmmo),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_isJumping(false),
	m_jumpVelocity(0.0f),
	m_hasShot(false),
	m_tackleFrame(0),
	m_tackleDir(VGet(0, 0, 0)),
	m_isTackling(false),
	m_tackleCooldown(0),
	m_tackleId(0),
	m_maxHealth(kMaxHealth)
{
	// �v���C���[���f���̓ǂݍ���
	m_modelHandle = MV1LoadModel("data/model/M4A1.mv1");
	assert(m_modelHandle != -1);

	// ���̓ǂݍ���
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

	// ���f���̑傫��
	MV1SetScale(m_modelHandle, VGet(10.0f, 10.0f, 5.0f));

	m_animBlendRate = kAnimBlendRate; // �A�j���[�V�����̃u�����h����ݒ�
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	// �v���C���[�̈ʒu���J�����ɐݒ�
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update(); // �J�����̍X�V
	m_pEffect->Update(); // �G�t�F�N�g�̍X�V

	//UpdateAnime(m_prevAnimData); // �O�̃A�j���[�V�����f�[�^���X�V
	//UpdateAnime(m_nextAnimData); //	���̃A�j���[�V�����f�[�^���X�V
	//UpdateAnimeBlend();			 // �A�j���[�V�����̃u�����h���X�V

	// ���f���̈ʒu�Ɖ�]���X�V
	VECTOR modelOffset	  = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw		  = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch		  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot		  = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// ���f���̈ʒu��ݒ�
	MV1SetPosition(m_modelHandle, modelPos);

	// ���f���̉�]��ݒ�
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F , 0.0f));

	// �^�b�N���N�[���^�C������
	if (m_tackleCooldown > 0)
	{
		m_tackleCooldown--;

		// �N�[���^�C����0�ɂȂ����u�ԂɑS�G�̃^�b�N���q�b�g�t���O�����Z�b�g
		if (m_tackleCooldown == 0)
		{
			for (EnemyBase* enemy : enemyList)
			{
				if (enemy)
				{
					enemy->ResetTackleHitFlag();
				}
			}
		}
	}

	// �}�E�X�̍��N���b�N�Ŏˌ�(�^�b�N�����͎ˌ��s��)
	if (!m_isTackling && Mouse::IsPressLeft() && m_ammo > 0)
	{
		Shoot(m_bullets); // �e�𔭎�
		m_ammo--; // �e������炷
	}

	// �n�ʂɂ��邩�ǂ����̔���
	bool isOnGround = (m_modelPos.y <= kGroundY + 0.01f);

	// �E�N���b�N�Ń^�b�N���J�n
	if (!m_isTackling && m_tackleCooldown <= 0 && Mouse::IsTriggerRight())
	{
		m_isTackling = true;
		m_tackleFrame = kTackleDuration;
		m_tackleCooldown = kTackleCooldownMax; // �N�[���^�C���J�n
		m_tackleId++; // �^�b�N�����Ƃ�ID���X�V

		// �J�����̌�����3D���K���x�N�g�����쐬
		float yaw = m_pCamera->GetYaw();
		float pitch = m_pCamera->GetPitch();

		// �^�b�N���������v�Z
		m_tackleDir = VGet(
			cosf(pitch) * sinf(yaw),
			sinf(pitch),
			cosf(pitch) * cosf(yaw)
		);

		//	ChangeAnime(kTackleAnimName, false); // �^�b�N���A�j���[�V����

		// �^�b�N���J�n����FOV���L���A�J���������Ɉ���
		if (m_pCamera)
		{
			m_pCamera->SetTargetFOV(100.0f * DX_PI_F / 180.0f);
			VECTOR offset = m_pCamera->GetOffset();
			offset.z = 30.0f;
			m_pCamera->SetOffset(offset);
		}
	}

		// �^�b�N�����̏���
		if (m_isTackling)
		{
			m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, kTackleSpeed));

			// �n�ʂ�艺�ɍs���Ȃ��悤�ɐ���
			if (m_modelPos.y < kGroundY)
			{
				m_modelPos.y = kGroundY;
			}

			// �^�b�N����������쐬
			TackleInfo tackleInfo = GetTackleInfo();

			// �e�G�Ƀ^�b�N������n����Update
			for (EnemyBase* enemy : enemyList)
			{
				// �G��nullptr�̏ꍇ�̓X�L�b�v
				if (!enemy) continue;

				// �G�̍X�V����
				enemy->Update(m_bullets, tackleInfo, *this);
			}

#ifdef _DEBUG
			// �^�b�N������J�v�Z���̃f�o�b�O�`��
			DebugUtil::DrawCapsule(
				tackleInfo.capA,
				tackleInfo.capB,
				tackleInfo.radius,
				16,
				0x00ff00,
				false
			);
#endif
			m_tackleFrame--;
			// �^�b�N���I������
			if (m_tackleFrame <= 0)
			{
				m_isTackling = false;

				// �^�b�N���I������FOV�ƃJ�����I�t�Z�b�g�����ɖ߂�
				if (m_pCamera)
				{
					m_pCamera->ResetFOV();
					m_pCamera->ResetOffset();
				}

				// �^�b�N����̃A�j���[�V�����J��
				/*if (m_isMoving)
				{
					ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
				}
				else
				{
					ChangeAnime(kIdleAnimName, true);
				}*/
			}
			// �^�b�N�����͑��̈ړ��E�W�����v�𖳌���
			return;
		}

		// �^�b�N�����łȂ���� bullets �̂ݓn��
		TackleInfo tackleInfo{};
		for (EnemyBase* enemy : enemyList)
		{
			if (!enemy) continue;
			enemy->Update(m_bullets, tackleInfo, *this);
		}
	
		// �e�̍X�V
		Bullet::UpdateBullets(m_bullets);

		// ����L�[����
		const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
		bool isRunning = wantRun; // �����Ă��邩�ǂ����̃t���O

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

		// �X�y�[�X�L�[���������u�Ԃ̂݃W�����v
		if (keyState[KEY_INPUT_SPACE] && !m_prevKeyState[KEY_INPUT_SPACE] && isOnGround && !m_isJumping && !m_isTackling)
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

		//// �A�j���[�V�������I��������
		//if (m_nextAnimData.isEnd)
		//{
		//	if (m_isMoving)
		//	{
		//		// �ړ��A�j���[�V�����ɕύX
		//		ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
		//	}
		//	else
		//	{
		//		// �ҋ@�A�j���[�V�����ɕύX
		//		ChangeAnime(kIdleAnimName, true);
		//	}
		//}

		// �ړ����őO��͈ړ����Ă��Ȃ������ꍇ
		//if (isMoving && !m_isMoving)
		//{
		//	ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		//}
		//else if (!isMoving && m_isMoving)
		//{
		//	ChangeAnime(kIdleAnimName, true);
		//}
		//else if (isMoving && m_isMoving && (isRunning != m_isWasRunning))
		//{
		//	ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		//}

		m_isMoving = isMoving;  // �ړ����̏�Ԃ��X�V
		m_isWasRunning = isRunning; // �����Ă����Ԃ��X�V

		std::copy(std::begin(keyState), std::end(keyState), std::begin(m_prevKeyState));

		// �v���C���[�̎΂ߌ����󂩂�v���C���[������
		if (m_pDebugCamera) 
		{
			VECTOR target = m_modelPos;
			VECTOR offset = VGet(-200.0f, 150.0f, -200.0f); // �΂ߌ�����

			m_pDebugCamera->SetPos(VAdd(target, offset));
			m_pDebugCamera->SetTarget(target);
			m_pDebugCamera->SetFOV(60.0f * DX_PI_F / 180.0f);
		}
	}


void Player::Draw()
{
	DrawField(); // �t�B�[���h�̕`��

	// �v���C���[���f���̕`��
	MV1DrawModel(m_modelHandle); 

	// �e�̕`��
	Bullet::DrawBullets(m_bullets);

	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;
	GetScreenState(&screenW, &screenH, NULL);

	// �c�e���̕\��
	int ammoFontSize = 32;
	int ammoX = screenW - 200;
	int ammoY = screenH - 60;
	int ammoColor = GetColor(255, 255, 80);

	SetFontSize(ammoFontSize);
	DrawFormatString(ammoX, ammoY, ammoColor, "AMMO: %d", m_ammo);
	SetFontSize(16); // �t�H���g�T�C�Y�����ɖ߂�


	/*���̕`��*/
	float swordScreenX = -25.0f;
	float swordScreenY = -30;
	float swordScreenZ = 35.0f;

	VECTOR camPos = VGet(0, 0, -swordScreenZ); // �J�����̈ʒu
	VECTOR camTgt = VGet(0, 0, 0);			    // �J�����̃^�[�Q�b�g�ʒu
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt); 

	// ���̈ʒu
	MV1SetPosition(m_swordHandle, VGet(swordScreenX, swordScreenY, 0.0f));

	MV1SetRotationXYZ(m_swordHandle, VGet(0.0f, 200.0f, 0.0f)); // ���̉�]
	MV1SetScale(m_swordHandle, VGet(0.5f, 0.5f, 0.5f));		    // ���̃X�P�[��

	// ���̕`��
	MV1DrawModel(m_swordHandle);

	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw(); // �G�t�F�N�g�̕`��

	// �^�b�N���N�[���^�C���Q�[�W
	const int tackleGaugeX = 10;
	const int tackleGaugeY = 50;
	const int tackleGaugeWidth = 200;
	const int tackleGaugeHeight = 16;

	// �g
	DrawBox(tackleGaugeX - 1, tackleGaugeY - 1, tackleGaugeX + tackleGaugeWidth + 1, tackleGaugeY + tackleGaugeHeight + 1, GetColor(80, 80, 200), false);

	// �Q�[�W�{��
	float tackleRate = 1.0f - (m_tackleCooldown / static_cast<float>(kTackleCooldownMax));
	int tackleFilledWidth = static_cast<int>(tackleGaugeWidth * tackleRate);
	int tackleColor = GetColor(80, 180, 255);
	DrawBox(tackleGaugeX, tackleGaugeY, tackleGaugeX + tackleFilledWidth, tackleGaugeY + tackleGaugeHeight, tackleColor, true);

	// �e�L�X�g
	DrawFormatString(tackleGaugeX, tackleGaugeY - 18, 0xFFFFFF, "Tackle Cooldown");
	if (m_tackleCooldown > 0) 
	{
		DrawFormatString(tackleGaugeX + tackleGaugeWidth + 10, tackleGaugeY, 0xFF8080, "WAIT");
	}
	else 
	{
		DrawFormatString(tackleGaugeX + tackleGaugeWidth + 10, tackleGaugeY, 0x80FF80, "READY");
	}

	// HP�o�[�̃p�����[�^
    const int barWidth  = 200;
    const int barHeight = 24;
    const int margin    = 30;
    const int barX      = margin;
    const int barY      = screenH - barHeight - margin;

    // �ő�HP�i�K�v�ɉ����Ē萔���j
    const float maxHP = 100.0f; // ��: 100
    float hp = m_health;
    if (hp < 0) hp = 0;
    if (hp > maxHP) hp = maxHP;

    // HP����
    float hpRate = hp / maxHP;

    // �w�i
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(80, 80, 80), TRUE);

    // HP�o�[�{��
    DrawBox(barX, barY, barX + static_cast<int>(barWidth * hpRate), barY + barHeight, GetColor(255, 64, 64), TRUE);

    // �g
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(0, 0, 0), FALSE);

    // HP���l
    DrawFormatString(barX + 8, barY + 2, GetColor(255,255,255), "HP: %.0f / %.0f", hp, maxHP);

#ifdef _DEBUG
	if (m_pCamera)
	{
		// �f�o�b�O�J�����̈ʒu�ƃ^�[�Q�b�g���擾
		const VECTOR pos = m_pCamera->GetPos();
		const VECTOR tgt = m_pCamera->GetTarget();
		float fov = m_pCamera->GetFOV();

		// �T�u�J�����p�`��̈�
		int subW = 320, subH = 180; // �T�u�E�B���h�E�T�C�Y
		int subX = Game::kScreenWidth - subW;
		int subY = Game::kScreenHeigth - subH;

		// �T�u�J�����p�`���T�[�t�F�X�쐬
		int subScreen = MakeScreen(subW, subH, true);
		SetDrawScreen(subScreen);
		ClearDrawScreen();

		// �T�u�J�����ŕ`��
		m_pDebugCamera->SetCameraToDxLib();

		// �����Ńt�B�[���h�E�v���C���[���f���E�����蔻���`�� 
		DrawField(); // �t�B�[���h�`��
		MV1DrawModel(m_modelHandle); // �v���C���[���f���`��

		// �v���C���[�J�v�Z�������蔻��̃f�o�b�O�\��
		VECTOR capA, capB;
		float radius;
		GetCapsuleInfo(capA, capB, radius);
		DebugUtil::DrawCapsule(
			capA,
			capB,
			radius,
			16,
			0xff00ff,
			false
		);

		// ���C����ʂɖ߂�
		SetDrawScreen(DX_SCREEN_BACK);

		// �T�u��ʂ��E���ɓ]��
		DrawGraph(subX, subY, subScreen, false);

		// �T�[�t�F�X���
		DeleteGraph(subScreen);

		// �W���o��
		printf("[DebugCamera] Pos:(%.1f, %.1f, %.1f)  Target:(%.1f, %.1f, %.1f)  FOV:%.1f\n",
			pos.x, pos.y, pos.z, tgt.x, tgt.y, tgt.z, fov * 180.0f / DX_PI_F);
	}
#endif

}

// �t�B�[���h�̕`��
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

// �_���[�W���󂯂鏈��
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

// �v���C���[���V���b�g�\���ǂ���
bool Player::HasShot()
{
	bool shot = m_hasShot;
	m_hasShot = false; // ��Ԃ����Z�b�g
	return shot; // ���������ǂ�����Ԃ�
}

void Player::Shoot(std::vector<Bullet>& bullets)
{
    //// �N�[���_�E�����Ȃ甭�˂��Ȃ�
    //if (m_shotCooldownTimer > 0)
    //{
    //    return;
    //}

    //// �e�򂪂Ȃ��Ȃ甭�˂��Ȃ�
    //if (m_ammoCount <= 0)
    //{
    //    return;
    //}

    // �J�����̈ʒu�ƃJ�����������Ă�������̉����ڕW�_���擾
    // m_pCamera��GetPos()���\�b�h�������Ƃ�O��Ƃ��Ă��܂��B
    // �����R���p�C���G���[����������ꍇ�ACamera�N���X�̎��������m�F���������B
    VECTOR cameraPos = m_pCamera->GetPos();
    VECTOR cameraLookDir = GetGunRot(); // �J�����̌��݂̌���
    float targetDistance = 1000.0f; // �\���ɉ��������i��: 1000�P�ʁj
    VECTOR targetPoint = VAdd(cameraPos, VScale(cameraLookDir, targetDistance));

    // �e�̔��ˈʒu����ڕW�_�ւ̕����x�N�g�����v�Z
    VECTOR gunPos = GetGunPos();
    VECTOR bulletDirection = VNorm(VSub(targetPoint, gunPos));

    // �e�ۂ𔭎�
    bullets.emplace_back(gunPos, bulletDirection); // �V���� bulletDirection ���g�p
    m_ammo--;
 //   m_pEffect->PlaySound(Effect::SoundType::Shot);
//    m_pEffect->SpawnEffect(Effect::EffectType::MuzzleFlash, gunPos, bulletDirection); // �G�t�F�N�g�̌������X�V

    // ���˃N�[���_�E����ݒ�
    //m_shotCooldownTimer = kShotCooldown;

    // �A�j���[�V�����֘A
    ChangeAnime("Shoot", false); // �ˌ��A�j���[�V�������Đ�
  //  SetAnimSpeed(kShotAnimSpeed);
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

// �e�̌������擾
VECTOR Player::GetGunRot() const
{
	// ���̊֐��̓J�����̏����Ȍ�����Ԃ����̂Ƃ��Ĉێ����܂�
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}

// �^�b�N�������擾
Player::TackleInfo Player::GetTackleInfo() const
{
	TackleInfo info;
	info.isTackling = m_isTackling;
	if (m_isTackling)
	{
		float tackleHeight = kTackleHitHeight;
		info.tackleId = m_tackleId; // �^�b�N��ID���Z�b�g

		// �v���C���[�̑̂̒��S�ʒu
		VECTOR bodyCenter = m_modelPos;
		bodyCenter.y += kModelOffsetY;

		// �v���C���[�̑O�ʒ��S�i�̂̒��S����O����kTackleHitRange�����i�߂�j
		VECTOR frontCenter = VAdd(bodyCenter, VScale(m_tackleDir, kTackleHitRange));

		// �J�v�Z���̒��S����O�ʒ��S�̏㉺�ɐL�΂�
		VECTOR capA = VAdd(frontCenter, VGet(0, -tackleHeight * 0.5f, 0));
		VECTOR capB = VAdd(frontCenter, VGet(0, tackleHeight * 0.5f, 0));

		info.capA = capA;
		info.capB = capB;
		info.radius = kTackleHitRadius;
		info.damage = kTackleDamage;
	}
	return info;
}

// �J�v�Z�������擾
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
	// �v���C���[�̑̂̒��S����ɁA�c���̃J�v�Z����z��
	constexpr float kCapsuleHeight = 100.0f; // �v���C���[�̐g��
	constexpr float kCapsuleRadius = 80.0f;  // �v���C���[�̑̂̔��a

	VECTOR center = m_modelPos;
	center.y += 60.0f; // �������獘�`��������𒆐S��

	capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	radius = kCapsuleRadius;
}

void Player::AddHp(float value)
{
	m_health += value; // �̗͂����Z
	if (m_health > m_maxHealth)
	{
		m_health = m_maxHealth; // �ő�̗͂𒴂��Ȃ��悤�ɐ���
	}

	if (m_health < 0.0f)
	{
		m_health = 0.0f; // �̗͂����ɂȂ�Ȃ��悤�ɐ���
	}
}
