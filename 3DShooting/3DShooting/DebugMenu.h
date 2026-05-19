#pragma once
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// デバッグメニューを管理するクラス。
/// ツリー構造のメニューをマウスで操作し、ゲームのデバッグ機能を切り替える。
/// </summary>
class DebugMenu
{
public:
    /// <summary>
    /// メニューの 1 項目を表す構造体
    /// </summary>
    struct MenuItem
    {
        std::string                  name;           // 項目名
        std::vector<MenuItem>        children;        // 子項目リスト
        std::function<void()>        action;          // 項目実行時のコールバック
        std::function<std::string()> stateTextGetter; // 状態テキストを返すコールバック
        bool                         isOpen = false;  // 子項目リストが展開されているか
    };

    /// <summary>
    /// コンストラクタ（メニュー構造を構築し初期選択パスを設定する）
    /// </summary>
    DebugMenu();
    ~DebugMenu() = default;

    /// <summary>
    /// 毎フレームの入力処理を行う
    /// </summary>
    void Update();

    /// <summary>
    /// メニューを描画する
    /// </summary>
    /// <param name="x">描画開始 X 座標</param>
    /// <param name="y">描画開始 Y 座標</param>
    void Draw(int x, int y);

private:
    /// <summary>
    /// メニューアイテムを再帰的に描画する
    /// </summary>
    /// <param name="item">描画するメニューアイテム</param>
    /// <param name="x">描画 X 座標</param>
    /// <param name="y">描画 Y 座標（描画後に更新される）</param>
    /// <param name="depth">階層の深さ</param>
    /// <param name="currentPath">このアイテムのパス</param>
    /// <param name="selectedPath">現在選択中のパス</param>
    /// <param name="mouseX">マウスの X 座標</param>
    /// <param name="mouseY">マウスの Y 座標</param>
    /// <param name="leftClicked">このフレームに左クリックされたかどうか</param>
    void DrawItem(MenuItem& item, int& x, int& y, int depth, const std::vector<int>& currentPath, const std::vector<int>& selectedPath, int mouseX, int mouseY, bool leftClicked);

    /// <summary>
    /// マウス入力によるメニュー操作を処理する
    /// </summary>
    void HandleInput();

    /// <summary>
    /// 現在選択されているメニューアイテムを取得する
    /// </summary>
    /// <returns>選択中のアイテムへのポインタ。無効なパスの場合は nullptr</returns>
    MenuItem* GetSelectedItem();

    /// <summary>プレイヤー設定メニューを構築して返す</summary>
    MenuItem CreatePlayerMenu();

    /// <summary>敵設定メニューを構築して返す</summary>
    MenuItem CreateEnemyMenu();

    /// <summary>シーン切り替えメニューを構築して返す</summary>
    MenuItem CreateSceneMenu();

    /// <summary>アイテム設定メニューを構築して返す</summary>
    MenuItem CreateItemMenu();

    /// <summary>画面設定メニューを構築して返す</summary>
    MenuItem CreateScreenMenu();

    /// <summary>ステージ・コリジョン設定メニューを構築して返す</summary>
    MenuItem CreateStageMenu();

    MenuItem         m_root;         // ルートメニューアイテム
    std::vector<int> m_selectedPath; // 現在選択中のアイテムへのパス（インデックス列）
};
