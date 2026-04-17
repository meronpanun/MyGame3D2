#include "Player.h"
#include "AnimationManager.h"
#include "Bullet.h"
#include "Camera.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "DebugUtil.h"
#include "DirectionIndicator.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "Game.h"
#include "InputManager.h"
#include "TaskTutorialManager.h"
#include "SceneGameOver.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "ShellCasing.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

namespace PlayerConstants
{
    // カプセルコライダーのサイズ
    constexpr float kCapsuleHeight = 100.0f;
    constexpr float kCapsuleRadius = 50.0f;

    // タックル関連
    constexpr int kTackleDuration = 35;        // タックル持続フレーム数
    constexpr float kTackleHitRange = 450.0f;  // タックルの前方有効距離
    constexpr float kTackleHitRadius = 250.0f; // タックルの横幅（半径）
    constexpr float kTackleHitHeight = 100.0f; // タックルの高さ

    // 銃の揺れ関連の定数
    constexpr float kGunSwayAmount = 0.7f;  // 銃モデルの揺れの強さ
    constexpr float kGunSwayDamping = 0.8f; // 銃モデルの揺れの減衰率

    // Update関連
    constexpr float kFrameRate = 60.0f;
    constexpr float kDeltaTime = 1.0f / kFrameRate;
    constexpr float kPlayerColliderYOffset = 60.0f;
    constexpr float kTackleFov = 100.0f;          // タックル中のカメラFOV
    constexpr float kTackleCameraZOffset = 30.0f; // タックル中のカメラZオフセット
    constexpr float kConcentrationLineEffectZOffset = 15.0f; // 集中線エフェクトのZオフセット
    constexpr float kHpBarAnimSpeed = 1.5f;      // HPバーのアニメーション速度
    constexpr int kLowAmmoThreshold = 10;        // 弾薬が少ないと判断する閾値
    constexpr float kLowHealthThreshold = 30.0f; // 体力が少ないと判断する閾値
    constexpr float kWarningBlinkSpeed = 1.5f;   // 警告UIの点滅速度
    constexpr float kLowHealthEffectMaxAlpha = 0.7f; // 体力低下UIの最大アルファ値
    constexpr float kIdleSwaySpeed = 1.5f;           // 揺れの速さ
    constexpr float kIdleSwayAmount = 0.04f;         // 揺れの量
    constexpr float kLockOnAngleCos = 0.966f;        // cos(15度)
    constexpr float kLockOnMaxScreenOffsetY = 100.0f; // 画面中央からの垂直方向の最大オフセット
    constexpr float kTackleStopMargin = 45.0f; // タックル停止判定のマージン

    // 盾UI関連
    constexpr int kShieldImageGaugeSpacing = 10; // 盾UIとクールダウンゲージの間隔
    constexpr int kShieldImageActiveAlpha = 255; // 使用可能な盾UIのアルファ値
    constexpr int kShieldImageCooldownAlpha = 128; // クールダウン中の盾UIのアルファ値
    constexpr int kShieldUIYPosition = 420;
    constexpr int kShieldUIYOffset = 30; // 盾UIのY軸調整オフセット

    // フォント関連
    constexpr int kDefaultFontThickness = 3; // フォントの太さ
    constexpr int kAmmoFont = 32;            // 弾薬フォントサイズ
    constexpr int kHpFont = 20;              // HPフォントサイズ
    constexpr int kWarningFont = 24;         // 警告フォントサイズ
    constexpr char kDefaultFontName[] = "Arial Black";
    constexpr char kWarningFontName[] = "HGPｺﾞｼｯｸE";
    constexpr int kDefaultFontType = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;

    // ダメージエフェクト
    constexpr float kDamageEffectDuration = 30.0f; // ダメージエフェクトの持続時間
    constexpr int kDamageEffectColorR = 255;
    constexpr int kDamageEffectColorG = 0;
    constexpr int kDamageEffectColorB = 0;

    // 回復エフェクト
    constexpr float kHealEffectDuration = 45.0f; // 回復エフェクトの持続時間
    constexpr int kHealEffectColorR = 0;
    constexpr int kHealEffectColorG = 255;
    constexpr int kHealEffectColorB = 0;

    // 弾薬取得エフェクト
    constexpr float kAmmoEffectDuration = 45.0f; // 弾薬取得エフェクトの持続時間
    constexpr int kAmmoEffectColorR = 255;
    constexpr int kAmmoEffectColorG = 128;
    constexpr int kAmmoEffectColorB = 0;

    // カメラシェイク
    constexpr float kTakeDamageShakePower = 5.0f; // 攻撃を受けた時の揺れの強さ
    constexpr int kTakeDamageShakeDuration = 15;  // 攻撃を受けた時の揺れの持続時間
    constexpr float kARShootShakePower = 4.0f;    // ARを撃った時の揺れの強さ
    constexpr float kSGShootShakePower = 32.0f;   // SGを撃った時の揺れの強さ
    constexpr int kShootShakeDuration = 8;        // 撃った時の揺れの持続時間
    constexpr float kShieldBreakGunShakePower = 10.0f; // 盾破壊時の銃の揺れの強さ
    constexpr float kShieldBreakGunShakeDuration = 30.0f; // 盾破壊時の銃の揺れの持続時間

    // HpUI関連
    constexpr int kHpBarWidth = 200;
    constexpr int kHpBarHeight = 24;
    constexpr int kHpBarMargin = 30;
    constexpr int kHealthUiImageSize = 64;
    constexpr int kHealthUiImageBarSpacing = 10;
    constexpr float kMaxHp = 100.0f;
    constexpr int kHpTextOffsetX = 8;
    constexpr int kHpTextOffsetY = 2;

    // 色関連
    constexpr unsigned int kColorWhite = 0xffffff;
    constexpr unsigned int kColorLowAmmo = 0xd3381c;
    constexpr unsigned int kColorTackleGaugeBorder = 0x5050C8;
    constexpr unsigned int kColorTackleGaugeFill = 0x50B4ff;
    constexpr unsigned int kColorHpBarBg = 0x505050;
    constexpr unsigned int kColorHpBarDamage = 0xFFD700;
    constexpr unsigned int kColorHpBarFill = 0xff4040;
    constexpr unsigned int kColorHpBarBorder = 0x000000;

    // タックルヒット時のSE音量 (DxLibの設定上0〜255が範囲、255で最大音量)
    constexpr int kTackleHitVolume = 255;
}

Player::Player()
    : m_playerHitSEHandle(-1), m_tackleSEHandle(-1), m_tackleHitSEHandle(-1), m_recoverySEHandle(-1)
    , m_ammoItemSEHandle(-1), m_modelPos(VGet(0, 0, 0)), m_pEffect(nullptr)
    , m_pCamera(std::make_shared<Camera>()), m_pos(VGet(0, 0, 0))
    , m_health(100.0f), m_healthBarAnim(100.0f), m_healthBarAnimTimer(0.0f)
    , m_hasShot(false), m_tackleFrame(0), m_tackleDir(VGet(0, 0, 0))
    , m_isTackling(false), m_tackleCooldown(0), m_tackleId(0)
    , m_concentrationLineEffectHandle(-1), m_isLowHealth(false)
    , m_lowHealthBlinkTimer(0.0f), m_ammoTextFlashTimer(0.0f)
    , m_idleSwayTimer(0.0f), m_gunSwayOffset(VGet(0, 0, 0))
    , m_gunSwayRotOffset(VGet(0, 0, 0)), m_isDead(false), m_deathTimer(0.0f)
    , m_pDirectionIndicator(nullptr), m_isLockingOn(false)
    , m_lockedOnEnemy(nullptr), m_isTargetAvailable(false)
    , m_isAimingAtEnemy(false), m_shouldIgnoreGuardInput(false)
    , m_hasPlayedTackleHitSE(false)
    , m_isInvincible(false), m_isInfiniteAmmo(false), m_isFlightMode(false)
    , m_tackleCooldownMax(0.0f), m_tackleSpeed(0.0f), m_tackleDamage(0.0f)
    , m_maxShieldDurability(0.0f), m_shieldRegenRate(0.0f)
    , m_pAnimManager(nullptr), m_isTutorial(false)
    , m_startAnimTimer(0.0f), m_startAnimDuration(60.0f), m_isStartAnimating(false)
    , m_hasLandedAtStart(false), m_uiFadeTimer(0.0f), m_uiFadeDuration(60.0f), m_isUiFadeStarted(false)
{
    // SEの読み込み
    m_playerHitSEHandle = LoadSoundMem("data/sound/SE/PlayerHit.mp3");
    assert(m_playerHitSEHandle != -1);
    m_tackleSEHandle = LoadSoundMem("data/sound/SE/Tackle.mp3");
    assert(m_tackleSEHandle != -1);
    m_tackleHitSEHandle = LoadSoundMem("data/sound/SE/TackleHit.mp3");
    assert(m_tackleHitSEHandle != -1);
    ChangeVolumeSoundMem(PlayerConstants::kTackleHitVolume, m_tackleHitSEHandle);
    m_recoverySEHandle = LoadSoundMem("data/sound/SE/RecoveryItem.mp3");
    assert(m_recoverySEHandle != -1);
    m_lightLandingSEHandle = LoadSoundMem("data/sound/SE/LightLanding.mp3");
    assert(m_lightLandingSEHandle != -1);
    m_heavyLandingSEHandle = LoadSoundMem("data/sound/SE/HeavyLanding.mp3");
    assert(m_heavyLandingSEHandle != -1);
}

Player::~Player()
{
    // SEの解放
    DeleteSoundMem(m_playerHitSEHandle);
    DeleteSoundMem(m_tackleSEHandle);
    DeleteSoundMem(m_tackleHitSEHandle);
    DeleteSoundMem(m_recoverySEHandle);
    DeleteSoundMem(m_ammoItemSEHandle);
    DeleteSoundMem(m_lightLandingSEHandle);
    DeleteSoundMem(m_heavyLandingSEHandle);
}

void Player::Init(bool isTutorial)
{
    m_isTutorial = isTutorial;
    // CSVからPlayerのTransform情報を取得
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "Player")
        {
            m_pos = data.pos;
            m_modelPos = data.pos;
            m_scale = data.scale;
            m_health = data.hp;
            m_maxHealth = data.hp;
            m_moveSpeed = data.speed;
            m_tackleCooldownMax = data.tackleCooldown;
            m_tackleSpeed = data.tackleSpeed;
            m_tackleDamage = data.tackleDamage;
            m_runSpeed = data.runSpeed;
            m_arInitAmmo = data.arInitAmmo;
            m_sgInitAmmo = data.sgInitAmmo;
            m_arMaxAmmo = data.arInitAmmo;
            m_sgMaxAmmo = data.sgInitAmmo;
            m_bulletPower = data.bulletPower;
            m_sgBulletPower = data.sgBulletPower;
            m_maxShieldDurability = data.maxShieldDurability;
            m_shieldRegenRate = data.shieldRegenRate;

            // 武器モデルのスケールと回転を設定
            m_weaponManager.SetWeaponScale(data.scale);
            m_weaponManager.SetWeaponRotation(data.rot);

            // コンポーネントの初期化
            m_weaponManager.Init(m_arInitAmmo, m_sgInitAmmo, m_arMaxAmmo, m_sgMaxAmmo,
                m_bulletPower, m_sgBulletPower);
            m_movement.Init(m_modelPos, m_moveSpeed, m_runSpeed, m_scale.x);
            m_shieldSystem.Init(m_maxShieldDurability, m_shieldRegenRate);
            break;
        }
    }
    m_pCamera->Init(); // カメラの初期化

    // 開始演出のアニメーション初期化
    m_isStartAnimating = false; // 着地するまで待機
    m_hasLandedAtStart = false; 
    m_startAnimTimer = 0.0f;
    m_startAnimDuration = 60.0f; // 1秒間（60フレーム）
    m_uiFadeTimer = 0.0f;
    m_uiFadeDuration = 60.0f;
    m_isUiFadeStarted = false;
}

void Player::Update(const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData)
{
    unsigned char keyState[256];
    GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

    // コンポーネントの更新
    float deltaTime = PlayerConstants::kDeltaTime * Game::GetTimeScale();

    // 開始アニメーションタイマーの更新
    if (!m_hasLandedAtStart && m_movement.IsOnGround())
    {
        m_hasLandedAtStart = true;
        m_isStartAnimating = true;
    }

    if (m_isStartAnimating)
    {
        m_startAnimTimer += 1.0f * Game::GetTimeScale();
        if (m_startAnimTimer >= m_startAnimDuration)
        {
            m_isStartAnimating = false;
            m_startAnimTimer = m_startAnimDuration;
        }
    }

    // UIフェードインの制御
    if (m_hasLandedAtStart && !m_isStartAnimating)
    {
        m_isUiFadeStarted = true;
    }

    if (m_isUiFadeStarted && m_uiFadeTimer < m_uiFadeDuration)
    {
        m_uiFadeTimer += 1.0f * Game::GetTimeScale();
        if (m_uiFadeTimer > m_uiFadeDuration) m_uiFadeTimer = m_uiFadeDuration;
    }

    bool isInputDisabled = m_isStartAnimating || !m_hasLandedAtStart;

    VECTOR playerPos = m_movement.GetPos();
    bool isGuarding = m_shieldSystem.IsGuarding();
    bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();

    // タックル中もコライダーを更新（移動処理は内部でスキップされる）    // 移動・物理更新
    m_movement.Update(deltaTime, m_pCamera.get(), m_isDead, m_isTackling, m_isFlightMode, collisionData, isInputDisabled);

    // 着地SEの再生
    if (m_movement.JustLanded())
    {
        float impactVel = fabsf(m_movement.GetImpactVelocity());
        if (impactVel >= 10.0f)
        {
            PlaySoundMem(m_heavyLandingSEHandle, DX_PLAYTYPE_BACK);
        }
        else
        {
            PlaySoundMem(m_lightLandingSEHandle, DX_PLAYTYPE_BACK);
        }
    }

    m_modelPos = m_movement.GetPos();
    // 敵接近時のダッシュ解除
    CheckEnemyProximity(enemyList);

    // 位置同期
    if (!m_isTackling)
    {
        m_modelPos = m_movement.GetPos();
    }

    // 武器マネージャー更新
    PlayerWeaponManager::UpdateContext weaponContext = {
        deltaTime,        m_modelPos,        m_pCamera.get(),
        isGuarding,       m_isDead,          m_isTackling,
        m_isLockingOn,    isSwitchingWeapon, m_allowedAttackType,
        m_isInfiniteAmmo, enemyList,         collisionData
    };
    m_weaponManager.Update(weaponContext);

    // 武器切り替え
    if (!isInputDisabled)
    {
        UpdateWeaponSwitching(keyState);
    }

    // カメラ位置設定
    m_pCamera->SetPlayerPos(m_modelPos);

    // カメラ更新
    m_pCamera->Update(isInputDisabled);

    // Sway更新
    float yawDelta = m_pCamera->GetYawDelta();
    m_shieldSystem.Update(deltaTime, m_pCamera.get(), m_modelPos, isGuarding,
        m_isTackling, isSwitchingWeapon,
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(), yawDelta,
        m_movement.IsMoving());

    // ガード入力処理
    bool shouldGuard = !isInputDisabled && !m_isDead && !m_isTackling &&
        InputManager::GetInstance()->IsPressMouseRight() &&
        !m_shouldIgnoreGuardInput && !m_shieldSystem.IsShieldBroken();
    m_shieldSystem.SetGuarding(shouldGuard);
    bool currentIsGuarding = m_shieldSystem.IsGuarding();

    // シールド投擲
    if (!isInputDisabled && (m_allowedAttackType == AttackType::None ||
        m_allowedAttackType == AttackType::ShieldThrow) &&
        !m_isDead && !m_isTackling && !currentIsGuarding && !isSwitchingWeapon &&
        keyState[KEY_INPUT_R] && !m_prevKeyState[KEY_INPUT_R])
    {
        if (m_shieldSystem.IsShieldThrown())
        {
            m_shieldSystem.ImmediateReturnShield(m_modelPos);
        }
        else
        {
            m_shieldSystem.ThrowShield(m_pCamera.get(), m_modelPos);
        }
    }
    else if (m_isTutorial && keyState[KEY_INPUT_R] && !m_prevKeyState[KEY_INPUT_R] && m_allowedAttackType != AttackType::ShieldThrow)
    {
         TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::ShieldThrow);
    }

    m_shieldSystem.UpdateShieldThrow(deltaTime, m_pCamera.get(), m_modelPos,
        enemyList, collisionData, m_pEffect,
        currentIsGuarding, m_prevIsGuarding);
    m_prevIsGuarding = currentIsGuarding; // 更新

    // 銃揺れ計算
    m_gunSwayOffset.x -= yawDelta * PlayerConstants::kGunSwayAmount;
    m_gunSwayOffset.x *= PlayerConstants::kGunSwayDamping;
    m_gunSwayRotOffset.y -= yawDelta * PlayerConstants::kGunSwayAmount * 0.5f;
    m_gunSwayRotOffset.y *= PlayerConstants::kGunSwayDamping;

    // 待機揺れ
    m_idleSwayTimer += deltaTime;
    if (!m_movement.IsMoving())
    {
        VECTOR idleSway =
            VGet(sinf(m_idleSwayTimer * PlayerConstants::kIdleSwaySpeed * 2.0f) * PlayerConstants::kIdleSwayAmount,
                cosf(m_idleSwayTimer * PlayerConstants::kIdleSwaySpeed) * PlayerConstants::kIdleSwayAmount, 0.0f);
        m_gunSwayOffset = VAdd(m_gunSwayOffset, idleSway);
    }

    if (m_pEffect) m_pEffect->Update();

    m_weaponManager.UpdateSGAnimation(m_pAnimManager, deltaTime);

    // タックルクールダウン
    if (m_tackleCooldown > 0)
    {
        m_tackleCooldown--;
        if (m_tackleCooldown == 0)
        {
            for (EnemyBase* enemy : enemyList)
            {
                if (enemy) enemy->ResetTackleHitFlag();
            }
        }
    }

    // 射撃
    if (!isInputDisabled)
    {
        UpdateShooting();
    }

    // 右クリックガード解除で入力無視解除
    if (!InputManager::GetInstance()->IsPressMouseRight())
    {
        m_shouldIgnoreGuardInput = false;
    }

    // ロックオン更新
    UpdateLockOn(enemyList, collisionData);

    // ガードエフェクト更新
    m_shieldSystem.UpdateGuardEffect(m_pEffect, m_pCamera.get(), m_modelPos, isSwitchingWeapon);
    m_shieldSystem.UpdateSparkEffect(m_pEffect, m_modelPos, m_pCamera.get());

    // タックル更新 (開始判定と実行中の処理含む)
    UpdateTackle(enemyList, collisionData);

    // タックルが実行されていない場合のみ、通常の敵更新を行う
    if (!m_isTackling)
    {
        TackleInfo tackleInfo = GetTackleInfo(); // 初期値(isTackling=false)
        for (EnemyBase* enemy : enemyList)
        {
            if (!enemy) continue;
            EnemyUpdateContext context = { m_bullets, tackleInfo, *this, enemyList, collisionData, m_pEffect };
            enemy->Update(context);
        }
    }

    // 弾更新
    Bullet::UpdateBullets(m_bullets, m_modelPos, collisionData);

    if (m_pCamera)
    {
        m_pCamera->SetHeadBobbingState(m_movement.IsMoving(), m_movement.WasRunning());
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
            m_healthBarAnim -= PlayerConstants::kHpBarAnimSpeed;
            if (m_healthBarAnim < m_health) m_healthBarAnim = m_health;
        }
        else
        {
            m_healthBarAnim += PlayerConstants::kHpBarAnimSpeed;
            if (m_healthBarAnim > m_health) m_healthBarAnim = m_health;
        }
    }

    // 体力低下警告
    if (m_health <= PlayerConstants::kLowHealthThreshold)
    {
        m_isLowHealth = true;
        m_lowHealthBlinkTimer += PlayerConstants::kDeltaTime;
    }
    else
    {
        m_isLowHealth = false;
        m_lowHealthBlinkTimer = 0.0f;
    }
    m_effectManager.Update(deltaTime, m_isLowHealth, m_lowHealthBlinkTimer);

    if (m_ammoTextFlashTimer > 0.0f)
    {
        m_ammoTextFlashTimer -= 1.0f;
    }

    ShellCasing::UpdateShellCasings(m_shellCasings);
}

void Player::Draw3D()
{
    bool isTryingToGuard = !m_isDead && !m_isTackling &&
        InputManager::GetInstance()->IsPressMouseRight() &&
        !m_shouldIgnoreGuardInput &&
        !m_shieldSystem.IsShieldBroken();
    bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();

    float startAnimOffsetY = 0.0f;
    if (!m_hasLandedAtStart)
    {
        startAnimOffsetY = 200.0f;
    }
    else if (m_isStartAnimating)
    {
        float progress = m_startAnimTimer / m_startAnimDuration;
        float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
        startAnimOffsetY = (1.0f - easeOut) * 200.0f;
    }

    // カメラのジャンプ・着地揺れを銃の揺れに反映
    VECTOR totalSway = m_gunSwayOffset;

    PlayerWeaponManager::DrawContext weaponDrawContext = {
        m_modelPos,
        m_pCamera.get(),
        totalSway,
        m_weaponManager.GetGunShakeOffset(),
        m_gunSwayRotOffset,
        m_shieldSystem.GetGuardAnimTimer(),
        m_shieldSystem.GetGuardAnimDuration(),
        isSwitchingWeapon,
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(),
        m_weaponManager.GetPreviousWeaponType(),
        isTryingToGuard,
        m_isTackling,
        startAnimOffsetY
    };
    m_weaponManager.Draw3D(weaponDrawContext);

    // 弾と薬莢の描画
    Bullet::DrawBullets(m_bullets);
    ShellCasing::DrawShellCasings(m_shellCasings);

    // シールドソーの描画（投げられている場合のみ）
    if (m_shieldSystem.IsShieldThrown())
    {
        m_shieldSystem.DrawShieldThrow(m_pCamera.get(), m_modelPos);
    }
}

void Player::DrawShield()
{
    float startAnimOffsetY = 0.0f;
    if (!m_hasLandedAtStart)
    {
        startAnimOffsetY = 200.0f;
    }
    else if (m_isStartAnimating)
    {
        float progress = m_startAnimTimer / m_startAnimDuration;
        float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
        startAnimOffsetY = (1.0f - easeOut) * 200.0f;
    }

    m_shieldSystem.Draw(m_pCamera.get(), m_modelPos, m_isTackling,
        m_weaponManager.IsSwitchingWeapon(),
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(),
        startAnimOffsetY);
}

void Player::DrawUI()
{
    // UIの透明度を計算 (0.0 ～ 1.0)
    float baseAlpha = 0.0f;
    if (m_isUiFadeStarted)
    {
        baseAlpha = m_uiFadeTimer / m_uiFadeDuration;
    }

    // UI描画をPlayerUIクラスに委譲
    m_ui.Draw(m_isDead, m_shieldSystem.IsGuarding(), m_lockedOnEnemy,
        m_isTargetAvailable, m_health, m_healthBarAnim, m_maxHealth,
        m_isLowHealth, m_lowHealthBlinkTimer, m_ammoTextFlashTimer,
        m_weaponManager, m_shieldSystem, baseAlpha);

    // エフェクトの描画
    m_effectManager.Draw();
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
void Player::TakeDamage(float damage, const VECTOR& attackerPos, bool isParryable)
{
    if (m_isDead) return;

    if (m_isTutorial) damage *= 0.5f;

    if (m_isInvincible) return;

    // ダメージを受けたらダッシュ解除
    m_movement.CancelRunMode();

    // 方向インジケーターに攻撃者の位置を通知
    if (m_pDirectionIndicator && (attackerPos.x != 0.0f || attackerPos.z != 0.0f))
    {
        m_pDirectionIndicator->ShowAttackedEnemyDirection(Vec3(attackerPos));
    }

    if (m_shieldSystem.IsGuarding() && !m_shieldSystem.IsShieldBroken()) // ガード中で盾が壊れていなければ
    {
        // ジャストガード（パリィ）判定
        // ダメージ計算前に行う
        if (m_shieldSystem.IsJustGuarded() && isParryable)
        {
            // 特定の敵（Acid, Boss）のパリィ弾の時のみスローにするため、ここでは呼ばない
        }

        // カメラシェイクを発生
        if (m_pCamera)
        {
            m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
        }

        // 盾の前方にスパークエフェクトを再生
        if (m_pEffect)
        {
            VECTOR forward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
            VECTOR effectPos = VAdd(m_modelPos, VScale(forward, 80.0f));
        }

        float remainingDamage = m_shieldSystem.TakeDamage(damage, m_pEffect, m_pCamera.get(), m_modelPos);
        if (remainingDamage > 0)
        {
            // 銃を揺らす
            m_weaponManager.ShakeGun(PlayerConstants::kShieldBreakGunShakePower, PlayerConstants::kShieldBreakGunShakeDuration);
            m_health -= remainingDamage; // 残ったダメージをHPに適用

            // チュートリアル中は死なない
            if (m_isTutorial && m_health <= 0.0f)
            {
                m_health = 1.0f;
            }

            // HPバーアニメーション用タイマーをリセット
            m_healthBarAnimTimer = 0.0f;
            // ダメージエフェクトを発動
            m_effectManager.TriggerDamageEffect(
                PlayerConstants::kDamageEffectDuration, PlayerConstants::kDamageEffectColorR, PlayerConstants::kDamageEffectColorG,
                PlayerConstants::kDamageEffectColorB);
            // 被弾SEを再生
            PlaySoundMem(m_playerHitSEHandle, DX_PLAYTYPE_BACK);

            // カメラシェイクを発生
            if (m_pCamera)
            {
                m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
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

    // チュートリアル中は死なない
    if (m_isTutorial && m_health <= 0.0f)
    {
        m_health = 1.0f;
    }

    // HPバーアニメーション用タイマーをリセット
    m_healthBarAnimTimer = 0.0f;
    // ダメージエフェクトを発動
    m_effectManager.TriggerDamageEffect(PlayerConstants::kDamageEffectDuration,
        PlayerConstants::kDamageEffectColorR, PlayerConstants::kDamageEffectColorG,
        PlayerConstants::kDamageEffectColorB);
    // 被弾SEを再生
    PlaySoundMem(m_playerHitSEHandle, DX_PLAYTYPE_BACK);

    // カメラシェイクを発生
    if (m_pCamera)
    {
        m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
    }
}

// 弾の取得
std::vector<Bullet>& Player::GetBullets() { return m_bullets; }

// プレイヤーがショット可能かどうか
bool Player::HasShot()
{
    bool shot = m_hasShot;
    m_hasShot = false; // 状態をリセット
    return shot;       // 撃ったかどうかを返す
}

void Player::Shoot(std::vector<Bullet>& bullets)
{
    m_weaponManager.Shoot(bullets, m_modelPos, m_pCamera.get(), m_pEffect,
        m_pAnimManager, m_shellCasings);
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

std::string Player::GetGroundedObjectName() const
{
    return m_movement.GetGroundedObjectName();
}

// タックル情報を取得
Player::TackleInfo Player::GetTackleInfo() const
{
    TackleInfo info;
    info.isTackling = m_isTackling;
    if (m_isTackling)
    {
        float tackleHeight = PlayerConstants::kTackleHitHeight;
        info.tackleId = m_tackleId; // タックルIDをセット

        // プレイヤーの体の中心位置
        VECTOR bodyCenter = m_modelPos;
        constexpr float kAROffsetY = 20.0f; // ARオフセットY
        bodyCenter.y += kAROffsetY;

        // プレイヤーの前面中心（体の中心から前方へkTackleHitRangeだけ進める）
        VECTOR frontCenter = VAdd(bodyCenter, VScale(m_tackleDir, PlayerConstants::kTackleHitRange));

        // カプセルの始点と終点を設定 (体から前方への範囲全体)
        // 以前は前方の一点に縦長のカプセルを作っていたが、
        // それだと密着時に当たらないため、プレイヤーの位置から伸ばす形に変更
        info.capA = bodyCenter;
        info.capB = frontCenter;
        info.radius = PlayerConstants::kTackleHitRadius;
        info.damage = m_tackleDamage;
    }
    return info;
}

// カプセル情報を取得
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
    // m_movementのコライダーから直接取得
    auto collider = m_movement.GetBodyCollider();
    capA = collider->GetSegmentA();
    capB = collider->GetSegmentB();
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
    m_effectManager.TriggerHealEffect(PlayerConstants::kHealEffectDuration, PlayerConstants::kHealEffectColorR,
        PlayerConstants::kHealEffectColorG, PlayerConstants::kHealEffectColorB);
}

void Player::AddARAmmo(int value)
{
    m_weaponManager.AddARAmmo(value);
    // 弾薬取得時にエフェクトを発動
    m_effectManager.TriggerAmmoEffect(PlayerConstants::kAmmoEffectDuration, PlayerConstants::kAmmoEffectColorR,
        PlayerConstants::kAmmoEffectColorG, PlayerConstants::kAmmoEffectColorB);
    m_ammoTextFlashTimer = 60.0f;
}

void Player::AddSGAmmo(int value)
{
    m_weaponManager.AddSGAmmo(value);
    // 弾薬取得時にエフェクトを発動
    m_effectManager.TriggerAmmoEffect(PlayerConstants::kAmmoEffectDuration, PlayerConstants::kAmmoEffectColorR,
        PlayerConstants::kAmmoEffectColorG, PlayerConstants::kAmmoEffectColorB);
    m_ammoTextFlashTimer = 60.0f;
}

int Player::GetCurrentAmmo() const { return m_weaponManager.GetCurrentAmmo(); }

int Player::GetMaxAmmo() const { return m_weaponManager.GetMaxAmmo(); }

void Player::SetAttackRestrictions(AttackType allowedAttack)
{
    m_allowedAttackType = allowedAttack;
}

void Player::ShakeGun(float power, float duration)
{
    m_weaponManager.ShakeGun(power, duration);
}

bool Player::IsAimingAtEnemy() const { return m_isAimingAtEnemy; }

bool Player::IsJustGuarded() const { return m_shieldSystem.IsJustGuarded(); }

WeaponType Player::GetCurrentWeaponType() const
{
    return m_weaponManager.GetCurrentWeaponType();
}

// 武器を切り替える
void Player::SwitchWeapon(WeaponType weaponType)
{
    m_weaponManager.SwitchWeapon(weaponType);
}

bool Player::CheckLineOfSight(const VECTOR& start, const VECTOR& end,
    const std::vector<Stage::StageCollisionData>& collisionData) const
{
    for (const auto& col : collisionData)
    {
        HITRESULT_LINE result =
            HitCheck_Line_Triangle(start, end, col.v1, col.v2, col.v3);
        if (result.HitFlag)
        {
            return false;
        }
    }
    return true;
}

void Player::CheckEnemyProximity(const std::vector<EnemyBase*>& enemyList)
{
    if (m_isTackling) return;

    for (const auto& enemy : enemyList)
    {
        if (!enemy || !enemy->IsAlive())
            continue;

        // プレイヤーと敵の距離をチェック
        // カプセル半径の和 + マージン
        constexpr float kEnemyCollisionDist = 100.0f;
        VECTOR diff = VSub(m_movement.GetPos(), enemy->GetPos());
        float distSq = VSize(diff); // VSizeも2乗を返すわけではないので注意。VSizeはsqrtを取る。
        // ここでは距離そのもので比較
        if (distSq < kEnemyCollisionDist)
        {
            m_movement.CancelRunMode();
            break;
        }
    }
}

void Player::UpdateWeaponSwitching(const unsigned char* keyState)
{
    if (m_shieldSystem.IsGuarding()) return;

    if (keyState[KEY_INPUT_1] && !m_prevKeyState[KEY_INPUT_1])
    {
        // 武器切り替え自体は制限していない
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
        WeaponType nextWeapon = (currentWeapon == WeaponType::AssaultRifle)
            ? WeaponType::Shotgun
            : WeaponType::AssaultRifle;
        m_weaponManager.SwitchWeapon(nextWeapon);
    }
}

void Player::UpdateShooting()
{
    // シールド投げチュートリアル中は射撃を許可するため、制限条件からShieldThrowを除外する
    bool isShootRestricted = m_allowedAttackType != AttackType::None &&
                             m_allowedAttackType != AttackType::Shoot &&
                             m_allowedAttackType != AttackType::ShieldThrow;

    if (m_isDead ||
        isShootRestricted || 
        m_isTackling || m_shieldSystem.IsGuarding() || m_isLockingOn || m_weaponManager.IsSwitchingWeapon())
    {
        // チュートリアル中の射撃制限通知
        if (m_isTutorial && !m_isDead && !m_isTackling && !m_shieldSystem.IsGuarding() && !m_isLockingOn && !m_weaponManager.IsSwitchingWeapon())
        {
             if (InputManager::GetInstance()->IsPressMouseLeft())
             {
                 // 許可されていないのに撃とうとした（シールド投げタスク中は射撃を許可するため通知を除外）
                 if (m_allowedAttackType != AttackType::Shoot && m_allowedAttackType != AttackType::Shotgun && 
                     m_allowedAttackType != AttackType::ShieldThrow && m_allowedAttackType != AttackType::None)
                 {
                      TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::Shoot);
                 }
             }
        }
        return;
    }

    if (InputManager::GetInstance()->IsPressMouseLeft() &&
        (m_weaponManager.GetCurrentAmmo() > 0 || m_isInfiniteAmmo) &&
        m_weaponManager.CanShoot())
    {
        m_weaponManager.Shoot(m_bullets, m_modelPos, m_pCamera.get(), m_pEffect,
            m_pAnimManager, m_shellCasings);
        m_weaponManager.ConsumeAmmo();
    }
}

void Player::UpdateLockOn(const std::vector<EnemyBase*>& enemyList,
    const std::vector<Stage::StageCollisionData>& collisionData)
{
    // ロックオン可能な敵がいるかどうかの判定
    m_isTargetAvailable = false;

    VECTOR camPos = m_pCamera->GetPos();
    VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));

    // タックルが届く最大距離 (移動距離 + 当たり判定の突き出し分)
    float tackleMaxReach = m_tackleSpeed * PlayerConstants::kTackleDuration + PlayerConstants::kTackleHitRange;
    float tackleMaxReachSq = tackleMaxReach * tackleMaxReach;


    for (EnemyBase* enemy : enemyList)
    {
        if (!enemy || !enemy->IsAlive())
            continue;

        VECTOR enemyPos = enemy->GetPos();
        enemyPos.y += 70.0f; // 敵の胴体あたりをターゲットにするためのオフセット
        VECTOR toEnemyDir = VNorm(VSub(enemyPos, camPos));

        // 距離チェック
        VECTOR diff = VSub(m_modelPos, enemy->GetPos());
        float distSq = VSize(diff) * VSize(diff);

        // 距離が届かない場合はロックオン対象外
        if (distSq > tackleMaxReachSq)
        {
            continue;
        }

        // プレイヤーの前方一定角度内にいるか
        if (VDot(camDir, toEnemyDir) > PlayerConstants::kLockOnAngleCos)
        {
            VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

            // 画面内にいるか、かつ垂直方向の範囲内か
            if (screenPos.z > 0)
            {
                float dx = screenPos.x - (Game::GetScreenWidth() / 2.0f);
                float dy = screenPos.y - (Game::GetScreenHeight() / 2.0f);

                // 垂直方向の範囲チェック
                if (fabs(dy) < PlayerConstants::kLockOnMaxScreenOffsetY)
                {
                    // 視線チェック
                    if (CheckLineOfSight(camPos, enemyPos, collisionData))
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
    VECTOR rayEnd = VAdd(camPos, VScale(camDir, 5000.0f));

    for (const auto& enemy : enemyList)
    {
        if (!enemy || !enemy->IsAlive())
        {
            continue;
        }

        VECTOR hitPos;
        float hitDistSq;
        EnemyBase::HitPart part =
            enemy->CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);

        if (part == EnemyBase::HitPart::Body || part == EnemyBase::HitPart::Head)
        {
            // 照準が合っている敵に対しても視線チェック
            if (CheckLineOfSight(camPos, hitPos, collisionData))
            {
                m_isAimingAtEnemy = true;
                break;
            }
        }
    }

    // タックルクールダウン中でない場合のみロックオンを許可
    bool shouldGuard = m_shieldSystem.IsGuarding();

    if (m_shieldSystem.IsGuarding() && m_tackleCooldown <= 0)
    {
        m_isLockingOn = true;
        m_lockedOnEnemy = nullptr;
        float minScreenDistSq = -1.0f;

        for (EnemyBase* enemy : enemyList)
        {
            if (!enemy || !enemy->IsAlive())
                continue;

            // 距離チェック
            VECTOR diff = VSub(m_modelPos, enemy->GetPos());
            float distSqWorld = VSize(diff) * VSize(diff);
            if (distSqWorld > tackleMaxReachSq)
            {
                continue;
            }

            VECTOR enemyPos = enemy->GetPos();
            enemyPos.y += 70.0f;
            VECTOR toEnemyDir = VNorm(VSub(enemyPos, camPos));

            if (VDot(camDir, toEnemyDir) > PlayerConstants::kLockOnAngleCos)
            {
                VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

                if (screenPos.z > 0)
                {
                    float dx = screenPos.x - (Game::GetScreenWidth() / 2.0f);
                    float dy = screenPos.y - (Game::GetScreenHeight() / 2.0f);

                    if (fabs(dy) < PlayerConstants::kLockOnMaxScreenOffsetY)
                    {
                        // 視線チェック
                        if (CheckLineOfSight(camPos, enemyPos, collisionData))
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
    }
    else
    {
        m_isLockingOn = false;
        m_lockedOnEnemy = nullptr;
    }
}

void Player::UpdateTackle(const std::vector<EnemyBase*>& enemyList,
    const std::vector<Stage::StageCollisionData>& collisionData)
{
    // タックル開始判定
    // ロックオン中に左クリックでタックル開始
    if (m_isLockingOn && m_lockedOnEnemy &&
        InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        // タックルが許可されていない場合
        if (m_isTutorial && m_allowedAttackType != AttackType::None && m_allowedAttackType != AttackType::Tackle)
        {
            TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::Tackle);
            return;
        }

        if ((m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::Tackle) && m_tackleCooldown <= 0)
        {
            m_isTackling = true;

            PlaySoundMem(m_tackleSEHandle, DX_PLAYTYPE_BACK); // タックルSE再生
            m_tackleFrame = PlayerConstants::kTackleDuration;
            m_tackleCooldown = m_tackleCooldownMax; // クールタイム開始
            m_tackleId++;                           // タックルごとにIDを更新
            m_hasPlayedTackleHitSE = false;         // ヒットSEフラグをリセット

            // ロックオンした敵の方向をタックル方向とする
            m_tackleDir = VNorm(VSub(m_lockedOnEnemy->GetPos(), m_modelPos));

            // タックル開始時にFOVを広げ、カメラを後ろに引く
            if (m_pCamera)
            {
                m_pCamera->SetTargetFOV(PlayerConstants::kTackleFov * DX_PI_F / 180.0f);
                VECTOR offset = m_pCamera->GetOffset();
                offset.z = PlayerConstants::kTackleCameraZOffset;
                m_pCamera->SetOffset(offset);

                // 集中線エフェクトを再生
                if (m_pEffect)
                {
                    VECTOR camPos = m_pCamera->GetPos();
                    m_concentrationLineEffectHandle =
                        m_pEffect->PlayConcentrationLine(camPos.x, camPos.y, camPos.z);
                }
            }
            m_isLockingOn = false; // タックル開始したらロックオン解除
            m_lockedOnEnemy = nullptr;
        }
    }

    // タックル中の処理
    if (m_isTackling)
        {
            m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, m_tackleSpeed));

            // m_movementの位置も同期
            m_movement.SetPos(m_modelPos);

            if (m_modelPos.y < PlayerMovement::GetGroundY())
            {
                m_modelPos.y = PlayerMovement::GetGroundY();
            }

            // ステージとの衝突判定
            CollisionResult res = Collision::CheckStageCollision(
                m_modelPos, PlayerConstants::kCapsuleHeight, PlayerConstants::kCapsuleRadius, PlayerConstants::kPlayerColliderYOffset,
                collisionData);

            // m_movementの位置も同期
            m_movement.SetPos(m_modelPos);

            // タックル判定情報を作成
            TackleInfo tackleInfo = GetTackleInfo();

            // タックル停止判定用
            bool isBodyHit = false;
            VECTOR bodyCapA, bodyCapB;
            float bodyRadius;
            GetCapsuleInfo(bodyCapA, bodyCapB, bodyRadius);
            CapsuleCollider bodyCol(bodyCapA, bodyCapB, bodyRadius + PlayerConstants::kTackleStopMargin);

            // 各敵と衝突判定
            for (EnemyBase* enemy : enemyList)
            {
                if (!enemy)
                    continue;

                EnemyUpdateContext context = { m_bullets, tackleInfo,    *this,
                                              enemyList, collisionData, m_pEffect };
                enemy->Update(context);

                // タックル停止判定
                if (m_isTackling && !isBodyHit && enemy->IsAlive())
                {
                    auto enemyCollider = enemy->GetBodyCollider();
                    if (enemyCollider && bodyCol.IsIntersects(enemyCollider.get()))
                    {
                        isBodyHit = true;
                    }
                }
            }

#ifdef _DEBUG
            DebugUtil::DrawCapsule(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius,
                16, 0x00ff00, false);
#endif
            m_tackleFrame--;
            // タックル終了判定
            if (m_tackleFrame <= 0 || isBodyHit)
            {
                if (isBodyHit)
                {
                    if (m_pCamera)
                    {
                        m_pCamera->Shake(20.0f, 10);
                    }

                    if (!m_hasPlayedTackleHitSE)
                    {
                        PlaySoundMem(m_tackleHitSEHandle, DX_PLAYTYPE_BACK);
                        m_hasPlayedTackleHitSE = true;
                    }

                    // ノックバック適用
                    // タックル方向の逆ベクトル方向に弾き飛ばす
                    VECTOR knockbackDir = VScale(m_tackleDir, -1.0f);
                    m_movement.ApplyKnockback(VScale(knockbackDir, 15.0f));
                }

                m_isTackling = false;
                m_shouldIgnoreGuardInput = true;

                if (m_pCamera)
                {
                    m_pCamera->ResetFOV();
                    m_pCamera->ResetOffset();
                }

                if (m_concentrationLineEffectHandle != -1)
                {
                    StopEffekseer3DEffect(m_concentrationLineEffectHandle);
                    m_concentrationLineEffectHandle = -1;
                }
            }

            // 集中線エフェクト更新
            if (m_concentrationLineEffectHandle != -1 && m_pCamera)
            {
                VECTOR camPos = m_pCamera->GetPos();
                VECTOR camDir = VNorm(VSub(m_pCamera->GetTarget(), camPos));
                VECTOR effectPos = VAdd(camPos, VScale(camDir, PlayerConstants::kConcentrationLineEffectZOffset));
                SetPosPlayingEffekseer3DEffect(m_concentrationLineEffectHandle,
                    effectPos.x, effectPos.y, effectPos.z);

                float pitch = -m_pCamera->GetPitch();
                float yaw = m_pCamera->GetYaw();
                SetRotationPlayingEffekseer3DEffect(m_concentrationLineEffectHandle,
                    pitch, yaw, 0.0f);
            }
        }
}
