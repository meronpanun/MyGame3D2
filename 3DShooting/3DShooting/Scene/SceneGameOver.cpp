#include "SceneGameOver.h"
#include "EffekseerForDXLib.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include "Game.h"
#include <cassert>

namespace
{
    constexpr int kButtonWidth = 220;       // ボタンの幅
    constexpr int kButtonHeight = 60;       // ボタンの高さ
    constexpr int kButtonSpacing = 40;      // ボタン間のスペース
    constexpr int kBgImageSize = 1024;      // 背景画像のサイズ
    constexpr float kScrollSpeed = 1.0f;    // 背景のスクロール速度
    constexpr int kImageChangeInterval = 3; // 画像切り替え間隔（フレーム数）
}

SceneGameOver::SceneGameOver(int wave, int killCount, int score)
    : m_wave(wave)
    , m_killCount(killCount)
    , m_score(score)
    , m_bgmHandle(-1)
    , m_isBGMStarted(false)
    , m_backgroundHandle(-1)
    , m_scrollX(0.0f)
    , m_scrollY(0.0f)
    , m_currentImageIndex(0)
    , m_imageChangeTimer(0)
    , m_imageChangeInterval(kImageChangeInterval)
{
  // BGMのロード
  m_bgmHandle = LoadSoundMem("data/sound/BGM/GameOverBGM.mp3");
  assert(m_bgmHandle != -1);
  m_returnSEHandle = LoadSoundMem("data/sound/SE/ButtonReturn.mp3");
  assert(m_returnSEHandle != -1);

  // 背景画像のロード
  m_backgroundHandle = LoadGraph("data/image/BackGrand.png");
  assert(m_backgroundHandle != -1);

  // ゲームオーバー画像のロード
  m_gameOverImageHandle = LoadGraph("data/image/GameOverZombie.png");
  assert(m_gameOverImageHandle != -1);
  m_gameOverImageHandle2 = LoadGraph("data/image/GameOverZombie2.png");
  assert(m_gameOverImageHandle2 != -1);
  m_gameOverImageHandle3 = LoadGraph("data/image/GameOverZombie3.png");
  assert(m_gameOverImageHandle3 != -1);

  // フォントの作成
  float scale = Game::GetUIScale();
  m_japaneseFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(20 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_japaneseFontHandle != -1);
  m_arialBlackFontHandle = CreateFontToHandle( "Arial Black", (int)(32 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_arialBlackFontHandle != -1);
  m_arialBlackLargeFontHandle = CreateFontToHandle("Arial Black", (int)(64 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_arialBlackLargeFontHandle != -1);
  m_japaneseLargeFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(36 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_japaneseLargeFontHandle != -1);
  m_japaneseButtonFontHandle = CreateFontToHandle("HGPｺﾞｼｯｸE", (int)(24 * scale), 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_japaneseButtonFontHandle != -1);
}

SceneGameOver::~SceneGameOver()
{
  // BGMの解放
  DeleteSoundMem(m_bgmHandle);
  DeleteSoundMem(m_returnSEHandle);

  // 背景画像の解放
  DeleteGraph(m_backgroundHandle);
  DeleteGraph(m_gameOverImageHandle);
  DeleteGraph(m_gameOverImageHandle2);
  DeleteGraph(m_gameOverImageHandle3);

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
  ScoreManager::Instance().SetTargetDisplayScore(
  ScoreManager::Instance().GetTotalScore());

  m_isBGMStarted = false;

  // BGM再生（既に再生中でなければ）
  if (CheckSoundMem(m_bgmHandle) == 0) {
    PlaySoundMem(m_bgmHandle, DX_PLAYTYPE_LOOP);
    m_isBGMStarted = true;
  }
}

SceneBase *SceneGameOver::Update() {
  UpdateLayout();

  // 背景をスクロール
  m_scrollX += kScrollSpeed;
  m_scrollY += kScrollSpeed;
  if (m_scrollX > kBgImageSize)
    m_scrollX -= kBgImageSize;
  if (m_scrollY > kBgImageSize)
    m_scrollY -= kBgImageSize;

  // スコア演出用の更新
  ScoreManager::Instance().Update();

  // 画像切り替え演出の更新
  m_imageChangeTimer++;
  if (m_imageChangeTimer >= m_imageChangeInterval) {
    // ランダムで画像を選択（0:通常, 1:乱れ2, 2:乱れ3）
    m_currentImageIndex = rand() % 3;

    m_imageChangeTimer = 0;

    // より頻繁な切り替え
    m_imageChangeInterval = 5 + (rand() % 30);
  }

  if (InputManager::GetInstance()->IsTriggerMouseLeft()) {
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();
    if (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
        mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2) {
      // BGMを停止
      StopSoundMem(m_bgmHandle);
      PlaySoundMem(m_returnSEHandle, DX_PLAYTYPE_BACK); // 戻るボタンSE再生

      // スコアをリセット
      ScoreManager::Instance().ResetAll();

      return new SceneTitle(true);
    }
    if (mousePos.x >= m_layout.retryBtnX1 && mousePos.x <= m_layout.retryBtnX2 &&
        mousePos.y >= m_layout.retryBtnY1 && mousePos.y <= m_layout.retryBtnY2) {
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

void SceneGameOver::Draw() {
  // 背景を描画
  int screenW, screenH;
  GetScreenState(&screenW, &screenH, nullptr);

  // スクロール位置を画像サイズで割った余りを計算
  int offsetX = (int)m_scrollX % kBgImageSize;
  int offsetY = (int)m_scrollY % kBgImageSize;

  // 負の値になった場合、正の値に補正
  if (offsetX < 0)
    offsetX += kBgImageSize;
  if (offsetY < 0)
    offsetY += kBgImageSize;

  // 2x2のタイル状に背景を描画（画面全体を覆うように）
  for (int y = -1; y < 2; y++) {
    for (int x = -1; x < 2; x++) {
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

  // 現在の画像インデックスに応じて画像を選択
  int currentImageHandle;
  switch (m_currentImageIndex) {
  case 0:
    currentImageHandle = m_gameOverImageHandle;
    break;
  case 1:
    currentImageHandle = m_gameOverImageHandle2;
    break;
  case 2:
    currentImageHandle = m_gameOverImageHandle3;
    break;
  default:
    currentImageHandle = m_gameOverImageHandle;
    break;
  }

  // 画像描画
  DrawExtendGraph(m_layout.imageDrawX, m_layout.imageDrawY, 
                  m_layout.imageDrawX + m_layout.imageDrawWidth, 
                  m_layout.imageDrawY + m_layout.imageDrawHeight,
                  currentImageHandle, true);

  // リザルト表示エリアの背景（少し濃い黒）
  SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
  DrawBox(m_layout.resBgX, m_layout.resBgY, 
          m_layout.resBgX + m_layout.resBgW, 
          m_layout.resBgY + m_layout.resBgH, 0x000000, true);
  SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

  // 枠線
  DrawBox(m_layout.resBgX, m_layout.resBgY, 
          m_layout.resBgX + m_layout.resBgW, 
          m_layout.resBgY + m_layout.resBgH, 0xffffff, false);

  float scale = Game::GetUIScale();
  int textInterval = (int)(70 * scale);
  int textY = m_layout.textBaseY;

  // テキスト配置
  char waveStr[64];
  sprintf_s(waveStr, sizeof(waveStr), "%dウェーブ生き残った", m_wave);
  // ウェーブ数は中央寄せ
  int waveStrW = GetDrawStringWidthToHandle(waveStr, strlen(waveStr),
                                            m_japaneseLargeFontHandle);
  DrawFormatStringToHandle((screenW - waveStrW) / 2, textY, 0xffffff,
                           m_japaneseLargeFontHandle, "%s", waveStr);
  textY += textInterval;

  char killStr[64];
  int killCount = m_killCount; 
  DrawFormatStringToHandle(m_layout.textLabelX, textY, 0xffffff, m_japaneseLargeFontHandle,
                           "倒した敵の数");
  DrawFormatStringToHandle(m_layout.textValueX, textY, 0xffffff, m_japaneseLargeFontHandle,
                           "%d", killCount);
  textY += textInterval;

  char scoreStr[64];
  DrawFormatStringToHandle(m_layout.textLabelX, textY, 0xffffff, m_japaneseLargeFontHandle,
                           "スコア");
  DrawFormatStringToHandle(m_layout.textValueX, textY, 0xffffff, m_japaneseLargeFontHandle,
                           "%d", ScoreManager::Instance().GetDisplayScore());

  // マウス位置取得
  Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

  // タイトルボタン
  // ホバー判定
  bool isTitleHover = (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
                       mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2);
  unsigned int titleBtnColor = isTitleHover ? 0xaaaaaa : 0x666666;

  DrawBox(m_layout.titleBtnX1, m_layout.titleBtnY1, m_layout.titleBtnX2, m_layout.titleBtnY2, titleBtnColor, true);
  DrawBox(m_layout.titleBtnX1, m_layout.titleBtnY1, m_layout.titleBtnX2, m_layout.titleBtnY2, 0xffffff,
          false); // 枠線

  int titleTextWidth = GetDrawStringWidthToHandle("タイトルに戻る", -1,
                                                  m_japaneseButtonFontHandle);
  int titleTextHeight = (int)(24 * Game::GetUIScale());

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
  int retryTextHeight = (int)(24 * Game::GetUIScale());
      
  DrawFormatStringToHandle(m_layout.retryBtnX1 + (m_layout.btnW - retryTextWidth) / 2,
                           m_layout.retryBtnY1 + (m_layout.btnH - retryTextHeight) / 2, 0xffffff,
                           m_japaneseButtonFontHandle, "リトライ");
}

void SceneGameOver::UpdateLayout() {
  int screenW = Game::GetScreenWidth();
  int screenH = Game::GetScreenHeight();
  float scale = Game::GetUIScale();

  // 画像サイズの計算
  float imageAspect = 1024.0f / 1110.0f;
  float screenAspect = (float)screenW / (float)screenH;
  int drawWidth, drawHeight;
  if (imageAspect > screenAspect) {
    drawWidth = screenW;
    drawHeight = (int)(screenW / imageAspect);
  } else {
    drawHeight = screenH;
    drawWidth = (int)(screenH * imageAspect);
  }
  // 縮小スケール
  const float kScale = 0.4f;
  m_layout.imageDrawWidth = (int)(drawWidth * kScale);
  m_layout.imageDrawHeight = (int)(drawHeight * kScale);

  // 画面上部に配置
  const int kTopMargin = (int)(screenH * 0.04f);
  m_layout.imageDrawX = (screenW - m_layout.imageDrawWidth) / 2;
  m_layout.imageDrawY = kTopMargin;

  // リザルト表示エリア
  m_layout.resBgW = (int)(700 * scale);
  m_layout.resBgH = (int)(260 * scale);
  m_layout.resBgX = (screenW - m_layout.resBgW) / 2;
  m_layout.resBgY = m_layout.imageDrawY + m_layout.imageDrawHeight + (int)(20 * scale);

  // テキスト配置
  m_layout.textLabelX = m_layout.resBgX + (int)(100 * scale);
  m_layout.textValueX = m_layout.resBgX + (int)(450 * scale);
  m_layout.textBaseY = m_layout.resBgY + (int)(40 * scale);

  // ボタン
  m_layout.btnW = (int)(270 * scale);
  m_layout.btnH = (int)(70 * scale);
  int btnSpacing = (int)(60 * scale);
  int centerX = screenW / 2;
  int btnBaseY = m_layout.resBgY + m_layout.resBgH + (int)(40 * scale);

  // タイトルに戻るボタン
  m_layout.titleBtnX1 = centerX - m_layout.btnW - btnSpacing / 2;
  m_layout.titleBtnY1 = btnBaseY;
  m_layout.titleBtnX2 = centerX - btnSpacing / 2;
  m_layout.titleBtnY2 = btnBaseY + m_layout.btnH;

  // リトライボタン
  m_layout.retryBtnX1 = centerX + btnSpacing / 2;
  m_layout.retryBtnY1 = btnBaseY;
  m_layout.retryBtnX2 = centerX + m_layout.btnW + btnSpacing / 2;
  m_layout.retryBtnY2 = btnBaseY + m_layout.btnH;
}
