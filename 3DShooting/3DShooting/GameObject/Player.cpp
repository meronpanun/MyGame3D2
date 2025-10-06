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
#include <cmath>
#include <cassert>
#include <algorithm>

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

	constexpr int   kTackleGaugeWidth  = 200;  
	constexpr int   kTackleGaugeHeight = 16;   

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f; // カプセルコライダーの高さ
	constexpr float kCapsuleRadius = 50.0f;  // カプセルコライダーの半径

	// 1秒あたりの発射回数
	constexpr float kShootRate = 10.0f;

	// X,Z座標の移動範囲制限
	constexpr float kLimitMoveX = 2800.0f;
	constexpr float kLimitMoveZ = 2800.0f;

	// カメラを左右に振った際の横揺れ関連の定数
	constexpr float kGunSwayAmount   = 0.2f;  // 銃モデルの揺れの強さ
	constexpr float kSwordSwayAmount = 0.6f;  // 剣モデルの揺れの強さ
	constexpr float kSwayDamping     = 0.9f;  // 剣モデルの揺れの減衰率

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
	 
	// 剣関連
	constexpr float kSwordBaseScreenW         = 640.0f; 
	constexpr float kSwordBaseScreenH         = 480.0f;
	constexpr float kSwordCamZ                = -35.0f;
	constexpr float kSwordCamTargetFactor     = 0.3f;   // 補正値
	constexpr float kSwordWaitX				  = -20.0f; 
	constexpr float kSwordWaitY				  = -30.0f; 
	constexpr float kSwordWaitZ				  = -10.0f;
	constexpr float kSwordWaitRotY			  = 30.0f; 
	constexpr float kSwordAnimStartAngle      = 25.0f;  // 振りかぶり開始角度
	constexpr float kSwordAnimEndAngle        = 180.0f; // 振り下ろし終了角度
	constexpr float kSwordPivotZ			  = -20.0f; // 剣の回転軸のZ位置
	constexpr float kSwordModelScale          = 0.5f;   // 剣モデルのスケール
	constexpr float kDefaultSwordAnimDuration = 0.05f;  // 剣振りアニメーションのデフォルト時間

	// 剣UI関連
	constexpr int   kSwordImageWidth         = 64;
	constexpr int   kSwordImageHeight        = 96; 
	constexpr int   kSwordImageGaugeSpacing  = 10;  // 剣UIとクールダウンゲージの間隔
	constexpr int   kSwordImageActiveAlpha   = 255; // 使用可能な剣UIのアルファ値
	constexpr int   kSwordImageCooldownAlpha = 128; // クールダウン中の剣UIのアルファ値

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
	m_swordModelHandle(-1),
	m_swordImageHandle(-1),
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
	m_pEnemy(std::make_shared<EnemyNormal>()),
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
	m_isSwordAnimating(false),
	m_swordAnimTimer(0.0f),
	m_swordAnimDuration(kDefaultSwordAnimDuration),
	m_concentrationLineEffectHandle(-1),
	m_noAmmoImageHandle(-1),
	m_gunImageHandle(-1),
	m_lowAmmoGunImageHandle(-1),
	m_noAmmoGunImageHandle(-1),
	m_isLowAmmo(false),
	m_lowAmmoBlinkTimer(0.0f),
	m_showLowAmmoWarning(false),
    m_showNoAmmoWarning(false),
	m_isLowHealth(false),
	m_lowHealthBlinkTimer(0.0f),
	m_ammoTextFlashTimer(0.0f),
	m_gunSwayOffset(VGet(0, 0, 0)),
	m_gunSwayRotOffset(VGet(0, 0, 0)),
	m_swordSwayOffset(VGet(0, 0, 0)),
	m_swordSwayRotOffset(VGet(0, 0, 0))
{
	// プレイヤーモデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/AR_M.mv1");
	assert(m_modelHandle != -1);

	// 剣モデルの読み込み
	m_swordModelHandle = MV1LoadModel("data/model/Sword.mv1");
	assert(m_swordModelHandle != -1);

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

	// 剣UI画像の読み込み
	m_swordImageHandle = LoadGraph("data/image/sword.png");
	assert(m_swordImageHandle != -1);

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
	MV1DeleteModel(m_swordModelHandle);

	// 画像の解放
	DeleteGraph(m_ammoImageHandle);
	DeleteGraph(m_noAmmoImageHandle);
	DeleteGraph(m_noHealthImageHandle);
	DeleteGraph(m_gunImageHandle);
	DeleteGraph(m_lowAmmoGunImageHandle);
	DeleteGraph(m_noAmmoGunImageHandle);
	DeleteGraph(m_healthUiImageHandle);
	DeleteGraph(m_swordImageHandle);

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

    // 剣のアニメーションタイマー更新
    if (m_isSwordAnimating)
    {
        m_swordAnimTimer += kDeltaTime;
        if (m_swordAnimTimer >= m_swordAnimDuration)
        {
            m_isSwordAnimating = false;
            m_swordAnimTimer = 0.0f;
        }
    }

    m_pCamera->Update(); // カメラの更新

    // Swayの計算
    float yawDelta = m_pCamera->GetYawDelta();

    m_gunSwayOffset.x -= yawDelta * kGunSwayAmount;
    m_swordSwayOffset.x -= yawDelta * kSwordSwayAmount;

    m_gunSwayOffset.x *= kSwayDamping;
    m_swordSwayOffset.x *= kSwayDamping;

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

	// モデルの位置を設定
	MV1SetPosition(m_modelHandle, VAdd(modelPos, m_gunSwayOffset));
	
	// モデルの回転を設定
	MV1SetRotationXYZ(m_modelHandle, VAdd(VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F , 0.0f), m_gunSwayRotOffset));

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

	// マウスの左クリックで射撃（タックル中は射撃不可）
	if ((m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::Shoot) && !m_isTackling && Mouse::IsPressLeft() && (m_ammo > 0 || m_isInfiniteAmmo) && m_shootCooldownTimer <= 0.0f)
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

	// 右クリックでタックル開始
	if ((m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::Tackle) && !m_isTackling && m_tackleCooldown <= 0 && Mouse::IsTriggerRight())
	{
		m_isTackling = true;
		m_isSwordAnimating = true; // 剣のアニメーション開始
		m_swordAnimTimer = 0.0f;   // タイマーリセット

		PlaySoundMem(m_tackleSEHandle, DX_PLAYTYPE_BACK); // タックルSE再生
		m_tackleFrame = kTackleDuration;
		m_tackleCooldown = m_tackleCooldownMax; // クールタイム開始
		m_tackleId++; // タックルごとにIDを更新

		// カメラの向きで3D正規化ベクトルを作成
		float yaw   = m_pCamera->GetYaw();
		float pitch = m_pCamera->GetPitch();

		// タックル方向を計算
		m_tackleDir = VGet(
			cosf(pitch) * sinf(yaw),
			sinf(pitch),
			cosf(pitch) * cosf(yaw)
		);

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

	// スペースキーを押した瞬間のみジャンプ
	if (keyState[KEY_INPUT_SPACE] && !m_prevKeyState[KEY_INPUT_SPACE] && isOnGround && !m_isJumping && !m_isTackling)
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

	// 移動方向がある場合
	if (moveDir.x != 0.0f || moveDir.z != 0.0f)
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

	// 弾薬低下の警告表示処理
	if (m_ammo == 0 && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer += kDeltaTime; // タイマー更新
		m_showNoAmmoWarning = true;
	}
	else if (m_ammo <= kLowAmmoThreshold && !m_isInfiniteAmmo)
	{
		m_isLowAmmo = true;
		m_lowAmmoBlinkTimer += kDeltaTime; // タイマー更新
		m_showNoAmmoWarning = false;
	}
	else
	{
		m_isLowAmmo = false;
		m_lowAmmoBlinkTimer = 0.0f;
		m_showNoAmmoWarning = false;
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
}

void Player::Draw()
{
	// プレイヤーモデルの描画
	MV1DrawModel(m_modelHandle); 

	// 弾の描画
	Bullet::DrawBullets(m_bullets);

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
	
	// 剣とゲージを合わせたUI全体の幅を計算
	int tackleUIWidth = kTackleGaugeWidth + kSwordImageGaugeSpacing + kSwordImageWidth;
	
	// タックルUIのX座標を画面右端に合わせる
	int tackleGaugeX = screenW - tackleUIWidth - kHpBarMargin; // HPバーと同じマージンを使用
	
	// 枠
	DrawBox(tackleGaugeX - 1, tackleUIY - 1, tackleGaugeX + kTackleGaugeWidth + 1, tackleUIY + kTackleGaugeHeight + 1, kColorTackleGaugeBorder, false);
	
	// ゲージ本体
	float tackleRate = 1.0f - (m_tackleCooldown / static_cast<float>(m_tackleCooldownMax));
	int tackleFilledWidth = static_cast<int>(kTackleGaugeWidth * tackleRate);
	DrawBox(tackleGaugeX, tackleUIY, tackleGaugeX + tackleFilledWidth, tackleUIY + kTackleGaugeHeight, kColorTackleGaugeFill, true);
	
	// 剣の画像を描画
	int swordImageX = tackleGaugeX + kTackleGaugeWidth + kSwordImageGaugeSpacing; // ゲージの右側に配置
	int swordImageY = tackleUIY + (kTackleGaugeHeight - kSwordImageHeight) * 0.5f; // ゲージと中央揃え
	
	// クールダウン中は半透明、準備完了時は不透明
	int alpha = (m_tackleCooldown > 0) ? kSwordImageCooldownAlpha : kSwordImageActiveAlpha;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawExtendGraph(swordImageX, swordImageY, swordImageX + kSwordImageWidth, swordImageY + kSwordImageHeight, m_swordImageHandle, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドモードを元に戻す

	// 警告表示ロジック
	if (m_isLowHealth && (m_isLowAmmo || m_showNoAmmoWarning))
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
	else if (m_isLowAmmo || m_showNoAmmoWarning)
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
		const char* text = (m_showNoAmmoWarning) ? "残弾なし" : "残弾僅か";
		int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_warningFontHandle);
		int textX = (screenW - textWidth) * 0.5f;
		int textY = drawY + kWarningImageSize + kWarningTextYOffset;
		unsigned int textColor = (alphaInt << 24) | kColorWhite;
		DrawStringToHandle(textX, textY, text, textColor, m_warningFontHandle);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	/*剣の描画*/
	// 画面サイズに応じてスケーリング
	float scaleW = screenW / kSwordBaseScreenW;
	float scaleH = screenH / kSwordBaseScreenH;
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
	VECTOR swordCamPos = VGet(0, 0, kSwordCamZ * scaleAvg);
	swordCamPos.x += totalCameraOffset.x;
	swordCamPos.y += totalCameraOffset.y;
	VECTOR swordCamTarget = VGet(totalCameraOffset.x * kSwordCamTargetFactor, totalCameraOffset.y * kSwordCamTargetFactor, 0);
	SetCameraPositionAndTarget_UpVecY(swordCamPos, swordCamTarget);

	// 描画するかどうかのフラグ
	bool shouldDrawSword = (m_tackleCooldown <= 0) || m_isSwordAnimating;
	if (!m_isSwordAnimating && m_tackleCooldown > 0) 
	{
		shouldDrawSword = false; // クールダウン中は非表示
	}

	if (shouldDrawSword)
	{
		// 待機状態の剣の位置と回転を定義
		VECTOR waitPos    = VAdd(VGet(kSwordWaitX * scaleW, kSwordWaitY * scaleH, kSwordWaitZ), m_swordSwayOffset);
		VECTOR waitRotVec = VAdd(VGet(0.0f, kSwordWaitRotY, 0.0f), m_swordSwayRotOffset);
		// 待機状態の基本となる変換行列を作成 (回転 * 平行移動)
		MATRIX matWaitRot  = MGetRotY(waitRotVec.y);
		MATRIX matWaitPos  = MGetTranslate(waitPos);
		MATRIX waitMatrix  = MMult(matWaitRot, matWaitPos);
		MATRIX finalMatrix = waitMatrix;

		if (m_isSwordAnimating)
		{
			float animProgress = m_swordAnimTimer / m_swordAnimDuration;
			// イージング
			animProgress = -0.5f * (cosf(DX_PI_F * animProgress) - 1.0f);

			// 回転角度を計算 (左 -> 右)
			float startAngle   = kSwordAnimStartAngle * (DX_PI_F / 180.0f);
			float endAngle     = kSwordAnimEndAngle * (DX_PI_F / 180.0f);
			float currentAngle = startAngle + (endAngle - startAngle) * animProgress;

			// モデルのローカル座標におけるピボット（手持ち部分）の位置
			VECTOR pivotOffset = VGet(0.0f, 0.0f, kSwordPivotZ);

			// 行列を使ってピボット回転を実装
			MATRIX matPivotTrans    = MGetTranslate(VScale(pivotOffset, -1.0f));
			MATRIX matRotate        = MGetRotY(currentAngle);
			MATRIX matPivotTransInv = MGetTranslate(pivotOffset);

			// 最終的な行列をMMultで計算
			// finalMatrix = (待機時の行列) -> (ピボットを原点へ) -> (回転) -> (ピボットを戻す)
			MATRIX tempMat1 = MMult(waitMatrix, matPivotTrans);
			MATRIX tempMat2 = MMult(tempMat1, matRotate);
			finalMatrix     = MMult(tempMat2, matPivotTransInv);
		}

		// モデルに最終的な変換行列を適用
		MV1SetMatrix(m_swordModelHandle, finalMatrix);
		MV1SetScale(m_swordModelHandle, VGet(kSwordModelScale * scaleAvg, kSwordModelScale * scaleAvg, kSwordModelScale * scaleAvg));
		MV1DrawModel(m_swordModelHandle);
	}

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

// ダメージを受ける処理
void Player::TakeDamage(float damage)
{
	if (m_isInvincible) return; // 無敵モード中はダメージを受けない

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
