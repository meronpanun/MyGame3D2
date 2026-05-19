#pragma once
#include "DxLib.h"

/// <summary>
/// スポーンエリアの形状と種別を保持する構造体（SpawnAreaData.csv から読み込む）
/// </summary>
struct SpawnAreaInfo
{
    int    type   = 0;   // エリア種別（0: メイン, 1: チュートリアル）
    VECTOR center = {0}; // エリアの中心座標
    VECTOR size   = {0}; // エリアのサイズ（スケール）
};
