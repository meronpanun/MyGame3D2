#include "Effect.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include <algorithm>
#include <assert.h>
#include <string>
#include <time.h>
#include <vector>

namespace {
// 各エフェクト拡大率
constexpr float kMuzzleFlashEffectScale = 2.0f;
constexpr float kMuzzleFlashEffectScale2 = 3.0f;
constexpr float kMuzzleFlashEffectScale3 = 3.5f;
constexpr float kMuzzleFlashEffectScale4 = 2.5f;
constexpr float kLossOfBloodEffectScale = 2.5f;
constexpr float kConcentrationLineEffectScale = 20.0f;
constexpr float kGuardEffectScale = 10.5f;
constexpr float kSparkEffectScale = 20.0f;

// エフェクトのカリング距離 (これ以上離れたら再生しない)
constexpr float kEffectCullDistance = 3000.0f;

bool ShouldPlayEffect(float x, float y, float z) {
  VECTOR cameraPos = GetCameraPosition();
  VECTOR effectPos = VGet(x, y, z);
  VECTOR diff = VSub(effectPos, cameraPos);
  return VSquareSize(diff) <= kEffectCullDistance * kEffectCullDistance;
}
} // namespace

Effect::Effect()
    : m_lossOfBloodEffectHandle(-1), m_concentrationLineEffectHandle(-1),
      m_muzzleFlashEffectHandles{-1, -1, -1, -1, -1} {
  // 乱数のシードを設定
  srand(time(NULL));

  // エフェクトハンドルの読み込み
  m_muzzleFlashEffectHandles[0] = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash.efkefc", kMuzzleFlashEffectScale);
  m_muzzleFlashEffectHandles[1] = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash2.efkefc", kMuzzleFlashEffectScale);
  m_muzzleFlashEffectHandles[2] = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash3.efkefc", kMuzzleFlashEffectScale);
  m_muzzleFlashEffectHandles[3] = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash4.efkefc", kMuzzleFlashEffectScale3);
  m_muzzleFlashEffectHandles[4] = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash5.efkefc", kMuzzleFlashEffectScale2);
  for (int i = 0; i < 5; ++i) {
    assert(m_muzzleFlashEffectHandles[i] != -1);
  }

  // 出血エフェクトハンドルの読み込み
  m_lossOfBloodEffectHandle = LoadEffekseerEffect(
      "data/Effekseer/LossOfBlood.efkefc", kLossOfBloodEffectScale);
  assert(m_lossOfBloodEffectHandle != -1);
  // 集中線エフェクトハンドルの読み込み
  m_concentrationLineEffectHandle = LoadEffekseerEffect(
      "data/Effekseer/ConcentrationLine.efkefc", kConcentrationLineEffectScale);
  assert(m_concentrationLineEffectHandle != -1);

  // ガードエフェクトハンドルの読み込み
  m_guardEffectHandle =
      LoadEffekseerEffect("data/Effekseer/Circle.efkefc", kGuardEffectScale);
  assert(m_guardEffectHandle != -1);

  // スパークエフェクトハンドルの読み込み
  m_sparkEffectHandle =
      LoadEffekseerEffect("data/Effekseer/Spark.efkefc", kSparkEffectScale);
  assert(m_sparkEffectHandle != -1);
  m_sparkEffectHandle2 = LoadEffekseerEffect(
      "data/Effekseer/MuzzleFlash5.efkefc", kMuzzleFlashEffectScale4);
  assert(m_sparkEffectHandle2 != -1);

  // 酸エフェクトハンドルの読み込み
  m_acidEffectHandle =
      LoadEffekseerEffect("data/Effekseer/ParryBullet.efkefc", 15.0f);
  assert(m_acidEffectHandle != -1);

  // 通常弾エフェクトハンドルの読み込み
  m_normalBulletEffectHandle =
      LoadEffekseerEffect("data/Effekseer/NormalBullet.efkefc", 15.0f);
  assert(m_normalBulletEffectHandle != -1);
}

Effect::~Effect() {
  // エフェクトのハンドルを削除
  for (int i = 0; i < 5; ++i) {
    DeleteEffekseerEffect(m_muzzleFlashEffectHandles[i]);
  }
  DeleteEffekseerEffect(m_lossOfBloodEffectHandle);
  DeleteEffekseerEffect(m_concentrationLineEffectHandle);
  DeleteEffekseerEffect(m_guardEffectHandle);
  DeleteEffekseerEffect(m_sparkEffectHandle);
  DeleteEffekseerEffect(m_sparkEffectHandle2);
  DeleteEffekseerEffect(m_acidEffectHandle);
  DeleteEffekseerEffect(m_normalBulletEffectHandle);
}

void Effect::Init() {}

void Effect::Update() {
  // ゲームが一時停止中はエフェクトの更新を行わない
  if (Game::IsPaused())
    return;

  // 3Dエフェクトの更新
  UpdateEffekseer3D();
}

void Effect::Draw() {
  // 3Dエフェクトの描画
  DrawEffekseer3D();
}

// マズルフラッシュを再生する
int Effect::PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY,
                            float rotZ) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  int index = rand() % 5;
  if (m_muzzleFlashEffectHandles[index] != -1) {
    int handle = PlayEffekseer3DEffect(m_muzzleFlashEffectHandles[index]);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// 出血エフェクトを再生する
int Effect::PlayLossOfBlood(float x, float y, float z, float rotX, float rotY,
                            float rotZ) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_lossOfBloodEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_lossOfBloodEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
      SetSpeedPlayingEffekseer3DEffect(handle, 5.0f); // 再生速度を5倍に
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// 集中線エフェクトを再生する
int Effect::PlayConcentrationLine(float x, float y, float z) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_concentrationLineEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_concentrationLineEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// ガードエフェクトを再生する
int Effect::PlayGuardEffect(float x, float y, float z, float rotX, float rotY,
                            float rotZ) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_guardEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_guardEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      SetRotationPlayingEffekseer3DEffect(handle, rotX, rotY, rotZ);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// スパークエフェクトを再生する
int Effect::PlaySparkEffect(float x, float y, float z) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_sparkEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_sparkEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// スパークエフェクト2を再生する
int Effect::PlaySparkEffect2(float x, float y, float z) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_sparkEffectHandle2 != -1) {
    int handle = PlayEffekseer3DEffect(m_sparkEffectHandle2);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      SetScalePlayingEffekseer3DEffect(handle, kMuzzleFlashEffectScale4,
                                       kMuzzleFlashEffectScale4,
                                       kMuzzleFlashEffectScale4);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// 酸エフェクトを再生する
int Effect::PlayAcidEffect(float x, float y, float z) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_acidEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_acidEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

// 通常弾エフェクトを再生する
int Effect::PlayNormalBulletEffect(float x, float y, float z) {
  if (!ShouldPlayEffect(x, y, z))
    return -1;

  if (m_normalBulletEffectHandle != -1) {
    int handle = PlayEffekseer3DEffect(m_normalBulletEffectHandle);
    if (handle != -1) {
      SetPosPlayingEffekseer3DEffect(handle, x, y, z);
      m_playingEffectHandles.push_back(handle);
    }
    return handle;
  }
  return -1;
}

void Effect::StopAllEffects() {
  for (int handle : m_playingEffectHandles) {
    StopEffekseer3DEffect(handle);
  }
  m_playingEffectHandles.clear();
}
