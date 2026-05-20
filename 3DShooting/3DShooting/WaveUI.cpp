#include "WaveUI.h"
#include "DebugUtil.h"
#include "WaveManager.h"
#include "Game.h"
#include <algorithm>
#include <cstdio>

namespace
{
    constexpr int   kWaveImageDrawWidth          = 150;  // ウェーブ画像の最終表示幅（px）
    constexpr int   kWaveAnimDuration            = 45;   // 中央→上部スライドにかけるフレーム数
    constexpr int   kWaveAnimHoldDuration        = 30;   // スライド完了後のホールドフレーム数
    constexpr int   kWaveAnimInitialHoldDuration = 30;   // スライド開始前の初期ホールドフレーム数
    constexpr float kWaveAnimStartSizeRatio      = 0.4f; // 初期表示サイズ（画面幅に対する比率）
}

WaveUI::WaveUI(WaveManager* waveManager)
    : m_pWaveManager(waveManager)
    , m_waveImageAnimTimer(0)
    , m_waveImageAnimDuration(kWaveAnimDuration)
    , m_waveImageAnimHoldDuration(kWaveAnimHoldDuration)
    , m_waveImageAnimInitialHoldDuration(kWaveAnimInitialHoldDuration)
    , m_isWaveImageAnimating(false)
{
    for (int i = 0; i < 5; ++i) m_waveImages[i] = -1;
}

WaveUI::~WaveUI()
{
    for (int i = 0; i < 5; ++i)
    {
        if (m_waveImages[i] >= 0)
        {
            DeleteGraph(m_waveImages[i]);
            m_waveImages[i] = -1;
        }
    }
}

void WaveUI::Init()
{
    m_waveImages[0] = LoadGraph("data/image/wave1.png");
    m_waveImages[1] = LoadGraph("data/image/wave2.png");
    m_waveImages[2] = LoadGraph("data/image/wave3.png");
    m_waveImages[3] = LoadGraph("data/image/wave4.png");
    m_waveImages[4] = LoadGraph("data/image/wave5.png");
}

void WaveUI::Update(float deltaTime)
{
    // WaveManager が Starting 状態に遷移したらアニメーションを開始する
    if (m_pWaveManager &&
        m_pWaveManager->GetState() == WaveManager::WaveState::Starting &&
        !m_isWaveImageAnimating)
    {
        StartWaveAnimation();
    }

    if (m_isWaveImageAnimating)
    {
        m_waveImageAnimTimer += static_cast<int>(1.0f * Game::GetTimeScale());

        int totalDuration = m_waveImageAnimInitialHoldDuration
                          + m_waveImageAnimDuration
                          + m_waveImageAnimHoldDuration;
        if (m_waveImageAnimTimer >= totalDuration)
        {
            m_isWaveImageAnimating = false;
        }
    }
}

void WaveUI::StartWaveAnimation()
{
    m_isWaveImageAnimating = true;
    m_waveImageAnimTimer   = 0;
}

void WaveUI::Draw()
{
    if (!m_pWaveManager) return;

    int  currentWave        = m_pWaveManager->GetCurrentWave();
    bool isWaveActive       = m_pWaveManager->IsWaveActive();
    bool isAllWavesCompleted = m_pWaveManager->IsAllWavesCompleted();

    if (isAllWavesCompleted || currentWave < 1 || currentWave > 5) return;
    if (!m_isWaveImageAnimating && !isWaveActive)                  return;

    int img = m_waveImages[currentWave - 1];
    if (img == -1) return;

    int imgW = 0, imgH = 0;
    GetGraphSize(img, &imgW, &imgH);
    int screenW = 0, screenH = 0;
    GetScreenState(&screenW, &screenH, NULL);

    // 最終位置・サイズ（画面上部中央の小サイズ）
    int targetDrawW = kWaveImageDrawWidth;
    int targetDrawH = imgH * targetDrawW / imgW;
    int targetX     = static_cast<int>((screenW - targetDrawW) * 0.5f);
    int targetY     = 0;

    // 初期位置・サイズ（画面中央の大サイズ）
    int startDrawW = static_cast<int>(screenW * kWaveAnimStartSizeRatio);
    int startDrawH = imgH * startDrawW / imgW;
    int startX     = static_cast<int>((screenW - startDrawW) * 0.5f);
    int startY     = static_cast<int>((screenH - startDrawH) * 0.5f);

    int currentX, currentY, currentDrawW, currentDrawH;

    if (m_isWaveImageAnimating)
    {
        float t;
        if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration)
        {
            t = 0.0f; // 初期ホールド中は補間を 0 に固定
        }
        else if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration)
        {
            t = static_cast<float>(m_waveImageAnimTimer - m_waveImageAnimInitialHoldDuration)
              / static_cast<float>(m_waveImageAnimDuration);
        }
        else
        {
            t = 1.0f; // ホールド中は補間を終端に固定
        }
        t = (std::min)(1.0f, t);

        currentX     = static_cast<int>(startX     + (targetX     - startX)     * t);
        currentY     = static_cast<int>(startY     + (targetY     - startY)     * t);
        currentDrawW = static_cast<int>(startDrawW + (targetDrawW - startDrawW) * t);
        currentDrawH = static_cast<int>(startDrawH + (targetDrawH - startDrawH) * t);
    }
    else
    {
        currentX     = targetX;
        currentY     = targetY;
        currentDrawW = targetDrawW;
        currentDrawH = targetDrawH;
    }

    DrawExtendGraph(currentX, currentY, currentX + currentDrawW, currentY + currentDrawH, img, true);
}

void WaveUI::DrawDebugSpawnAreas(const std::vector<SpawnAreaInfo>& spawnAreaList, bool isTutorial)
{
    for (const auto& area : spawnAreaList)
    {
        // 現在のモード（Tutorial / Main）と一致するエリアのみ描画
        if ((isTutorial && area.type == 1) || (!isTutorial && area.type == 0))
        {
            // 高さ層タイプによってワイヤーフレーム色を変える
            unsigned int color;
            if (area.type == 0)
            {
                if      (std::abs(area.center.y - 200.0f) < 10.0f) color = 0x00ff00; // 緑（下段）
                else if (std::abs(area.center.y - 562.0f) < 10.0f) color = 0xffff00; // 黄（中段）
                else if (std::abs(area.center.y - 962.0f) < 10.0f) color = 0xff0000; // 赤（上段）
                else                                               color = 0x00ffff; // シアン（その他）
            }
            else
            {
                color = 0xff00ff; // マゼンタ（チュートリアル）
            }

            // 直方体の AABB をワイヤーフレームで描画する
            VECTOR minPos = VSub(area.center, VScale(area.size, 0.5f));
            VECTOR maxPos = VAdd(area.center, VScale(area.size, 0.5f));

            // 上面
            DrawLine3D(VGet(minPos.x, maxPos.y, minPos.z), VGet(maxPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, maxPos.y, minPos.z), VGet(maxPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(maxPos.x, maxPos.y, maxPos.z), VGet(minPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, maxPos.y, maxPos.z), VGet(minPos.x, maxPos.y, minPos.z), color);

            // 下面
            DrawLine3D(VGet(minPos.x, minPos.y, minPos.z), VGet(maxPos.x, minPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, minPos.z), VGet(maxPos.x, minPos.y, maxPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, maxPos.z), VGet(minPos.x, minPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, minPos.y, maxPos.z), VGet(minPos.x, minPos.y, minPos.z), color);

            // 縦辺
            DrawLine3D(VGet(minPos.x, minPos.y, minPos.z), VGet(minPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, minPos.z), VGet(maxPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, maxPos.z), VGet(maxPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, minPos.y, maxPos.z), VGet(minPos.x, maxPos.y, maxPos.z), color);
        }
    }
}
