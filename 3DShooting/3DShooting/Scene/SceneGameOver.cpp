#include "SceneGameOver.h"
#include "EffekseerForDXLib.h"
#include "SceneTitle.h"
#include "SceneMain.h"
#include "Mouse.h"
#include <cassert>

namespace
{
    constexpr int kButtonWidth = 220;
    constexpr int kButtonHeight = 60;
    constexpr int kButtonSpacing = 40;
    constexpr float kScrollSpeed = 1.0f; // 背景のスクロール速度
    constexpr int kBgImageSize = 1024;   // 背景画像のサイズ
}

SceneGameOver::SceneGameOver(int wave, int killCount, int score) : 
    m_wave(wave),
    m_killCount(killCount),
    m_score(score), 
    m_bgmHandle(-1), 
    m_bgmStarted(false), 
    m_backgroundHandle(-1), 
    m_scrollX(0.0f), 
    m_scrollY(0.0f)
{
    // BGMのロード
    m_bgmHandle = LoadSoundMem("data/sound/BGM/GameOverBGM.mp3");
    assert(m_bgmHandle != -1);
    m_returnSEHandle = LoadSoundMem("data/sound/SE/ButtonReturn.mp3");
    assert(m_returnSEHandle != -1);

    // 背景画像のロード
    m_backgroundHandle = LoadGraph("data/image/BackGrand.png");
    assert(m_backgroundHandle != -1);
}

SceneGameOver::~SceneGameOver()
{
    // BGMの解放
    DeleteSoundMem(m_bgmHandle);
    DeleteSoundMem(m_returnSEHandle);
    // 背景画像の解放
    DeleteGraph(m_backgroundHandle);
}

void SceneGameOver::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);
    m_bgmStarted = false;
    // BGM再生（既に再生中でなければ）
    if (CheckSoundMem(m_bgmHandle) == 0)
    {
        PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
        m_bgmStarted = true;
    }
}

SceneBase* SceneGameOver::Update()
{
    // 背景をスクロール
    m_scrollX += kScrollSpeed;
    m_scrollY += kScrollSpeed;
    if (m_scrollX > kBgImageSize) m_scrollX -= kBgImageSize;
    if (m_scrollY > kBgImageSize) m_scrollY -= kBgImageSize;

    if (Mouse::IsTriggerLeft())
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW / 2;
        int baseY = screenH / 2 + 180;
        // タイトルに戻るボタン
        int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing/2;
        int titleBtnY1 = baseY;
        int titleBtnX2 = centerX - kButtonSpacing/2;
        int titleBtnY2 = baseY + kButtonHeight;
        // リトライボタン
        int retryBtnX1 = centerX + kButtonSpacing/2;
        int retryBtnY1 = baseY;
        int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing/2;
        int retryBtnY2 = baseY + kButtonHeight;
        Vec2 mousePos = Mouse::GetPos();
        if (mousePos.x >= titleBtnX1 && mousePos.x <= titleBtnX2 && mousePos.y >= titleBtnY1 && mousePos.y <= titleBtnY2)
        {
            // BGMを停止
            StopSoundMem(m_bgmHandle);
            PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生
            return new SceneTitle(true);
        }
        if (mousePos.x >= retryBtnX1 && mousePos.x <= retryBtnX2 && mousePos.y >= retryBtnY1 && mousePos.y <= retryBtnY2)
        {
            // BGMを停止
            StopSoundMem(m_bgmHandle);
            PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生
            return new SceneMain();
        }
    }
    return nullptr;
}

void SceneGameOver::Draw()
{
    // 背景を描画
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    // スクロール位置を画像サイズで割った余りを計算
    int offsetX = (int)m_scrollX % kBgImageSize;
    int offsetY = (int)m_scrollY % kBgImageSize;
    
    // 負の値になった場合、正の値に補正
    if (offsetX < 0) offsetX += kBgImageSize;
    if (offsetY < 0) offsetY += kBgImageSize;

    // 2x2のタイル状に背景を描画（画面全体を覆うように）
    for (int y = -1; y < 2; y++)
    {
        for (int x = -1; x < 2; x++)
        {
            int drawX = x * kBgImageSize + offsetX;
            int drawY = y * kBgImageSize + offsetY;
            DrawExtendGraph(drawX, drawY, 
                          drawX + kBgImageSize, drawY + kBgImageSize, 
                          m_backgroundHandle, TRUE);
        }
    }

    SetFontSize(48);
    DrawString(screenW / 2 - 200, screenH / 2 - 100, "Game Over", 0xff0000);
    SetFontSize(36);
    char waveStr[64];
    sprintf_s(waveStr, sizeof(waveStr), "到達ウェーブ: %d", m_wave);
    DrawString(screenW / 2 - 150, screenH / 2 - 20, waveStr, 0xffffff);
    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数: %d", m_killCount);
    DrawString(screenW / 2 - 150, screenH / 2 + 20, killStr, 0xffffff);
    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "スコア: %d", m_score);
    DrawString(screenW / 2 - 150, screenH / 2 + 60, scoreStr, 0xffffff);
    SetFontSize(16);
    // ボタン描画
    int centerX = screenW / 2;
    int baseY = screenH / 2 + 180;
    int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing/2;
    int titleBtnY1 = baseY;
    int titleBtnX2 = centerX - kButtonSpacing/2;
    int titleBtnY2 = baseY + kButtonHeight;
    int retryBtnX1 = centerX + kButtonSpacing/2;
    int retryBtnY1 = baseY;
    int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing/2;
    int retryBtnY2 = baseY + kButtonHeight;
    // タイトルボタン
    DrawBox(titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2, 0x888888, true);
    DrawString(titleBtnX1 + 30, titleBtnY1 + 18, "タイトルに戻る", 0xffffff);
    // リトライボタン
    DrawBox(retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2, 0x888888, true);
    DrawString(retryBtnX1 + 50, retryBtnY1 + 18, "リトライ", 0xffffff);
}
