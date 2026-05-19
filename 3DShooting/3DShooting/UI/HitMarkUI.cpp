#include "HitMarkUI.h"
#include "Game.h"
#include <algorithm>

namespace
{
    // 表示タイマー・アニメーション
    constexpr float kDuration          = 20.0f; // ヒットマークの表示フレーム数
    constexpr float kTimerDecrement    =  1.0f; // 1フレームあたりのタイマー減算量

    // 距離減衰（遠距離ほど透明になる）
    constexpr float kMinDistance = 300.0f; // 減衰が始まる最小距離
    constexpr float kMaxDistance = 2000.0f; // 最大減衰距離（これ以上は最小不透明度）
    constexpr float kMaxRatio    =   1.0f; // 近距離時の不透明度係数
    constexpr float kMinRatio    =   0.2f; // 遠距離時の不透明度係数

    // 描画パラメータ
    constexpr int kAlphaMax         = 255; // アルファ最大値
    constexpr int kLineLength       =  12; // ヒットマークの線の長さ
    constexpr int kCenterSpacing    =   4; // 中心からの隙間幅
    constexpr int kLineThickness    =   2; // 線の太さ
    constexpr int kDoubleLineOffset =   2; // ヘッドショット二重線のオフセット

    // ヒット部位別カラー
    constexpr unsigned int kColorHeadShot = 0xffd700; // ヘッドショット時（金色）
    constexpr unsigned int kColorBodyShot = 0xff4500; // ボディショット時（オレンジ赤）
}

HitMarkUI::HitMarkUI()
    : m_timer(0.0f)
    , m_distance(0.0f)
    , m_type(EnemyBase::HitPart::Body)
{
}

void HitMarkUI::Init()
{
}

void HitMarkUI::Update(float deltaTime)
{
    if (m_timer > 0)
    {
        m_timer -= kTimerDecrement * Game::GetTimeScale();
    }
}

void HitMarkUI::Trigger(EnemyBase::HitPart type, float distance)
{
    m_type = type;
    m_distance = distance;
    m_timer = kDuration;
}

void HitMarkUI::Draw()
{
    if (m_timer <= 0) return;

    float ratio = 1.0f;
    if (m_distance > kMinDistance)
    {
        float t = (m_distance - kMinDistance) / (kMaxDistance - kMinDistance);
        t = (std::min)(t, 1.0f);
        ratio = kMaxRatio * (1.0f - t) + kMinRatio * t;
    }

    int alpha = static_cast<int>((kAlphaMax * m_timer * ratio) / kDuration);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    unsigned int color = (m_type == EnemyBase::HitPart::Head) ? kColorHeadShot : kColorBodyShot;

    float scale = Game::GetUIScale();
    int scaledLineLength    = static_cast<int>(kLineLength    * scale);
    int scaledCenterSpacing = static_cast<int>(kCenterSpacing * scale);
    int scaledThickness     = (std::max)(1, static_cast<int>(kLineThickness * scale));
    int centerX = static_cast<int>(Game::GetScreenWidth() * 0.5f);
    int centerY = static_cast<int>(Game::GetScreenHeight() * 0.5f);
    
    // クロスの4本線を描画
    DrawLine(centerX - scaledLineLength, centerY - scaledLineLength, centerX - scaledCenterSpacing, centerY - scaledCenterSpacing, color, scaledThickness);
    DrawLine(centerX + scaledCenterSpacing, centerY + scaledCenterSpacing, centerX + scaledLineLength, centerY + scaledLineLength, color, scaledThickness);
    DrawLine(centerX - scaledLineLength, centerY + scaledLineLength, centerX - scaledCenterSpacing, centerY + scaledCenterSpacing, color, scaledThickness);
    DrawLine(centerX + scaledCenterSpacing, centerY - scaledCenterSpacing, centerX + scaledLineLength, centerY - scaledLineLength, color, scaledThickness);

    // ヘッドショット二重線
    if (m_type == EnemyBase::HitPart::Head)
    {
        int offset = static_cast<int>(kDoubleLineOffset * scale);
        DrawLine(centerX - scaledLineLength - offset, centerY - scaledLineLength + offset, centerX - scaledCenterSpacing - offset, centerY - scaledCenterSpacing + offset, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing + offset, centerY + scaledCenterSpacing - offset, centerX + scaledLineLength + offset, centerY + scaledLineLength - offset, color, scaledThickness);
        DrawLine(centerX - scaledLineLength - offset, centerY + scaledLineLength - offset, centerX - scaledCenterSpacing - offset, centerY + scaledCenterSpacing - offset, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing + offset, centerY - scaledCenterSpacing + offset, centerX + scaledLineLength + offset, centerY - scaledLineLength + offset, color, scaledThickness);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
