#pragma once
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// デバッグメニューを管理するクラス
/// </summary>
class DebugMenu
{
public:
    struct MenuItem
    {
        std::string name;
        std::vector<MenuItem> children;
        std::function<void()> action;
        std::function<std::string()> stateTextGetter; // 状態表示用のテキストを取得する関数
        bool isOpen = false;
    };

    DebugMenu();
    ~DebugMenu();

    void Update();
    void Draw(int x, int y);

private:
    void DrawItem(MenuItem& item, int& x, int& y, int depth, const std::vector<int>& currentPath, const std::vector<int>& selectedPath, int mouseX, int mouseY, bool leftClicked);
    void HandleInput();
    MenuItem* GetSelectedItem();

    MenuItem m_root;
    std::vector<int> m_selectedPath;
};

