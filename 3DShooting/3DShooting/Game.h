#pragma once
#include <vector>

class Game
{
public:
    // ��ʏ���萔��`
    //static constexpr int kScreenWidth  = 1920;
    //static constexpr int kScreenHeigth = 1080;
    //static constexpr int kScreenWidth  = 1280;
    //static constexpr int kScreenHeigth = 720;
    static constexpr int kScreenWidth  = 640;
    static constexpr int kScreenHeigth = 480;
    static constexpr int kColorBitNum  = 32;

    static constexpr bool kDefaultWindowMode = true;

    // �O���[�o���ȃJ�������x
    static float g_cameraSensitivity;

private:
};
