#include "BossUI.h"
#include "EnemyBase.h"
#include "Game.h"
#include <algorithm>

namespace {
// ボスHPバー関連
constexpr int kBossHpBarWidth = 900;
constexpr int kBossHpBarHeight = 24;
constexpr int kBossHpBarY = 165;
constexpr int kBossHpTextY = 120;

// 色関連
constexpr unsigned int kColorWhite = 0xffffff;
constexpr unsigned int kColorHpBarBg = 0x303030;
constexpr unsigned int kColorHpBarFill = 0xcc0000;   // ボスは真紅
constexpr unsigned int kColorHpBarDamage = 0xaaaaaa; // 被ダメ時はグレー
constexpr unsigned int kColorHpBarBorder = 0xffffff;

constexpr float kAnimSpeed = 0.05f; // HPバーの追従速度
} // namespace

BossUI::BossUI() : m_healthBarAnim(0.0f), m_fontHandle(-1) {
  m_fontHandle = CreateFontToHandle("ＭＳ ゴシック", 36, 3,
                                    DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
}

BossUI::~BossUI() {
  if (m_fontHandle != -1) {
    DeleteFontToHandle(m_fontHandle);
  }
}

void BossUI::Draw(const std::vector<std::shared_ptr<EnemyBase>> &enemyList) {
  // 生存しているボスを探す
  EnemyBase *pBoss = nullptr;
  for (auto &enemy : enemyList) {
    if (enemy->IsBoss() && enemy->IsAlive()) {
      pBoss = enemy.get();
      break;
    }
  }

  if (!pBoss) {
    m_healthBarAnim = 0.0f;
    return;
  }

  float hp = pBoss->GetHp();
  float maxHp = pBoss->GetMaxHp();

  // 初回時やリセット用
  if (m_healthBarAnim <= 0.0f) {
    m_healthBarAnim = hp;
  }

  // アニメーション更新
  if (m_healthBarAnim > hp) {
    m_healthBarAnim -= (m_healthBarAnim - hp) * kAnimSpeed;
    if (m_healthBarAnim < hp)
      m_healthBarAnim = hp;
  } else if (m_healthBarAnim < hp) {
    m_healthBarAnim = hp;
  }

  DrawBossHPBar(hp, maxHp);
}

void BossUI::DrawBossHPBar(float hp, float maxHp) {
  int screenW = Game::kScreenWidth;
  GetScreenState(&screenW, NULL, NULL);

  int barX = (screenW - kBossHpBarWidth) / 2;
  int barY = kBossHpBarY;

  // HP割合
  float hpRate = hp / maxHp;
  float animRate = m_healthBarAnim / maxHp;

  // 背景
  DrawBox(barX, barY, barX + kBossHpBarWidth, barY + kBossHpBarHeight,
          kColorHpBarBg, true);

  // アニメーションバー（ダメージ演出用）
  if (m_healthBarAnim > hp) {
    int animW = static_cast<int>(kBossHpBarWidth * animRate);
    DrawBox(barX, barY, barX + animW, barY + kBossHpBarHeight,
            kColorHpBarDamage, true);
  }

  // HPバー本体
  int hpW = static_cast<int>(kBossHpBarWidth * hpRate);
  DrawBox(barX, barY, barX + hpW, barY + kBossHpBarHeight, kColorHpBarFill,
          true);

  // 枠
  DrawBox(barX, barY, barX + kBossHpBarWidth, barY + kBossHpBarHeight,
          kColorHpBarBorder, false);

  // ボス名テキスト
  const char *bossName = "BOSS";
  int textW = GetDrawStringWidthToHandle(
      bossName, static_cast<int>(strlen(bossName)), m_fontHandle);
  DrawStringToHandle((screenW - textW) / 2, kBossHpTextY, bossName, kColorWhite,
                     m_fontHandle);
}
