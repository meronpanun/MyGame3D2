#include "DebugMenu.h"
#include "DxLib.h"

DebugMenu::DebugMenu()
{
    // メニュー構造の定義
    m_root.name = "Root";
    m_root.children = {
        {"Character", {
            {"Player", {}, nullptr},
            {"Enemy", {}, nullptr}
        }},
        {"Scene", {}, nullptr},
        {"Item", {}, nullptr}
    };
    m_selectedPath = {};
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
        currentPath = {(int)i};
        DrawItem(m_root.children[i], x, currentY, 0, currentPath, m_selectedPath, mouseX, mouseY, leftClicked);
    }

    prevLeftClick = nowLeftClick;
}

void DebugMenu::DrawItem(MenuItem& item, int& x, int& y, int depth, const std::vector<int>& currentPath, const std::vector<int>& selectedPath, int mouseX, int mouseY, bool leftClicked)
{
    bool isSelected = (currentPath == selectedPath);

    int itemX = x + depth * 20;
    int itemY = y;
    int itemWidth = 150;
    int itemHeight = 20;

    bool isHovered = (mouseX >= itemX && mouseX <= itemX + itemWidth &&
                      mouseY >= itemY && mouseY <= itemY + itemHeight);

    if (isHovered && leftClicked)
    {
        m_selectedPath = currentPath;
        isSelected = true;
        if (!item.children.empty()) {
            item.isOpen = !item.isOpen;
        }
    }

    int color = isHovered ? 0x0000ff : 0xffffff;
    DrawString(itemX, itemY, item.name.c_str(), color);
    y += 20;

    if (item.isOpen && !item.children.empty())
    {        std::vector<int> childPath = currentPath;
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
        if (m_selectedPath.size() > 1) {
            m_selectedPath.pop_back();
            MenuItem* selected = GetSelectedItem();
            if (selected) {
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
