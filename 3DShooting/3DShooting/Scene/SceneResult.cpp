#include "SceneResult.h"
#include "ScoreManager.h"
#include "EffekseerForDXLib.h"
#include "SceneMain.h"
#include "SceneTitle.h"
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

SceneResult::SceneResult() :
    m_bgmHandle(-1),
	m_gameClearImageHandle(-1),
    m_isBGMStarted(false),
    m_backgroundHandle(-1),
    m_scrollX(0.0f), 
    m_scrollY(0.0f),
    m_japaneseFontHandle(-1),
    m_arialBlackFontHandle(-1),
    m_arialBlackLargeFontHandle(-1),
    m_japaneseLargeFontHandle(-1),
    m_japaneseButtonFontHandle(-1)
{
    // BGMのロード
    m_bgmHandle = LoadSoundMem("data/sound/BGM/GameClearBGM.mp3");
    assert(m_bgmHandle != -1);
    m_returnSEHandle = LoadSoundMem("data/sound/SE/ButtonReturn.mp3");
    assert(m_returnSEHandle != -1);

    // 背景画像のロード
    m_backgroundHandle = LoadGraph("data/image/GameClearBackGrand.png");
    assert(m_backgroundHandle != -1);

	// ゲームクリア画像のロード
	m_gameClearImageHandle = LoadGraph("data/image/GameClear.png");
	assert(m_gameClearImageHandle != -1);

    // フォントの作成
    m_japaneseFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 20, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseFontHandle != -1);
    m_arialBlackFontHandle = CreateFontToHandle("Arial Black", 32, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackFontHandle != -1);
    m_arialBlackLargeFontHandle = CreateFontToHandle("Arial Black", 64, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackLargeFontHandle != -1);
    m_japaneseLargeFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseLargeFontHandle != -1);
    m_japaneseButtonFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", 24, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseButtonFontHandle != -1);
}

SceneResult::~SceneResult()
{
    // BGMの解放
    DeleteSoundMem(m_bgmHandle);
    DeleteSoundMem(m_returnSEHandle);

    // 背景画像の解放
    DeleteGraph(m_backgroundHandle);

	// ゲームクリア画像の解放
	DeleteGraph(m_gameClearImageHandle);

    // フォントの解放
    DeleteFontToHandle(m_japaneseFontHandle);
    DeleteFontToHandle(m_arialBlackFontHandle);
    DeleteFontToHandle(m_arialBlackLargeFontHandle);
    DeleteFontToHandle(m_japaneseLargeFontHandle);
    DeleteFontToHandle(m_japaneseButtonFontHandle);
}

void SceneResult::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);
    // スコア保存
    ScoreManager::Instance().SaveScore(ScoreManager::Instance().GetTotalScore());

    // カウントアップ演出用初期化
    ScoreManager::Instance().ResetDisplayValues();
    ScoreManager::Instance().SetTargetDisplayValues(
        ScoreManager::Instance().GetScore(),
        ScoreManager::Instance().GetTotalScore(),
        ScoreManager::Instance().GetBodyKillCount(),
        ScoreManager::Instance().GetHeadKillCount()
    );

    m_isBGMStarted = false;

    // BGM再生（既に再生中でなければ）
    if (CheckSoundMem(m_bgmHandle) == 0)
    {
        PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
        m_isBGMStarted = true;
    }
}

SceneBase* SceneResult::Update()
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
        int btnY = screenH - 80;
        int btnW = 120;
        int btnH = 36;
        int btnSpacing = 24;
        int centerX = screenW * 0.5f;
        // タイトルボタン
        int titleBtnX1 = centerX - btnW - btnSpacing * 0.5f;
        int titleBtnY1 = btnY;
        int titleBtnX2 = centerX - btnSpacing * 0.5f;
        int titleBtnY2 = btnY + btnH;
        // リトライボタン
        int retryBtnX1 = centerX + btnSpacing * 0.5f;
        int retryBtnY1 = btnY;
        int retryBtnX2 = centerX + btnW + btnSpacing * 0.5f;
        int retryBtnY2 = btnY + btnH;
        Vec2 mousePos = Mouse::GetPos();
        if (mousePos.x >= titleBtnX1 && mousePos.x <= titleBtnX2 && mousePos.y >= titleBtnY1 && mousePos.y <= titleBtnY2)
        {
            // BGMを停止
            StopSoundMem(m_bgmHandle);
            PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生

			// スコアをリセット
			ScoreManager::Instance().ResetAll();

            return new SceneTitle(true);
        }
        if (mousePos.x >= retryBtnX1 && mousePos.x <= retryBtnX2 && mousePos.y >= retryBtnY1 && mousePos.y <= retryBtnY2)
        {
            // BGMを停止
            StopSoundMem(m_bgmHandle);
            PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生

            // スコアをリセット
			ScoreManager::Instance().ResetAll();

            return new SceneMain(true);
        }
    }
    return nullptr;
}

void SceneResult::Draw()
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
                          m_backgroundHandle, true);
        }
    }

	// ゲームクリア画像を描画
    DrawExtendGraph(0, -250, screenW, screenH - 200, m_gameClearImageHandle, true);
    
    int y = 250;
    char scoreStr[64];
    sprintf_s(scoreStr, sizeof(scoreStr), "合計スコア 　 : %d", ScoreManager::Instance().GetDisplayTotalScore());
    DrawFormatStringToHandle(screenW * 0.5f - 200, y, 0xffffff, m_japaneseLargeFontHandle, "%s", scoreStr);
    y += 48; 

    int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
    char killStr[64];
    sprintf_s(killStr, sizeof(killStr), "倒した敵の数 : %d", killCount);
    DrawFormatStringToHandle(screenW * 0.5f - 200, y, 0xffffff, m_japaneseLargeFontHandle, "%s", killStr);
    y += 48; 

    char timeStr[64];
    sprintf_s(timeStr, sizeof(timeStr), "クリアタイム 　: %.1f秒", SceneMain::GetElapsedTime());
    DrawFormatStringToHandle(screenW * 0.5f - 200, y, 0xffffff, m_japaneseLargeFontHandle, "%s", timeStr);
    y += 60; 

    // ハイスコア表示
    int highScoreTextWidth = GetDrawStringWidthToHandle("--- High Score ---", -1, m_arialBlackLargeFontHandle);
    DrawFormatStringToHandle(screenW * 0.5f - highScoreTextWidth * 0.5f, y, 0xffff00, m_arialBlackLargeFontHandle, "--- High Score ---");
    y += 80; 
    const auto& scores = ScoreManager::Instance().GetHighScores();
    for (int i = 0; i < 3 && i < (int)scores.size(); ++i) 
    {
        char highStr[64];
        sprintf_s(highStr, sizeof(highStr), "%d位: %d", i+1, scores[i]);
        int highStrWidth = GetDrawStringWidthToHandle(highStr, -1, m_japaneseLargeFontHandle);
        unsigned int color = 0xffffff;
        if (i == 0) 
        {
            color = 0xffd700;
        }
        else if (i == 1)
        {
            color = 0xc0c0c0;
        }
        else if (i == 2) 
        {
            color = 0xff8c00;
        }
        DrawFormatStringToHandle(screenW * 0.5f - highStrWidth * 0.5f, y, color, m_japaneseLargeFontHandle, "%s", highStr);
        y += 48; 
    }

    // ボタン描画
    int btnY = screenH - 100; 
    int btnW = 180;
    int btnH = 50; 
    int btnSpacing = 40;
    int centerX = screenW * 0.5f;
    // タイトルボタン
    int titleBtnX1 = centerX - btnW - btnSpacing * 0.5f;
    int titleBtnY1 = btnY;
    int titleBtnX2 = centerX - btnSpacing * 0.5f;
    int titleBtnY2 = btnY + btnH;
    DrawBox(titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2, 0x888888, true);
    int titleTextWidth = GetDrawStringWidthToHandle("タイトルに戻る", -1, m_japaneseButtonFontHandle);
    DrawFormatStringToHandle(titleBtnX1 + (btnW - titleTextWidth) * 0.5f, titleBtnY1 + (btnH - 24) * 0.5f, 0xffffff, m_japaneseButtonFontHandle, "タイトルに戻る");
    // リトライボタン
    int retryBtnX1 = centerX + btnSpacing * 0.5f;
    int retryBtnY1 = btnY;
    int retryBtnX2 = centerX + btnW + btnSpacing * 0.5f;
    int retryBtnY2 = btnY + btnH;
    DrawBox(retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2, 0x888888, true);
    int retryTextWidth = GetDrawStringWidthToHandle("リトライ", -1, m_japaneseButtonFontHandle);
    DrawFormatStringToHandle(retryBtnX1 + (btnW - retryTextWidth) * 0.5f, retryBtnY1 + (btnH - 24) * 0.5f, 0xffffff, m_japaneseButtonFontHandle, "リトライ");
}
