#pragma once

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
	void PlayMuzzleFlash(float x, float y, float z, float rotX, float rotY, float rotZ);

	void PlayLossOfBlood(float x, float y, float z, float rotX, float rotY, float rotZ);

	int PlayConcentrationLine(float x, float y, float z, float scale);

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

	/// <summary>
	/// 酸エフェクトを再生する
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="y">Y座標</param>
	/// <param name="z">Z座標</param>
	int PlayAcidEffect(float x, float y, float z);

private:
	int m_muzzleFlashEffectHandles[5]; // マズルフラッシュのエフェクトハンドル配列
	int m_lossOfBloodEffectHandle; // 出血エフェクトハンドル
	int m_concentrationLineEffectHandle; // 集中線エフェクトハンドル
	int m_guardEffectHandle; // ガードエフェクトハンドル
	int m_sparkEffectHandle; // スパークエフェクトハンドル
	int m_acidEffectHandle; // 酸エフェクトハンドル
};

