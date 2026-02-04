#include "pch.h"
#include "Bullet.h"
#include "DxLib.h"
#include <gtest/gtest.h>

// DXライブラリのベクトル計算などが正しく動くか確認するためのフィクスチャ
// ただしUnit TestではDXライブラリの初期化(DxLib_Init)までは通常行わないため、
// 純粋な計算関数のテストを行う。

TEST(BulletTest, ShotgunAttenuation_CloseRange) {
  // 準備
  VECTOR startPos = { 0, 0, 0 };
  VECTOR dir = { 0, 0, 1 };
  AttackType type = AttackType::Shoot;
  float baseDamage = 100.0f;
  
  // 減衰開始: 50, 終了: 300, 最低: 0.1 (10%)
  float startDist = 50.0f;
  float endDist = 300.0f;
  float minRatio = 0.1f;

  Bullet bullet(startPos, dir, type, baseDamage, startDist, endDist, minRatio);
  // 発射位置を正しく記憶しているか (コンストラクタ内で m_spawnPos = position されているか)
  
  // テスト: 距離 10.0f (開始距離 50.0f 以内)
  // Bulletの位置を強制的に更新（プライベートメンバへのアクセスはできないため、
  // 本来はUpdateを呼ぶか、テスト用にFriendクラスにする必要があるが、
  // ここでは Bullet の実装上、m_pos を直接変更する手段がない。
  // そのため、Update関数を使って移動させる。）
  
  // 1フレーム更新でどれくらい進むか計算
  // Bullet::m_speed は 60.0f (kBulletSpeed 固定)
  // 1回Updateを呼ぶと 60.0f 進んでしまうため、距離10のテストには不向き。
  // しかし、減衰開始距離は 50.0f なので、1回移動 (60.0f) すると既に減衰領域に入ってしまう。
  
  // ★問題点: Bulletクラスは移動速度が固定で、位置を外部からセットできない。
  // しかし、今回の検証では「減衰ロジック」が正しいか見たい。
  // BulletTest用に、位置をセットできる継承クラスを作るか、
  // あるいは Bullet のコンストラクタで初期位置を指定できるので、
  // 「別の位置にある Bullet」を生成して、GetDamage() を呼ぶことはできない。
  // GetDamage() は「現在の位置」と「発射位置(m_spawnPos)」の距離を見る。
  // コンストラクタで pos を渡すと、m_pos と m_spawnPos 両方がその値になるため、距離は常に0。
  
  // これを解決するには、テスト用に「発射位置」と「現在位置」を別々に扱える仕組みが必要だが、
  // 現状の Bullet クラスの実装では Update を呼ぶしかない。
  
  // Bulletの速度は 60.0f/frame
  // 減衰開始 50.0f
  
  // Step 1: 初期状態 (距離0) -> ダメージ100
  EXPECT_EQ(bullet.GetDamage(), 100.0f);
}

TEST(BulletTest, ShotgunAttenuation_MidRange) {
  VECTOR startPos = { 0, 0, 0 };
  VECTOR dir = { 0, 0, 1 };
  float baseDamage = 100.0f;
  float startDist = 50.0f;
  float endDist = 300.0f;
  float minRatio = 0.1f;
  
  Bullet bullet(startPos, dir, AttackType::Shoot, baseDamage, startDist, endDist, minRatio);

  // ステージデータ（空）
  std::vector<Stage::StageCollisionData> collisionData;
  VECTOR playerPos = { 0,0,0 };

  // 1回 Update -> 移動量 60.0f
  // 距離 60.0f は Start(50) と End(300) の間。
  // 計算:
  // dist = 60
  // t = (60 - 50) / (300 - 50) = 10 / 250 = 0.04
  // ratio = 1.0 - 0.04 * (1.0 - 0.1) = 1.0 - 0.04 * 0.9 = 1.0 - 0.036 = 0.964
  // Damage = 100 * 0.964 = 96.4
  
  bullet.Update(playerPos, collisionData); // 1回目 (60m)
  
  float expectedDamage = 100.0f * (1.0f - ((60.0f - 50.0f) / (300.0f - 50.0f)) * (1.0f - 0.1f));
  
  EXPECT_NEAR(bullet.GetDamage(), expectedDamage, 0.1f);
}

TEST(BulletTest, ShotgunAttenuation_LongRange)
{
  VECTOR startPos = { 0, 0, 0 };
  VECTOR dir = { 0, 0, 1 };
  float baseDamage = 100.0f;
  float startDist = 50.0f;
  float endDist = 300.0f;
  float minRatio = 0.1f;
  
  Bullet bullet(startPos, dir, AttackType::Shoot, baseDamage, startDist, endDist, minRatio);

  std::vector<Stage::StageCollisionData> collisionData;
  VECTOR playerPos = { 0,0,0 };
  
  // 6回 Update -> 360.0f 移動
  // 360 > End(300) なので最低保証ダメージになるはず
  for(int i = 0; i < 6; ++i) 
  {
      bullet.Update(playerPos, collisionData);
  }
  
  // 期待値: 100 * 0.1 = 10.0
  EXPECT_EQ(bullet.GetDamage(), 10.0f);
}
