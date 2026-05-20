#pragma once
#include "DxLib.h"
#include <string>
#include <vector>

/// <summary>
/// CSV から読み込んだ 1 オブジェクト分のトランスフォームとゲームパラメータを保持する構造体。
/// 全フィールドにインクラス初期値を持ち、デフォルトコンストラクタで安全に初期化される。
/// </summary>
struct ObjectTransformData
{
    std::string name  = "";                          // オブジェクト名
    VECTOR      pos   = VGet(0.0f, 0.0f, 0.0f);     // 位置（DxLib 座標系）
    VECTOR      rot   = VGet(0.0f, 0.0f, 0.0f);     // 回転角度（度）
    VECTOR      scale = VGet(1.0f, 1.0f, 1.0f);     // スケール

    // キャラクター共通パラメータ
    float attack         = 0.0f; // 攻撃力
    float hp             = 0.0f; // 体力
    float speed          = 0.0f; // 歩行速度（プレイヤー用）
    float runSpeed       = 0.0f; // 走行速度（プレイヤー用）
    float chaseSpeed     = 0.0f; // 追跡速度（敵用）

    // タックル関連
    float tackleCooldown = 0.0f; // タックルのクールタイム
    float tackleSpeed    = 0.0f; // タックル中の移動速度
    float tackleDamage   = 0.0f; // タックルのダメージ量

    // 武器・弾薬関連
    float bulletPower    = 0.0f; // アサルトライフルの弾威力
    float sgBulletPower  = 0.0f; // ショットガンの弾威力
    int   arInitAmmo     = 0;    // アサルトライフルの初期弾薬数
    int   sgInitAmmo     = 0;    // ショットガンの初期弾薬数

    // 盾関連
    float maxShieldDurability = 0.0f; // 盾の最大耐久値
    float shieldRegenRate     = 0.0f; // 盾の回復速度
};

/// <summary>
/// CSV ファイルからオブジェクトのトランスフォームとゲームパラメータを読み込む純静的ユーティリティクラス。
/// Unity エクスポートの座標（cm 単位）を DxLib 座標（m 単位）へ変換して返す。
/// </summary>
class TransformDataLoader
{
public:
    /// <summary>
    /// 指定した CSV ファイルを読み込み、全行分の ObjectTransformData を返す。
    /// ファイルが開けない場合は空のベクターを返す。
    /// </summary>
    /// <param name="fileName">CSV ファイルパス</param>
    /// <returns>読み込んだ ObjectTransformData の配列</returns>
    static std::vector<ObjectTransformData> LoadDataCSV(const char* fileName);
};
