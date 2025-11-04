#include "Player.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "EffekseerForDXLib.h"
#include "Game.h" 
#include "Mouse.h"
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
#include <cmath>
#include <cassert>
#include <algorithm>
#include "PlayerConfig.h"

namespace
{
	// 銃のオフセット
	constexpr float kGunOffsetX = 80.0f;
	constexpr float kGunOffsetY = 20.0f;
	constexpr float kGunOffsetZ = 60.0f;

	// マズルフラッシュエフェクトのオフセット
	constexpr float kMuzzleFlashEffectOffsetX = -20.0f;
	constexpr float kMuzzleFlashEffectOffsetY = 30.0f;
	constexpr float kMuzzleFlashEffectOffsetZ = 80.0f;

	// 重力とジャンプ関連
	constexpr float kGravity   = 0.35f; // 重力の強さ
	constexpr float kJumpPower = 7.0f;  // ジャンプの初速
	constexpr float kGroundY   = 0.0f;  // 地面のY座標

	// タックル関連
	constexpr int   kTackleDuration  = 20;     // タックル持続フレーム数
	constexpr float kTackleHitRange  = 250.0f; // タックルの前方有効距離
	constexpr float kTackleHitRadius = 250.0f; // タックルの横幅（半径）
	constexpr float kTackleHitHeight = 100.0f; // タックルの高さ

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f; // カプセルコライダーの高さ
	constexpr float kCapsuleRadius = 50.0f;  // カプセルコライダーの半径

	// 1秒あたりの発射回数
	constexpr float kShootRate = 10.0f;

	// X,Z座標の移動範囲制限
	constexpr float kLimitMoveX = 2800.0f;
	constexpr float kLimitMoveZ = 2800.0f;

	// カメラを左右に振った際の横揺れ関連の定数
	constexpr float kShieldSwayAmount = 4.0f;  // 盾モデルの揺れの強さ
	constexpr float kShieldSwayDamping     = 0.9f;  // 盾モデルの揺れの減衰率

	// 銃の揺れ関連の定数
	constexpr float kGunSwayAmount   = 0.02f;  // 銃モデルの揺れの強さ (剣より小さく設定)
	constexpr float kGunSwayDamping  = 0.95f; // 銃モデルの揺れの減衰率 (剣より大きく設定し、より早く収束させる)

	// Update関連
	constexpr float kFrameRate						= 60.0f; 
	constexpr float kDeltaTime						= 1.0f / kFrameRate; 
	constexpr float kPlayerColliderYOffset		    = 60.0f;  
	constexpr float kTackleFov						= 100.0f; // タックル中のカメラFOV
	constexpr float kTackleCameraZOffset            = 30.0f;  // タックル中のカメラZオフセット
	constexpr float kConcentrationLineEffectScale   = 20.0f;  // 集中線エフェクトのスケール
	constexpr float kConcentrationLineEffectZOffset = 15.0f;  // 集中線エフェクトのZオフセット
	constexpr float kGroundCheckTolerance			= 0.01f;  // 地面接地判定の許容誤差
	constexpr float kJumpSwayPower					= 5.0f;   // ジャンプ時の揺れの強さ　
	constexpr float kLandingSwayPower				= 5.0f;   // 着地時の揺れの強さ
	constexpr float kHpBarAnimSpeed					= 1.5f;   // HPバーのアニメーション速度
	constexpr int   kLowAmmoThreshold				= 10;     // 弾薬が少ないと判断する閾値
	constexpr float kLowHealthThreshold				= 30.0f;  // 体力が少ないと判断する閾値
	constexpr float kWarningBlinkSpeed				= 1.5f;   // 警告UIの点滅速度
	constexpr float kLowHealthEffectMaxAlpha		= 0.7f;   // 体力低下UIの最大アルファ値
	constexpr float kIdleSwaySpeed					= 1.5f;   // 揺れの速さ
	constexpr float kIdleSwayAmount					= 0.04f;  // 揺れの量
	 
	// 盾関連
	constexpr float kShieldBaseScreenW     = 640.0f; 
	constexpr float kShieldBaseScreenH     = 480.0f;
	constexpr float kShieldCamZ            = -35.0f;
	constexpr float kShieldCamTargetFactor = 0.3f;   // 補正値
	constexpr float kShieldWaitX		   = -20.0f; 
	constexpr float kShieldWaitY		   = -45.0f; 
	constexpr float kShieldWaitZ		   = -10.0f;
	constexpr float kShieldPivotZ		   = -25.0f; // 盾の回転軸のZ位置
	constexpr float kShieldModelScale      = 2.0f;   // 盾モデルのスケール
	constexpr float kGuardAnimDuration     = 0.1f;   // ガードアニメーションの時間
    constexpr float kGuardEffectOffsetZ     = 60.0f;  // ガードエフェクトのZ軸オフセット
    constexpr float kGuardEffectOffsetX    = 10.0f;  // ガードエフェクトのX軸オフセット
	constexpr int   kGuardEffectDuration   = 60;     // ガードエフェクトの持続フレーム数

	// 盾UI関連
	constexpr int   kShieldImageWidth         = 64;
	constexpr int   kShieldImageHeight        = 96; 
	constexpr int   kShieldImageGaugeSpacing  = 10;  // 盾UIとクールダウンゲージの間隔
	constexpr int   kShieldImageActiveAlpha   = 255; // 使用可能な盾UIのアルファ値
	constexpr int   kShieldImageCooldownAlpha = 128; // クールダウン中の盾UIのアルファ値

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
	constexpr float kTakeDamageShakePower    = 5.0f; // 攻撃を受けた時の揺れの強さ
	constexpr int   kTakeDamageShakeDuration = 15;   // 攻撃を受けた時の揺れの持続時間
	constexpr float kShootShakePower		 = 6.0f; // 撃った時の揺れの強さ
	constexpr int   kShootShakeDuration      = 8;    // 撃った時の揺れの持続時間

	// 銃UI関連
	constexpr int   kGunImageWidth   = 200;
	constexpr int   kGunImageHeight  = 133;
	constexpr int   kGunImageMarginX = 20;
	constexpr int   kGunImageMarginY = 20; 

	// 弾薬UI関連
	constexpr int   kAmmoImageSize          = 48;   
	constexpr int   kAmmoTextHeight		    = 32;   
	constexpr char  kAmmoTextMaxWidthStr[]  = "999";
	constexpr int   kAmmoImageTextSpacing   = 10; 
	constexpr int   kAmmoUIYOffset		    = 105;
	constexpr int   kAmmoUIGunCenterOffsetX = 20; 

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
	m_modelHandle(-1),
	m_shieldModelHandle(-1),
	m_shieldImageHandle(-1),
	m_ammoImageHandle(-1),
	m_shootSEHandle(-1),
	m_playerHitSEHandle(-1),
	m_tackleSEHandle(-1),
	m_recoverySEHandle(-1),
	m_ammoItemSEHandle(-1),
	m_ammo(0),
	m_modelPos(VGet(0, 0, 0)),
	m_pEffect(nullptr),
	m_pCamera(std::make_shared<Camera>()),
	m_pDebugCamera(std::make_shared<Camera>()),
	m_pBodyCollider(std::make_shared<CapsuleCollider>()),
	m_isMoving(false),
	m_isWasRunning(false),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_healthBarAnim(100.0f),
	m_healthBarAnimTimer(0.0f),
	m_isJumping(false),
	m_wasJumping(false),
	m_jumpVelocity(0.0f),
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
	m_shootCooldown(0.0f),
	m_shootCooldownTimer(0.0f),
	m_shootRate(kShootRate),
	m_isInvincible(false),
	m_isInfiniteAmmo(false),
	m_tackleCooldownMax(0.0f),
	m_tackleSpeed(0.0f),
	m_tackleDamage(0.0f),
	m_isShieldAnimating(false),
	m_shieldAnimTimer(0.0f),
	m_shieldAnimDuration(0.1f),
	m_concentrationLineEffectHandle(-1),
	m_noAmmoImageHandle(-1),
	m_gunImageHandle(-1),
	m_lowAmmoGunImageHandle(-1),
	m_noAmmoGunImageHandle(-1),
	m_isLowAmmo(false),
	m_lowAmmoBlinkTimer(0.0f),
	m_showLowAmmoWarning(false),
    m_isNoAmmoWarning(false),
	m_isLowHealth(false),
	m_lowHealthBlinkTimer(0.0f),
	m_ammoTextFlashTimer(0.0f),
	m_idleSwayTimer(0.0f),
	m_gunSwayOffset(VGet(0, 0, 0)),
	m_gunSwayRotOffset(VGet(0, 0, 0)),
	m_shieldSwayOffset(VGet(0, 0, 0)),
	m_shieldSwayRotOffset(VGet(0, 0, 0)),
	m_isDead(false),
	m_deathTimer(0.0f),
	m_pDirectionIndicator(nullptr),
	m_isLockingOn(false),
	m_lockedOnEnemy(nullptr),
	m_isGuarding(false),
	m_wasGuarding(false),
	m_guardAnimTimer(0.0f),
	m_guardAnimDuration(kGuardAnimDuration),
	m_guardEffectHandle(-1),
	m_guardEffectTimer(0),
	m_ignoreGuardInput(false),
	m_shieldDurability(PlayerConfig::MAX_SHIELD_DURABILITY),
	m_maxShieldDurability(PlayerConfig::MAX_SHIELD_DURABILITY),
	m_isShieldBroken(false)
{
	// プレイヤーモデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/AR_M.mv1");
	assert(m_modelHandle != -1);

	// 薬莢排出口フレームのインデックスを検索
	m_ejectionPortFrame = MV1SearchFrame(m_modelHandle, "AR_M_Ejection_Port");

	// 盾モデルの読み込み
	m_shieldModelHandle = MV1LoadModel("data/model/Shield.mv1");
	assert(m_shieldModelHandle != -1);

	// 弾画像の読み込み
	m_ammoImageHandle = LoadGraph("data/image/ammo.png");
	assert(m_ammoImageHandle != -1);

	// 弾薬切れ画像の読み込み
	m_noAmmoImageHandle = LoadGraph("data/image/NoAmmo.png");
	assert(m_noAmmoImageHandle != -1);

	// 体力低下画像の読み込み
	m_noHealthImageHandle = LoadGraph("data/image/NoHealthUI.png");
	assert(m_noHealthImageHandle != -1);

	// 銃UI画像の読み込み
	m_gunImageHandle = LoadGraph("data/image/Gun.png");
	assert(m_gunImageHandle != -1);
	m_lowAmmoGunImageHandle = LoadGraph("data/image/LowAmmoGun.png");
	assert(m_lowAmmoGunImageHandle != -1);
	m_noAmmoGunImageHandle = LoadGraph("data/image/NoAmmoGun.png");
	assert(m_noAmmoGunImageHandle != -1);

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
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);
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
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_shieldModelHandle);

	// 画像の解放
	DeleteGraph(m_ammoImageHandle);
	DeleteGraph(m_noAmmoImageHandle);
	DeleteGraph(m_noHealthImageHandle);
	DeleteGraph(m_gunImageHandle);
	DeleteGraph(m_lowAmmoGunImageHandle);
	DeleteGraph(m_noAmmoGunImageHandle);
	DeleteGraph(m_healthUiImageHandle);
	DeleteGraph(m_shieldImageHandle);
	DeleteGraph(m_lockOnUIHandle);

	// SEの解放
	DeleteSoundMem(m_shootSEHandle);
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
			m_pos				= data.pos;
			m_modelPos			= data.pos;
			m_scale				= data.scale;
			m_health			= data.hp;
			m_maxHealth			= data.hp;
			m_moveSpeed		    = data.speed;
			m_tackleCooldownMax = data.tackleCooldown;
			m_tackleSpeed		= data.tackleSpeed;
			m_tackleDamage		= data.tackleDamage;
			m_runSpeed			= data.runSpeed;
			m_initialAmmo		= data.initialAmmo;
			m_bulletPower		= data.bulletPower;
			m_shieldDurability = PlayerConfig::MAX_SHIELD_DURABILITY; // 最大耐久値に設定
			m_maxShieldDurability = PlayerConfig::MAX_SHIELD_DURABILITY; // 最大耐久値を設定
			m_isShieldBroken = false; // 盾は壊れていない
			MV1SetScale(m_modelHandle, data.scale);
			MV1SetRotationXYZ(m_modelHandle, data.rot);
			break;
		}
	}

	if (!m_pEffect) 
	{
		m_pEffect = std::make_shared<Effect>();
		m_pEffect->Init();
	}
	m_pCamera->Init(); // カメラの初期化

	m_shootCooldown = 1.0f / m_shootRate; // 発射クールタイムを設定

	// CSVの初期弾薬数を反映
	m_ammo = m_initialAmmo;
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
    unsigned char keyState[256];
    GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

    // クールタイムタイマー減算
    if (m_shootCooldownTimer > 0.0f) 
    {
        m_shootCooldownTimer -= kDeltaTime;
        if (m_shootCooldownTimer < 0.0f) m_shootCooldownTimer = 0.0f;
    }

    // プレイヤーの位置をカメラに設定
    m_pCamera->SetPlayerPos(m_modelPos);

    // 盾のアニメーションタイマー更新
    if (m_isShieldAnimating)
    {
        m_shieldAnimTimer += kDeltaTime;
        if (m_shieldAnimTimer >= m_shieldAnimDuration)
        {
            m_isShieldAnimating = false;
            m_shieldAnimTimer = 0.0f;
        }
    }

    // Swayの計算
    float yawDelta = m_pCamera->GetYawDelta();

    m_shieldSwayOffset.x -= yawDelta * kShieldSwayAmount;
    m_shieldSwayOffset.x *= kShieldSwayDamping;

    // 銃のSwayの計算
    m_gunSwayOffset.x -= yawDelta * kGunSwayAmount;
    m_gunSwayOffset.x *= kGunSwayDamping;
    m_gunSwayRotOffset.y -= yawDelta * kGunSwayAmount * 0.5f; // 回転の揺れも追加
    m_gunSwayRotOffset.y *= kGunSwayDamping;

    // 待機時の揺れ
    m_idleSwayTimer += kDeltaTime;
    if (!m_isMoving)
    {
        // サイン波とコサイン波を使って、ゆっくりとした円運動のような揺れを生成
        VECTOR idleSway = VGet(
            sinf(m_idleSwayTimer * kIdleSwaySpeed * 2.0f) * kIdleSwayAmount,
            cosf(m_idleSwayTimer * kIdleSwaySpeed) * kIdleSwayAmount,
            0.0f
        );

        // 既存のSwayに加算
        m_gunSwayOffset = VAdd(m_gunSwayOffset, idleSway);
        m_shieldSwayOffset = VAdd(m_shieldSwayOffset, idleSway);
    }

    if (m_pEffect)
    {
        m_pEffect->Update(); // エフェクトの更新
    }

	// プレイヤーのカプセルコライダーを毎フレーム更新
	VECTOR center = m_modelPos;
	center.y += kPlayerColliderYOffset; // 足元から腰～胸あたりを中心に
	VECTOR capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	VECTOR capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	m_pBodyCollider->SetSegment(capA, capB);
	m_pBodyCollider->SetRadius(kCapsuleRadius);

	// モデルの位置と回転を更新
	VECTOR modelOffset	  = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
	MATRIX rotYaw		  = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch		  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot		  = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// ガードアニメーションの進行度を計算（イージング付き）
	float guardAnimProgress = m_guardAnimTimer / m_guardAnimDuration;
	float gunOffsetY = -200.0f * (1.0f - cosf(guardAnimProgress * DX_PI_F * 0.5f)); // イージング
	modelPos.y += gunOffsetY;

	// モデルの位置を設定
	MV1SetPosition(m_modelHandle, VAdd(modelPos, m_gunSwayOffset));
	
	// モデルの回転を設定
	MV1SetRotationXYZ(m_modelHandle, VAdd(VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F , 0.0f), m_gunSwayRotOffset));

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
	if (!m_isDead && (m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::Shoot) && !m_isTackling && !m_isGuarding && !m_isLockingOn && Mouse::IsPressLeft() && (m_ammo > 0 || m_isInfiniteAmmo) && m_shootCooldownTimer <= 0.0f)
	{
		Shoot(m_bullets); // 弾を発射

		// 弾薬無限モードでない場合のみ弾薬を減らす
		if (!m_isInfiniteAmmo)
		{
			m_ammo--; // 弾薬を減らす
		}

		m_shootCooldownTimer = m_shootCooldown; // クールタイムリセット
	}

	// 地面にいるかどうかの判定
	bool isOnGround = (m_modelPos.y <= kGroundY + kGroundCheckTolerance);

	// 右クリック長押しでガード＆ロックオン
	if (!Mouse::IsPressRight())
	{
		m_ignoreGuardInput = false;
	}
	if (!m_isDead && !m_isTackling && Mouse::IsPressRight() && !m_ignoreGuardInput)
	{
		m_isGuarding = true;
		m_isLockingOn = true;
		m_lockedOnEnemy = nullptr; // ロックオンターゲット検索前にリセット

		constexpr float kLockOnAngleCos = 0.966f; // cos(15度)
		float minScreenDistSq = -1.0f;

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

				// 画面内にいるか
				if (screenPos.z > 0)
				{
					float dx = screenPos.x - (Game::kScreenWidth / 2.0f);
					float dy = screenPos.y - (Game::kScreenHeigth / 2.0f);
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
	else
	{
		m_isGuarding = false;
		m_isLockingOn = false;
		m_lockedOnEnemy = nullptr;
		// ガード解除時に盾の耐久値を回復させる
		m_shieldDurability += PlayerConfig::SHIELD_REGEN_RATE * kDeltaTime;
		if (m_shieldDurability > m_maxShieldDurability)
		{
			m_shieldDurability = m_maxShieldDurability;
			m_isShieldBroken = false; // 完全回復したら壊れていない状態に
		}
	}

	// ガード開始時にエフェクトを再生
	if (m_isGuarding && !m_wasGuarding)
	{
		if (m_pEffect)
		{
			float pitch = -m_pCamera->GetPitch();
			float yaw = m_pCamera->GetYaw();
			VECTOR forward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
			VECTOR right = VGet(sinf(yaw + DX_PI_F * 0.5f), 0, cosf(yaw + DX_PI_F * 0.5f));
			VECTOR effectPos = VAdd(m_modelPos, VAdd(VScale(forward, kGuardEffectOffsetZ), VScale(right, kGuardEffectOffsetX)));
			m_guardEffectHandle = m_pEffect->PlayGuardEffect(effectPos.x, effectPos.y, effectPos.z, pitch, yaw, 0.0f);
			m_guardEffectTimer = kGuardEffectDuration; 
		}
	}
	// ガード終了時（解除された場合）
	else if (!m_isGuarding && m_wasGuarding)
	{
		if (m_guardEffectHandle != -1)
		{
			StopEffekseer3DEffect(m_guardEffectHandle);
			m_guardEffectHandle = -1;
		}
		m_guardEffectTimer = 0; // タイマーをリセット
	}
	    
	// ガードエフェクトのタイマー処理と追従
	if (m_guardEffectTimer > 0)
	{
		m_guardEffectTimer--; // タイマーをデクリメント
	  
		if (m_guardEffectHandle != -1)
		{
			float yaw = m_pCamera->GetYaw();
			VECTOR forward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
			VECTOR right = VGet(sinf(yaw + DX_PI_F * 0.5f), 0, cosf(yaw + DX_PI_F * 0.5f));
	     	VECTOR effectPos = VAdd(m_modelPos, VAdd(VScale(forward, kGuardEffectOffsetZ), VScale(right, kGuardEffectOffsetX))); 
	        SetPosPlayingEffekseer3DEffect(m_guardEffectHandle, effectPos.x, effectPos.y, effectPos.z);
	    
	    	float pitch = -m_pCamera->GetPitch();
	    	SetRotationPlayingEffekseer3DEffect(m_guardEffectHandle, pitch, yaw, 0.0f);
	   	}
	    
	   	// タイマーが切れたらエフェクトを停止
	    if (m_guardEffectTimer <= 0)
	    {
	    	if (m_guardEffectHandle != -1)
	    	{
	    		StopEffekseer3DEffect(m_guardEffectHandle);
	    		m_guardEffectHandle = -1;
	    	}
	    }
	}
	    
    // ロックオン中に左クリックでタックル	
	if (m_isLockingOn && m_lockedOnEnemy && Mouse::IsTriggerLeft() && m_tackleCooldown <= 0)
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
				m_concentrationLineEffectHandle = m_pEffect->PlayConcentrationLine(0.0f, 0.0f, 0.0f, kConcentrationLineEffectScale);
			}
		}
		m_isLockingOn = false; // タックル開始したらロックオン解除
		m_lockedOnEnemy = nullptr;
	}

	// ガードアニメーションタイマーの更新
	if (m_isGuarding)
	{
		m_guardAnimTimer += kDeltaTime;
		if (m_guardAnimTimer > m_guardAnimDuration)
		{
			m_guardAnimTimer = m_guardAnimDuration;
		}
	}
	else
	{
		m_guardAnimTimer -= kDeltaTime;
		if (m_guardAnimTimer < 0.0f)
		{
			m_guardAnimTimer = 0.0f;
		}
	}

	// タックル中の処理
	if (m_isTackling)
	{
		m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, m_tackleSpeed));

		// 地面より下に行かないように制限
		if (m_modelPos.y < kGroundY)
		{
			m_modelPos.y = kGroundY;
		}

		// タックル判定情報を作成
		TackleInfo tackleInfo = GetTackleInfo();

		// 各敵にタックル情報を渡してUpdate
		for (EnemyBase* enemy : enemyList)
		{
			// 敵がnullptrの場合はスキップ
			if (!enemy) continue;

			// 敵の更新処理
			enemy->Update(m_bullets, tackleInfo, *this, enemyList, m_pEffect.get());
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
		enemy->Update(m_bullets, tackleInfo, *this, enemyList, m_pEffect.get());
	}
	
	// 弾の更新
	Bullet::UpdateBullets(m_bullets, m_modelPos);

	// 走るキー入力
	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
	bool isRunning = wantRun; // 走っているかどうかのフラグ

	float moveSpeed = isRunning ? m_runSpeed : m_moveSpeed; // 移動速度の設定
	bool isMoving = false; // 移動中かどうかのフラグ

	// 移動方向の初期化
	VECTOR moveDir = VGet(0, 0, 0);

	// キー入力による移動方向の設定
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

	// スペースキーを押した瞬間のみジャンプ（死亡中はジャンプ不可）
	if (!m_isDead && keyState[KEY_INPUT_SPACE] && !m_prevKeyState[KEY_INPUT_SPACE] && isOnGround && !m_isJumping && !m_isTackling)
	{
		m_jumpVelocity = kJumpPower;
		m_isJumping = true;
		m_pCamera->ApplyJumpSway(kJumpSwayPower);
	}

	// ジャンプ中または空中なら重力適用
	if (m_isJumping || !isOnGround)
	{
		m_modelPos.y += m_jumpVelocity; // ジャンプの速度を適用
		m_jumpVelocity -= kGravity;     // 重力を適用

		// 着地判定
		if (m_modelPos.y <= kGroundY)
		{
			m_modelPos.y = kGroundY; // 地面に着地
			m_jumpVelocity = 0.0f;   // ジャンプ速度をリセット
			m_isJumping = false;     // ジャンプ状態を解除

			// 着地した瞬間の処理
			if (m_wasJumping)
			{
				m_pCamera->ApplyLandingSway(kLandingSwayPower);
			}
		}
	}

	// 移動方向がある場合（死亡中は移動不可）
	if (!m_isDead && (moveDir.x != 0.0f || moveDir.z != 0.0f))
	{
		// 移動方向の長さを計算
		float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		moveDir.x /= len;
		moveDir.z /= len;
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
		isMoving = true;
	}

	// X,Z座標の移動範囲制限
	m_modelPos.x = std::clamp(m_modelPos.x, -kLimitMoveX, kLimitMoveX);
	m_modelPos.z = std::clamp(m_modelPos.z, -kLimitMoveZ, kLimitMoveZ);

	m_isMoving = isMoving;      // 移動中の状態を更新
	m_isWasRunning = isRunning; // 走っている状態を更新
	m_wasJumping = m_isJumping; // ジャンプ状態を更新

	// Head Bobbing状態をカメラに設定
	if (m_pCamera)
	{
		m_pCamera->SetHeadBobbingState(isMoving, isRunning);
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

	m_wasGuarding = m_isGuarding;

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

	// 弾薬低下の警告表示処理
	if (m_ammo == 0 && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer += kDeltaTime; // タイマー更新
		m_isNoAmmoWarning = true;
	}
	else if (m_ammo <= kLowAmmoThreshold && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = true;
		m_lowAmmoBlinkTimer += kDeltaTime; // タイマー更新
		m_isNoAmmoWarning = false;
	}
	else
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer = 0.0f;
		m_isNoAmmoWarning = false;
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

void Player::Draw()
{
	// プレイヤーモデルの描画
	MV1DrawModel(m_modelHandle); 

	// ガード中にターゲットがいない場合にテキストを表示
	if (m_isGuarding && !m_lockedOnEnemy)
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

	// 弾の描画
	Bullet::DrawBullets(m_bullets);

	ShellCasing::DrawShellCasings(m_shellCasings);

	if (!m_isDead)
	{
		int screenW = Game::kScreenWidth;
		int screenH = Game::kScreenHeigth;
		GetScreenState(&screenW, &screenH, NULL);

		// HPバーのY座標を計算
		const int barY = screenH - kHpBarHeight - kHpBarMargin;
		
		// タックルUIのY座標をHPバーに合わせる
		const int tackleUIY = barY;
		
		// 銃UIをタックルUIの上に配置
		int gunImageY = tackleUIY - kGunImageHeight - kGunImageMarginY;
		int gunImageX = screenW - kGunImageWidth - kGunImageMarginX;
		
		// 銃UI画像の描画
		int gunHandle = m_gunImageHandle;
		if (m_ammo == 0 && !m_isInfiniteAmmo)
		{
			gunHandle = m_noAmmoGunImageHandle;
		}
		else if (m_isLowAmmo)
		{
			gunHandle = m_lowAmmoGunImageHandle;
		}
		DrawExtendGraph(gunImageX, gunImageY, gunImageX + kGunImageWidth, gunImageY + kGunImageHeight, gunHandle, true);
		
		// 残弾数の表示
		// 弾薬UI全体の幅を計算
		int ammoTextWidth = GetDrawStringWidthToHandle(kAmmoTextMaxWidthStr, strlen(kAmmoTextMaxWidthStr), m_fontHandle);
		int ammoUIWidth = kAmmoImageSize + kAmmoImageTextSpacing + ammoTextWidth;
		
		// 弾薬UIのX座標 
		int ammoUIX = gunImageX + (kGunImageWidth * 0.5f) - (ammoUIWidth * 0.5f) + kAmmoUIGunCenterOffsetX;
		// 弾薬UIのY座標 
		int ammoUIY = gunImageY + kGunImageHeight - kAmmoImageSize - kAmmoUIYOffset;
		
		// ammo画像の描画
		int ammoImageX = ammoUIX;
		int ammoImageY = ammoUIY;
		DrawExtendGraph(ammoImageX, ammoImageY, ammoImageX + kAmmoImageSize, ammoImageY + kAmmoImageSize, m_ammoImageHandle, true);
		
		// 弾薬数のテキスト描画
		int ammoTextX = ammoImageX + kAmmoImageSize + kAmmoImageTextSpacing;
		int ammoTextY = ammoUIY + (kAmmoImageSize - kAmmoTextHeight) * 0.5f;
		
		// 弾薬無限モードの場合は「∞」を表示
		if (m_isInfiniteAmmo)
		{
			DrawFormatStringToHandle(ammoTextX, ammoTextY, kColorWhite, m_fontHandle, "∞");
		}
		else
		{
			// デフォルトの色を決定
			int textColor = m_isLowAmmo ? kColorLowAmmo : kColorWhite;
		
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
		
			DrawFormatStringToHandle(ammoTextX, ammoTextY, textColor, m_fontHandle, "%d", m_ammo);
		}
		
		// 盾耐久値の描画
		float shieldDurabilityRate = m_shieldDurability / m_maxShieldDurability;
		if (shieldDurabilityRate < 0.0f) shieldDurabilityRate = 0.0f;
		if (shieldDurabilityRate > 1.0f) shieldDurabilityRate = 1.0f;

		// 盾のテクスチャサイズを取得
		int shieldTexW, shieldTexH;
		GetGraphSize(m_shieldImageHandle, &shieldTexW, &shieldTexH);

		// 盾ゲージのサイズと位置
		// 幅を固定し、アスペクト比を維持するように高さを計算
		const int shieldGaugeWidth = 200;
		const int shieldGaugeHeight = (int)((float)shieldGaugeWidth * shieldTexW / shieldTexH);
		float scale = (float)shieldGaugeWidth / shieldTexH;

		int shieldGaugeX = screenW - shieldGaugeWidth - kHpBarMargin;
		int shieldGaugeY = tackleUIY - (shieldGaugeHeight - kHpBarHeight) * 0.5f; 

		// ゲージの背景（半透明の盾）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawRotaGraph3F(
			shieldGaugeX + shieldGaugeWidth * 0.5f, 
			shieldGaugeY + shieldGaugeHeight * 0.5f, 
			shieldTexW * 0.5f,
			shieldTexH * 0.5f,
			scale,
			scale,
			-DX_PI_F * 0.5f, 
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
				-DX_PI_F * 0.5f,
				m_shieldImageHandle,
				true
			);

			// 描画範囲をリセット
			SetDrawArea(0, 0, screenW, screenH);
		}

		// 警告表示ロジック
		if (m_isLowHealth && (m_isLowAmmo || m_isNoAmmoWarning))
		{
			float alpha = (sinf(m_lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
			int alphaInt = static_cast<int>(alpha * 255);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);

			const char* text = "体力低下";
			int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
			int textX = (screenW - textWidth) * 0.5f;
			int textY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset + kWarningImageSize + kWarningTextYOffset;
			unsigned int textColor = (alphaInt << 24) | kColorWhite;
			DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);

			int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

			int leftDrawX = (screenW * 0.5f) - kWarningImageSize - (kWarningImageSpacing * 0.5f);
			DrawExtendGraph(leftDrawX, drawY, leftDrawX + kWarningImageSize, drawY + kWarningImageSize, m_noHealthImageHandle, true);

			int rightDrawX = (screenW * 0.5f) + (kWarningImageSpacing * 0.5f);
			DrawExtendGraph(rightDrawX, drawY, rightDrawX + kWarningImageSize, drawY + kWarningImageSize, m_noAmmoImageHandle, true);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (m_isLowHealth)
		{
			// フェードイン・アウトのアルファ値を計算
			float alpha = (sinf(m_lowHealthBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
			int alphaInt = static_cast<int>(alpha * 255);

			// 画像の描画サイズと位置
			int drawX = (screenW - kWarningImageSize) * 0.5f;
			int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

			// 画像を描画
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
			DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize, drawY + kWarningImageSize, m_noHealthImageHandle, true);

			// テキストを描画
			const char* text = "体力低下";
			int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
			int textX = (screenW - textWidth) * 0.5f;
			int textY = drawY + kWarningImageSize + kWarningTextYOffset;
			unsigned int textColor = (alphaInt << 24) | kColorWhite;
			DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else if (m_isLowAmmo || m_isNoAmmoWarning)
		{
			// フェードイン・アウトのアルファ値を計算
			float alpha = (sinf(m_lowAmmoBlinkTimer * 2.0f * DX_PI_F / kWarningBlinkSpeed) + 1.0f) * 0.5f;
			int alphaInt = static_cast<int>(alpha * 255);

			// 画像の描画サイズと位置
			int drawX = (screenW - kWarningImageSize) * 0.5f;
			int drawY = (screenH - kWarningImageSize) * 0.5f + kWarningImageYOffset;

			// 画像を描画
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
			DrawExtendGraph(drawX, drawY, drawX + kWarningImageSize, drawY + kWarningImageSize, m_noAmmoImageHandle, true);

			// テキストを描画
			const char* text = (m_isNoAmmoWarning) ? "残弾なし" : "残弾僅か";
			int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
			int textX = (screenW - textWidth) * 0.5f;
			int textY = drawY + kWarningImageSize + kWarningTextYOffset;
			unsigned int textColor = (alphaInt << 24) | kColorWhite;
			DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		/*盾の描画*/
		// 画面サイズに応じてスケーリング
		float scaleW = screenW / kShieldBaseScreenW;
		float scaleH = screenH / kShieldBaseScreenH;
		float scaleAvg = (scaleW + scaleH) * 0.5f;

		// カメラオフセット設定
		VECTOR totalCameraOffset = VGet(0, 0, 0);
		if (m_pCamera)
		{
			VECTOR shakeOffset       = m_pCamera->GetShakeOffset();
			VECTOR headBobOffset     = m_pCamera->GetHeadBobOffset();
			VECTOR landingSwayOffset = m_pCamera->GetLandingSwayOffset();
			VECTOR jumpSwayOffset    = m_pCamera->GetJumpSwayOffset();
			totalCameraOffset    = VAdd(shakeOffset, headBobOffset);
			totalCameraOffset    = VAdd(totalCameraOffset, landingSwayOffset);
			totalCameraOffset    = VAdd(totalCameraOffset, jumpSwayOffset);
		}
		VECTOR shieldCamPos = VGet(0, 0, kShieldCamZ * scaleAvg);
		shieldCamPos.x += totalCameraOffset.x;
		shieldCamPos.y += totalCameraOffset.y;
		VECTOR shieldCamTarget = VGet(totalCameraOffset.x * kShieldCamTargetFactor, totalCameraOffset.y * kShieldCamTargetFactor, 0);
		SetCameraPositionAndTarget_UpVecY(shieldCamPos, shieldCamTarget);

		// ガードアニメーションの進行度を計算
		float guardAnimProgress = m_guardAnimTimer / m_guardAnimDuration;
		float easeProgress = 1.0f - cosf(guardAnimProgress * DX_PI_F * 0.5f); // イージング

		// 待機位置とガード位置を定義
		VECTOR waitPos = VAdd(VGet(kShieldWaitX * scaleW, kShieldWaitY * scaleH, kShieldWaitZ), m_shieldSwayOffset);
		VECTOR guardPos = VGet(0.0f, kShieldWaitY * scaleH, -15.0f); // 中央の位置

		// 進行度に応じて位置を補間
		VECTOR currentPos = VAdd(waitPos, VScale(VSub(guardPos, waitPos), easeProgress));

		// 待機回転とガード回転を定義
		constexpr float kShieldWaitAngleY = -0.3f; // 待機時のY軸回転角度
		VECTOR waitRot = VGet(0.0f, DX_PI_F + kShieldWaitAngleY, 0.0f);
		VECTOR guardRot = VGet(0.0f, DX_PI_F, 0.0f);

		// 進行度に応じて回転を補間
		VECTOR currentRot = VAdd(waitRot, VScale(VSub(guardRot, waitRot), easeProgress));

		// タックル中の盾アニメーション
		if (m_isTackling)
		{
			constexpr float kTackleShieldThrust = 20.0f; // タックル時の盾を突き出す量
			currentPos = guardPos; // ガード位置（中央）を基準にする
			currentPos.z += kTackleShieldThrust;
			currentRot = guardRot; // 回転もガード状態（正面）にする
		}

		// ガード中は小刻みに揺らす
		if (m_isGuarding)
		{
			constexpr float kGuardShakeAmount = 0.4f; // 揺れの量
			currentPos.x += ((float)rand() / RAND_MAX - 0.5f) * kGuardShakeAmount;
			currentPos.y += ((float)rand() / RAND_MAX - 0.5f) * kGuardShakeAmount;
		}

		// モデルの位置と回転を直接設定
		MV1SetPosition(m_shieldModelHandle, currentPos);
		MV1SetRotationXYZ(m_shieldModelHandle, currentRot);
		MV1SetScale(m_shieldModelHandle, VGet(kShieldModelScale * scaleAvg, kShieldModelScale * scaleAvg, kShieldModelScale * scaleAvg));
		MV1DrawModel(m_shieldModelHandle);

		// メインカメラに戻す
		m_pCamera->SetCameraToDxLib();

		if (m_pEffect)
		{
			m_pEffect->Draw(); // エフェクトの描画
		}
		
		// HPバーのパラメータ
		const int healthUiImageX = kHpBarMargin;
		const int healthUiImageY = screenH - kHpBarHeight - kHpBarMargin + (kHpBarHeight - kHealthUiImageSize) * 0.5f;
		DrawExtendGraph(healthUiImageX, healthUiImageY, healthUiImageX + kHealthUiImageSize, healthUiImageY + kHealthUiImageSize, m_healthUiImageHandle, true);
	    const int barX      = healthUiImageX + kHealthUiImageSize + kHealthUiImageBarSpacing;

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
	        int animEnd   = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
	        DrawBox(animStart, barY, animEnd, barY + kHpBarHeight, kColorHpBarDamage, true);
	    }
	    else if (m_healthBarAnim < m_health)
	    {
	        // 回復時（明るい緑のバー）
	        int animStart = barX + static_cast<int>(kHpBarWidth * hpAnimRate);
	        int animEnd   = barX + static_cast<int>(kHpBarWidth * hpRate);
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

	if (m_isGuarding && !m_isShieldBroken) // ガード中で盾が壊れていなければ
	{
		m_shieldDurability -= damage; // 盾の耐久値を減らす
		if (m_shieldDurability <= 0.0f)
		{
			m_shieldDurability = 0.0f;
			m_isShieldBroken = true; // 盾が壊れた
			// 盾が完全に壊れたら、残りのダメージをプレイヤーが受ける
			float remainingDamage = -m_shieldDurability; // 正の値に変換
			if (remainingDamage > 0) {
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

	            // 方向インジケーターに攻撃者の位置を通知
	            if (m_pDirectionIndicator && (attackerPos.x != 0.0f || attackerPos.z != 0.0f))
	            {
	            	m_pDirectionIndicator->ShowAttackedEnemyDirection(Vec3(attackerPos));
	            }
			}
		}
		return; // 盾で防いだ場合はここで処理を終了
	}

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
    // 画面中央（カメラ中心）からレティクル方向へ発射
    VECTOR cameraPos = m_pCamera->GetPos();
    VECTOR cameraForward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));

	VECTOR gunPos = GetGunPos();
	VECTOR gunDir = GetGunRot();

    // 画面中央から出ているように見せるため、カメラ前方に小さくオフセット
    VECTOR spawnPos = VAdd(cameraPos, VScale(cameraForward, 0.0f));

    // 弾丸を発射（起点: 画面中央、方向: レティクル方向）
    bullets.emplace_back(spawnPos, cameraForward, AttackType::Shoot, m_bulletPower);

	// 薬莢を生成
	VECTOR ejectionPos = GetEjectionPortPos();
	VECTOR ejectionDir = VGet(sinf(m_pCamera->GetYaw() + DX_PI_F * 0.5f), 0.5f, cosf(m_pCamera->GetYaw() + DX_PI_F * 0.5f));
	m_shellCasings.emplace_back(ejectionPos, ejectionDir);

	// SEを再生
	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);

	float rotX = -m_pCamera->GetPitch();
	float rotY = m_pCamera->GetYaw();
	float rotZ = 0.0f;

	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ); // マズルフラッシュエフェクトを発生
	}

	// カメラシェイクを発生
	if (m_pCamera)
	{
		m_pCamera->Shake(kShootShakePower, kShootShakeDuration); // 強さ・フレーム数
	}
}

// 銃の位置を取得
VECTOR Player::GetGunPos() const
{
	// モデルのオフセットと回転を計算
	VECTOR modelOffset        = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ); // モデルのオフセット
	MATRIX rotYaw             = MGetRotY(m_pCamera->GetYaw());        // カメラのヨー回転
	MATRIX rotPitch           = MGetRotX(-m_pCamera->GetPitch());	  // カメラのピッチ回転
	MATRIX modelRot           = MMult(rotPitch, rotYaw);			  // モデルの回転行列を計算
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);	  // オフセットを回転
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset); // モデルの位置とオフセットを組み合わせて銃の位置を計算

	VECTOR gunOffset = VGet(kMuzzleFlashEffectOffsetX, kMuzzleFlashEffectOffsetY, kMuzzleFlashEffectOffsetZ); // マズルフラッシュのオフセット
	VECTOR gunPos    = VTransform(gunOffset, modelRot); // 銃のオフセットを回転

	// 銃の位置を計算して返す
	return VAdd(modelPosition, gunPos);
}

// 銃の向きを取得
VECTOR Player::GetGunRot() const
{
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}

// 薬莢の排出位置を取得
VECTOR Player::GetEjectionPortPos() const
{
	if (m_ejectionPortFrame != -1)
	{
		return MV1GetFramePosition(m_modelHandle, m_ejectionPortFrame);
	}
}

std::shared_ptr<CapsuleCollider> Player::GetBodyCollider() const
{
	return m_pBodyCollider;
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
		bodyCenter.y += kGunOffsetY;

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
	// m_pBodyCollider から直接取得
	capA   = m_pBodyCollider->GetSegmentA();
	capB   = m_pBodyCollider->GetSegmentB();
	radius = m_pBodyCollider->GetRadius();
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

void Player::AddAmmo(int value)
{
    m_ammo += value;
    if (m_ammo < 0) m_ammo = 0;
    // 弾薬取得時にエフェクトを発動
    m_ammoEffect.Trigger(kAmmoEffectDuration, kAmmoEffectColorR, kAmmoEffectColorG, kAmmoEffectColorB);
    m_ammoTextFlashTimer = 60.0f; // テキストフラッシュタイマーを開始
}

void Player::SetAttackRestrictions(AttackType allowedAttack)
{
    m_allowedAttackType = allowedAttack;
}
