#pragma once
#include "DxLib.h"

/// <summary>
/// EnemyNormal と EnemyRunner で共通して使用する定数群。
/// 各エネミー固有の名前空間から using 宣言で参照することで、
/// 呼び出し側のコードを変更せずに重複定義を排除する。
/// </summary>
namespace EnemySharedConstants
{
    /// ヘッドショット判定コライダーの中心補正値
    inline const VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f };

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 20.0f;  // 体のカプセルコライダー半径
    constexpr float kBodyColliderHeight = 110.0f; // 体のカプセルコライダー高さ

    // 移動関連
    constexpr float kRotateSpeedPerFrame = 0.05f; // フレームあたりの旋回速度（ラジアン）

    // 徘徊関連
    constexpr int   kWanderTimerInterval = 120;    // 徘徊位置更新間隔（フレーム数）
    constexpr float kWanderMinDist       = 300.0f; // 徘徊時の最小距離
    constexpr int   kWanderDistRange     = 400;    // 徘徊時の距離のランダム幅

    // 当たり判定関連
    constexpr float kPushBackEpsilon = 0.0001f; // ゼロ除算防止のための最小距離の二乗閾値

    // 描画関連
    constexpr float kDrawDistanceSq     = 5000.0f * 5000.0f; // 最大描画距離の二乗
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;   // 常に描画する近距離の二乗
    constexpr float kDrawDotThreshold   = 0.4f;              // 視野内判定に使う内積閾値
}
