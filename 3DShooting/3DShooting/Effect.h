#pragma once
#include <vector>
#include "DxLib.h"

/// <summary>
/// エフェクトの読み込み・再生・更新・描画を管理するクラス。
/// Effekseer を使用した 3D エフェクトを一元管理する。
/// </summary>
class Effect
{
public:
    Effect();
    virtual ~Effect();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Init();

    /// <summary>
    /// 毎フレームの更新処理（ポーズ中はスキップ）
    /// </summary>
    void Update();

    /// <summary>
    /// 3D エフェクトを描画する
    /// </summary>
    void Draw();

    /// <summary>
    /// マズルフラッシュを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <param name="rotX">X軸回転</param>
    /// <param name="rotY">Y軸回転</param>
    /// <param name="rotZ">Z軸回転</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ);

    /// <summary>
    /// 出血エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <param name="rotX">X軸回転</param>
    /// <param name="rotY">Y軸回転</param>
    /// <param name="rotZ">Z軸回転</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ);

    /// <summary>
    /// 集中線エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayConcentrationLine(float x, float y, float z);

    /// <summary>
    /// ガードエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <param name="rotX">X軸回転</param>
    /// <param name="rotY">Y軸回転</param>
    /// <param name="rotZ">Z軸回転</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayGuardEffect(float x, float y, float z, float rotX, float rotY, float rotZ);

    /// <summary>
    /// スパークエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <param name="speed">再生速度（デフォルト 1.0）</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlaySparkEffect(float x, float y, float z, float speed = 1.0f);

    /// <summary>
    /// スパークエフェクト2を再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlaySparkEffect2(float x, float y, float z);

    /// <summary>
    /// 酸エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayAcidEffect(float x, float y, float z);

    /// <summary>
    /// 通常弾エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayNormalBulletEffect(float x, float y, float z);

    /// <summary>
    /// 近接範囲攻撃エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayCloseRangeAttackEffect(float x, float y, float z);

    /// <summary>
    /// ボスシールドエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayBossShieldEffect(float x, float y, float z);

    /// <summary>
    /// シールドヒットエフェクトを再生する
    /// </summary>
    /// <param name="pos">座標</param>
    /// <param name="normal">法線</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayShieldHitEffect(const VECTOR& pos, const VECTOR& normal);

    /// <summary>
    /// シールド破壊可能エフェクトを再生する
    /// </summary>
    /// <param name="pos">座標</param>
    /// <returns>再生中エフェクトハンドル。失敗時は -1</returns>
    int PlayShieldBreakEffect(const VECTOR& pos);

    /// <summary>
    /// 再生中のエフェクトをすべて停止する
    /// </summary>
    void StopAllEffects();

    /// <summary>
    /// ボスシールドエフェクトの最大再生フレーム数を返す
    /// </summary>
    /// <returns>最大フレーム数。ハンドルが無効な場合は 0</returns>
    int GetBossShieldEffectDuration() const;

private:
    static constexpr int kMuzzleFlashCount = 5;            // マズルフラッシュのバリエーション数

    int              m_muzzleFlashEffectHandles[kMuzzleFlashCount]; // マズルフラッシュのエフェクトハンドル配列
    int              m_lossOfBloodEffectHandle;            // 出血エフェクトハンドル
    int              m_concentrationLineEffectHandle;      // 集中線エフェクトハンドル
    int              m_guardEffectHandle;                  // ガードエフェクトハンドル
    int              m_sparkEffectHandle;                  // スパークエフェクトハンドル
    int              m_sparkEffectHandle2;                 // スパークエフェクトハンドル2
    int              m_acidEffectHandle;                   // 酸エフェクトハンドル
    int              m_normalBulletEffectHandle;           // 通常弾エフェクトハンドル
    int              m_closeRangeAttackEffectHandle;       // 近接範囲攻撃エフェクトハンドル
    int              m_bossShieldEffectHandle;             // ボスシールドエフェクトハンドル
    int              m_shieldHitEffectHandle;              // シールドヒットエフェクトハンドル
    int              m_shieldBreakEffectHandle;            // シールド破壊可能エフェクトハンドル

    std::vector<int> m_playingEffectHandles;               // 再生中のエフェクトハンドル
};
