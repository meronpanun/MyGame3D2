#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

class Camera;

/// <summary>
/// プレイヤークラス
/// </summary>
class Player
{
public:
	Player();
	virtual ~Player();

	void Init();
	void Update();
	void Draw();

	/// <summary>
	/// カメラへのアクセスを提供
	/// </summary>
	/// <returns>カメラの共有ポインタ</returns>
	std::shared_ptr<Camera> GetCamera() const { return m_pCamera; }

	/// <summary>
	/// フィールドの描画
	/// </summary>
	void DrawField();

private:
	/// <summary>
	/// 弾を発射
	/// </summary>
	void Shoot();

	/// <summary>
	/// アニメーション状態管理構造体
	/// </summary>
	struct AnimData
	{
		int   attachNo = -1;    // アタッチ番号
		float count    = 0.0f;  // アニメカウント
		bool  isLoop   = false; // アニメーションが終わった時ループするか、true:ループする false:最後のフレームで停止
		bool  isEnd    = false; // アニメーションが終了したか
	};

	/// <summary>
	/// アニメーションをアタッチ
	/// </summary>
	/// <param name="data">アニメーションデータ</param>
	/// <param name="animName">アニメーション名</param>
	/// <param name="isLoop">ループするか</param>
	void AttachAnime(AnimData& data, const char* animName, bool isLoop);

	/// <summary>
	/// アニメーションを更新
	/// </summary>
	/// <param name="data">アニメーションデータ</param>
	void UpdateAnime(AnimData& data);

	/// <summary>
	/// アニメーションブレンド率の更新
	/// </summary>
	void UpdateAnimeBlend();

	/// <summary>
	/// アニメーションを変更
	/// </summary>
	/// <param name="animName">アニメーション名</param>
	/// <param name="isLoop">ループするか</param>
	void ChangeAnime(const char* animName, bool isLoop);

	/// <summary>
	/// 銃の位置と回転を取得
	/// </summary>
	VECTOR GetGunPos() const;
	VECTOR GetGunRot() const;

private:
	// カメラの管理
	std::shared_ptr<Camera> m_pCamera;

	// モデルの位置
	VECTOR m_modelPos;

	/* アニメーション再生に必要なデータをまとめたもの */
	AnimData m_nextAnimData; // 最後に設定したアニメ情報
	AnimData m_prevAnimData; // 一つ前に設定したアニメ情報

	// アニメーションブレンド率
	float m_animBlendRate;  // 0.0f:前のアニメーション 1.0f:次のアニメーション
	int   m_modelHandle;    // モデルのハンドル
	int   m_shootSEHandle;  // 弾を撃つSEのハンドル
	bool  m_isMoving;       // 移動中かどうか
	bool  m_isWasRunning;   // 走っていたかどうか

	float m_stamina; // スタミナ
	bool  m_isCanRun; // 走れるかどうか
};

