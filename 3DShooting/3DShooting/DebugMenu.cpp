#include "DebugMenu.h"
#include "DxLib.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include "SceneOption.h"
#include "SceneGameOver.h"
#include "Stage.h"
#include <cassert>

namespace
{
    // UI定数
	constexpr int   kIndentWidth          = 20;  // インデント幅
	constexpr int   kItemWidth            = 200; // アイテムの幅
	constexpr int   kItemHeight           = 15;  // アイテムの高さ
	constexpr int   kTextLineSpacing      = 20;  // テキスト行間隔
	constexpr int   kIndicatorOffsetX     = -10; // インジケーターのXオフセット
	constexpr int   kIndicatorTextOffset  = 10;  // インジケーターとテキストの間隔
	constexpr int   kIndicatorOffsetY     = 14;  // インジケーターのYオフセット
    constexpr float kIndicatorClosedAngle = -DX_PI_F * 0.5f; // 90度時計回り
}


DebugMenu::DebugMenu()
{
    // メニュー構造の定義
    m_root.name = "Root";
    m_root.children = {
        {"Character", {
            {"Player", {
                {"Invincible", {}, [this]() {
                    if (Game::m_pPlayer)
                    {
                        bool isInvincible = !Game::m_pPlayer->IsInvincible();
                        Game::m_pPlayer->SetInvincible(isInvincible);
                    }
                }, []() {
                    if (Game::m_pPlayer)
                    {
                        return Game::m_pPlayer->IsInvincible() ? "[ON]" : "[OFF]";
                    }
                    return "[N/A]";
                }},
                {"Infinite Ammo", {}, [this]() {
                    if (Game::m_pPlayer)
                    {
                        bool isInfiniteAmmo = !Game::m_pPlayer->IsInfiniteAmmo();
                        Game::m_pPlayer->SetInfiniteAmmo(isInfiniteAmmo);
                    }
                }, []() {
                    if (Game::m_pPlayer)
                    {
                        return Game::m_pPlayer->IsInfiniteAmmo() ? "[ON]" : "[OFF]";
                    }
                    return "[N/A]";
                }},
                {"Flight Mode", {}, [this]() {
                    if (Game::m_pPlayer)
                    {
                        bool isFlightMode = !Game::m_pPlayer->IsFlightMode();
                        Game::m_pPlayer->SetFlightMode(isFlightMode);
                    }
                }, []() {
                    if (Game::m_pPlayer)
                    {
                        return Game::m_pPlayer->IsFlightMode() ? "[ON]" : "[OFF]";
                    }
                    return "[N/A]";
                }}
            }, nullptr},
            {"Enemy", {}, nullptr}
        }},
        {"Scene", {
            {"Skip Tutorial", {}, []() {
                SceneMain::s_isSkipTutorial = !SceneMain::s_isSkipTutorial;
            }, []() {
                return SceneMain::s_isSkipTutorial ? "[ON]" : "[OFF]";
            }},
            {"TitleScene", {}, []() {
                if (Game::m_pSceneManager)
                {
                    Game::m_pSceneManager->RequestChangeScene(new SceneTitle(false));
                }
            }},
            {"MainScene", {}, []() {
                if (Game::m_pSceneManager)
                {
                    Game::m_pSceneManager->RequestChangeScene(new SceneMain(false));
                }
            }},
            {"ResultScene", {}, []() {
                if (Game::m_pSceneManager)
                {
                    Game::m_pSceneManager->RequestChangeScene(new SceneResult());
                }
            }},
            {"OptionScene", {}, []() {
                if (Game::m_pSceneManager)
                {
                    // オプション画面は現在のシーンを引数に取るが、デバッグ遷移なのでnullptrを渡す
                    Game::m_pSceneManager->RequestChangeScene(new SceneOption(nullptr));
                }
            }},
            {"GameOverScene", {}, []() {
                if (Game::m_pSceneManager)
                {
                    // ゲームオーバー画面はwave, killCount, scoreを引数に取るので、適当な値を渡す
                    Game::m_pSceneManager->RequestChangeScene(new SceneGameOver(0, 0, 0));
                }
            }}
        }},
        {"Item", {}, nullptr},
        {"Collision", {
            {"Stage Collision", {}, []() {
                bool isDraw = !Stage::IsDrawCollision();
                Stage::SetDrawCollision(isDraw);
            }, []() {
                return Stage::IsDrawCollision() ? "[ON]" : "[OFF]";
            }}
        }}
    };
    m_selectedPath = {};
}

DebugMenu::~DebugMenu()
{
}

void DebugMenu::Update()
{
    HandleInput();
}

void DebugMenu::Draw(int x, int y)
{
    int currentY = y;
    std::vector<int> currentPath;
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    static int prevLeftClick = 0;
    int nowLeftClick = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    bool leftClicked = (nowLeftClick && !prevLeftClick);

    for (size_t i = 0; i < m_root.children.size(); ++i)
    {
        currentPath = { (int)i };
        DrawItem(m_root.children[i], x, currentY, 0, currentPath, m_selectedPath, mouseX, mouseY, leftClicked);
    }

    prevLeftClick = nowLeftClick;
}

void DebugMenu::DrawItem(MenuItem& item, int& x, int& y, int depth, const std::vector<int>& currentPath, const std::vector<int>& selectedPath, int mouseX, int mouseY, bool leftClicked)
{
    bool isSelected = (currentPath == selectedPath);

    int itemX = x + depth * kIndentWidth;
    int itemY = y;
    int itemWidth = kItemWidth;
    int itemHeight = kItemHeight;

    bool isHovered = (mouseX >= itemX && mouseX <= itemX + itemWidth &&
        mouseY >= itemY && mouseY <= itemY + itemHeight);

    if (isHovered && leftClicked)
    {
        m_selectedPath = currentPath;
        isSelected = true;
        if (item.action)
        {
            item.action();
        }
        else if (!item.children.empty())
        {
            item.isOpen = !item.isOpen;
        }
    }

    int color = isHovered ? 0x0000ff : 0xffffff;
    // 子項目がある場合、開閉状態に応じたインジケーター（三角形文字）を描画
    if (!item.children.empty())
    {
        const TCHAR* indicatorChar = _T("▼");// 下向きの三角形文字を使用
        float rotationAngle = 0.0f; // 閉じた状態: 右向き

        if (!item.isOpen)  // 閉じた状態の場合、右向きに回転
        {
            rotationAngle = kIndicatorClosedAngle; // 90度時計回り
        }

        // 文字のサイズを取得して、回転の中心と描画位置を調整
        int charWidth = GetDrawStringWidth(indicatorChar, 1); // '▼' の幅を取得
        int charHeight = GetFontSize(); // 現在のフォントの高さ

        // 回転の中心を文字の中心に設定
        double rotCenterX = charWidth * 0.5f;
        double rotCenterY = charHeight * 0.5f;

        // 描画位置を計算
        int indicatorDrawX = itemX + kIndicatorOffsetX;
        int indicatorDrawY = itemY + (itemHeight * 0.5f) - (charHeight * 0.5f) + kIndicatorOffsetY;

        // DrawRotaString で回転して描画
        DrawRotaString(indicatorDrawX, indicatorDrawY,
            1.0, 1.0,
            rotCenterX, rotCenterY,
            rotationAngle, color, 0, false, indicatorChar);
    }

    // テキストの描画
    int textStartX = itemX;
    if (!item.children.empty())
    {
        textStartX += kIndicatorTextOffset; // インジケータの分だけテキストを右にずらす
    }

    std::string displayText = item.name;
    if (item.stateTextGetter)
    {
        displayText += " " + item.stateTextGetter();
    }

    DrawString(textStartX, itemY, displayText.c_str(), color);
    y += kTextLineSpacing;

    if (item.isOpen && !item.children.empty())
    {
        std::vector<int> childPath = currentPath;
        childPath.push_back(0);
        for (size_t i = 0; i < item.children.size(); ++i)
        {
            childPath.back() = i;
            DrawItem(item.children[i], x, y, depth + 1, childPath, selectedPath, mouseX, mouseY, leftClicked);
        }
    }
}

void DebugMenu::HandleInput()
{
    static int prevRightClick = 0;
    int nowRightClick = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;

    if (nowRightClick && !prevRightClick)
    {
        if (m_selectedPath.size() > 1)
        {
            m_selectedPath.pop_back();
            MenuItem* selected = GetSelectedItem();
            if (selected)
            {
                selected->isOpen = false;
            }
        }
    }

    prevRightClick = nowRightClick;
}

DebugMenu::MenuItem* DebugMenu::GetSelectedItem()
{
    MenuItem* item = &m_root;
    for (int index : m_selectedPath)
    {
        if (index < item->children.size())
        {
            item = &item->children[index];
        }
        else
        {
            return nullptr;
        }
    }
    return item;
}