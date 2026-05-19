#pragma once
#include "SceneBase.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"
#include "SafeHandle.h"
#include <vector>
#include <memory>

/// <summary>
/// 背景ゾンビ一体分の描画用データ
/// </summary>
struct ZombieData
{
    VECTOR pos;          // 位置
    float angleY;        // Y軸回転
    float animTime;      // 現在のアニメーション再生時間
    float animSpeed;     // アニメーション再生速度
    int animIndex;       // 再生するアニメーションのインデックス
    float totalAnimTime; // アニメーションの総再生時間
};

/// <summary>
/// タイトルシーンクラス。3D背景とゾンビ群を描画し、クリック待機でメインシーンへ遷移する
/// </summary>
class SceneTitle : public SceneBase
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="skipLogo">タイトルロゴのフェードイン演出をスキップするならtrue</param>
    SceneTitle(bool skipLogo = false);

    virtual ~SceneTitle() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Init() override;

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <returns>遷移先シーンのポインタ（遷移しない場合はthisを返す）</returns>
    SceneBase* Update() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() override;

private:
    // リソース管理
    ManagedFont  m_font;       // フォント
    ManagedGraph m_titleLogo;  // タイトルロゴ画像
    ManagedGraph m_banner;     // バナー（看板テキスト）画像
    int m_zombieVoiceTimer;    // ゾンビ環境ボイスの再生間隔タイマー

    // フェード・シーン遷移管理
    int  m_fadeAlpha;      // タイトルロゴのフェードアルファ値
    int  m_fadeFrame;      // フェードのフレームカウント
    int  m_sceneFadeAlpha; // シーンフェードのアルファ値
    bool m_isFadeComplete; // タイトルロゴのフェードイン完了フラグ
    bool m_isFadeOut;      // フェードアウト中フラグ
    bool m_isSceneFadeIn;  // シーンフェードイン中フラグ

    // 演出管理
    int m_waitFrame; // フェードイン後の待機フレームカウント

    // 「ゲームスタート」テキスト点滅
    int m_gameStartTextAlpha;    // テキストのアルファ値
    int m_gameStartTextAlphaDir; // アルファ値の増減方向（+1 or -1）

    // 看板の揺れ演出
    float m_billboardShakeOffsetX; // 看板のX軸揺れオフセット
    float m_billboardShakeOffsetY; // 看板のY軸揺れオフセット
    float m_billboardShakePower;   // 看板の揺れの強さ（フレームごとに減衰）
    int   m_shakeTimer;            // 揺れ計算用タイマー

    // フェンスの揺れ演出（画面内の左:0・中央:1・右:2 の3枚分）
    float m_fenceShakePower[3];

    // 3D背景モデル
    SafeHandle<ModelDeleter> m_skyDome;               // スカイドーム
    SafeHandle<ModelDeleter> m_floorModel;            // 床
    SafeHandle<ModelDeleter> m_fenceModel;            // フェンス
    SafeHandle<ModelDeleter> m_armoryBillboardModel;  // Armory看板
    SafeHandle<ModelDeleter> m_hangarV3Model;         // HangarV3（白い建物）
    SafeHandle<ModelDeleter> m_hangarModel;           // Hangar（オレンジの建物）
    SafeHandle<ModelDeleter> m_containerModel;        // コンテナ
    SafeHandle<ModelDeleter> m_zombieModel;           // ゾンビ（共有モデル）
    std::vector<ZombieData>  m_zombies;               // 背景ゾンビのリスト

    int m_animIndexIdle;    // IDLEアニメーションのインデックス
    int m_animIndexAtkHead; // ATK_HEADアニメーションのインデックス
};
