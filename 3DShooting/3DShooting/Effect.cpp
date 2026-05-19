#include "Effect.h"
#include "DxLib.h"
#include "EffekseerWarningSuppress.h"
#include "Game.h"
#include <algorithm>
#include <assert.h>
#include <string>
#include <time.h>
#include <vector>
#include <cmath>

namespace
{
    // 各エフェクト倍率
    constexpr float kMuzzleFlashEffectScale       = 2.0f;
    constexpr float kMuzzleFlashEffectScale2      = 3.0f;
    constexpr float kMuzzleFlashEffectScale3      = 3.5f;
    constexpr float kMuzzleFlashEffectScale4      = 2.5f;
    constexpr float kLossOfBloodEffectScale       = 2.5f;
    constexpr float kConcentrationLineEffectScale = 20.0f;
    constexpr float kGuardEffectScale             = 10.5f;
    constexpr float kSparkEffectScale             = 20.0f;
    constexpr float kBossShieldEffectScale        = 30.0f;
    constexpr float kAcidEffectScale              = 15.0f;
    constexpr float kNormalBulletEffectScale      = 15.0f;
    constexpr float kCloseRangeAttackScale        = 50.0f;
    constexpr float kShieldHitEffectScale         = 5.0f;
    constexpr float kShieldHitDisplayScale        = 2.0f;
    constexpr float kShieldBreakEffectScale       = 50.0f;
    constexpr float kLossOfBloodSpeed             = 5.0f;

    // エフェクトのカリング距離（これ以上離れたら再生しない）
    constexpr float kEffectCullDistance = 3000.0f;

    bool ShouldPlayEffect(float x, float y, float z)
    {
        VECTOR cameraPos = GetCameraPosition();
        VECTOR effectPos = VGet(x, y, z);
        VECTOR diff      = VSub(effectPos, cameraPos);
        return VSquareSize(diff) <= kEffectCullDistance * kEffectCullDistance;
    }
}

Effect::Effect()
    : m_lossOfBloodEffectHandle(-1)
    , m_concentrationLineEffectHandle(-1)
    , m_closeRangeAttackEffectHandle(-1)
    , m_bossShieldEffectHandle(-1)
    , m_shieldHitEffectHandle(-1)
    , m_shieldBreakEffectHandle(-1)
    , m_muzzleFlashEffectHandles{ -1, -1, -1, -1, -1 }
{
    srand(static_cast<unsigned int>(time(NULL)));

    m_muzzleFlashEffectHandles[0] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash.efkefc",  kMuzzleFlashEffectScale);
    m_muzzleFlashEffectHandles[1] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash2.efkefc", kMuzzleFlashEffectScale);
    m_muzzleFlashEffectHandles[2] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash3.efkefc", kMuzzleFlashEffectScale);
    m_muzzleFlashEffectHandles[3] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash4.efkefc", kMuzzleFlashEffectScale3);
    m_muzzleFlashEffectHandles[4] = LoadEffekseerEffect("data/Effekseer/MuzzleFlash5.efkefc", kMuzzleFlashEffectScale2);
    for (int i = 0; i < kMuzzleFlashCount; ++i)
    {
        assert(m_muzzleFlashEffectHandles[i] != -1);
    }

    m_lossOfBloodEffectHandle       = LoadEffekseerEffect("data/Effekseer/LossOfBlood.efkefc",       kLossOfBloodEffectScale);
    assert(m_lossOfBloodEffectHandle != -1);
    m_concentrationLineEffectHandle = LoadEffekseerEffect("data/Effekseer/ConcentrationLine.efkefc", kConcentrationLineEffectScale);
    assert(m_concentrationLineEffectHandle != -1);
    m_guardEffectHandle             = LoadEffekseerEffect("data/Effekseer/Circle.efkefc",             kGuardEffectScale);
    assert(m_guardEffectHandle != -1);
    m_sparkEffectHandle             = LoadEffekseerEffect("data/Effekseer/Spark.efkefc",              kSparkEffectScale);
    assert(m_sparkEffectHandle != -1);
    m_sparkEffectHandle2            = LoadEffekseerEffect("data/Effekseer/MuzzleFlash5.efkefc",       kMuzzleFlashEffectScale4);
    assert(m_sparkEffectHandle2 != -1);
    m_acidEffectHandle              = LoadEffekseerEffect("data/Effekseer/ParryBullet.efkefc",        kAcidEffectScale);
    assert(m_acidEffectHandle != -1);
    m_normalBulletEffectHandle      = LoadEffekseerEffect("data/Effekseer/NormalBullet.efkefc",       kNormalBulletEffectScale);
    assert(m_normalBulletEffectHandle != -1);
    m_closeRangeAttackEffectHandle  = LoadEffekseerEffect("data/Effekseer/CloseRangeAttack.efkefc",   kCloseRangeAttackScale);
    assert(m_closeRangeAttackEffectHandle != -1);
    m_bossShieldEffectHandle        = LoadEffekseerEffect("data/Effekseer/Shield.efkefc",             kBossShieldEffectScale);
    assert(m_bossShieldEffectHandle != -1);
    m_shieldHitEffectHandle         = LoadEffekseerEffect("data/Effekseer/HitBurst.efkefc",           kShieldHitEffectScale);
    assert(m_shieldHitEffectHandle != -1);
    m_shieldBreakEffectHandle       = LoadEffekseerEffect("data/Effekseer/ShieldBreak.efkefc",        kShieldBreakEffectScale);
    assert(m_shieldBreakEffectHandle != -1);
}

Effect::~Effect()
{
    for (int i = 0; i < kMuzzleFlashCount; ++i)
    {
        DeleteEffekseerEffect(m_muzzleFlashEffectHandles[i]);
    }
    DeleteEffekseerEffect(m_lossOfBloodEffectHandle);
    DeleteEffekseerEffect(m_concentrationLineEffectHandle);
    DeleteEffekseerEffect(m_guardEffectHandle);
    DeleteEffekseerEffect(m_sparkEffectHandle);
    DeleteEffekseerEffect(m_sparkEffectHandle2);
    DeleteEffekseerEffect(m_acidEffectHandle);
    DeleteEffekseerEffect(m_normalBulletEffectHandle);
    DeleteEffekseerEffect(m_closeRangeAttackEffectHandle);
    DeleteEffekseerEffect(m_bossShieldEffectHandle);
    DeleteEffekseerEffect(m_shieldHitEffectHandle);
}

void Effect::Init()
{
}

void Effect::Update()
{
    if (Game::IsPaused()) return;

    UpdateEffekseer3D();
}

void Effect::Draw()
{
    DrawEffekseer3D();
}

int Effect::PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    int index = rand() % kMuzzleFlashCount;
    if (m_muzzleFlashEffectHandles[index] != -1)
    {
        int handle = PlayEffekseer3DEffect(m_muzzleFlashEffectHandles[index]);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_lossOfBloodEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_lossOfBloodEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
            SetSpeedPlayingEffekseer3DEffect(handle, kLossOfBloodSpeed);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayConcentrationLine(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_concentrationLineEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_concentrationLineEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayGuardEffect(float x, float y, float z, float rotX, float rotY, float rotZ)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_guardEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_guardEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlaySparkEffect(float x, float y, float z, float speed)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_sparkEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_sparkEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            if (speed != 1.0f)
            {
                SetSpeedPlayingEffekseer3DEffect(handle, speed);
            }
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlaySparkEffect2(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_sparkEffectHandle2 != -1)
    {
        int handle = PlayEffekseer3DEffect(m_sparkEffectHandle2);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            SetScalePlayingEffekseer3DEffect(handle,
                kMuzzleFlashEffectScale4,
                kMuzzleFlashEffectScale4,
                kMuzzleFlashEffectScale4);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayAcidEffect(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_acidEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_acidEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayNormalBulletEffect(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_normalBulletEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_normalBulletEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

void Effect::StopAllEffects()
{
    for (int handle : m_playingEffectHandles)
    {
        StopEffekseer3DEffect(handle);
    }
    m_playingEffectHandles.clear();
}

int Effect::PlayCloseRangeAttackEffect(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_closeRangeAttackEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_closeRangeAttackEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayBossShieldEffect(float x, float y, float z)
{
    if (!ShouldPlayEffect(x, y, z)) return -1;

    if (m_bossShieldEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_bossShieldEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, x, y, z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::GetBossShieldEffectDuration() const
{
    if (m_bossShieldEffectHandle == -1) return 0;

    auto effectRef = GetEffekseerEffect(m_bossShieldEffectHandle);
    if (effectRef == nullptr) return 0;

    return effectRef->CalculateTerm().TermMax;
}

int Effect::PlayShieldHitEffect(const VECTOR& pos, const VECTOR& normal)
{
    if (!ShouldPlayEffect(pos.x, pos.y, pos.z)) return -1;

    if (m_shieldHitEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_shieldHitEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);

            // 法線から回転角を計算
            float hDist = sqrtf(normal.x * normal.x + normal.z * normal.z);
            float rotY  = atan2f(normal.x, normal.z);  // Yaw（Y軸回転）
            float rotX  = -atan2f(normal.y, hDist);    // Pitch（X軸回転）

            SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, 0.0f);
            SetScalePlayingEffekseer3DEffect(handle,
                kShieldHitDisplayScale,
                kShieldHitDisplayScale,
                kShieldHitDisplayScale);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}

int Effect::PlayShieldBreakEffect(const VECTOR& pos)
{
    if (!ShouldPlayEffect(pos.x, pos.y, pos.z)) return -1;

    if (m_shieldBreakEffectHandle != -1)
    {
        int handle = PlayEffekseer3DEffect(m_shieldBreakEffectHandle);
        if (handle != -1)
        {
            SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
            m_playingEffectHandles.push_back(handle);
        }
        return handle;
    }
    return -1;
}
