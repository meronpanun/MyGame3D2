#pragma once
#include <string>

/// <summary>
/// アニメーション管理クラス
/// </summary>
class AnimationManager
{
public:
    struct AnimData {
        int attachNo = -1;    // アタッチされているアニメーションの番号
        float count = 0.0f;   // アニメーションのカウント
        bool isLoop = false;  // ループアニメーションかどうか
        bool isEnd = false;   // アニメーションが終了したか
        float blendRate = 0.0f; // ブレンド率
        std::string animName; // アニメーション名
    };

    // アニメーションをアタッチ
    void Attach(int modelHandle, AnimData& data, const char* animName, bool isLoop);
    // アニメーションの更新
    void Update(int modelHandle, AnimData& data, float animSpeed = 1.0f);
    // アニメーションの変更（ブレンド対応）
    void Change(int modelHandle, AnimData& prev, AnimData& next, const char* animName, bool isLoop);
    // アニメーションのブレンドを更新
    void UpdateBlend(int modelHandle, AnimData& prev, AnimData& next, float& blendRate, float blendStep = 1.0f / 8.0f);
    // アニメーションが終了したか
    bool IsEnd(const AnimData& data) const { return data.isEnd; }
};

