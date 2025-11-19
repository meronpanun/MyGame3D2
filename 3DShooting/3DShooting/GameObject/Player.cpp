#include "Player.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "EffekseerForDXLib.h"
#include "Game.h" 
#include "InputManager.h"
#include "Camera.h"
#include "Effect.h"
#include "Bullet.h"
#include "SceneManager.h"
#include "SceneMain.h"
#include "SceneGameOver.h"
#include "DebugUtil.h"
#include "CapsuleCollider.h"
#include "TransformDataLoader.h"
#include "DirectionIndicator.h"
#include "ShellCasing.h"
#include "AnimationManager.h"
#include <cmath>
#include <cassert>
#include <algorithm>

namespace
{
	// タックル関連
	constexpr int   kTackleDuration  = 20;     // タックル持続フレーム数
	constexpr float kTackleHitRange  = 250.0f; // タックルの前方有効距離
	constexpr float kTackleHitRadius = 250.0f; // タックルの横幅（半径）
	constexpr float kTackleHitHeight = 100.0f; // タックルの高さ

	// 銃の揺れ関連の定数
	constexpr float kGunSwayAmount   = 0.7f; // 銃モデルの揺れの強さ 
	constexpr float kGunSwayDamping  = 0.8f; // 銃モデルの揺れの減衰率 

	// Update関連
	constexpr float kFrameRate						= 60.0f; 
	constexpr float kDeltaTime						= 1.0f / kFrameRate; 
	constexpr float kPlayerColliderYOffset		    = 60.0f;  
	constexpr float kTackleFov						= 100.0f; // タックル中のカメラFOV
	constexpr float kTackleCameraZOffset            = 30.0f;  // タックル中のカメラZオフセット
	constexpr float kConcentrationLineEffectZOffset = 15.0f;  // 集中線エフェクトのZオフセット
	constexpr float kJumpSwayPower					= 5.0f;   // ジャンプ時の揺れの強さ　
	constexpr float kLandingSwayPower				= 5.0f;   // 着地時の揺れの強さ
	constexpr float kHpBarAnimSpeed					= 1.5f;   // HPバーのアニメーション速度
	constexpr int   kLowAmmoThreshold				= 10;     // 弾薬が少ないと判断する閾値
	constexpr float kLowHealthThreshold				= 30.0f;  // 体力が少ないと判断する閾値
	constexpr float kWarningBlinkSpeed				= 1.5f;   // 警告UIの点滅速度
	constexpr float kLowHealthEffectMaxAlpha		= 0.7f;   // 体力低下UIの最大アルファ値
	constexpr float kIdleSwaySpeed					= 1.5f;   // 揺れの速さ
	constexpr float kIdleSwayAmount					= 0.04f;  // 揺れの量
	 
	// 盾UI関連
	constexpr int   kShieldImageGaugeSpacing  = 10;  // 盾UIとクールダウンゲージの間隔
	constexpr int   kShieldImageActiveAlpha   = 255; // 使用可能な盾UIのアルファ値
	constexpr int   kShieldImageCooldownAlpha = 128; // クールダウン中の盾UIのアルファ値
	constexpr int   kShieldUIYPosition		  = 420;
	constexpr int   kShieldUIYOffset		  = 30; // 盾UIのY軸調整オフセット

	// フォント関連
	constexpr int   kDefaultFontThickness  = 3;  // フォントの太さ
	constexpr int   kAmmoFont			   = 32; // 弾薬フォントサイズ
	constexpr int   kHpFont				   = 20; // HPフォントサイズ
	constexpr int   kWarningFont		   = 24; // 警告フォントサイズ
	constexpr char  kDefaultFontName[]     = "Arial Black";
	constexpr char  kWarningFontName[]     = "HGPｺﾞｼｯｸE";
	constexpr int   kDefaultFontType	   = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;

	// ダメージエフェクト
	constexpr float kDamageEffectDuration = 30.0f; // ダメージエフェクトの持続時間
	constexpr int   kDamageEffectColorR   = 255; 
	constexpr int   kDamageEffectColorG   = 0;
	constexpr int   kDamageEffectColorB   = 0;

	// 回復エフェクト
	constexpr float kHealEffectDuration   = 45.0f; // 回復エフェクトの持続時間
	constexpr int   kHealEffectColorR     = 0;
	constexpr int   kHealEffectColorG     = 255;
	constexpr int   kHealEffectColorB     = 0;

	// 弾薬取得エフェクト
	constexpr float kAmmoEffectDuration   = 45.0f; // 弾薬取得エフェクトの持続時間
	constexpr int   kAmmoEffectColorR     = 255;
	constexpr int   kAmmoEffectColorG     = 128;
	constexpr int   kAmmoEffectColorB     = 0;

	// カメラシェイク
	constexpr float kTakeDamageShakePower		 = 5.0f;  // 攻撃を受けた時の揺れの強さ
	constexpr int   kTakeDamageShakeDuration	 = 15;    // 攻撃を受けた時の揺れの持続時間
	constexpr float kARShootShakePower			 = 4.0f;  // ARを撃った時の揺れの強さ
	constexpr float kSGShootShakePower			 = 32.0f; // SGを撃った時の揺れの強さ
	constexpr int   kShootShakeDuration			 = 8;     // 撃った時の揺れの持続時間
	constexpr float kShieldBreakGunShakePower    = 10.0f; // 盾破壊時の銃の揺れの強さ
	constexpr float kShieldBreakGunShakeDuration = 30.0f; // 盾破壊時の銃の揺れの持続時間

	// アサルトライフルUI関連
	constexpr int   kARImageWidth   = 200;
	constexpr int   kARImageHeight  = 133;
	constexpr int   kARImageMarginX = 40;
	constexpr int   kARImageMarginY = -60;

	// ショットガンUI関連
	constexpr int   kSGImageWidth   = 200;
	constexpr int   kSGImageHeight  = 64;
	constexpr int   kSGImageMarginX = 40;
	constexpr int   kSGImageMarginY = -20;

	// 弾薬UI関連
	constexpr int   kAmmoTextHeight		    = 32;   
	constexpr char  kAmmoTextMaxWidthStr[]  = "999";
	constexpr int   kAmmoTextGunOffsetX     = 20;   
	constexpr int   kAmmoTextGunOffsetY     = -15;  

	// 警告UI関連
	constexpr int   kWarningImageSize    = 128; 
	constexpr int   kWarningImageYOffset = 160; 
	constexpr int   kWarningTextYOffset  = 5;   
	constexpr int   kWarningImageSpacing = 20;  

	// HpUI関連
	constexpr int   kHpBarWidth			     = 200;
	constexpr int   kHpBarHeight		     = 24;
	constexpr int   kHpBarMargin			 = 30; 
	constexpr int   kHealthUiImageSize       = 64; 
	constexpr int   kHealthUiImageBarSpacing = 10; 
	constexpr float kMaxHp                   = 100.0f;
	constexpr int   kHpTextOffsetX		     = 8;
	constexpr int   kHpTextOffsetY		     = 2;

	// 色関連
	constexpr unsigned int kColorWhite			   = 0xffffff;
	constexpr unsigned int kColorLowAmmo           = 0xd3381c;
	constexpr unsigned int kColorTackleGaugeBorder = 0x5050C8;
	constexpr unsigned int kColorTackleGaugeFill   = 0x50B4ff;
	constexpr unsigned int kColorHpBarBg           = 0x505050;
	constexpr unsigned int kColorHpBarDamage	   = 0xFFD700;
	constexpr unsigned int kColorHpBarFill		   = 0xff4040;
	constexpr unsigned int kColorHpBarBorder       = 0x000000;
}

Player::Player() :
	m_shieldImageHandle(-1),
	m_playerHitSEHandle(-1),
	m_tackleSEHandle(-1),
	m_recoverySEHandle(-1),
	m_ammoItemSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pEffect(nullptr),
	m_pCamera(std::make_shared<Camera>()),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_healthBarAnim(100.0f),
	m_healthBarAnimTimer(0.0f),
	m_hasShot(false),
	m_tackleFrame(0),
	m_tackleDir(VGet(0, 0, 0)),
	m_isTackling(false),
	m_tackleCooldown(0),
	m_tackleId(0),
	m_damageEffectAlpha(0.0f),
	m_damageEffectTimer(0.0f),
	m_healEffectAlpha(0.0f),
	m_healEffectTimer(0.0f),
	m_ammoEffectAlpha(0.0f),
	m_ammoEffectTimer(0.0f),
	m_isInvincible(false),
	m_isInfiniteAmmo(false),
	m_tackleCooldownMax(0.0f),
	m_tackleSpeed(0.0f),
	m_tackleDamage(0.0f),
	m_concentrationLineEffectHandle(-1),
	m_noAmmoImageHandle(-1),
	m_arImageHandle(-1),
	m_lowAmmoARImageHandle(-1),
	m_noAmmoARImageHandle(-1),
	m_sgImageHandle(-1),
	m_lowAmmoSGImageHandle(-1),
	m_noAmmoSGImageHandle(-1),
	m_showLowAmmoWarning(false),
	m_isLowHealth(false),
	m_lowHealthBlinkTimer(0.0f),
	m_ammoTextFlashTimer(0.0f),
	m_idleSwayTimer(0.0f),
	m_gunSwayOffset(VGet(0, 0, 0)),
	m_gunSwayRotOffset(VGet(0, 0, 0)),
	m_isDead(false),
	m_deathTimer(0.0f),
	m_pDirectionIndicator(nullptr),
	m_isLockingOn(false),
	m_lockedOnEnemy(nullptr),
	m_isTargetAvailable(false),
	m_isAimingAtEnemy(false),
	m_ignoreGuardInput(false),
	m_maxShieldDurability(0.0f),
	m_shieldRegenRate(0.0f),
	m_pAnimManager(nullptr)
{
	// 弾薬切れ画像の読み込み
	m_noAmmoImageHandle = LoadGraph("data/image/NoAmmo.png");
	assert(m_noAmmoImageHandle != -1);

	// 体力低下画像の読み込み
	m_noHealthImageHandle = LoadGraph("data/image/NoHealthUI.png");
	assert(m_noHealthImageHandle != -1);

	// アサルトライフルUI画像の読み込み
	m_arImageHandle = LoadGraph("data/image/ARUI.png");
	assert(m_arImageHandle != -1);
	m_lowAmmoARImageHandle = LoadGraph("data/image/LowAmmoARUI.png");
	assert(m_lowAmmoARImageHandle != -1);
	m_noAmmoARImageHandle = LoadGraph("data/image/NoAmmoARUI.png");
	assert(m_noAmmoARImageHandle != -1);

	// ショットガンUI画像の読み込み
	m_sgImageHandle = LoadGraph("data/image/SGUI.png");
	assert(m_sgImageHandle != -1);
	m_lowAmmoSGImageHandle = LoadGraph("data/image/LowAmmoSGUI.png");
	assert(m_lowAmmoSGImageHandle != -1);
	m_noAmmoSGImageHandle = LoadGraph("data/image/NoAmmoSGUI.png");
	assert(m_noAmmoSGImageHandle != -1);

	// HPUI画像の読み込み
	m_healthUiImageHandle = LoadGraph("data/image/HealthUI.png");
	assert(m_healthUiImageHandle != -1);

	// 盾UI画像の読み込み
	m_shieldImageHandle = LoadGraph("data/image/ShieldUI.png");
	assert(m_shieldImageHandle != -1);

	// ロックオンUI画像の読み込み
	m_lockOnUIHandle = LoadGraph("data/image/LockOnUI.png");
	assert(m_lockOnUIHandle != -1);

	// SEの読み込み
	m_playerHitSEHandle = LoadSoundMem("data/sound/SE/PlayerHit.mp3");
	assert(m_playerHitSEHandle != -1);
	m_tackleSEHandle = LoadSoundMem("data/sound/SE/Tackle.mp3");
	assert(m_tackleSEHandle != -1);
	m_recoverySEHandle = LoadSoundMem("data/sound/SE/RecoveryItem.mp3");
	assert(m_recoverySEHandle != -1);

    // フォントの作成
    m_fontHandle = CreateFontToHandle(kDefaultFontName, kAmmoFont, kDefaultFontThickness, kDefaultFontType);
    assert(m_fontHandle != -1);
    m_hpFontHandle = CreateFontToHandle(kDefaultFontName, kHpFont, kDefaultFontThickness, kDefaultFontType);
    assert(m_hpFontHandle != -1);
	m_warningFontHandle = CreateFontToHandle(kWarningFontName, kWarningFont, kDefaultFontThickness, kDefaultFontType);
	assert(m_warningFontHandle != -1);
}

Player::~Player()
{
	// 画像の解放
	DeleteGraph(m_noAmmoImageHandle);
	DeleteGraph(m_noHealthImageHandle);
	DeleteGraph(m_arImageHandle);
	DeleteGraph(m_lowAmmoARImageHandle);
	DeleteGraph(m_noAmmoARImageHandle);
	DeleteGraph(m_sgImageHandle);
	DeleteGraph(m_lowAmmoSGImageHandle);
	DeleteGraph(m_noAmmoSGImageHandle);
	DeleteGraph(m_healthUiImageHandle);
	DeleteGraph(m_shieldImageHandle);
	DeleteGraph(m_lockOnUIHandle);

	// SEの解放
	DeleteSoundMem(m_playerHitSEHandle);
	DeleteSoundMem(m_tackleSEHandle);
	DeleteSoundMem(m_recoverySEHandle);
	DeleteSoundMem(m_ammoItemSEHandle);

    // フォントの解放
    DeleteFontToHandle(m_fontHandle);
    DeleteFontToHandle(m_hpFontHandle);
	DeleteFontToHandle(m_warningFontHandle);
}

void Player::Init()
{
	// CSVからPlayerのTransform情報を取得
	auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
	for (const auto& data : dataList) 
	{
		if (data.name == "Player") 
		{
			m_pos				   = data.pos;
			m_modelPos			   = data.pos;
			m_scale				   = data.scale;
			m_health			   = data.hp;
			m_maxHealth			   = data.hp;
			m_moveSpeed		       = data.speed;
			m_tackleCooldownMax    = data.tackleCooldown;
			m_tackleSpeed		   = data.tackleSpeed;
			m_tackleDamage		   = data.tackleDamage;
			m_runSpeed			   = data.runSpeed;
			m_arInitAmmo           = data.arInitAmmo;
			m_sgInitAmmo           = data.sgInitAmmo;
			m_arMaxAmmo            = data.arInitAmmo;
			m_sgMaxAmmo            = data.sgInitAmmo;
			m_bulletPower		   = data.bulletPower;
			m_sgBulletPower		   = data.sgBulletPower;
			m_maxShieldDurability  = data.maxShieldDurability; 
			m_shieldRegenRate	   = data.shieldRegenRate;
			
			// 武器モデルのスケールと回転を設定
			m_weaponManager.SetWeaponScale(data.scale);
			m_weaponManager.SetWeaponRotation(data.rot);
			
			// コンポーネントの初期化
			m_weaponManager.Init(m_arInitAmmo, m_sgInitAmmo, m_arMaxAmmo, m_sgMaxAmmo, m_bulletPower, m_sgBulletPower);
			m_movement.Init(m_modelPos, m_moveSpeed, m_runSpeed, m_scale.x);
			m_shieldSystem.Init(m_maxShieldDurability, m_shieldRegenRate);
			break;
		}
	}
	m_pCamera->Init(); // カメラの初期化
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
    unsigned char keyState[256];
    GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	// コンポーネントの更新
	float deltaTime = kDeltaTime;
	VECTOR playerPos = m_movement.GetPos();
	bool isGuarding = m_shieldSystem.IsGuarding();
	bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();

	// タックル中もコライダーを更新する必要があるため、常にUpdateを呼ぶ
	// ただし、タックル中は移動処理はスキップされる
	m_movement.Update(deltaTime, m_pCamera.get(), m_isDead, m_isTackling);
	
	// タックル中でない場合は位置を同期
	if (!m_isTackling)
	{
		m_modelPos = m_movement.GetPos(); // 位置を同期
	}
	
	m_weaponManager.Update(deltaTime, m_modelPos, m_pCamera.get(), isGuarding, m_isDead, m_isTackling, m_isLockingOn, isSwitchingWeapon, m_allowedAttackType, m_isInfiniteAmmo);

	// 武器切り替え（ガード中は不可）
	if (!isGuarding)
	{
		if (keyState[KEY_INPUT_1] && !m_prevKeyState[KEY_INPUT_1])
		{
			m_weaponManager.SwitchWeapon(WeaponType::AssaultRifle);
		}
		else if (keyState[KEY_INPUT_2] && !m_prevKeyState[KEY_INPUT_2])
		{
			m_weaponManager.SwitchWeapon(WeaponType::Shotgun);
		}

		// マウスホイールで武器切り替え
		int wheelRot = InputManager::GetInstance()->GetMouseWheelRotVol();
		if (wheelRot != 0)
		{
			WeaponType currentWeapon = m_weaponManager.GetCurrentWeaponType();
			WeaponType nextWeapon = (currentWeapon == WeaponType::AssaultRifle) ? WeaponType::Shotgun : WeaponType::AssaultRifle;
			m_weaponManager.SwitchWeapon(nextWeapon);
		}
	}

    // プレイヤーの位置をカメラに設定
    m_pCamera->SetPlayerPos(m_modelPos);

	// Swayの計算
	float yawDelta = m_pCamera->GetYawDelta();

	// 盾システムの更新
	m_shieldSystem.Update(deltaTime, m_pCamera.get(), m_modelPos, isGuarding, m_isTackling, isSwitchingWeapon, m_weaponManager.GetWeaponSwitchTimer(), m_weaponManager.GetWeaponSwitchDuration(), yawDelta);

	// 銃のSwayの計算（一時的に保持）
	m_gunSwayOffset.x -= yawDelta * kGunSwayAmount;
	m_gunSwayOffset.x *= kGunSwayDamping;
	m_gunSwayRotOffset.y -= yawDelta * kGunSwayAmount * 0.5f;
	m_gunSwayRotOffset.y *= kGunSwayDamping;

	// 待機時の揺れ
	m_idleSwayTimer += deltaTime;
	bool isMoving = m_movement.IsMoving();
	if (!isMoving)
	{
		// サイン波とコサイン波を使って、ゆっくりとした円運動のような揺れを生成
		VECTOR idleSway = VGet(
			sinf(m_idleSwayTimer * kIdleSwaySpeed * 2.0f) * kIdleSwayAmount,
			cosf(m_idleSwayTimer * kIdleSwaySpeed) * kIdleSwayAmount,
			0.0f
		);

		// 既存のSwayに加算
		m_gunSwayOffset = VAdd(m_gunSwayOffset, idleSway);
	}

    if (m_pEffect)
    {
        m_pEffect->Update(); // エフェクトの更新
    }

	// ショットガンアニメーション更新
	m_weaponManager.UpdateSGAnimation(m_pAnimManager, deltaTime);

	// カメラの更新
	m_pCamera->Update();

	// タックルクールタイム減少
	if (m_tackleCooldown > 0)
	{
		m_tackleCooldown--;

		// クールタイムが0になった瞬間に全敵のタックルヒットフラグをリセット
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

	// マウスの左クリックで射撃（タックル中、ガード中は射撃不可、死亡中も射撃不可）
	if (!m_isDead && (m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::Shoot) && !m_isTackling && !isGuarding && !m_isLockingOn && !isSwitchingWeapon && InputManager::GetInstance()->IsPressMouseLeft() && (m_weaponManager.GetCurrentAmmo() > 0 || m_isInfiniteAmmo) && m_weaponManager.CanShoot())
	{
		m_weaponManager.Shoot(m_bullets, m_modelPos, m_pCamera.get(), m_pEffect, m_pAnimManager, m_shellCasings);
		m_weaponManager.ConsumeAmmo();
	}

	// 地面にいるかどうかの判定
	bool isOnGround = (m_modelPos.y <= PlayerMovement::GetGroundY() + PlayerMovement::GetGroundCheckTolerance());

	// 右クリック長押しでガード＆ロックオン
	if (!InputManager::GetInstance()->IsPressMouseRight())
	{
		m_ignoreGuardInput = false;
	}
	// ロックオン可能な敵がいるかどうかの判定
	m_isTargetAvailable = false;
	{
		constexpr float kLockOnAngleCos = 0.966f; // cos(15度)
		constexpr float kLockOnMaxScreenOffsetY = 100.0f; // 画面中央からの垂直方向の最大オフセット

		VECTOR camPos = m_pCamera->GetPos();
		VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));

		for (EnemyBase* enemy : enemyList)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			VECTOR enemyPos = enemy->GetPos();
			enemyPos.y += 70.0f; // 敵の胴体あたりをターゲットにするためのオフセット
			VECTOR toEnemyDir = VNorm(VSub(enemyPos, camPos));

			// プレイヤーの前方一定角度内にいるか
			if (VDot(camDir, toEnemyDir) > kLockOnAngleCos)
			{
				VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

				// 画面内にいるか、かつ垂直方向の範囲内か
				if (screenPos.z > 0)
				{
					float dx = screenPos.x - (Game::kScreenWidth / 2.0f);
					float dy = screenPos.y - (Game::kScreenHeigth / 2.0f);

					// 垂直方向の範囲チェック
					if (fabs(dy) < kLockOnMaxScreenOffsetY)
					{
						m_isTargetAvailable = true;
						break; // 1体でも見つかればOK
					}
				}
			}
		}
	}

	// 敵に照準が合っているかどうかの判定
	m_isAimingAtEnemy = false;
	VECTOR camPos = m_pCamera->GetPos();
	VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));
	VECTOR rayEnd = VAdd(camPos, VScale(camDir, 5000.0f)); 

	for (const auto& enemy : enemyList)
	{
		if (!enemy || !enemy->IsAlive())
		{
			continue;
		}

		VECTOR hitPos;
		float hitDistSq;
		EnemyBase::HitPart part = enemy->CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);

		if (part == EnemyBase::HitPart::Body || part == EnemyBase::HitPart::Head)
		{
			m_isAimingAtEnemy = true;
			break;
		}
	}

	// 右クリック長押しでガード
	bool shouldGuard = !m_isDead && !m_isTackling && InputManager::GetInstance()->IsPressMouseRight() && !m_ignoreGuardInput && !m_shieldSystem.IsShieldBroken();
	m_shieldSystem.SetGuarding(shouldGuard);
	isGuarding = m_shieldSystem.IsGuarding();

	// タックルクールダウン中でない場合のみロックオンを許可
	if (shouldGuard && m_tackleCooldown <= 0)
	{
		m_isLockingOn = true;
		m_lockedOnEnemy = nullptr;

		constexpr float kLockOnAngleCos = 0.966f;
		constexpr float kLockOnMaxScreenOffsetY = 100.0f;
		float minScreenDistSq = -1.0f;

		VECTOR camPos = m_pCamera->GetPos();
		VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));

		for (EnemyBase* enemy : enemyList)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			VECTOR enemyPos = enemy->GetPos();
			enemyPos.y += 70.0f;
			VECTOR toEnemyDir = VNorm(VSub(enemyPos, camPos));

			if (VDot(camDir, toEnemyDir) > kLockOnAngleCos)
			{
				VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

				if (screenPos.z > 0)
				{
					float dx = screenPos.x - (Game::kScreenWidth / 2.0f);
					float dy = screenPos.y - (Game::kScreenHeigth / 2.0f);

					if (fabs(dy) < kLockOnMaxScreenOffsetY)
					{
						float distSq = dx * dx + dy * dy;

						if (minScreenDistSq < 0 || distSq < minScreenDistSq)
						{
							minScreenDistSq = distSq;
							m_lockedOnEnemy = enemy;
						}
					}
				}
			}
		}
	}
	else
	{
		m_isLockingOn = false;
		m_lockedOnEnemy = nullptr;
	}

	// ガードエフェクトの更新
	m_shieldSystem.UpdateGuardEffect(m_pEffect, m_pCamera.get(), m_modelPos, isSwitchingWeapon);
	m_shieldSystem.UpdateSparkEffect(m_pEffect, m_modelPos, m_pCamera.get());	   

    // ロックオン中に左クリックでタックル	
	if (m_isLockingOn && m_lockedOnEnemy && InputManager::GetInstance()->IsTriggerMouseLeft() && m_tackleCooldown <= 0)
	{
		m_isTackling = true;

		PlaySoundMem(m_tackleSEHandle, DX_PLAYTYPE_BACK); // タックルSE再生
		m_tackleFrame = kTackleDuration;
		m_tackleCooldown = m_tackleCooldownMax; // クールタイム開始
		m_tackleId++; // タックルごとにIDを更新

		// ロックオンした敵の方向をタックル方向とする
		m_tackleDir = VNorm(VSub(m_lockedOnEnemy->GetPos(), m_modelPos));

		// タックル開始時にFOVを広げ、カメラを後ろに引く
		if (m_pCamera)
		{
			m_pCamera->SetTargetFOV(kTackleFov * DX_PI_F / 180.0f);
			VECTOR offset = m_pCamera->GetOffset();
			offset.z = kTackleCameraZOffset;
			m_pCamera->SetOffset(offset);

			// 集中線エフェクトを再生
			if (m_pEffect)
			{
				m_concentrationLineEffectHandle = m_pEffect->PlayConcentrationLine(0.0f, 0.0f, 0.0f);
			}
		}
		m_isLockingOn = false; // タックル開始したらロックオン解除
		m_lockedOnEnemy = nullptr;
	}

	// タックル中の処理
	if (m_isTackling)
	{
		m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, m_tackleSpeed));
		
		// m_movementの位置も同期
		m_movement.SetPos(m_modelPos);

		// 地面より下に行かないように制限
		if (m_modelPos.y < PlayerMovement::GetGroundY())
		{
			m_modelPos.y = PlayerMovement::GetGroundY();
			m_movement.SetPos(m_modelPos);
		}

		// タックル判定情報を作成
		TackleInfo tackleInfo = GetTackleInfo();

		// 各敵にタックル情報を渡してUpdate
		for (EnemyBase* enemy : enemyList)
		{
			// 敵がnullptrの場合はスキップ
			if (!enemy) continue;

			// 敵の更新処理
			enemy->Update(m_bullets, tackleInfo, *this, enemyList, m_pEffect);
		}

#ifdef _DEBUG
		// タックル判定カプセルのデバッグ描画
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
		// タックル終了判定
		if (m_tackleFrame <= 0)
		{
			m_isTackling = false;
			m_ignoreGuardInput = true; // ガード入力を無視

			// タックル終了時にFOVとカメラオフセットを元に戻す
			if (m_pCamera)
			{
				m_pCamera->ResetFOV();
				m_pCamera->ResetOffset();
			}

			// 集中線エフェクトを停止
			if (m_concentrationLineEffectHandle != -1)
			{
				StopEffekseer3DEffect(m_concentrationLineEffectHandle);
				m_concentrationLineEffectHandle = -1;
			}
		}
		// タックル中は他の移動・ジャンプを無効化

		// 集中線エフェクトをカメラに追従させる
		if (m_concentrationLineEffectHandle != -1)
		{
			VECTOR camPos = m_pCamera->GetPos();
			VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));
			VECTOR effectPos = VAdd(camPos, VScale(camDir, kConcentrationLineEffectZOffset)); // カメラの少し前に出す
			SetPosPlayingEffekseer3DEffect(m_concentrationLineEffectHandle, effectPos.x, effectPos.y, effectPos.z);

			// エフェクトをカメラの向きに合わせる
			float pitch = -m_pCamera->GetPitch();
			float yaw = m_pCamera->GetYaw();
			SetRotationPlayingEffekseer3DEffect(m_concentrationLineEffectHandle, pitch, yaw, 0.0f);
		}

		return;
	}

	// 各敵に更新処理を行うためのタックル情報を作成
	TackleInfo tackleInfo{}; 
	for (EnemyBase* enemy : enemyList)
	{
		if (!enemy) continue;
		enemy->Update(m_bullets, tackleInfo, *this, enemyList, m_pEffect);
	}
	
	// 弾の更新
	Bullet::UpdateBullets(m_bullets, m_modelPos);

	// 移動処理はm_movement.Update()で処理済み
	// Head Bobbing状態をカメラに設定
	if (m_pCamera)
	{
		m_pCamera->SetHeadBobbingState(m_movement.IsMoving(), m_movement.IsWasRunning());
	}

	if (m_isDead)
	{
		DeathUpdate();
		return;
	}

	if (m_health <= 0.0f)
	{
		m_isDead = true;
		m_deathTimer = 0.0f;
	}

	std::copy(std::begin(keyState), std::end(keyState), std::begin(m_prevKeyState));

	// HPバーアニメーション
	if (m_healthBarAnim != m_health)
	{
		if (m_healthBarAnim > m_health)
		{
			// ダメージ: アニメーション値を減少させる
			m_healthBarAnim -= kHpBarAnimSpeed;
			if (m_healthBarAnim < m_health)
			{
				m_healthBarAnim = m_health;
			}
		}
		else
		{
			// 回復: アニメーション値を増加させる
			m_healthBarAnim += kHpBarAnimSpeed;
			if (m_healthBarAnim > m_health)
			{
				m_healthBarAnim = m_health;
			}
		}
	}

	// 体力低下の警告表示処理
	if (m_health <= kLowHealthThreshold)
	{
	    m_isLowHealth = true;
		m_lowHealthBlinkTimer += kDeltaTime; // タイマー更新
	}
	else
	{
	    m_isLowHealth = false;
	    m_lowHealthBlinkTimer = 0.0f;
	}
	// エフェクトの更新
	// ダメージエフェクト
	if (m_damageEffect.timer > 0)
	{
	    m_damageEffect.timer -= 1.0f;
	    m_damageEffect.alpha -= 1.0f / m_damageEffect.duration;
	    if (m_damageEffect.alpha < 0) m_damageEffect.alpha = 0;
	}
	else if (m_isLowHealth)
	{
		float alpha = (sinf(m_lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
		m_damageEffect.alpha = alpha * kLowHealthEffectMaxAlpha;
		m_damageEffect.colorR = 255;
		m_damageEffect.colorG = 0;
		m_damageEffect.colorB = 0;
	}
    else
	{
	    m_damageEffect.alpha = 0.0f;
	}
		
	// 回復エフェクト
	if (m_healEffect.timer > 0)
	{
	    m_healEffect.timer -= 1.0f;
	    m_healEffect.alpha -= 1.0f / m_healEffect.duration;
		if (m_healEffect.alpha < 0) m_healEffect.alpha = 0;
	}
		
	// 弾薬エフェクト
	if (m_ammoEffect.timer > 0)
	{
	    m_ammoEffect.timer -= 1.0f;
		m_ammoEffect.alpha -= 1.0f / m_ammoEffect.duration;
		if (m_ammoEffect.alpha < 0) m_ammoEffect.alpha = 0;
	}

	// 残弾数テキストのフラッシュタイマー更新
    if (m_ammoTextFlashTimer > 0.0f)
    {
        m_ammoTextFlashTimer -= 1.0f;
    }

	ShellCasing::UpdateShellCasings(m_shellCasings);
}

void Player::Draw3D()
{
	// 武器描画処理をコンポーネントに委譲
	bool isTryingToGuard = !m_isDead && !m_isTackling && InputManager::GetInstance()->IsPressMouseRight() && !m_ignoreGuardInput && !m_shieldSystem.IsShieldBroken();
	bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();
	m_weaponManager.Draw3D(m_modelPos, m_pCamera.get(), m_gunSwayOffset, m_weaponManager.GetGunShakeOffset(), m_gunSwayRotOffset, m_shieldSystem.GetGuardAnimTimer(), m_shieldSystem.GetGuardAnimDuration(), isSwitchingWeapon, m_weaponManager.GetWeaponSwitchTimer(), m_weaponManager.GetWeaponSwitchDuration(), m_weaponManager.GetPreviousWeaponType(), isTryingToGuard);

	// 弾と薬莢の描画
	Bullet::DrawBullets(m_bullets);
	ShellCasing::DrawShellCasings(m_shellCasings);
}

void Player::DrawShield()
{
	m_shieldSystem.Draw(m_pCamera.get(), m_modelPos, m_isTackling, m_weaponManager.IsSwitchingWeapon(), m_weaponManager.GetWeaponSwitchTimer(), m_weaponManager.GetWeaponSwitchDuration());
}

void Player::DrawUI()
{
	// ガード中にターゲットがいない場合にテキストを表示
	if (m_shieldSystem.IsGuarding() && !m_lockedOnEnemy && !m_isTargetAvailable)
	{
		const char* text = "ターゲットなし";
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, NULL);
		int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
		int textX = (screenW - textWidth) / 2;
		int textY = screenH / 2 + 30; // レティクルの少し下に表示
		DrawStringToHandle(textX, textY, text, kColorWhite, m_warningFontHandle);
	}

	// ロックオンUIの描画
	if (m_lockedOnEnemy)
	{
		constexpr float kLockOnUISize = 64.0f;
		constexpr float kLockOnUIYOffset = 90.0f; // UIを足元から上に移動させるためのオフセット

		VECTOR enemyPos = m_lockedOnEnemy->GetPos();
		enemyPos.y += kLockOnUIYOffset; // Y座標を調整して体の中心に近づける
		VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

		if (screenPos.z > 0) // 画面内にあるか
		{
			float halfSize = kLockOnUISize / 2.0f;
			DrawExtendGraph(
				screenPos.x - halfSize, screenPos.y - halfSize,
				screenPos.x + halfSize, screenPos.y + halfSize,
				m_lockOnUIHandle, true
			);
		}
	}

	if (!m_isDead)
	{
		int screenW = Game::kScreenWidth;
		int screenH = Game::kScreenHeigth;
		GetScreenState(&screenW, &screenH, NULL);

		// HPバーのY座標を計算
		const int barY = screenH - kHpBarHeight - kHpBarMargin;

		// タックルUIのY座標をHPバーに合わせる
		const int tackleUIY = barY;

		// アサルトライフルUIをタックルUIの下に配置
		int gunImageY = tackleUIY - kARImageHeight - kARImageMarginY;
		int gunImageX = screenW - kARImageWidth - kARImageMarginX;

		// 銃UI画像の描画
		int gunHandle = -1;
		int gunImageWidth = 0;
		int gunImageHeight = 0;
		int gunImageMarginX = 0;
		int gunImageMarginY = 0;

		WeaponType currentWeaponType = m_weaponManager.GetCurrentWeaponType();
		bool isLowAmmo = m_weaponManager.IsLowAmmo();
		bool isInfiniteAmmo = m_weaponManager.IsInfiniteAmmo();
		int currentAmmo = m_weaponManager.GetCurrentAmmo();
		
		switch (currentWeaponType)
		{
		case WeaponType::AssaultRifle:
			gunImageWidth = kARImageWidth;
			gunImageHeight = kARImageHeight;
			gunImageMarginX = kARImageMarginX;
			gunImageMarginY = kARImageMarginY;
			if (currentAmmo == 0 && !isInfiniteAmmo)
			{
				gunHandle = m_noAmmoARImageHandle;
			}
			else if (isLowAmmo)
			{
				gunHandle = m_lowAmmoARImageHandle;
			}
			else
			{
				gunHandle = m_arImageHandle;
			}
			break;
		case WeaponType::Shotgun:
			gunImageWidth = kSGImageWidth;
			gunImageHeight = kSGImageHeight;
			gunImageMarginX = kSGImageMarginX;
			gunImageMarginY = kSGImageMarginY;
			if (currentAmmo == 0 && !isInfiniteAmmo)
			{
				gunHandle = m_noAmmoSGImageHandle;
			}
			else if (isLowAmmo)
			{
				gunHandle = m_lowAmmoSGImageHandle;
			}
			else
			{
				gunHandle = m_sgImageHandle;
			}
			break;
		default:
			break;
		}

		gunImageY = tackleUIY - gunImageHeight - gunImageMarginY;
		gunImageX = screenW - gunImageWidth - gunImageMarginX;

		DrawExtendGraph(gunImageX, gunImageY, gunImageX + gunImageWidth, gunImageY + gunImageHeight, gunHandle, true);

		// 残弾数の表示
		int ammoTextWidth = GetDrawStringWidthToHandle(kAmmoTextMaxWidthStr, strlen(kAmmoTextMaxWidthStr), m_fontHandle);
		
		// 弾薬数UIの位置をAR基準で固定計算
		int arGunImageX = screenW - kARImageWidth - kARImageMarginX;
		int arGunImageY = tackleUIY - kARImageHeight - kARImageMarginY;
		int ammoTextX = arGunImageX - kAmmoTextGunOffsetX - ammoTextWidth;
		int ammoTextY = arGunImageY + (kARImageHeight - kAmmoTextHeight) * 0.5f + kAmmoTextGunOffsetY;

		// 弾薬無限モードの場合は「∞」を表示
		if (isInfiniteAmmo)
		{
			DrawFormatStringToHandle(ammoTextX, ammoTextY, kColorWhite, m_fontHandle, "∞");
		}
		else
		{
			// デフォルトの色を決定
			int textColor = isLowAmmo ? kColorLowAmmo : kColorWhite;

			// フラッシュタイマーが作動中なら色を補間
			if (m_ammoTextFlashTimer > 0.0f)
			{
				float flashProgress = m_ammoTextFlashTimer / 60.0f;

				// ターゲットの色（デフォルト色）のRGB成分
				int targetR = (textColor >> 16) & 0xFF;
				int targetG = (textColor >> 8) & 0xFF;
				int targetB = textColor & 0xFF;

				// フラッシュの色（黄色）のRGB成分
				int flashR = 255;
				int flashG = 255;
				int flashB = 0;

				// 線形補間
				int currentR = static_cast<int>(flashR * flashProgress + targetR * (1.0f - flashProgress));
				int currentG = static_cast<int>(flashG * flashProgress + targetG * (1.0f - flashProgress));
				int currentB = static_cast<int>(flashB * flashProgress + targetB * (1.0f - flashProgress));

				textColor = GetColor(currentR, currentG, currentB);
			}

			DrawFormatStringToHandle(ammoTextX, ammoTextY, textColor, m_fontHandle, "%d", GetCurrentAmmo());
		}

		// 盾耐久値の描画
		float shieldBarAnim = m_shieldSystem.GetBarAnim();
		float maxShieldDurability = m_shieldSystem.GetMaxDurability();
		float shieldDurabilityRate = shieldBarAnim / maxShieldDurability;
		if (shieldDurabilityRate < 0.0f) shieldDurabilityRate = 0.0f;
		if (shieldDurabilityRate > 1.0f) shieldDurabilityRate = 1.0f;

		// 盾のテクスチャサイズを取得
		int shieldTexW, shieldTexH;
		GetGraphSize(m_shieldImageHandle, &shieldTexW, &shieldTexH);

		// 盾ゲージのサイズと位置
		const int shieldGaugeHeight = 150; // 縦向きのゲージの高さ
		const int shieldGaugeWidth = (int)((float)shieldGaugeHeight * shieldTexW / shieldTexH); // 縦向きのゲージの幅
		float scale = (float)shieldGaugeHeight / shieldTexH;

		int shieldGaugeX = screenW - shieldGaugeWidth - kHpBarMargin;
		int shieldGaugeY = kShieldUIYPosition + kShieldUIYOffset;

		// ゲージの背景（半透明の盾）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawRotaGraph3F(
			shieldGaugeX + shieldGaugeWidth * 0.5f,
			shieldGaugeY + shieldGaugeHeight * 0.5f,
			shieldTexW * 0.5f,
			shieldTexH * 0.5f,
			scale,
			scale,
			0.0f,
			m_shieldImageHandle,
			true
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// ゲージ本体
		if (shieldDurabilityRate > 0.0f)
		{
			int filledWidth = (int)(shieldGaugeWidth * shieldDurabilityRate);
			// 描画範囲を設定してクリッピング
			SetDrawArea(shieldGaugeX, shieldGaugeY, shieldGaugeX + filledWidth, shieldGaugeY + shieldGaugeHeight);

			// 盾を満タン状態で描画
			DrawRotaGraph3F(
				shieldGaugeX + shieldGaugeWidth * 0.5f,
				shieldGaugeY + shieldGaugeHeight * 0.5f,
				shieldTexW * 0.5f,
				shieldTexH * 0.5f,
				scale,
				scale,
				0.0f,
				m_shieldImageHandle,
				true
			);

			// 描画範囲をリセット
			SetDrawArea(0, 0, screenW, screenH);
		}

		// 警告表示ロジック
		// 体力低下と弾薬低下の警告を分離して処理
		// 体力低下の警告
		if (m_isLowHealth)
		{
			float alpha = (sinf(m_lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
			int alphaInt = static_cast<int>(alpha * 255);
			int drawX = (screenW - kWarningImageSize) * 0.5f;
			int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

			// 弾薬警告も表示する必要がある場合は、体力警告を左にずらす
			bool isLowAmmoForHealth = m_weaponManager.IsLowAmmo();
			bool isNoAmmoWarningForHealth = m_weaponManager.IsNoAmmoWarning();
			bool isSwitchingWeaponForHealth = m_weaponManager.IsSwitchingWeapon();
			bool prevWeaponHadLowAmmoForHealth = m_weaponManager.GetPrevWeaponHadLowAmmo();
			bool prevWeaponHadNoAmmoForHealth = m_weaponManager.GetPrevWeaponHadNoAmmo();
			if (isLowAmmoForHealth || isNoAmmoWarningForHealth || (isSwitchingWeaponForHealth && (prevWeaponHadLowAmmoForHealth || prevWeaponHadNoAmmoForHealth)))
			{
				drawX = (screenW * 0.5f) - kWarningImageSize - (kWarningImageSpacing * 0.5f);
			}

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
			DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize, drawY + kWarningImageSize, m_noHealthImageHandle, true);

			const char* text = "体力低下";
			int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
			int textX = drawX + (kWarningImageSize - textWidth) / 2;
			int textY = drawY + kWarningImageSize + kWarningTextYOffset;
			unsigned int textColor = (alphaInt << 24) | kColorWhite;
			DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// 弾薬低下の警告
		bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();
		bool prevWeaponHadLowAmmo = m_weaponManager.GetPrevWeaponHadLowAmmo();
		bool prevWeaponHadNoAmmo = m_weaponManager.GetPrevWeaponHadNoAmmo();
		float weaponSwitchTimer = m_weaponManager.GetWeaponSwitchTimer();
		float weaponSwitchDuration = m_weaponManager.GetWeaponSwitchDuration();
		
		bool isNoAmmoWarning = m_weaponManager.IsNoAmmoWarning();
		bool currentNeedsWarning = isLowAmmo || isNoAmmoWarning;
		bool prevNeedsWarning = prevWeaponHadLowAmmo || prevWeaponHadNoAmmo;
		
		bool shouldDraw = false;
		float fadeAlpha = 1.0f;

		if (isSwitchingWeapon)
		{
			float halfDuration = weaponSwitchDuration / 2.0f;
			if (weaponSwitchTimer < halfDuration)
			{
				// フェードアウト
				if (prevNeedsWarning)
				{
					shouldDraw = true;
					fadeAlpha = 1.0f - (weaponSwitchTimer / halfDuration);
				}
			}
			else
			{
				// フェードイン
				if (currentNeedsWarning)
				{
					shouldDraw = true;
					fadeAlpha = (weaponSwitchTimer - halfDuration) / halfDuration;
				}
			}
		}
		else if (currentNeedsWarning)
		{
			shouldDraw = true;
		}

		if (shouldDraw)
		{
			bool isFadingOut = isSwitchingWeapon && (weaponSwitchTimer < weaponSwitchDuration / 2.0f);
			bool isNoAmmo = isFadingOut ? prevWeaponHadNoAmmo : isNoAmmoWarning;
			const char* text = isNoAmmo ? "残弾なし" : "残弾僅か";

			float lowAmmoBlinkTimer = m_weaponManager.GetLowAmmoBlinkTimer();
			float blinkAlpha = (sinf(lowAmmoBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
			int alphaInt = static_cast<int>(blinkAlpha * fadeAlpha * 255);

			int drawX = (screenW - kWarningImageSize) * 0.5f;
			int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

			// 体力警告も表示する必要がある場合は、弾薬警告を右にずらす
			if (m_isLowHealth)
			{
				drawX = (screenW * 0.5f) + (kWarningImageSpacing * 0.5f);
			}

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
			DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize, drawY + kWarningImageSize, m_noAmmoImageHandle, true);

			int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
			int textX = drawX + (kWarningImageSize - textWidth) / 2;
			int textY = drawY + kWarningImageSize + kWarningTextYOffset;
			unsigned int textColor = (alphaInt << 24) | kColorWhite;
			DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		if (m_pEffect)
		{
			m_pEffect->Draw(); // エフェクトの描画
		}

		// HPバーのパラメータ
		const int healthUiImageX = kHpBarMargin;
		const int healthUiImageY = screenH - kHpBarHeight - kHpBarMargin + (kHpBarHeight - kHealthUiImageSize) * 0.5f;
		DrawExtendGraph(healthUiImageX, healthUiImageY, healthUiImageX + kHealthUiImageSize, healthUiImageY + kHealthUiImageSize, m_healthUiImageHandle, true);
		const int barX = healthUiImageX + kHealthUiImageSize + kHealthUiImageBarSpacing;

		// 最大HP
		float hp = m_health;

		if (hp < 0) hp = 0;
		if (hp > kMaxHp) hp = kMaxHp;

		float hpAnim = m_healthBarAnim;

		if (hpAnim < 0) hpAnim = 0;
		if (hpAnim > kMaxHp) hpAnim = kMaxHp;

		// HP割合
		float hpRate = hp / kMaxHp;
		float hpAnimRate = hpAnim / kMaxHp;

		// 背景
		DrawBox(barX, barY, barX + kHpBarWidth, barY + kHpBarHeight, kColorHpBarBg, true);

		// HPバー本体（実際の体力を反映）
		DrawBox(barX, barY, barX + static_cast<int>(kHpBarWidth * hpRate), barY + kHpBarHeight, kColorHpBarFill, true);

		// アニメーションバー（ゴーストバー）
		if (m_healthBarAnim > m_health)
		{
			// ダメージ時（黄色いバー）
			int animStart = barX + static_cast<int>(kHpBarWidth * hpRate);
			int animEnd = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
			DrawBox(animStart, barY, animEnd, barY + kHpBarHeight, kColorHpBarDamage, true);
		}
		else if (m_healthBarAnim < m_health)
		{
			// 回復時（明るい緑のバー）
			int animStart = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
			int animEnd = barX + static_cast<int>(kHpBarWidth * hpRate);
			DrawBox(animStart, barY, animEnd, barY + kHpBarHeight, 0x90EE90, true);
		}

		// 枠
		DrawBox(barX, barY, barX + kHpBarWidth, barY + kHpBarHeight, kColorHpBarBorder, false);

		// HP数値
		DrawFormatStringToHandle(barX + kHpTextOffsetX, barY + kHpTextOffsetY, kColorWhite, m_hpFontHandle, "%.0f", m_healthBarAnim);
	}
	// ダメージエフェクト描画
	DrawEffectFeedback(m_damageEffect);

	// 回復エフェクト描画
	DrawEffectFeedback(m_healEffect);

	// 弾薬エフェクト描画
	DrawEffectFeedback(m_ammoEffect);
}

void Player::DrawEffectFeedback(Player::EffectFeedback& effect)
{
    if (effect.alpha > 0.0f)
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW * 0.5f;
        int centerY = screenH * 0.5f;
        float maxDistance = sqrtf((float)(screenW * screenW + screenH * screenH)) * 0.5f;
        float edgeWidth = maxDistance * 0.4f;
        const int stepSize = 8;
        for (int y = 0; y < screenH; y += stepSize)
        {
            for (int x = 0; x < screenW; x += stepSize)
            {
                float distanceFromCenter = sqrtf((float)((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)));
                float distanceFromEdge = maxDistance - distanceFromCenter;
                float edgeIntensity = 0.0f;
                if (distanceFromEdge < edgeWidth)
                {
                    edgeIntensity = 1.0f - (distanceFromEdge / edgeWidth);
                }
                int alpha = static_cast<int>(effect.alpha * 180 * edgeIntensity);
                if (alpha > 0)
                {
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                    DrawBox(x, y, x + stepSize, y + stepSize, GetColor(effect.colorR, effect.colorG, effect.colorB), true);
                }
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Player::DeathUpdate()
{
    m_healthBarAnim = 0.0f; // HPバーを即座に0にする

	if (m_pCamera)
	{
		m_pCamera->PlayDeathAnimation(m_deathTimer);
	}

	m_deathTimer += 1.0f / 60.0f;
}

// ダメージを受ける処理
void Player::TakeDamage(float damage, const VECTOR& attackerPos)
{
	if (m_isDead)
	{
		return;
	}

	if (m_isInvincible)
	{
		return;
	}

	// 方向インジケーターに攻撃者の位置を通知
	if (m_pDirectionIndicator && (attackerPos.x != 0.0f || attackerPos.z != 0.0f))
	{
		m_pDirectionIndicator->ShowAttackedEnemyDirection(Vec3(attackerPos));
	}

	if (m_shieldSystem.IsGuarding() && !m_shieldSystem.IsShieldBroken()) // ガード中で盾が壊れていなければ
	{
		// カメラシェイクを発生
		if (m_pCamera)
		{
			m_pCamera->Shake(kTakeDamageShakePower, kTakeDamageShakeDuration);
		}

		// 盾の前方にスパークエフェクトを再生
		if (m_pEffect)
		{
			VECTOR forward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
			VECTOR effectPos = VAdd(m_modelPos, VScale(forward, 80.0f));
			// スパークエフェクトは盾システムで管理されるため、ここでは再生のみ
		}

		float remainingDamage = m_shieldSystem.TakeDamage(damage, m_pEffect, m_pCamera.get(), m_modelPos);
		if (remainingDamage > 0)
		{
			// 銃を揺らす
			m_weaponManager.ShakeGun(kShieldBreakGunShakePower, kShieldBreakGunShakeDuration);
			m_health -= remainingDamage; // 残ったダメージをHPに適用

			// HPバーアニメーション用タイマーをリセット
			m_healthBarAnimTimer = 0.0f;
			// ダメージエフェクトを発動
			m_damageEffect.Trigger(kDamageEffectDuration, kDamageEffectColorR, kDamageEffectColorG, kDamageEffectColorB);
			// 被弾SEを再生
			PlaySoundMem(m_playerHitSEHandle, DX_PLAYTYPE_BACK);

			// カメラシェイクを発生
			if (m_pCamera)
			{
				m_pCamera->Shake(kTakeDamageShakePower, kTakeDamageShakeDuration);
			}
		}

		return; // 盾で防いだ場合はここで処理を終了
	}

	// ガードしていない、または盾が壊れている場合は直接ダメージを受ける
	// 方向インジケーターに攻撃者の位置を通知
	if (m_pDirectionIndicator && (attackerPos.x != 0.0f || attackerPos.z != 0.0f))
	{
		m_pDirectionIndicator->ShowAttackedEnemyDirection(Vec3(attackerPos));
	}

	m_health -= damage; // ダメージを適用
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
	// HPバーアニメーション用タイマーをリセット
	m_healthBarAnimTimer = 0.0f;
	// ダメージエフェクトを発動
	m_damageEffect.Trigger(kDamageEffectDuration, kDamageEffectColorR, kDamageEffectColorG, kDamageEffectColorB);
	// 被弾SEを再生
	PlaySoundMem(m_playerHitSEHandle, DX_PLAYTYPE_BACK);

	// カメラシェイクを発生
	if (m_pCamera)
	{
		m_pCamera->Shake(kTakeDamageShakePower, kTakeDamageShakeDuration);
	}
}

// 弾の取得
std::vector<Bullet>& Player::GetBullets()
{
	return m_bullets;
}

// プレイヤーがショット可能かどうか
bool Player::HasShot()
{
	bool shot = m_hasShot;
	m_hasShot = false; // 状態をリセット
	return shot; // 撃ったかどうかを返す
}

void Player::Shoot(std::vector<Bullet>& bullets)
{
	m_weaponManager.Shoot(bullets, m_modelPos, m_pCamera.get(), m_pEffect, m_pAnimManager, m_shellCasings);
}

// 銃の位置を取得
VECTOR Player::GetGunPos() const
{
	return m_weaponManager.GetGunPos(m_modelPos, m_pCamera.get());
}

// 銃の向きを取得
VECTOR Player::GetGunRot() const
{
	return m_weaponManager.GetGunRot(m_pCamera.get());
}

// 薬莢の排出位置を取得
VECTOR Player::GetEjectionPortPos() const
{
	return m_weaponManager.GetEjectionPortPos();
}

std::shared_ptr<CapsuleCollider> Player::GetBodyCollider() const
{
	return m_movement.GetBodyCollider();
}

// タックル情報を取得
Player::TackleInfo Player::GetTackleInfo() const
{
	TackleInfo info;
	info.isTackling = m_isTackling;
	if (m_isTackling)
	{
		float tackleHeight = kTackleHitHeight;
		info.tackleId = m_tackleId; // タックルIDをセット

		// プレイヤーの体の中心位置
		VECTOR bodyCenter = m_modelPos;
		constexpr float kAROffsetY = 20.0f; // ARオフセットY
		bodyCenter.y += kAROffsetY;

		// プレイヤーの前面中心（体の中心から前方へkTackleHitRangeだけ進める）
		VECTOR frontCenter = VAdd(bodyCenter, VScale(m_tackleDir, kTackleHitRange));

		// カプセルの中心軸を前面中心の上下に伸ばす
		VECTOR capA = VAdd(frontCenter, VGet(0, -tackleHeight * 0.5f, 0));
		VECTOR capB = VAdd(frontCenter, VGet(0, tackleHeight * 0.5f, 0));

		info.capA = capA;
		info.capB = capB;
		info.radius = kTackleHitRadius;
		info.damage = m_tackleDamage;
	}
	return info;
}

// カプセル情報を取得
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
	// m_movementのコライダーから直接取得
	auto collider = m_movement.GetBodyCollider();
	capA   = collider->GetSegmentA();
	capB   = collider->GetSegmentB();
	radius = collider->GetRadius();
}

void Player::AddHp(float value)
{
    m_health += value; // 体力を加算
    if (m_health > m_maxHealth)
    {
        m_health = m_maxHealth; // 最大体力を超えないように制限
    }
    if (m_health < 0.0f)
    {
        m_health = 0.0f; // 体力が負にならないように制限
    }
    // 回復時にエフェクトを発動
    m_healEffect.Trigger(kHealEffectDuration, kHealEffectColorR, kHealEffectColorG, kHealEffectColorB);
}

void Player::AddARAmmo(int value)
{
	m_weaponManager.AddARAmmo(value);
	// 弾薬取得時にエフェクトを発動
	m_ammoEffect.Trigger(kAmmoEffectDuration, kAmmoEffectColorR, kAmmoEffectColorG, kAmmoEffectColorB);
	m_ammoTextFlashTimer = 60.0f;
}

void Player::AddSGAmmo(int value)
{
	m_weaponManager.AddSGAmmo(value);
	// 弾薬取得時にエフェクトを発動
	m_ammoEffect.Trigger(kAmmoEffectDuration, kAmmoEffectColorR, kAmmoEffectColorG, kAmmoEffectColorB);
	m_ammoTextFlashTimer = 60.0f;
}

int Player::GetCurrentAmmo() const
{
	return m_weaponManager.GetCurrentAmmo();
}

int Player::GetMaxAmmo() const
{
	return m_weaponManager.GetMaxAmmo();
}

void Player::SetAttackRestrictions(AttackType allowedAttack)
{
    m_allowedAttackType = allowedAttack;
}

void Player::ShakeGun(float power, float duration)
{
	m_weaponManager.ShakeGun(power, duration);
}

bool Player::IsAimingAtEnemy() const
{
	return m_isAimingAtEnemy;
}

bool Player::IsJustGuarded() const
{
	return m_shieldSystem.IsJustGuarded();
}

void Player::PlayParryEffect(const VECTOR& pos)
{
    if (m_pEffect)
    {
        m_pEffect->PlayParryEffect(pos.x, pos.y, pos.z);
    }
}

WeaponType Player::GetCurrentWeaponType() const
{
	return m_weaponManager.GetCurrentWeaponType();
}

// 武器を切り替える
void Player::SwitchWeapon(WeaponType weaponType)
{
	m_weaponManager.SwitchWeapon(weaponType);
}