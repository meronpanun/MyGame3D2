#include "ScoreManager.h"
#include <cmath>

ScoreManager& ScoreManager::Instance() {
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager() : m_score(0), m_combo(0) {}

int ScoreManager::AddScore(bool isHeadShot) {
    int baseScore = isHeadShot ? 200 : 100;
    m_combo++;
    float comboRate = std::pow(1.1f, m_combo - 1);
    m_lastComboRate = comboRate;
    int add = static_cast<int>(baseScore * comboRate);
    m_score += add;
    m_comboTimer = kComboGraceFrame; // コンボ猶予リセット
    if (isHeadShot) {
        m_headKillCount++;
    } else {
        m_bodyKillCount++;
    }
    return add;
}

void ScoreManager::Update() {
    if (m_combo > 0 && m_comboTimer > 0) {
        m_comboTimer--;
        if (m_comboTimer <= 0) {
            m_combo = 0;
            m_lastComboRate = 1.0f;
        }
    }
}

void ScoreManager::ResetCombo() {
    m_combo = 0;
}

int ScoreManager::GetScore() const {
    return m_score;
}

int ScoreManager::GetCombo() const {
    return m_combo;
}
