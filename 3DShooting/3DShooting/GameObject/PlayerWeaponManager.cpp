#include "PlayerWeaponManager.h"
#include "Bullet.h"
#include "Camera.h"
#include "Effect.h"
#include "ShellCasing.h"
#include "EnemyBase.h"
#include "Collision.h"
#include "AnimationManager.h"
#include "Game.h"
#include <cmath>
#include <cassert>
#include <algorithm>

namespace
{
	// アサルトライフルオフセット
	constexpr float kAROffsetX = 80.0f;
	constexpr float kAROffsetY = 20.0f;
	constexpr float kAROffsetZ = 60.0f;

	// アサルトライフルマズルフラッシュエフェクトのオフセット
	constexpr float kARMuzzleFlashEffectOffsetX = -20.0f;
	constexpr float kARMuzzleFlashEffectOffsetY = 30.0f;
	constexpr float kARMuzzleFlashEffectOffsetZ = 80.0f;

	// ショットガンオフセット
	constexpr float kSGOffsetX = 80.0f;
	constexpr float kSGOffsetY = -30.0f;
	constexpr float kSGOffsetZ = 60.0f;

	// ショットガンマズルフラッシュエフェクトのオフセット
	constexpr float kSGMuzzleFlashEffectOffsetX = 30.0f;
	constexpr float kSGMuzzleFlashEffectOffsetY = 60.0f;
	constexpr float kSGMuzzleFlashEffectOffsetZ = 210.0f;

	// 1秒あたりの発射回数
	constexpr float kARShootRate = 10.0f;
	constexpr float kSGShootRate = 1.3f;

	// 武器切り替えアニメーション
	constexpr float kWeaponSwitchDuration = 1.1f;

	// 弾薬が少ないと判断する閾値
	constexpr int kLowAmmoThreshold = 10;

	// カメラシェイク
	constexpr float kARShootShakePower  = 4.0f;
	constexpr float kSGShootShakePower  = 32.0f;
	constexpr int   kShootShakeDuration = 8;

	// Update関連
	constexpr float kFrameRate = 60.0f;
	constexpr float kDeltaTime = 1.0f / kFrameRate;
}

PlayerWeaponManager::PlayerWeaponManager() :
	m_arHandle(-1),
	m_sgHandle(-1),
	m_ejectionPortFrame(-1),
	m_arAmmo(0),
	m_sgAmmo(0),
	m_arMaxAmmo(0),
	m_sgMaxAmmo(0),
	m_bulletPower(0.0f),
	m_sgBulletPower(0.0f),
	m_shootCooldown(1.0f / kARShootRate),
	m_shootCooldownTimer(0.0f),
	m_arShootRate(kARShootRate),
	m_currentWeaponType(WeaponType::AssaultRifle),
	m_previousWeaponType(WeaponType::AssaultRifle),
	m_currentWeaponIndex(0),
	m_isSwitchingWeapon(false),
	m_weaponSwitchTimer(0.0f),
	m_weaponSwitchDuration(kWeaponSwitchDuration),
	m_prevWeaponHadLowAmmo(false),
	m_prevWeaponHadNoAmmo(false),
	m_isLowAmmo(false),
	m_isNoAmmoWarning(false),
	m_lowAmmoBlinkTimer(0.0f),
	m_isInfiniteAmmo(false),
	m_gunShakeOffset(VGet(0, 0, 0)),
	m_gunShakeTimer(0.0f),
	m_gunShakePower(0.0f),
	m_isSGAnimPlaying(false),
	m_sgAnimTime(0.0f),
	m_shotSEHandle(-1),
	m_sgShotSEHandle(-1),
	m_pullBackOffset(0.0f)
{
	// アサルトライフルモデルの読み込み
	m_arHandle = MV1LoadModel("data/model/AR.mv1");
	assert(m_arHandle != -1);

	// ショットガンモデルの読み込み
	m_sgHandle = MV1LoadModel("data/model/SG.mv1");
	assert(m_sgHandle != -1);

	// 薬莢排出口フレームのインデックスを検索
	m_ejectionPortFrame = MV1SearchFrame(m_arHandle, "AR_M_Ejection_Port");

	// SEの読み込み
	m_shotSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shotSEHandle != -1);
	m_sgShotSEHandle = LoadSoundMem("data/sound/SE/ShotgunShot.mp3");
	assert(m_sgShotSEHandle != -1);
}

PlayerWeaponManager::~PlayerWeaponManager()
{
	// モデルの解放
	MV1DeleteModel(m_arHandle);
	MV1DeleteModel(m_sgHandle);

	// SEの解放
	DeleteSoundMem(m_shotSEHandle);
	DeleteSoundMem(m_sgShotSEHandle);
}

void PlayerWeaponManager::Init(int arInitAmmo, int sgInitAmmo, int arMaxAmmo, int sgMaxAmmo, float bulletPower, float sgBulletPower, float arShootRate)
{
	m_arAmmo = arInitAmmo;
	m_sgAmmo = sgInitAmmo;
	m_arMaxAmmo = arMaxAmmo;
	m_sgMaxAmmo = sgMaxAmmo;
	m_bulletPower = bulletPower;
	m_sgBulletPower = sgBulletPower;
	m_arShootRate = arShootRate;

	// 武器リストの初期化
	m_weaponTypes.push_back(WeaponType::AssaultRifle);
	m_weaponTypes.push_back(WeaponType::Shotgun);

	// 初期武器を設定
	SwitchWeapon(m_weaponTypes[m_currentWeaponIndex]);
}

void PlayerWeaponManager::Update(float deltaTime, const VECTOR& playerPos, Camera* pCamera, bool isGuarding, bool isDead, bool isTackling, bool isLockingOn, bool isSwitchingWeapon, AttackType allowedAttackType, bool isInfiniteAmmo, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData)
{
	m_isInfiniteAmmo = isInfiniteAmmo;

	// クールタイムタイマー減算
	if (m_shootCooldownTimer > 0.0f)
	{
		m_shootCooldownTimer -= deltaTime;
		if (m_shootCooldownTimer < 0.0f) m_shootCooldownTimer = 0.0f;
	}

	// 武器切り替えアニメーションの更新
	if (m_isSwitchingWeapon)
	{
		m_weaponSwitchTimer += deltaTime;
		if (m_weaponSwitchTimer >= m_weaponSwitchDuration)
		{
			m_isSwitchingWeapon = false;
			m_weaponSwitchTimer = 0.0f;
		}
	}

	// 銃のシェイク処理
	if (m_gunShakeTimer > 0.0f)
	{
		m_gunShakeTimer -= 1.0f;
		float shake = sinf(m_gunShakeTimer) * m_gunShakePower;
		m_gunShakeOffset.x = ((float)rand() / RAND_MAX - 0.5f) * shake;
		m_gunShakeOffset.y = ((float)rand() / RAND_MAX - 0.5f) * shake;
		if (m_gunShakeTimer <= 0.0f)
		{
			m_gunShakeOffset = VGet(0, 0, 0);
		}
	}

	// 弾薬低下の警告表示処理
	int currentAmmo = GetCurrentAmmo();
	if (currentAmmo == 0 && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer += deltaTime;
		m_isNoAmmoWarning = true;
	}
	else if (currentAmmo <= kLowAmmoThreshold && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = true;
		m_lowAmmoBlinkTimer += deltaTime;
		m_isNoAmmoWarning = false;
	}
	else
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer = 0.0f;
		m_isNoAmmoWarning = false;
	}

	// 銃の引き込み判定
	float targetPullBack = CalculatePullBackOffset(playerPos, pCamera, enemyList, collisionData);
	// スムーズに補間 (1秒で目標に近づく程度の速さ)
	m_pullBackOffset += (targetPullBack - m_pullBackOffset) * (1.0f - powf(0.1f, deltaTime * 10.0f));

	// 画面外（カメラの後ろ）に消えないようにクランプ。
	// 初期Zオフセットが60.0f程度なので、80.0f程度を上限とする。
	if (m_pullBackOffset > 80.0f) m_pullBackOffset = 80.0f;
}

void PlayerWeaponManager::Draw3D(const VECTOR& playerPos, Camera* pCamera, const VECTOR& gunSwayOffset, const VECTOR& gunShakeOffset, const VECTOR& gunSwayRotOffset, float guardAnimTimer, float guardAnimDuration, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, WeaponType previousWeaponType, bool isTryingToGuard)
{
	// モデルの位置と回転を更新
	MATRIX rotYaw = MGetRotY(pCamera->GetYaw());
	MATRIX rotPitch = MGetRotX(-pCamera->GetPitch());
	MATRIX modelRot = MMult(rotPitch, rotYaw);

	auto GetWeaponTransform = [&](WeaponType type) {
		VECTOR offset;
		int handle = -1;
		switch (type)
		{
		case WeaponType::AssaultRifle:
			offset = VGet(kAROffsetX, kAROffsetY, kAROffsetZ);
			handle = m_arHandle;
			break;
		case WeaponType::Shotgun:
			offset = VGet(kSGOffsetX, kSGOffsetY, kSGOffsetZ);
			handle = m_sgHandle;
			break;
		}
		return std::make_tuple(handle, offset);
	};

	// ガード入力中は武器を非表示にする（武器切り替え中でもガード入力があれば非表示）
	if (isTryingToGuard)
	{
		MV1SetVisible(m_arHandle, false);
		MV1SetVisible(m_sgHandle, false);
		return; // ガード中は武器を描画しない
	}
	else if (m_isSwitchingWeapon)
	{
		// 前の武器を下に隠す
		auto [prevHandle, prevOffset] = GetWeaponTransform(previousWeaponType);
		if (prevHandle != -1)
		{
			float progress = (std::min)(m_weaponSwitchTimer / (m_weaponSwitchDuration / 2.0f), 1.0f);
			float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
			float yOffset = easeOut * 300.0f;

			VECTOR rotModelOffset = VTransform(prevOffset, modelRot);
			VECTOR modelPos = VAdd(playerPos, rotModelOffset);
			modelPos.y -= yOffset;

			VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);
			
			// 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
			VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
			VECTOR camRight = VNorm(VCross(VGet(0, 1, 0), camForward));
			VECTOR camUp = VCross(camForward, camRight);

			// 現時点での checkDistance 概算値を使用して進行度を計算
			float checkDistance = (previousWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
			float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

			// 引き込みによる位置補正 (手前に引き、左上に寄せる)
			finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
			finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
			finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f));    // 上に寄せる

			// 最前面に描画するために Z バッファをクリア
			ClearDrawScreenZBuffer();

			MV1SetPosition(prevHandle, finalPos);

			// 回転の補正 (反時計回りにひねる等)
			VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
			baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる (ラジアン)
			
			MV1SetRotationXYZ(prevHandle, baseRot);
			MV1SetVisible(prevHandle, true);
			MV1DrawModel(prevHandle);
		}

		// 新しい武器を下から出す
		auto [currentHandle, currentOffset] = GetWeaponTransform(m_currentWeaponType);
		if (currentHandle != -1)
		{
			float progress = (std::max)(0.0f, (m_weaponSwitchTimer - (m_weaponSwitchDuration / 2.0f)) / (m_weaponSwitchDuration / 2.0f));
			float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
			float yOffset = (1.0f - easeOut) * 300.0f;

			VECTOR rotModelOffset = VTransform(currentOffset, modelRot);
			VECTOR modelPos = VAdd(playerPos, rotModelOffset);
			modelPos.y -= yOffset;

			VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);

			// 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
			VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
			VECTOR camRight = VNorm(VCross(VGet(0, 1, 0), camForward));
			VECTOR camUp = VCross(camForward, camRight);

			float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
			float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

			finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
			finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
			finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f));    // 上に寄せる

			// 最前面に描画するために Z バッファをクリア
			// (既に前の武器の描画でクリアされている可能性もあるが、安全のため)
			ClearDrawScreenZBuffer();

			MV1SetPosition(currentHandle, finalPos);

			// 回転の補正
			VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
			baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる
			
			MV1SetRotationXYZ(currentHandle, baseRot);
			MV1SetVisible(currentHandle, true);
			MV1DrawModel(currentHandle);
		}
	}
	else
	{
		// 通常時の武器表示
		auto [currentHandle, modelOffset] = GetWeaponTransform(m_currentWeaponType);
		int otherHandle = (m_currentWeaponType == WeaponType::AssaultRifle) ? m_sgHandle : m_arHandle;
		MV1SetVisible(otherHandle, false);

		if (currentHandle != -1)
		{
			MV1SetVisible(currentHandle, true);
			VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
			VECTOR modelPos = VAdd(playerPos, rotModelOffset);

			// ガードアニメーションの進行度を計算
			float guardAnimProgress = guardAnimTimer / guardAnimDuration;
			float gunOffsetY = -200.0f * (1.0f - cosf(guardAnimProgress * DX_PI_F * 0.5f));
			modelPos.y += gunOffsetY;

			// モデルの位置を設定
			VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);

			// 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
			VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
			VECTOR camRight = VNorm(VCross(VGet(0, 1, 0), camForward));
			VECTOR camUp = VCross(camForward, camRight);

			float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 220.0f : 240.0f;
			float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

			finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
			finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
			finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f));    // 上に寄せる

			// 最前面に描画するために Z バッファをクリア
			ClearDrawScreenZBuffer();

			MV1SetPosition(currentHandle, finalPos);

			// モデルの回転を設定 (ひねりを加える)
			VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
			baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる

			MV1SetRotationXYZ(currentHandle, baseRot);
			
			// モデルを描画
			MV1DrawModel(currentHandle);
		}
	}
}

void PlayerWeaponManager::SwitchWeapon(WeaponType weaponType)
{
	// 同じ武器への切り替え、または切り替え中は処理しない
	if (weaponType == m_currentWeaponType || m_isSwitchingWeapon)
	{
		return;
	}

	// 切り替え前の武器の弾薬警告状態を保存
	const int prevAmmo = GetCurrentAmmo();
	m_prevWeaponHadLowAmmo = (prevAmmo > 0 && prevAmmo <= kLowAmmoThreshold) && !m_isInfiniteAmmo;
	m_prevWeaponHadNoAmmo = (prevAmmo == 0) && !m_isInfiniteAmmo;

	m_isSwitchingWeapon = true;
	m_weaponSwitchTimer = 0.0f;
	m_previousWeaponType = m_currentWeaponType;
	m_currentWeaponType = weaponType;

	// 武器の種類に応じてクールダウンを設定
	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		m_shootCooldown = 1.0f / m_arShootRate;
		break;
	case WeaponType::Shotgun:
		m_shootCooldown = 1.0f / kSGShootRate;
		break;
	default:
		break;
	}
}

void PlayerWeaponManager::Shoot(std::vector<Bullet>& bullets, const VECTOR& playerPos, Camera* pCamera, Effect* pEffect, AnimationManager* pAnimManager, std::vector<ShellCasing>& shellCasings)
{
	// 画面中央（カメラ中心）からレティクル方向へ発射
	VECTOR cameraPos = pCamera->GetPos();
	VECTOR cameraForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));

	VECTOR gunPos = GetGunPos(playerPos, pCamera);
	VECTOR gunDir = GetGunRot(pCamera);

	// 画面中央から出ているように見せるため、カメラ前方に小さくオフセット
	VECTOR spawnPos = VAdd(cameraPos, VScale(cameraForward, 0.0f));

	int currentShotSEHandle = -1;
	int currentModelHandle = -1;
	VECTOR currentMuzzleFlashOffset = VGet(0, 0, 0);

	float shakePower = 0.0f;
	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		bullets.emplace_back(spawnPos, cameraForward, AttackType::Shoot, m_bulletPower);
		currentShotSEHandle = m_shotSEHandle;
		currentModelHandle = m_arHandle;
		currentMuzzleFlashOffset = VGet(kARMuzzleFlashEffectOffsetX, kARMuzzleFlashEffectOffsetY, kARMuzzleFlashEffectOffsetZ);
		shakePower = kARShootShakePower;
		break;
	case WeaponType::Shotgun:
		currentShotSEHandle = m_sgShotSEHandle;
		currentModelHandle = m_sgHandle;
		currentMuzzleFlashOffset = VGet(kSGMuzzleFlashEffectOffsetX, kSGMuzzleFlashEffectOffsetY, kSGMuzzleFlashEffectOffsetZ);
		shakePower = kSGShootShakePower;
		if (pAnimManager)
		{
			pAnimManager->PlayAnimation(m_sgHandle, "Armature.001|Armature.001|lever action_FIRE|Baked frames", false);
			m_isSGAnimPlaying = true;
			m_sgAnimTime = 0.0f;
		}
		// ショットガンは複数弾をばらけさせて発射
		for (int i = 0; i < 5; ++i)
		{
			float spreadX = ((float)GetRand(100) / 100.0f - 0.5f) * 0.1f;
			float spreadY = ((float)GetRand(100) / 100.0f - 0.5f) * 0.1f;

			VECTOR spreadDir = VAdd(cameraForward, VGet(spreadX, spreadY, 0));
			spreadDir = VNorm(spreadDir);

			bullets.emplace_back(spawnPos, spreadDir, AttackType::Shoot, m_sgBulletPower);
		}
		break;
	default:
		break;
	}

	// 薬莢を生成 (アサルトライフルの場合のみ)
	if (m_currentWeaponType == WeaponType::AssaultRifle)
	{
		VECTOR ejectionPos = GetEjectionPortPos();
		VECTOR ejectionDir = VGet(sinf(pCamera->GetYaw() + DX_PI_F * 0.5f), 0.5f, cosf(pCamera->GetYaw() + DX_PI_F * 0.5f));
		shellCasings.emplace_back(ejectionPos, ejectionDir);
	}

	// SEを再生
	PlaySoundMem(currentShotSEHandle, DX_PLAYTYPE_BACK);

	float rotX = -pCamera->GetPitch();
	float rotY = pCamera->GetYaw();
	float rotZ = 0.0f;

	if (pEffect)
	{
		// 引き込みによる「ひねり」を Z 回転に反映
		float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
		float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);
		rotZ += pullProgress * 1.5f;

		pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	// カメラシェイクを発生
	if (pCamera)
	{
		pCamera->Shake(shakePower, kShootShakeDuration);
	}

	// クールタイムリセット
	m_shootCooldownTimer = m_shootCooldown;
}

bool PlayerWeaponManager::CanShoot() const
{
	return m_shootCooldownTimer <= 0.0f;
}

int PlayerWeaponManager::GetCurrentAmmo() const
{
	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		return m_arAmmo;
	case WeaponType::Shotgun:
		return m_sgAmmo;
	default:
		return 0;
	}
}

int PlayerWeaponManager::GetMaxAmmo() const
{
	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		return m_arMaxAmmo;
	case WeaponType::Shotgun:
		return m_sgMaxAmmo;
	default:
		return 0;
	}
}

void PlayerWeaponManager::AddARAmmo(int value)
{
	m_arAmmo += value;
	if (m_arAmmo > m_arMaxAmmo) m_arAmmo = m_arMaxAmmo;
}

void PlayerWeaponManager::AddSGAmmo(int value)
{
	m_sgAmmo += value;
	if (m_sgAmmo > m_sgMaxAmmo) m_sgAmmo = m_sgMaxAmmo;
}

void PlayerWeaponManager::ConsumeAmmo()
{
	if (!m_isInfiniteAmmo)
	{
		if (m_currentWeaponType == WeaponType::AssaultRifle)
		{
			m_arAmmo--;
		}
		else if (m_currentWeaponType == WeaponType::Shotgun)
		{
			m_sgAmmo--;
		}
	}
}

VECTOR PlayerWeaponManager::GetGunPos(const VECTOR& playerPos, Camera* pCamera) const
{
	VECTOR modelOffset;
	VECTOR muzzleFlashOffset;

	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		modelOffset = VGet(kAROffsetX, kAROffsetY, kAROffsetZ);
		muzzleFlashOffset = VGet(kARMuzzleFlashEffectOffsetX, kARMuzzleFlashEffectOffsetY, kARMuzzleFlashEffectOffsetZ);
		break;
	case WeaponType::Shotgun:
		modelOffset = VGet(kSGOffsetX, kSGOffsetY, kSGOffsetZ);
		muzzleFlashOffset = VGet(kSGMuzzleFlashEffectOffsetX, kSGMuzzleFlashEffectOffsetY, kSGMuzzleFlashEffectOffsetZ);
		break;
	default:
		modelOffset = VGet(0, 0, 0);
		muzzleFlashOffset = VGet(0, 0, 0);
		break;
	}

	// モデルのオフセットと回転を計算
	MATRIX rotYaw = MGetRotY(pCamera->GetYaw());
	MATRIX rotPitch = MGetRotX(-pCamera->GetPitch());
	MATRIX modelRot = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR gunBasePosition = VAdd(playerPos, rotatedModelOffset);

	VECTOR rotatedMuzzleFlashOffset = VTransform(muzzleFlashOffset, modelRot);

	// 基本の銃口位置
	VECTOR gunMuzzlePos = VAdd(gunBasePosition, rotatedMuzzleFlashOffset);

	// 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
	VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));
	VECTOR camRight = VNorm(VCross(VGet(0, 1, 0), camForward));
	VECTOR camUp = VCross(camForward, camRight);

	float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
	float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

	gunMuzzlePos = VSub(gunMuzzlePos, VScale(camForward, m_pullBackOffset));
	gunMuzzlePos = VSub(gunMuzzlePos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
	gunMuzzlePos = VAdd(gunMuzzlePos, VScale(camUp, pullProgress * 20.0f));    // 上に寄せる

	return gunMuzzlePos;
}

VECTOR PlayerWeaponManager::GetGunRot(Camera* pCamera) const
{
	return VGet(
		cosf(pCamera->GetPitch()) * sinf(pCamera->GetYaw()),
		sinf(pCamera->GetPitch()),
		cosf(pCamera->GetPitch()) * cosf(pCamera->GetYaw())
	);
}

VECTOR PlayerWeaponManager::GetEjectionPortPos() const
{
	int ejectionPortFrame = MV1SearchFrame(m_arHandle, "AR_M_Ejection_Port");

	if (ejectionPortFrame != -1)
	{
		return MV1GetFramePosition(m_arHandle, ejectionPortFrame);
	}
	return VGet(0, 0, 0);
}

void PlayerWeaponManager::ShakeGun(float power, float duration)
{
	m_gunShakePower = power;
	m_gunShakeTimer = duration;
}

void PlayerWeaponManager::SetWeaponScale(const VECTOR& scale)
{
	MV1SetScale(m_arHandle, scale);
	MV1SetScale(m_sgHandle, scale);
}

void PlayerWeaponManager::SetWeaponRotation(const VECTOR& rot)
{
	MV1SetRotationXYZ(m_arHandle, rot);
	MV1SetRotationXYZ(m_sgHandle, rot);
}

void PlayerWeaponManager::UpdateSGAnimation(AnimationManager* pAnimManager, float deltaTime)
{
	if (m_isSGAnimPlaying)
	{
		m_sgAnimTime += 1.0f;
		if (pAnimManager)
		{
			pAnimManager->UpdateAnimationTime(m_sgHandle, m_sgAnimTime);

			// アニメーションが終了したかチェック
			float totalTime = pAnimManager->GetAnimationTotalTime(m_sgHandle, "Armature.001|Armature.001|lever action_FIRE|Baked frames");
			if (totalTime > 0 && m_sgAnimTime >= totalTime)
			{
				m_isSGAnimPlaying = false;
				int attachIndex = pAnimManager->GetCurrentAttachedAnimHandle(m_sgHandle);
				if (attachIndex != -1)
				{
					MV1DetachAnim(m_sgHandle, attachIndex);
					pAnimManager->ResetAttachedAnimHandle(m_sgHandle);
				}
			}
		}
	}
}

float PlayerWeaponManager::CalculatePullBackOffset(const VECTOR& playerPos, Camera* pCamera, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData) const
{
	if (!pCamera) return 0.0f;

	// 武器の種類に応じて判定距離を決定
	float checkDistance = 0.0f;
	switch (m_currentWeaponType)
	{
	case WeaponType::AssaultRifle:
		checkDistance = 160.0f; // アサルトライフルの長さ目安
		break;
	case WeaponType::Shotgun:
		checkDistance = 180.0f; // ショットガンの長さ目安
		break;
	}

	VECTOR camPos = pCamera->GetPos();
	VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), camPos));
	
	// 少し前から Ray を飛ばす
	VECTOR rayStart = VAdd(camPos, VScale(camForward, 10.0f)); 
	VECTOR rayEnd = VAdd(rayStart, VScale(camForward, checkDistance));

	float minT = 1.0f; // 0.0 - 1.0 の範囲で最も近い衝突点を探す
	bool hit = false;

	// ステージとの判定
	for (const auto& poly : collisionData)
	{
		float t = 0.0f;
		if (Collision::IntersectRayTriangle(rayStart, VScale(camForward, checkDistance), poly.v1, poly.v2, poly.v3, t))
		{
			if (t >= 0.0f && t < minT)
			{
				minT = t;
				hit = true;
			}
		}
	}

	// 敵との判定
	for (const auto& enemy : enemyList)
	{
		if (!enemy || !enemy->IsAlive()) continue;

		VECTOR hitPos;
		float hitDistSq;
		// EnemyBase::CheckHitPart を利用して Ray 判定を行う
		if (enemy->CheckHitPart(rayStart, rayEnd, hitPos, hitDistSq) != EnemyBase::HitPart::None)
		{
			float dist = sqrtf(hitDistSq);
			float t = dist / checkDistance;
			if (t < minT)
			{
				minT = t;
				hit = true;
			}
		}
	}

	if (hit)
	{
		// 衝突点から逆算して、引き込み量を決める
		// 最前面描画が有効なので、引き込み量自体は控えめに（0.5倍程度）して「ひねり」を強調する
		return (1.0f - minT) * checkDistance * 0.5f; 
	}

	return 0.0f;
}

