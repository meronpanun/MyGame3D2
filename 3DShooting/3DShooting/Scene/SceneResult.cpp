#include "SceneResult.h"
#include "EffekseerForDXLib.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include "ScoreManager.h"
#include "Game.h"
#include <cassert>

namespace
{
    constexpr int kButtonWidth = 220;    // ボタンの幅
    constexpr int kButtonHeight = 60;    // ボタンの高さ
    constexpr int kButtonSpacing = 40;   // ボタン間のスペース
    constexpr int kBgImageSize = 1024;   // 背景画像のサイズ
    constexpr float kScrollSpeed = 1.0f; // 背景のスクロール速度
}

SceneResult::SceneResult()
    : m_bgmHandle(-1)
    , m_gameClearImageHandle(-1)
    , m_isBGMStarted(false)
    , m_backgroundHandle(-1)
    , m_scrollX(0.0f)
    , m_scrollY(0.0f)
    , m_japaneseFontHandle(-1)
    , m_arialBlackFontHandle(-1)
    , m_arialBlackLargeFontHandle(-1)
    , m_japaneseLargeFontHandle(-1)
    , m_japaneseButtonFontHandle(-1)
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
    float scale = Game::GetUIScale();
    m_japaneseFontHandle =
        CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(30 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseFontHandle != -1);
    m_arialBlackFontHandle = CreateFontToHandle(
        "Arial Black", (int)(48 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackFontHandle != -1);
    m_arialBlackLargeFontHandle = CreateFontToHandle(
        "Arial Black", (int)(96 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_arialBlackLargeFontHandle != -1);
    m_japaneseLargeFontHandle =
        CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(54 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    assert(m_japaneseLargeFontHandle != -1);
    m_japaneseButtonFontHandle =
        CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(36 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
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
        ScoreManager::Instance().GetHeadKillCount());

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
    UpdateLayout();

    // 背景をスクロール
    m_scrollX += kScrollSpeed;
    m_scrollY += kScrollSpeed;
    if (m_scrollX > kBgImageSize) m_scrollX -= kBgImageSize;
    if (m_scrollY > kBgImageSize) m_scrollY -= kBgImageSize;

    // スコア演出用の更新
    ScoreManager::Instance().Update();

    if (InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

        if (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
            mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2)
        {
            // BGMを停止
            StopSoundMem(m_bgmHandle);
            PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生

            // スコアをリセット
            ScoreManager::Instance().ResetAll();

            return new SceneTitle(true);
        }
        if (mousePos.x >= m_layout.retryBtnX1 && mousePos.x <= m_layout.retryBtnX2 &&
            mousePos.y >= m_layout.retryBtnY1 && mousePos.y <= m_layout.retryBtnY2)
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
            DrawExtendGraph(drawX, drawY, drawX + kBgImageSize, drawY + kBgImageSize,
                m_backgroundHandle, true);
        }
    }

    // 全体への黒半透明オーバーレイで文字を読みやすくする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ゲームクリア画像を描画 (サイズと位置を調整)
    DrawExtendGraph(m_layout.imageDrawX, m_layout.imageDrawY,
        m_layout.imageDrawX + m_layout.imageDrawWidth,
        m_layout.imageDrawY + m_layout.imageDrawHeight,
        m_gameClearImageHandle, true);

    // リザルト表示エリアの背景（少し濃い黒）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(m_layout.resBgX, m_layout.resBgY, m_layout.resBgX + m_layout.resBgW, m_layout.resBgY + m_layout.resBgH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 枠線
    DrawBox(m_layout.resBgX, m_layout.resBgY, m_layout.resBgX + m_layout.resBgW, m_layout.resBgY + m_layout.resBgH, 0xffffff, false);

    int y = m_layout.textBaseY;

    // スコア、キル数、タイムの表示 (左寄せ気味に揃える)
    // 合計スコア
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFontHandle,
        "合計スコア");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFontHandle, "%d",
        ScoreManager::Instance().GetDisplayTotalScore());
    y += m_layout.textIntervalHigh;

    // 倒した敵の数
    int killCount = ScoreManager::Instance().GetBodyKillCount() +
        ScoreManager::Instance().GetHeadKillCount();
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFontHandle,
        "倒した敵の数");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFontHandle, "%d",
        killCount);
    y += m_layout.textIntervalHigh;

    // クリアタイム
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFontHandle,
        "クリアタイム");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFontHandle,
        "%.1f秒", SceneMain::GetElapsedTime());

    // ハイスコア表示
    int highScoreY = m_layout.highScoreY;
    float scale = Game::GetUIScale();
    int highScoreInterval = (int)(60 * scale);

    // タイトル
    int highScoreTextWidth = GetDrawStringWidthToHandle("--- High Score ---", -1,
        m_arialBlackFontHandle);
    DrawFormatStringToHandle(screenW / 2 - highScoreTextWidth / 2,
        highScoreY, 0xffff00, m_arialBlackFontHandle,
        "--- High Score ---");
    highScoreY += highScoreInterval;

    const auto& scores = ScoreManager::Instance().GetHighScores();
    for (int i = 0; i < 3 && i < (int)scores.size(); ++i)
    {
        char highStr[64];
        sprintf_s(highStr, sizeof(highStr), "%d位: %d", i + 1, scores[i]);
        int highStrWidth =
            GetDrawStringWidthToHandle(highStr, -1, m_japaneseLargeFontHandle);
        unsigned int color = 0xffffff;
        if (i == 0) color = 0xffd700; // Gold
        else if (i == 1) color = 0xc0c0c0; // Silver
        else if (i == 2) color = 0xff8c00; // Bronze

        DrawFormatStringToHandle(screenW / 2 - highStrWidth / 2, highScoreY,
            color, m_japaneseLargeFontHandle, "%s", highStr);
        highScoreY += highScoreInterval;
    }

    // ボタン描画
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    // タイトルボタン
    // ホバー判定
    bool isTitleHover = (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
        mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2);
    unsigned int titleBtnColor =
        isTitleHover ? 0xaaaaaa : 0x666666; // ホバー時は少し明るく

    DrawBox(m_layout.titleBtnX1, m_layout.titleBtnY1, m_layout.titleBtnX2, m_layout.titleBtnY2, titleBtnColor, true);
    DrawBox(m_layout.titleBtnX1, m_layout.titleBtnY1, m_layout.titleBtnX2, m_layout.titleBtnY2, 0xffffff,
        false); // 枠線
    int titleTextWidth = GetDrawStringWidthToHandle("タイトルに戻る", -1,
        m_japaneseButtonFontHandle);
    int titleTextHeight = (int)(36 * Game::GetUIScale());

    DrawFormatStringToHandle(m_layout.titleBtnX1 + (m_layout.btnW - titleTextWidth) / 2,
        m_layout.titleBtnY1 + (m_layout.btnH - titleTextHeight) / 2, 0xffffff,
        m_japaneseButtonFontHandle, "タイトルに戻る");

    // リトライボタン
    // ホバー判定
    bool isRetryHover = (mousePos.x >= m_layout.retryBtnX1 && mousePos.x <= m_layout.retryBtnX2 &&
        mousePos.y >= m_layout.retryBtnY1 && mousePos.y <= m_layout.retryBtnY2);
    unsigned int retryBtnColor = isRetryHover ? 0xaaaaaa : 0x666666;

    DrawBox(m_layout.retryBtnX1, m_layout.retryBtnY1, m_layout.retryBtnX2, m_layout.retryBtnY2, retryBtnColor, true);
    DrawBox(m_layout.retryBtnX1, m_layout.retryBtnY1, m_layout.retryBtnX2, m_layout.retryBtnY2, 0xffffff,
        false); // 枠線
    int retryTextWidth =
        GetDrawStringWidthToHandle("リトライ", -1, m_japaneseButtonFontHandle);
    int retryTextHeight = (int)(36 * Game::GetUIScale());

    DrawFormatStringToHandle(m_layout.retryBtnX1 + (m_layout.btnW - retryTextWidth) / 2,
        m_layout.retryBtnY1 + (m_layout.btnH - retryTextHeight) / 2, 0xffffff,
        m_japaneseButtonFontHandle, "リトライ");
}

void SceneResult::UpdateLayout()
{
    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();
    float scale = Game::GetUIScale();

    // Draw Game Clear Image
    m_layout.imageDrawWidth = (int)(800 * scale);
    m_layout.imageDrawHeight = (int)(200 * scale);
    m_layout.imageDrawX = (screenW - m_layout.imageDrawWidth) / 2;
    m_layout.imageDrawY = (int)(50 * scale);

    // Result BG
    m_layout.resBgW = (int)(700 * scale);
    m_layout.resBgH = (int)(240 * scale);
    m_layout.resBgX = (screenW - m_layout.resBgW) / 2;
    m_layout.resBgY = (int)(280 * scale);

    // Text
    m_layout.textLabelX = m_layout.resBgX + (int)(100 * scale);
    m_layout.textValueX = m_layout.resBgX + (int)(450 * scale);
    m_layout.textBaseY = m_layout.resBgY + (int)(40 * scale);
    m_layout.textIntervalHigh = (int)(70 * scale);

    // High Score
    m_layout.highScoreY = m_layout.textBaseY + (int)((70 + 70 + 100 + 20) * scale);

    // Buttons
    int bottomMargin = (int)(150 * scale);
    int btnY = screenH - bottomMargin;

    m_layout.btnW = (int)(270 * scale);
    m_layout.btnH = (int)(70 * scale);
    int btnSpacing = (int)(60 * scale);
    int centerX = screenW / 2;

    // Title Button
    m_layout.titleBtnX1 = centerX - m_layout.btnW - btnSpacing / 2;
    m_layout.titleBtnY1 = btnY;
    m_layout.titleBtnX2 = centerX - btnSpacing / 2;
    m_layout.titleBtnY2 = btnY + m_layout.btnH;

    // Retry Button
    m_layout.retryBtnX1 = centerX + btnSpacing / 2;
    m_layout.retryBtnY1 = btnY;
    m_layout.retryBtnX2 = centerX + m_layout.btnW + btnSpacing / 2;
    m_layout.retryBtnY2 = btnY + m_layout.btnH;
}
