#include "pch.h"
#include "Bullet.h"
#include "EffekseerForDXLib.h"

TEST(BulletTest, DamageAttenuation)
{
	// テスト準備
	VECTOR startPos = { 0, 0, 0 };
	VECTOR dir = { 0, 0, 1 };

	// 減衰開始:50, 終了:300, 最低倍率:0.1
	Bullet bullet(startPos, dir, AttackType::Shoot, 100.0f, 50.0f, 300.0f, 0.1f);
	// ケース1: 至近距離 (距離 0) -> 100ダメージ
	// 本来はBulletを少し動かすなどして位置を更新するか、GetDamageのロジックを検証できる状態にする

	// 検証
	EXPECT_EQ(bullet.GetDamage(), 100.0f);
}