#include "SceneGameOver.h"
#include "EffekseerForDXLib.h"
#include "SceneTitle.h"
#include "SceneMain.h"
#include "Mouse.h"
#include <cassert>

namespace
{
	constexpr int kButtonWidth   = 220;  // ボタンの幅
	constexpr int kButtonHeight  = 60;   // ボタンの高さ
	constexpr int kButtonSpacing = 40;   // ボタン間のスペース
    constexpr int kBgImageSize   = 1024; // 背景画像のサイズ
    constexpr float kScrollSpeed = 1.0f; // 背景のスクロール速度
}

SceneGameOver::SceneGameOver(int wave, int killCount, int score) : 
    m_wave(wave),
    m_killCount(killCount),
    m_score(score), 
    m_bgmHandle(-1), 
    m_isBGMStarted(false), 
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

    // フォントの作成
    m_japaneseFontHandle = CreateFontToHandle("Noto Sans Japanese", 20, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseFontHandle != -1);
    m_arialBlackFontHandle = CreateFontToHandle("Arial Black", 32, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackFontHandle != -1);
    m_arialBlackLargeFontHandle = CreateFontToHandle("Arial Black", 64, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackLargeFontHandle != -1);
    m_japaneseLargeFontHandle = CreateFontToHandle("Noto Sans Japanese", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseLargeFontHandle != -1);
    m_japaneseButtonFontHandle = CreateFontToHandle("Noto Sans Japanese", 24, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseButtonFontHandle != -1);
}

SceneGameOver::~SceneGameOver()
{
    // BGMの解放
    DeleteSoundMem(m_bgmHandle);
    DeleteSoundMem(m_returnSEHandle);

    // 背景画像の解放
    DeleteGraph(m_backgroundHandle);

    // フォントの解放
    DeleteFontToHandle(m_japaneseFontHandle);
    DeleteFontToHandle(m_arialBlackFontHandle);
    DeleteFontToHandle(m_arialBlackLargeFontHandle);
    DeleteFontToHandle(m_japaneseLargeFontHandle);
    DeleteFontToHandle(m_japaneseButtonFontHandle);
}

void SceneGameOver::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);

    // カウントアップ演出用スコア初期化
    ScoreManager::Instance().ResetDisplayScore();
    ScoreManager::Instance().SetTargetDisplayScore(ScoreManager::Instance().GetTotalScore());

    m_isBGMStarted = false;

    // BGM再生（既に再生中でなければ）
    if (CheckSoundMem(m_bgmHandle) == 0)
    {
        PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
        m_isBGMStarted = true;
    }
}

SceneBase* SceneGameOver::Update()
{
    // 背景をスクロール
    m_scrollX += kScrollSpeed;
    m_scrollY += kScrollSpeed;
    if (m_scrollX > kBgImageSize) m_scrollX -= kBgImageSize;
    if (m_scrollY > kBgImageSize) m_scrollY -= kBgImageSize;

    // スコア演出用の更新
    ScoreManager::Instance().Update();

    if (Mouse::IsTriggerLeft())
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW * 0.5f;
        int baseY = screenH * 0.5f + 180;
        // タイトルに戻るボタン
        int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing * 0.5f;
        int titleBtnY1 = baseY;
        int titleBtnX2 = centerX - kButtonSpacing * 0.5f;
        int titleBtnY2 = baseY + kButtonHeight;
        // リトライボタン
        int retryBtnX1 = centerX + kButtonSpacing * 0.5f;
        int retryBtnY1 = baseY;
        int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing * 0.5f;
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
            return new SceneMain(true);
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

    int gameOverTextWidth = GetDrawStringWidthToHandle("Game Over", -1, m_arialBlackLargeFontHandle);
    DrawFormatStringToHandle(screenW * 0.5f - gameOverTextWidth * 0.5f, screenH * 0.5f - 100, 0xff4500, m_arialBlackLargeFontHandle, "Game Over");

    char waveStr[64];
    sprintf_s(waveStr, sizeof(waveStr), "到達ウェーブ: %d", m_wave);
    DrawFormatStringToHandle(screenW * 0.5f - 200, screenH * 0.5f - 20, 0xffffff, m_japaneseLargeFontHandle, "%s", waveStr);

    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数: %d", m_killCount);
    DrawFormatStringToHandle(screenW * 0.5f - 200, screenH * 0.5f + 20, 0xffffff, m_japaneseLargeFontHandle, "%s", killStr);

    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "スコア: %d", ScoreManager::Instance().GetDisplayScore());
    DrawFormatStringToHandle(screenW * 0.5f - 200, screenH * 0.5f + 60, 0xffffff, m_japaneseLargeFontHandle, "%s", scoreStr);

    // ボタン描画
    int centerX = screenW * 0.5f;
    int baseY = screenH * 0.5f + 180;
    int titleBtnX1 = centerX - kButtonWidth - kButtonSpacing * 0.5f;
    int titleBtnY1 = baseY;
    int titleBtnX2 = centerX - kButtonSpacing * 0.5f;
    int titleBtnY2 = baseY + kButtonHeight;
    int retryBtnX1 = centerX + kButtonSpacing * 0.5f;
    int retryBtnY1 = baseY;
    int retryBtnX2 = centerX + kButtonWidth + kButtonSpacing * 0.5f;
    int retryBtnY2 = baseY + kButtonHeight;
    // タイトルボタン
    DrawBox(titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2, 0x888888, true);
    int titleTextWidth = GetDrawStringWidthToHandle("タイトルに戻る", -1, m_japaneseButtonFontHandle);
    DrawFormatStringToHandle(titleBtnX1 + (kButtonWidth - titleTextWidth) * 0.5f, titleBtnY1 + (kButtonHeight - 24) * 0.5f, 0xffffff, m_japaneseButtonFontHandle, "タイトルに戻る");
    // リトライボタン
    DrawBox(retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2, 0x888888, true);
    int retryTextWidth = GetDrawStringWidthToHandle("リトライ", -1, m_japaneseButtonFontHandle);
    DrawFormatStringToHandle(retryBtnX1 + (kButtonWidth - retryTextWidth) * 0.5f, retryBtnY1 + (kButtonHeight - 24) * 0.5f, 0xffffff, m_japaneseButtonFontHandle, "リトライ");
}
