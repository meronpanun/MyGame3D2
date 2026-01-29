#include "TutorialManager.h"
#include "EffekseerForDXLib.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "WaveManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>


namespace {
// 時間関連
constexpr float kFrameTime = 1.0f / 60.0f;    // 1フレームの時間
constexpr float kCompleteWaitTime = 2.0f;     // チュートリアル完了後の待機時間
constexpr float kStepCompleteWaitTime = 1.5f; // ステップ完了後の待機時間
constexpr float kMoveAccumGoalTime = 2.0f; // 移動チュートリアルの目標累積時間
constexpr float kViewAccumGoalTime = 1.0f; // 視点チュートリアルの目標累積時間
constexpr float kJumpAccumGoalTime =
    0.2f; // ジャンプチュートリアルの目標累積時間
constexpr float kRunAccumGoalTime = 1.0f;  // 走行チュートリアルの目標累積時間
constexpr float kCheckAnimDuration = 0.3f; // チェックマークのアニメーション時間

// UI関連
constexpr int kFontSize = 34;        // チュートリアルメッセージのフォントサイズ
constexpr int kDefaultFontSize = 24; // デフォルトのフォントサイズ
constexpr int kMessageOffsetX = 630; // メッセージのXオフセット
constexpr int kInitialYPos = 60;     // メッセージの初期Y座標
constexpr int kLineSpacing = 60;     // メッセージの行間
constexpr int kCheckMarkBaseSize = 60;     // チェックマークの基本サイズ
constexpr int kCheckMarkOffsetXMove = 420; // 移動チェックマークのXオフセット
constexpr int kCheckMarkOffsetXOthers =
    510;                              // それ以外のチェックマークのXオフセット
constexpr int kCheckMarkOffsetY = 30; // チェックマークのYオフセット
constexpr float kCheckMarkAnimScale =
    2.0f;                         // チェックマークアニメーションの最大スケール
constexpr int kKeyImageSize = 60; // キー画像のサイズ
constexpr int kKeyImageSpacing = 8;        // キー画像の間隔
constexpr int kKeyImageWidth = 120;        // キー画像の幅
constexpr int kKeyImageHeight = 60;        // キー画像の高さ
constexpr int kShiftImageWidth = 120;      // Shiftキー画像の幅
constexpr float kCheckMarkDrawSize = 1.0f; // チャックマーク画像の大きさ

// UIボックス関連
constexpr int kBoxPaddingX = 30;             // ボックスの左右パディング
constexpr int kBoxPaddingY = 15;             // ボックスの上下パディング
constexpr int kBoxAlpha = 180;               // ボックスのアルファ値
constexpr unsigned int kBoxColor = 0x000000; // ボックスの色

// アニメーション関連
constexpr float kUIAnimationSpeed = 15.0f;    // UIがスライドする速度
constexpr float kUIOffscreenOffsetX = 750.0f; // UIの画面外オフセット

// タイトル関連
constexpr int kTitleFontSize = 42;    // タイトルのフォントサイズ
constexpr int kTitleOffsetY = 15;     // タイトルのYオフセット
constexpr int kTitleColor = 0xFFFFFF; // タイトルの色

// マウスの移動量閾値
constexpr float kMouseMovementThreshold = 2.0f;

// メッセージ関連
constexpr float kMessageDisplayTime = 5.0f; // メッセージの表示時間
constexpr int kMessageTitleFontSize = 36;
constexpr int kMessageDetailFontSize = 28;
constexpr int kMessageOffsetY = 220;
constexpr int kMessageLineSpacing = 8;      // メッセージの行間
constexpr int kMessageTimeBarHeight = 6;    // タイムバーの高さ
constexpr int kMessageTimeBarPaddingY = 24; // タイムバーの上下のパディング
} // namespace

TutorialManager::TutorialManager()
    : m_step(Step::None), m_uiState(UIState::Hidden),
      m_uiXOffset(kUIOffscreenOffsetX), m_isMoveDone(false),
      m_isViewDone(false), m_checkMarkHandle(-1), m_prevMousePos{0, 0},
      m_moveAccumTime(0.0f), m_viewAccumTime(0.0f), m_completeWaitTime(0.0f),
      m_isCompletedDisplay(false), m_isMoveCheckAnim(false),
      m_moveCheckAnimTime(0.0f), m_isViewCheckAnim(false),
      m_viewCheckAnimTime(0.0f), m_isJumpDone(false), m_isRunDone(false),
      m_jumpAccumTime(0.0f), m_runAccumTime(0.0f), m_isJumpCheckAnim(false),
      m_jumpCheckAnimTime(0.0f), m_isRunCheckAnim(false),
      m_runCheckAnimTime(0.0f), m_stepCompleteWaitTime(0.0f),
      m_isStepCompleted(false) {
  // チェックマーク画像の読み込み
  m_checkMarkHandle = LoadGraph("data/image/CheckMark.png");
  assert(m_checkMarkHandle != -1);

  // キー画像の読み込み
  m_wKeyHandle = LoadGraph("data/image/W.png");
  m_aKeyHandle = LoadGraph("data/image/A.png");
  m_sKeyHandle = LoadGraph("data/image/S.png");
  m_dKeyHandle = LoadGraph("data/image/D.png");
  assert(m_wKeyHandle != -1);
  assert(m_aKeyHandle != -1);
  assert(m_sKeyHandle != -1);
  assert(m_dKeyHandle != -1);

  m_mouseMoveHorHandle = LoadGraph("data/image/MouseMoveHor.png");
  assert(m_mouseMoveHorHandle != -1);

  m_spaceKeyHandle = LoadGraph("data/image/Space.png");
  assert(m_spaceKeyHandle != -1);

  m_leftShiftKeyHandle = LoadGraph("data/image/LeftShift.png");
  assert(m_leftShiftKeyHandle != -1);

  m_crossHandle = LoadGraph("data/image/Cross.png");
  assert(m_crossHandle != -1);

  // フォントの作成
  m_japaneseFontHandle =
      CreateFontToHandle("HGPｺﾞｼｯｸE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_japaneseFontHandle != -1);
  m_japaneseLargeFontHandle =
      CreateFontToHandle("HGPｺﾞｼｯｸE", 54, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_japaneseLargeFontHandle != -1);
  m_messageDetailFontHandle =
      CreateFontToHandle("HGPｺﾞｼｯｸE", kMessageDetailFontSize, 3,
                         DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
  assert(m_messageDetailFontHandle != -1);
}

TutorialManager::~TutorialManager() {
  // チェックマーク画像の解放
  DeleteGraph(m_checkMarkHandle);

  // キー画像の解放
  DeleteGraph(m_wKeyHandle);
  DeleteGraph(m_aKeyHandle);
  DeleteGraph(m_sKeyHandle);
  DeleteGraph(m_dKeyHandle);
  DeleteGraph(m_mouseMoveHorHandle);
  DeleteGraph(m_spaceKeyHandle);
  DeleteGraph(m_leftShiftKeyHandle);
  DeleteGraph(m_crossHandle);

  // フォントの解放
  DeleteFontToHandle(m_japaneseFontHandle);
  DeleteFontToHandle(m_japaneseLargeFontHandle);
  DeleteFontToHandle(m_messageDetailFontHandle);
}

void TutorialManager::Init() {
  m_step = Step::Move;
  m_uiState = UIState::Hidden; // 最初は隠しておく
}

void TutorialManager::UpdateUI() {
  switch (m_uiState) {
  case UIState::Hidden:

    // 新しいステップが始まったらEntering状態へ
    if (m_step != Step::None && m_step != Step::Completed) {
      m_uiState = UIState::Entering;
    }
    break;
  case UIState::Entering:
    m_uiXOffset -= kUIAnimationSpeed;
    if (m_uiXOffset <= 0.0f) {
      m_uiXOffset = 0.0f;
      m_uiState = UIState::OnScreen;
    }
    break;
  case UIState::OnScreen:
    // ステップ完了を待つ
    break;
  case UIState::Exiting:
    m_uiXOffset += kUIAnimationSpeed;
    if (m_uiXOffset >= kUIOffscreenOffsetX) {
      m_uiXOffset = kUIOffscreenOffsetX;
      m_uiState = UIState::Hidden;

      // 最後のステップだったら完了演出へ
      if (m_step == Step::Run) {
        m_isCompletedDisplay = true;
        m_completeWaitTime = 0.0f;
      }

      // 次のステップへ
      m_step = static_cast<Step>(static_cast<int>(m_step) + 1);
    }
    break;
  }
}

void TutorialManager::Update() {
  UpdateUI();
  UpdateMessages();

  // チュートリアル完了後の待機演出
  if (m_isCompletedDisplay) {
    m_completeWaitTime += kFrameTime;
    if (m_completeWaitTime >= kCompleteWaitTime) {
      m_isCompletedDisplay = false;
      m_step = Step::Completed;
    }
    return; // 待機中は他の処理をしない
  }

  // アニメタイマーを進める
  if (m_isMoveCheckAnim)
    m_moveCheckAnimTime += kFrameTime;
  if (m_isViewCheckAnim)
    m_viewCheckAnimTime += kFrameTime;
  if (m_isJumpCheckAnim)
    m_jumpCheckAnimTime += kFrameTime;
  if (m_isRunCheckAnim)
    m_runCheckAnimTime += kFrameTime;

  // UIが表示されているときだけ入力チェック
  if (m_uiState != UIState::OnScreen)
    return;

  // ステップ完了後の待機処理
  if (m_isStepCompleted) {
    m_stepCompleteWaitTime += kFrameTime;
    if (m_stepCompleteWaitTime >= kStepCompleteWaitTime) {
      m_isStepCompleted = false;
      m_uiState = UIState::Exiting; // 退場開始
    }
    return; // 待機中は他の入力を受け付けない
  }

  switch (m_step) {
  case Step::Move:
    if (!m_isMoveDone) {
      bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) ||
                      CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);
      if (isMoving)
        m_moveAccumTime += kFrameTime;
      if (m_moveAccumTime >= kMoveAccumGoalTime) {
        m_isMoveDone = true;
        m_isMoveCheckAnim = true;
        m_moveCheckAnimTime = 0.0f;
      }
    } else if (m_moveCheckAnimTime >=
               kCheckAnimDuration) // チェックアニメ完了後
    {
      m_isStepCompleted = true; // 待機開始
      m_stepCompleteWaitTime = 0.0f;
    }
    break;
  case Step::View:
    if (!m_isViewDone) {
      Vec2 now = InputManager::GetInstance()->GetMousePos();
      float dx = now.x - m_prevMousePos.x;
      float dy = now.y - m_prevMousePos.y;

      if (std::abs(dx) > kMouseMovementThreshold ||
          std::abs(dy) > kMouseMovementThreshold) {
        m_viewAccumTime += kFrameTime;
      }

      if (m_viewAccumTime >= kViewAccumGoalTime) {
        m_isViewDone = true;
        m_isViewCheckAnim = true;
        m_viewCheckAnimTime = 0.0f;
      }
      m_prevMousePos = now;
    } else if (m_viewCheckAnimTime >= kCheckAnimDuration) {
      m_isStepCompleted = true; // 待機開始
      m_stepCompleteWaitTime = 0.0f;
    }
    break;
  case Step::Jump:
    if (!m_isJumpDone) {
      if (CheckHitKey(KEY_INPUT_SPACE))
        m_jumpAccumTime += kFrameTime;
      if (m_jumpAccumTime >= kJumpAccumGoalTime) {
        m_isJumpDone = true;
        m_isJumpCheckAnim = true;
        m_jumpCheckAnimTime = 0.0f;
      }
    } else if (m_jumpCheckAnimTime >= kCheckAnimDuration) {
      m_isStepCompleted = true; // 待機開始
      m_stepCompleteWaitTime = 0.0f;
    }
    break;
  case Step::Run:
    if (!m_isRunDone) {
      if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT)) {
        m_runAccumTime += kFrameTime;
      }

      if (m_runAccumTime >= kRunAccumGoalTime) {
        m_isRunDone = true;
        m_isRunCheckAnim = true;
        m_runCheckAnimTime = 0.0f;
      }
    } else if (m_runCheckAnimTime >= kCheckAnimDuration) {
      m_isStepCompleted = true; // 待機開始
      m_stepCompleteWaitTime = 0.0f;
    }
    break;
  }
}

void TutorialManager::Draw(int screenW, int screenH) {
  if (m_uiState == UIState::Hidden && m_messages.empty())
    return;

  DrawMessages(screenW, screenH);

  const char *text = "";
  bool is_done = false;
  bool is_check_anim = false;
  float check_anim_time = 0.0f;

  switch (m_step) {
  case Step::Move: {
    is_done = m_isMoveDone;
    is_check_anim = m_isMoveCheckAnim;
    check_anim_time = m_moveCheckAnimTime;

    const char *remaining_text = "で移動しよう!";
    int remaining_text_width = GetDrawStringWidthToHandle(
        remaining_text, strlen(remaining_text), m_japaneseFontHandle);

    int images_width = kKeyImageSize * 4 + kKeyImageSpacing * 3;
    int box_width = images_width + remaining_text_width + kCheckMarkBaseSize +
                    kBoxPaddingX * 2;
    int box_height = kKeyImageSize + kBoxPaddingY * 2;

    int box_x = screenW - box_width - 20 + m_uiXOffset;
    int box_y = 20;

    // 半透明の背景ボックスを描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
            true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // キー画像を描画
    int image_x = box_x + kBoxPaddingX;
    int image_y = box_y + kBoxPaddingY;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_wKeyHandle, true);
    image_x += kKeyImageSize + kKeyImageSpacing;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_aKeyHandle, true);
    image_x += kKeyImageSize + kKeyImageSpacing;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_sKeyHandle, true);
    image_x += kKeyImageSize + kKeyImageSpacing;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_dKeyHandle, true);

    // 残りのテキストを描画
    int text_x = image_x + kKeyImageSize + kKeyImageSpacing;
    int text_y = box_y + (box_height - kFontSize) * 0.5f;
    DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff,
                       m_japaneseFontHandle);

    // チェックマークを描画
    if (is_done && m_checkMarkHandle >= 0) {
      float scale = 1.0f;
      if (is_check_anim && check_anim_time < kCheckAnimDuration) {
        float t = check_anim_time / kCheckAnimDuration;
        scale = kCheckMarkAnimScale - t;
        if (scale < 1.0f)
          scale = 1.0f;
      }

      int size = static_cast<int>(kCheckMarkBaseSize * scale);
      int cx = text_x + remaining_text_width + kBoxPaddingX +
               (kCheckMarkBaseSize * 0.5f);
      int cy = box_y + box_height * 0.5f;
      DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f,
                      cy + size * 0.5f, m_checkMarkHandle, true);
    }
  } break;
  case Step::View: {
    is_done = m_isViewDone;
    is_check_anim = m_isViewCheckAnim;
    check_anim_time = m_viewCheckAnimTime;

    const char *remaining_text = "で視点を動かそう!";
    int remaining_text_width = GetDrawStringWidthToHandle(
        remaining_text, strlen(remaining_text), m_japaneseFontHandle);

    int images_width = kKeyImageSize;
    int box_width = images_width + remaining_text_width + kCheckMarkBaseSize +
                    kBoxPaddingX * 2;
    int box_height = kKeyImageSize + kBoxPaddingY * 2;
    int box_x = screenW - box_width - 20 + m_uiXOffset;
    int box_y = 20;

    // 半透明の背景ボックスを描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
            true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // キー画像を描画
    int image_x = box_x + kBoxPaddingX;
    int image_y = box_y + kBoxPaddingY;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_mouseMoveHorHandle, true);

    // 残りのテキストを描画
    int text_x = image_x + kKeyImageSize + 5;
    int text_y = box_y + (box_height - kFontSize) * 0.5f;
    DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff,
                       m_japaneseFontHandle);

    // チェックマークを描画
    if (is_done && m_checkMarkHandle >= 0) {
      float scale = 1.0f;
      if (is_check_anim && check_anim_time < kCheckAnimDuration) {
        float t = check_anim_time / kCheckAnimDuration;
        scale = kCheckMarkAnimScale - t;
        if (scale < 1.0f)
          scale = 1.0f;
      }

      int size = static_cast<int>(kCheckMarkBaseSize * scale);
      int cx = text_x + remaining_text_width + kBoxPaddingX +
               (kCheckMarkBaseSize * 0.5f);
      int cy = box_y + box_height * 0.5f;
      DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f,
                      cy + size * 0.5f, m_checkMarkHandle, true);
    }
  } break;
  case Step::Jump: {
    is_done = m_isJumpDone;
    is_check_anim = m_isJumpCheckAnim;
    check_anim_time = m_jumpCheckAnimTime;

    const char *remaining_text = "でジャンプ!";
    int remaining_text_width = GetDrawStringWidthToHandle(
        remaining_text, strlen(remaining_text), m_japaneseFontHandle);

    int images_width = kKeyImageWidth;
    int box_width = images_width + remaining_text_width + kCheckMarkBaseSize +
                    kBoxPaddingX * 2;
    int box_height = kKeyImageHeight + kBoxPaddingY * 2;

    int box_x = screenW - box_width - 20 + m_uiXOffset;
    int box_y = 20;

    // 半透明の背景ボックスを描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
            true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // キー画像を描画
    int image_x = box_x + kBoxPaddingX;
    int image_y = box_y + kBoxPaddingY;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageWidth,
                    image_y + kKeyImageHeight, m_spaceKeyHandle, true);

    // 残りのテキストを描画
    int text_x = image_x + kKeyImageWidth + 5;
    int text_y = box_y + (box_height - kFontSize) * 0.5f;
    DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff,
                       m_japaneseFontHandle);

    // チェックマークを描画
    if (is_done && m_checkMarkHandle >= 0) {
      float scale = 1.0f;
      if (is_check_anim && check_anim_time < kCheckAnimDuration) {
        float t = check_anim_time / kCheckAnimDuration;
        scale = kCheckMarkAnimScale - t;
        if (scale < 1.0f)
          scale = 1.0f;
      }

      int size = static_cast<int>(kCheckMarkBaseSize * scale);
      int cx = text_x + remaining_text_width + kBoxPaddingX +
               (kCheckMarkBaseSize * 0.5f);
      int cy = box_y + box_height * 0.5f;
      DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f,
                      cy + size * 0.5f, m_checkMarkHandle, true);
    }
  } break;
  case Step::Run: {
    is_done = m_isRunDone;
    is_check_anim = m_isRunCheckAnim;
    check_anim_time = m_runCheckAnimTime;

    const char *remaining_text = "で走ろう!";
    int remaining_text_width = GetDrawStringWidthToHandle(
        remaining_text, strlen(remaining_text), m_japaneseFontHandle);

    int images_width =
        kShiftImageWidth + kKeyImageSize + kKeyImageSize + kKeyImageSpacing * 2;
    int box_width = images_width + remaining_text_width + kCheckMarkBaseSize +
                    kBoxPaddingX * 2;
    int box_height = kShiftImageWidth * 0.5f + kBoxPaddingY * 2;

    int box_x = screenW - box_width - 20 + m_uiXOffset;
    int box_y = 20;

    // 半透明の背景ボックスを描画
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
            true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // キー画像を描画
    int image_x = box_x + kBoxPaddingX;
    int image_y = box_y + kBoxPaddingY;
    DrawExtendGraph(image_x, image_y, image_x + kShiftImageWidth,
                    image_y + kKeyImageSize, m_leftShiftKeyHandle, true);
    image_x += kShiftImageWidth + kKeyImageSpacing;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_crossHandle, true);
    image_x += kKeyImageSize + kKeyImageSpacing;
    DrawExtendGraph(image_x, image_y, image_x + kKeyImageSize,
                    image_y + kKeyImageSize, m_wKeyHandle, true);

    // 残りのテキストを描画
    int text_x = image_x + kKeyImageSize + 5;
    int text_y = box_y + (box_height - kFontSize) * 0.5f;
    DrawStringToHandle(text_x, text_y, remaining_text, 0xffffff,
                       m_japaneseFontHandle);

    // チェックマークを描画
    if (is_done && m_checkMarkHandle >= 0) {
      float scale = kCheckMarkDrawSize;
      if (is_check_anim && check_anim_time < kCheckAnimDuration) {
        float t = check_anim_time / kCheckAnimDuration;
        scale = kCheckMarkAnimScale - t;
        if (scale < 1.0f)
          scale = 1.0f;
      }

      int size = static_cast<int>(kCheckMarkBaseSize * scale);
      int cx = text_x + remaining_text_width + kBoxPaddingX +
               (kCheckMarkBaseSize * 0.5f);
      int cy = box_y + box_height * 0.5f;
      DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f,
                      cy + size * 0.5f, m_checkMarkHandle, true);
    }
  } break;
  default:
    return;

    if (m_step != Step::Move && m_step != Step::View && m_step != Step::Jump &&
        m_step != Step::Run) {
      int text_width =
          GetDrawStringWidthToHandle(text, strlen(text), m_japaneseFontHandle);
      int box_width = text_width + kCheckMarkBaseSize + kBoxPaddingX * 2;
      int box_height = kCheckMarkBaseSize + kBoxPaddingY * 2;
      int box_x = screenW - box_width - 20 + m_uiXOffset;
      int box_y = 20;

      // 半透明の背景ボックスを描画
      SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
      DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
              true);
      SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

      // テキストを描画
      int text_x = box_x + kBoxPaddingX;
      int text_y = box_y + (box_height - kFontSize) * 0.5f;
      DrawStringToHandle(text_x, text_y, text, 0xffffff, m_japaneseFontHandle);

      // チェックマークを描画
      if (is_done && m_checkMarkHandle >= 0) {
        float scale = 1.0f;
        if (is_check_anim && check_anim_time < kCheckAnimDuration) {
          float t = check_anim_time / kCheckAnimDuration;
          scale = kCheckMarkAnimScale - t;
          if (scale < 1.0f)
            scale = 1.0f;
        }

        int size = static_cast<int>(kCheckMarkBaseSize * scale);
        int cx =
            text_x + text_width + kBoxPaddingX + (kCheckMarkBaseSize * 0.5f);
        int cy = box_y + box_height * 0.5f;
        DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f,
                        cy + size * 0.5f, m_checkMarkHandle, true);
      }
    }
  }
}

// チュートリアルがアクティブかどうか
bool TutorialManager::IsActive() const {
  return m_step != Step::None && m_step != Step::Completed;
}

// チュートリアルが完了したかどうか
bool TutorialManager::IsCompleted() const { return m_step == Step::Completed; }

void TutorialManager::AddMessage(const std::string &title,
                                 const std::string &detail) {
  m_messages.push_back(
      {title, detail, kUIOffscreenOffsetX, 0.0f, UIState::Entering});
}

void TutorialManager::UpdateMessages() {
  for (auto &msg : m_messages) {
    switch (msg.state) {
    case UIState::Entering:
      msg.x_offset -= kUIAnimationSpeed;
      if (msg.x_offset <= 0.0f) {
        msg.x_offset = 0.0f;
        msg.state = UIState::OnScreen;
        msg.display_timer = 0.0f;
      }
      break;
    case UIState::OnScreen:
      msg.display_timer += kFrameTime;
      if (msg.display_timer >= kMessageDisplayTime) {
        msg.state = UIState::Exiting;
      }
      break;
    case UIState::Exiting:
      msg.x_offset += kUIAnimationSpeed;
      if (msg.x_offset >= kUIOffscreenOffsetX) {
        msg.state = UIState::Hidden;
      }
      break;
    case UIState::Hidden:
      break;
    }
  }

  // 隠れたメッセージを削除
  m_messages.erase(std::remove_if(m_messages.begin(), m_messages.end(),
                                  [](const TutorialMessage &msg) {
                                    return msg.state == UIState::Hidden;
                                  }),
                   m_messages.end());
}

void TutorialManager::DrawMessages(int screenW, int screenH) {
  int y_pos = kMessageOffsetY;
  for (const auto &msg : m_messages) {
    // detail文字列を'\n'で分割
    std::vector<std::string> detail_lines;
    std::string current_line;
    std::istringstream iss(msg.detail);
    while (std::getline(iss, current_line, '\n')) {
      detail_lines.push_back(current_line);
    }

    // 幅の計算
    int title_width = GetDrawStringWidthToHandle(
        msg.title.c_str(), msg.title.length(), m_japaneseLargeFontHandle);
    int max_detail_width = 0;
    for (const auto &line : detail_lines) {
      int line_width = GetDrawStringWidthToHandle(line.c_str(), line.length(),
                                                  m_messageDetailFontHandle);
      if (line_width > max_detail_width) {
        max_detail_width = line_width;
      }
    }
    int box_width =
        (std::max)(title_width, max_detail_width) + kBoxPaddingX * 2;

    // 高さの計算
    int box_height = kBoxPaddingY;
    box_height += kMessageTitleFontSize;
    box_height += kMessageTimeBarPaddingY;
    box_height += kMessageTimeBarHeight;
    box_height += kMessageTimeBarPaddingY;
    box_height += kMessageDetailFontSize * detail_lines.size();
    if (detail_lines.size() > 1) {
      box_height += kMessageLineSpacing * (detail_lines.size() - 1);
    }
    box_height += kBoxPaddingY;

    // 描画
    int box_x = screenW - box_width - 20 + msg.x_offset;
    int box_y = y_pos;

    // 背景ボックス
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
    DrawBox(box_x, box_y, box_x + box_width, box_y + box_height, kBoxColor,
            true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // テキストとタイムバーを上から順に描画
    int current_y = box_y + kBoxPaddingY;
    int content_x = box_x + kBoxPaddingX;
    int content_width = box_width - kBoxPaddingX * 2;

    // タイトル
    DrawStringToHandle(content_x, current_y, msg.title.c_str(), 0xFFFFFF,
                       m_japaneseLargeFontHandle);
    current_y += kMessageTitleFontSize + kMessageTimeBarPaddingY;

    // タイムバー
    if (msg.state == UIState::OnScreen) {
      float progress = 1.0f - (msg.display_timer / kMessageDisplayTime);
      int bar_width = static_cast<int>(content_width * progress);
      DrawBox(content_x, current_y, content_x + bar_width,
              current_y + kMessageTimeBarHeight, 0xFF0000, true);
    }
    current_y += kMessageTimeBarHeight + kMessageTimeBarPaddingY;

    // 詳細 (複数行、色分けあり)
    for (const auto &line : detail_lines) {
      int current_x = content_x;

      std::string s = line;
      std::string keyword1 = "回復アイテム";
      std::string keyword2 = "積極的に行動せよ";
      size_t pos1 = s.find(keyword1);
      size_t pos2 = s.find(keyword2);

      if (pos1 != std::string::npos) {
        std::string part1 = s.substr(0, pos1);
        std::string part3 = s.substr(pos1 + keyword1.length());

        DrawStringToHandle(current_x, current_y, part1.c_str(), 0xFFFFFF,
                           m_messageDetailFontHandle);
        current_x += GetDrawStringWidthToHandle(part1.c_str(), part1.length(),
                                                m_messageDetailFontHandle);

        DrawStringToHandle(current_x, current_y, keyword1.c_str(), 0xFFD700,
                           m_messageDetailFontHandle);
        current_x += GetDrawStringWidthToHandle(
            keyword1.c_str(), keyword1.length(), m_messageDetailFontHandle);

        DrawStringToHandle(current_x, current_y, part3.c_str(), 0xFFFFFF,
                           m_messageDetailFontHandle);
      } else if (pos2 != std::string::npos) {
        std::string part1 = s.substr(0, pos2);

        DrawStringToHandle(current_x, current_y, part1.c_str(), 0xFFFFFF,
                           m_messageDetailFontHandle);
        current_x += GetDrawStringWidthToHandle(part1.c_str(), part1.length(),
                                                m_messageDetailFontHandle);

        DrawStringToHandle(current_x, current_y, keyword2.c_str(), 0xFFD700,
                           m_messageDetailFontHandle);
      } else {
        DrawStringToHandle(content_x, current_y, line.c_str(), 0xFFFFFF,
                           m_messageDetailFontHandle);
      }

      current_y += kMessageDetailFontSize + kMessageLineSpacing;
    }

    y_pos += box_height + 10;
  }
}
