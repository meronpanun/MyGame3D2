#pragma once
#include "StageObject.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// <summary>
/// ステージのモデル描画・当たり判定データ・バウンディングボックスを管理するクラス。
/// CSV ファイルからオブジェクトのトランスフォームと衝突三角形データを読み込む。
/// </summary>
class Stage
{
public:
    /// <summary>
    /// ステージ当たり判定の三角形データ（CSV 1行分）
    /// </summary>
    struct StageCollisionData
    {
        std::string name; // オブジェクト名
        VECTOR      v1;   // 頂点1
        VECTOR      v2;   // 頂点2
        VECTOR      v3;   // 頂点3

        StageCollisionData()
            : name("")
            , v1(VGet(0.0f, 0.0f, 0.0f))
            , v2(VGet(0.0f, 0.0f, 0.0f))
            , v3(VGet(0.0f, 0.0f, 0.0f))
        {}
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    Stage();

    /// <summary>
    /// デストラクタ。モデルキャッシュを解放する。
    /// </summary>
    ~Stage();

    /// <summary>
    /// ステージをロードする
    /// </summary>
    /// <param name="isTutorial">true でチュートリアルステージ、false でメインステージ</param>
    void LoadStage(bool isTutorial);

    /// <summary>
    /// ロード済みオブジェクトと当たり判定データをクリアする
    /// </summary>
    void Clear();

    /// <summary>
    /// ステージオブジェクトを描画する。デバッグフラグが有効な場合は当たり判定も表示する。
    /// </summary>
    void Draw();

    /// <summary>
    /// 当たり判定三角形データのリストを返す
    /// </summary>
    /// <returns>StageCollisionData の配列</returns>
    const std::vector<StageCollisionData>& GetCollisionData() const { return m_collisionData; }

    /// <summary>
    /// ステージの AABB 最小座標を返す
    /// </summary>
    /// <returns>バウンディングボックスの最小頂点</returns>
    VECTOR GetMinBounds() const { return m_minBounds; }

    /// <summary>
    /// ステージの AABB 最大座標を返す
    /// </summary>
    /// <returns>バウンディングボックスの最大頂点</returns>
    VECTOR GetMaxBounds() const { return m_maxBounds; }

    /// <summary>当たり判定のデバッグ描画フラグを設定する</summary>
    static void SetDrawCollision(bool isDraw)         { s_shouldDrawCollision = isDraw; }
    /// <summary>当たり判定のデバッグ描画フラグを返す</summary>
    static bool ShouldDrawCollision()                 { return s_shouldDrawCollision; }
    /// <summary>チュートリアル当たり判定のデバッグ描画フラグを設定する</summary>
    static void SetDrawTutorialCollision(bool isDraw) { s_shouldDrawTutorialCollision = isDraw; }
    /// <summary>チュートリアル当たり判定のデバッグ描画フラグを返す</summary>
    static bool ShouldDrawTutorialCollision()         { return s_shouldDrawTutorialCollision; }

private:
    /// <summary>
    /// CSV ファイルから当たり判定三角形データを読み込む
    /// </summary>
    /// <param name="fileName">CSV ファイルパス</param>
    void LoadCollisionData(const char* fileName);

    /// <summary>
    /// 全オブジェクトの位置からステージの AABB を計算する
    /// </summary>
    void CalculateBounds();

    std::vector<StageObject>              m_objects;       // ステージオブジェクトリスト
    std::vector<StageCollisionData>       m_collisionData; // 当たり判定三角形データ
    std::unordered_map<std::string, int>  m_modelCache;    // モデルパスとハンドルのキャッシュ

    static bool s_shouldDrawCollision;         // 当たり判定デバッグ描画フラグ
    static bool s_shouldDrawTutorialCollision; // チュートリアル当たり判定デバッグ描画フラグ

    bool   m_isTutorial; // チュートリアルステージかどうか
    VECTOR m_minBounds;  // ステージ AABB の最小座標
    VECTOR m_maxBounds;  // ステージ AABB の最大座標
};
