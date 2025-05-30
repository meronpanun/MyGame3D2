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
	// アニメーション名
	const char* const kIdleAnimName = "Pistol_IDLE";
	const char* const kShotAnimName = "Pistol_FIRE";
	const char* const kWalkAnimName = "Pistol_WALK";
	const char* const kRunAnimName = "Pistol_RUN";
	//const char* const kReloadAnimName = "Pistol_RELOAD"; 

	constexpr float kMoveSpeed = 2.0f; // 移動速度
	constexpr float kRunSpeed = 5.0f;  // 走る速度

	constexpr float kModelOffsetX = 2.0f;
	constexpr float kModelOffsetY = 30.0f;
	constexpr float kModelOffsetZ = 25.0f;

	constexpr float kAnimBlendRate = 1.0f;

	constexpr float kGunOffsetX = 10.0f;
	constexpr float kGunOffsetY = 58.0f;
	constexpr float kGunOffsetZ = 12.0f;

	constexpr float kStaminaMax = 100.0f;
	constexpr float kStaminaRunCost = 0.8f;
	constexpr float kStaminaRecover = 0.5f;
	constexpr float kStaminaRunRecover = 30.0f;

	constexpr int kInitialAmmo = 700;
	//	constexpr int kMaxAmmo     = 10;  
	//	constexpr int kReloadTime  = 80;  


	constexpr int kMarginX = 20;
	constexpr int kMarginY = 20;
	constexpr int kFontHeight = 20;

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
	m_shotCooldown(0),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f)
{
	m_modelHandle = MV1LoadModel("data/image/Player.mv1");
	assert(m_modelHandle != -1);

	m_shieldHandle = MV1LoadModel("data/image/Shield.mv1");
	assert(m_shieldHandle != -1);

	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);

	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));
}

Player::~Player()
{
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_shieldHandle);

	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	m_pCamera->Init();

	m_pEffect->Init();

	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate;
}

void Player::Update()
{
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update();

	m_pEffect->Update();

	UpdateAnime(m_prevAnimData);
	UpdateAnime(m_nextAnimData);
	UpdateAnimeBlend();

	VECTOR modelOffset = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos = VAdd(m_modelPos, rotModelOffset);

	MV1SetPosition(m_modelHandle, modelPos);

	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	if (m_shotCooldown > 0)
	{
		m_shotCooldown--;
	}


	//if (m_isReloading) 
	//{
	//	m_reloadTimer--;
	//	if (m_reloadTimer <= 0)
	//	{
	//	
	//		int need = kInitialAmmo - m_ammo;
	//		if (need > 0)
	//		{
	//			
	//			int reloadAmount = (m_maxAmmo < need) ? m_maxAmmo : need; 
	//			m_ammo += reloadAmount;  
	//			m_maxAmmo -= reloadAmount; 

	//			
	//			if (m_maxAmmo < 0) m_maxAmmo = 0;
	//		}
	//		m_isReloading = false;
	//	}
	//}
//	else
//	{

	if (Mouse::IsTriggerLeft() && m_ammo > 0 && m_shotCooldown == 0)
	{
		Shoot();
		m_ammo--;
		m_shotCooldown = 10;
		ChangeAnime(kShotAnimName, false);
	}

	//if (CheckHitKey(KEY_INPUT_R) && m_ammo < kInitialAmmo && m_maxAmmo > 0)
	//{
	//	Reload();
	//	ChangeAnime(kReloadAnimName, false);
	//}
//	}


	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
	bool isRunning = false;
	if (wantRun && m_isCanRun && m_stamina > 0.0f)
	{
		isRunning = true;
	}
	float moveSpeed = isRunning ? kRunSpeed : kMoveSpeed;

	bool isMoving = false;


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

	if (moveDir.x != 0.0f || moveDir.z != 0.0f)
	{
		float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		moveDir.x /= len;
		moveDir.z /= len;
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
		isMoving = true;
	}

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

	if (m_stamina <= 0.0f)
	{
		m_isCanRun = false;
	}
	else if (!m_isCanRun && m_stamina >= kStaminaRunRecover)
	{
		m_isCanRun = true;
	}

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
	}

	m_isMoving = isMoving;
	m_isWasRunning = isRunning;
}
void Player::Draw()
{
	MV1DrawModel(m_modelHandle);

	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;

	float shieldScreenX = -25.0f;
	float shieldScreenY = -30;
	float shieldScreenZ = 42.0f;

	VECTOR camPos = VGet(0, 0, -shieldScreenZ);
	VECTOR camTgt = VGet(0, 0, 0);
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt);

	MV1SetPosition(m_shieldHandle, VGet(shieldScreenX, shieldScreenY, 0.0f));

	MV1SetRotationXYZ(m_shieldHandle, VGet(0.0f, DX_PI_F, 0.0f));
	MV1SetScale(m_shieldHandle, VGet(0.5f, 0.5f, 0.5f));

	MV1DrawModel(m_shieldHandle);

	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw();

	DrawField();

	const int bgWidth = 160;
	const int bgHeight = 48;
	int bgX = screenW - kMarginX - bgWidth;
	int bgY = screenH - kMarginY - bgHeight;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(bgX, bgY, bgX + bgWidth, bgY + bgHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int ammoX = bgX + 12;
	int ammoY = bgY + 8;
	DrawFormatString(ammoX, ammoY, 0xFFFFFF, "Ammo: %d", m_ammo/*, m_maxAmmo*/);

	DrawFormatString(10, 30, 0x000000, "Stamina: %.1f", m_stamina);

	const int gaugeX = 10;
	const int gaugeY = 50;
	const int gaugeWidth = 200;
	const int gaugeHeight = 16;

	DrawBox(gaugeX - 1, gaugeY - 1, gaugeX + gaugeWidth + 1, gaugeY + gaugeHeight + 1, GetColor(0x80, 0x80, 0x80), FALSE);

	float staminaRate = m_stamina / kStaminaMax;
	int filledWidth = static_cast<int>(gaugeWidth * staminaRate);

	int r = static_cast<int>((1.0f - staminaRate) * 255);
	int g = static_cast<int>(staminaRate * 255);
	int gaugeColor = GetColor(r, g, 0);

	DrawBox(gaugeX, gaugeY, gaugeX + filledWidth, gaugeY + gaugeHeight, gaugeColor, TRUE);
}

void Player::DrawField()
{
	SetUseLighting(false);

	const int gridSize = 100;
	const int fieldSize = 800;

	unsigned int color1 = GetColor(80, 80, 80);
	unsigned int color2 = GetColor(100, 100, 100);

	for (int z = -fieldSize * 0.5f; z < fieldSize * 0.5f; z += gridSize)
	{
		for (int x = -fieldSize * 0.5f; x < fieldSize * 0.5f; x += gridSize)
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
	m_health -= damage; // ダメージを適用
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
}

void Player::Shoot()
{
	VECTOR gunPos = GetGunPos();

	float rotX = -m_pCamera->GetPitch();
	float rotY = m_pCamera->GetYaw();
	float rotZ = 0.0f;

	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);
}

void Player::AttachAnime(AnimData& data, const char* animName, bool isLoop)
{
	int index = MV1GetAnimIndex(m_modelHandle, animName);

	data.attachNo = MV1AttachAnim(m_modelHandle, index, -1, false);
	data.count = 0.0f;
	data.isLoop = isLoop;
	data.isEnd = false;
}

void Player::UpdateAnime(AnimData& data)
{
	if (data.attachNo == -1) return;

	float animSpeed = 1.0f;
	//if (m_isReloading && data.attachNo == m_nextAnimData.attachNo) 
	//{
	//	animSpeed = 2.0f; 
	//}
	data.count += animSpeed;

	const float totalTime = MV1GetAttachAnimTotalTime(m_modelHandle, data.attachNo);

	if (data.isLoop)
	{
		while (data.count > totalTime)
		{
			data.count -= totalTime;
		}
	}
	else
	{
		if (data.count > totalTime)
		{
			data.count = totalTime;
			data.isEnd = true;
		}
	}
	MV1SetAttachAnimTime(m_modelHandle, data.attachNo, data.count);
}

void Player::UpdateAnimeBlend()
{
	if (m_nextAnimData.attachNo == -1 && m_prevAnimData.attachNo == -1) return;

	m_animBlendRate += 1.0f / 8.0f;
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f;
	}

	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

void Player::ChangeAnime(const char* animName, bool isLoop)
{
	MV1DetachAnim(m_modelHandle, m_prevAnimData.attachNo);

	m_prevAnimData = m_nextAnimData;

	AttachAnime(m_nextAnimData, animName, isLoop);

	m_animBlendRate = 0.0f;

	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

VECTOR Player::GetGunPos() const
{
	VECTOR modelOffset = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPosition = VAdd(m_modelPos, rotatedModelOffset);

	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
	VECTOR gunPos = VTransform(gunOffset, modelRot);
	return VAdd(modelPosition, gunPos);
}

VECTOR Player::GetGunRot() const
{
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
