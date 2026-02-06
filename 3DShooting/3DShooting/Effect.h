#pragma once
#include <vector>

/// <summary>
/// エフェクトクラス
/// </summary>
class Effect
{
public:
    Effect();
    virtual ~Effect();

    void Init();
    void Update();
    void Draw();

    /// <summary>
    /// マズルフラッシュを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ);

    int PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ);

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
    int PlayGuardEffect(float x, float y, float z, float rotX, float rotY, float rotZ);

    /// <summary>
    /// スパークエフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlaySparkEffect(float x, float y, float z);

    int PlaySparkEffect2(float x, float y, float z);

    /// <summary>
    /// 酸エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayAcidEffect(float x, float y, float z);

    /// <summary>
    /// 通常弾エフェクトを再生する
    /// </summary>
    /// <param name="x">X座標</param>
    /// <param name="y">Y座標</param>
    /// <param name="z">Z座標</param>
    int PlayNormalBulletEffect(float x, float y, float z);

    /// <summary>
    /// 再生中のエフェクトをすべて停止する
    /// </summary>
    void StopAllEffects();

private:
    int m_muzzleFlashEffectHandles[5]; // マズルフラッシュのエフェクトハンドル配列
    int m_lossOfBloodEffectHandle; // 出血エフェクトハンドル
    int m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
    int m_guardEffectHandle; // ガードエフェクトハンドル
    int m_sparkEffectHandle; // スパークエフェクトハンドル
    int m_sparkEffectHandle2; // スパークエフェクトハンドル2
    int m_acidEffectHandle; // 酸エフェクトハンドル
    int m_normalBulletEffectHandle; // 通常弾エフェクトハンドル

    std::vector<int> m_playingEffectHandles; // 再生中のエフェクトハンドル
};

