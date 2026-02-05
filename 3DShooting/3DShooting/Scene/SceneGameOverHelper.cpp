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
