#include "Player.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include "Bullet.h"
#include "SceneManager.h"
#include "SceneMain.h"
#include "SceneGameOver.h"
#include "DebugUtil.h"	
#include <cmath>
#include <cassert>
#include <algorithm>

namespace
{
	constexpr float kMoveSpeed = 3.0f; // 移動速度
	constexpr float kRunSpeed  = 6.0f; // 走る速度

	// モデルのオフセット
	constexpr float kModelOffsetX = 80.0f; 
	constexpr float kModelOffsetY = 20.0f;
	constexpr float kModelOffsetZ = 60.0f;

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// 銃のオフセット
	constexpr float kGunOffsetX = -55.0f;
	constexpr float kGunOffsetY = 55.0f;
	constexpr float kGunOffsetZ = 10.0f;

	// 初期弾薬数
	constexpr int kInitialAmmo = 7000;

	// UI関連
	constexpr int kMarginX    = 20; 
	constexpr int kMarginY    = 20;
	constexpr int kFontHeight = 20;

	// 重力とジャンプ関連
	constexpr float kGravity   = 0.35f; // 重力の強さ
	constexpr float kJumpPower = 7.0f;  // ジャンプの初速
	constexpr float kGroundY   = 0.0f;  // 地面のY座標

	// タックル関連
	constexpr int   kTackleDuration    = 20;     // タックル持続フレーム数
	constexpr int   kTackleCooldownMax = 120;    // クールタイム最大値
	constexpr float kTackleSpeed	   = 25.0f;  // タックル時の速度
	constexpr float kTackleHitRange    = 250.0f; // タックルの前方有効距離
	constexpr float kTackleHitRadius   = 250.0f; // タックルの横幅(半径)
	constexpr float kTackleHitHeight   = 100.0f; // タックルの高さ
	constexpr float kTackleDamage      = 10.0f;  // タックルダメージ

	// 体力
	constexpr float kMaxHealth = 100.0f; // 最大体力
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_pDebugCamera(std::make_shared<Camera>()),
	m_pEffect(std::make_shared<Effect>()),
	m_pEnemy(std::make_shared<EnemyNormal>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
	m_ammo(kInitialAmmo),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_isJumping(false),
	m_jumpVelocity(0.0f),
	m_hasShot(false),
	m_tackleFrame(0),
	m_tackleDir(VGet(0, 0, 0)),
	m_isTackling(false),
	m_tackleCooldown(0),
	m_tackleId(0),
	m_maxHealth(kMaxHealth)
{
	// プレイヤーモデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/M4A1.mv1");
	assert(m_modelHandle != -1);

	// 剣の読み込み
	m_swordHandle = MV1LoadModel("data/model/Sword.mv1");
	assert(m_swordHandle != -1);

	// SEの読み込み
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);
}

Player::~Player()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_swordHandle);

	// SEの解放
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	m_pCamera->Init(); // カメラの初期化
	m_pEffect->Init(); // エフェクトの初期化

	// モデルの大きさ
	MV1SetScale(m_modelHandle, VGet(10.0f, 10.0f, 5.0f));

	m_animBlendRate = kAnimBlendRate; // アニメーションのブレンド率を設定
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	// プレイヤーの位置をカメラに設定
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update(); // カメラの更新
	m_pEffect->Update(); // エフェクトの更新

	//UpdateAnime(m_prevAnimData); // 前のアニメーションデータを更新
	//UpdateAnime(m_nextAnimData); //	次のアニメーションデータを更新
	//UpdateAnimeBlend();			 // アニメーションのブレンドを更新

	// モデルの位置と回転を更新
	VECTOR modelOffset	  = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw		  = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch		  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot		  = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// モデルの位置を設定
	MV1SetPosition(m_modelHandle, modelPos);

	// モデルの回転を設定
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F , 0.0f));

	// タックルクールタイム減少
	if (m_tackleCooldown > 0)
	{
		m_tackleCooldown--;

		// クールタイムが0になった瞬間に全敵のタックルヒットフラグをリセット
		if (m_tackleCooldown == 0)
		{
			for (EnemyBase* enemy : enemyList)
			{
				if (enemy)
				{
					enemy->ResetTackleHitFlag();
				}
			}
		}
	}

	// マウスの左クリックで射撃(タックル中は射撃不可)
	if (!m_isTackling && Mouse::IsPressLeft() && m_ammo > 0)
	{
		Shoot(m_bullets); // 弾を発射
		m_ammo--; // 弾薬を減らす
	}

	// 地面にいるかどうかの判定
	bool isOnGround = (m_modelPos.y <= kGroundY + 0.01f);

	// 右クリックでタックル開始
	if (!m_isTackling && m_tackleCooldown <= 0 && Mouse::IsTriggerRight())
	{
		m_isTackling = true;
		m_tackleFrame = kTackleDuration;
		m_tackleCooldown = kTackleCooldownMax; // クールタイム開始
		m_tackleId++; // タックルごとにIDを更新

		// カメラの向きで3D正規化ベクトルを作成
		float yaw = m_pCamera->GetYaw();
		float pitch = m_pCamera->GetPitch();

		// タックル方向を計算
		m_tackleDir = VGet(
			cosf(pitch) * sinf(yaw),
			sinf(pitch),
			cosf(pitch) * cosf(yaw)
		);

		//	ChangeAnime(kTackleAnimName, false); // タックルアニメーション

		// タックル開始時にFOVを広げ、カメラを後ろに引く
		if (m_pCamera)
		{
			m_pCamera->SetTargetFOV(100.0f * DX_PI_F / 180.0f);
			VECTOR offset = m_pCamera->GetOffset();
			offset.z = 30.0f;
			m_pCamera->SetOffset(offset);
		}
	}

		// タックル中の処理
		if (m_isTackling)
		{
			m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, kTackleSpeed));

			// 地面より下に行かないように制限
			if (m_modelPos.y < kGroundY)
			{
				m_modelPos.y = kGroundY;
			}

			// タックル判定情報を作成
			TackleInfo tackleInfo = GetTackleInfo();

			// 各敵にタックル情報を渡してUpdate
			for (EnemyBase* enemy : enemyList)
			{
				// 敵がnullptrの場合はスキップ
				if (!enemy) continue;

				// 敵の更新処理
				enemy->Update(m_bullets, tackleInfo, *this);
			}

#ifdef _DEBUG
			// タックル判定カプセルのデバッグ描画
			DebugUtil::DrawCapsule(
				tackleInfo.capA,
				tackleInfo.capB,
				tackleInfo.radius,
				16,
				0x00ff00,
				false
			);
#endif
			m_tackleFrame--;
			// タックル終了判定
			if (m_tackleFrame <= 0)
			{
				m_isTackling = false;

				// タックル終了時にFOVとカメラオフセットを元に戻す
				if (m_pCamera)
				{
					m_pCamera->ResetFOV();
					m_pCamera->ResetOffset();
				}

				// タックル後のアニメーション遷移
				/*if (m_isMoving)
				{
					ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
				}
				else
				{
					ChangeAnime(kIdleAnimName, true);
				}*/
			}
			// タックル中は他の移動・ジャンプを無効化
			return;
		}

		// タックル中でなければ bullets のみ渡す
		TackleInfo tackleInfo{};
		for (EnemyBase* enemy : enemyList)
		{
			if (!enemy) continue;
			enemy->Update(m_bullets, tackleInfo, *this);
		}
	
		// 弾の更新
		Bullet::UpdateBullets(m_bullets);

		// 走るキー入力
		const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
		bool isRunning = wantRun; // 走っているかどうかのフラグ

		float moveSpeed = isRunning ? kRunSpeed : kMoveSpeed; // 移動速度の設定
		bool isMoving = false; // 移動中かどうかのフラグ

		// 移動方向の初期化s
		VECTOR moveDir = VGet(0, 0, 0);

		// キー入力による移動方向の設定
		if (CheckHitKey(KEY_INPUT_W))
		{
			moveDir.x += sinf(m_pCamera->GetYaw());
			moveDir.z += cosf(m_pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_S))
		{
			moveDir.x -= sinf(m_pCamera->GetYaw());
			moveDir.z -= cosf(m_pCamera->GetYaw());
		}
		if (CheckHitKey(KEY_INPUT_A))
		{
			moveDir.x += sinf(m_pCamera->GetYaw() - DX_PI_F * 0.5f);
			moveDir.z += cosf(m_pCamera->GetYaw() - DX_PI_F * 0.5f);
		}
		if (CheckHitKey(KEY_INPUT_D))
		{
			moveDir.x += sinf(m_pCamera->GetYaw() + DX_PI_F * 0.5f);
			moveDir.z += cosf(m_pCamera->GetYaw() + DX_PI_F * 0.5f);
		}

		// スペースキーを押した瞬間のみジャンプ
		if (keyState[KEY_INPUT_SPACE] && !m_prevKeyState[KEY_INPUT_SPACE] && isOnGround && !m_isJumping && !m_isTackling)
		{
			m_jumpVelocity = kJumpPower;
			m_isJumping = true;
			//ChangeAnime(kJumpAnimName, false);
		}

		// ジャンプ中または空中なら重力適用
		if (m_isJumping || !isOnGround)
		{
			m_modelPos.y += m_jumpVelocity; // ジャンプの速度を適用
			m_jumpVelocity -= kGravity;     // 重力を適用

			// 着地判定
			if (m_modelPos.y <= kGroundY)
			{
				m_modelPos.y = kGroundY; // 地面に着地
				m_jumpVelocity = 0.0f;   // ジャンプ速度をリセット
				m_isJumping = false;     // ジャンプ状態を解除
			}
		}

		// 移動方向がある場合
		if (moveDir.x != 0.0f || moveDir.z != 0.0f)
		{
			// 移動方向の長さを計算
			float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
			moveDir.x /= len;
			moveDir.z /= len;
			m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
			isMoving = true;
		}

		//// アニメーションが終了したら
		//if (m_nextAnimData.isEnd)
		//{
		//	if (m_isMoving)
		//	{
		//		// 移動アニメーションに変更
		//		ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
		//	}
		//	else
		//	{
		//		// 待機アニメーションに変更
		//		ChangeAnime(kIdleAnimName, true);
		//	}
		//}

		// 移動中で前回は移動していなかった場合
		//if (isMoving && !m_isMoving)
		//{
		//	ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		//}
		//else if (!isMoving && m_isMoving)
		//{
		//	ChangeAnime(kIdleAnimName, true);
		//}
		//else if (isMoving && m_isMoving && (isRunning != m_isWasRunning))
		//{
		//	ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		//}

		m_isMoving = isMoving;  // 移動中の状態を更新
		m_isWasRunning = isRunning; // 走っている状態を更新

		std::copy(std::begin(keyState), std::end(keyState), std::begin(m_prevKeyState));

		// プレイヤーの斜め後方上空からプレイヤーを見る
		if (m_pDebugCamera) 
		{
			VECTOR target = m_modelPos;
			VECTOR offset = VGet(-200.0f, 150.0f, -200.0f); // 斜め後方上空

			m_pDebugCamera->SetPos(VAdd(target, offset));
			m_pDebugCamera->SetTarget(target);
			m_pDebugCamera->SetFOV(60.0f * DX_PI_F / 180.0f);
		}
	}


void Player::Draw()
{
	DrawField(); // フィールドの描画

	// プレイヤーモデルの描画
	MV1DrawModel(m_modelHandle); 

	// 弾の描画
	Bullet::DrawBullets(m_bullets);

	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;
	GetScreenState(&screenW, &screenH, NULL);

	// 残弾数の表示
	int ammoFontSize = 32;
	int ammoX = screenW - 200;
	int ammoY = screenH - 60;
	int ammoColor = GetColor(255, 255, 80);

	SetFontSize(ammoFontSize);
	DrawFormatString(ammoX, ammoY, ammoColor, "AMMO: %d", m_ammo);
	SetFontSize(16); // フォントサイズを元に戻す


	/*剣の描画*/
	float swordScreenX = -25.0f;
	float swordScreenY = -30;
	float swordScreenZ = 35.0f;

	VECTOR camPos = VGet(0, 0, -swordScreenZ); // カメラの位置
	VECTOR camTgt = VGet(0, 0, 0);			    // カメラのターゲット位置
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt); 

	// 剣の位置
	MV1SetPosition(m_swordHandle, VGet(swordScreenX, swordScreenY, 0.0f));

	MV1SetRotationXYZ(m_swordHandle, VGet(0.0f, 200.0f, 0.0f)); // 剣の回転
	MV1SetScale(m_swordHandle, VGet(0.5f, 0.5f, 0.5f));		    // 剣のスケール

	// 剣の描画
	MV1DrawModel(m_swordHandle);

	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw(); // エフェクトの描画

	// タックルクールタイムゲージ
	const int tackleGaugeX = 10;
	const int tackleGaugeY = 50;
	const int tackleGaugeWidth = 200;
	const int tackleGaugeHeight = 16;

	// 枠
	DrawBox(tackleGaugeX - 1, tackleGaugeY - 1, tackleGaugeX + tackleGaugeWidth + 1, tackleGaugeY + tackleGaugeHeight + 1, GetColor(80, 80, 200), false);

	// ゲージ本体
	float tackleRate = 1.0f - (m_tackleCooldown / static_cast<float>(kTackleCooldownMax));
	int tackleFilledWidth = static_cast<int>(tackleGaugeWidth * tackleRate);
	int tackleColor = GetColor(80, 180, 255);
	DrawBox(tackleGaugeX, tackleGaugeY, tackleGaugeX + tackleFilledWidth, tackleGaugeY + tackleGaugeHeight, tackleColor, true);

	// テキスト
	DrawFormatString(tackleGaugeX, tackleGaugeY - 18, 0xFFFFFF, "Tackle Cooldown");
	if (m_tackleCooldown > 0) 
	{
		DrawFormatString(tackleGaugeX + tackleGaugeWidth + 10, tackleGaugeY, 0xFF8080, "WAIT");
	}
	else 
	{
		DrawFormatString(tackleGaugeX + tackleGaugeWidth + 10, tackleGaugeY, 0x80FF80, "READY");
	}

	// HPバーのパラメータ
    const int barWidth  = 200;
    const int barHeight = 24;
    const int margin    = 30;
    const int barX      = margin;
    const int barY      = screenH - barHeight - margin;

    // 最大HP（必要に応じて定数化）
    const float maxHP = 100.0f; // 例: 100
    float hp = m_health;
    if (hp < 0) hp = 0;
    if (hp > maxHP) hp = maxHP;

    // HP割合
    float hpRate = hp / maxHP;

    // 背景
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(80, 80, 80), TRUE);

    // HPバー本体
    DrawBox(barX, barY, barX + static_cast<int>(barWidth * hpRate), barY + barHeight, GetColor(255, 64, 64), TRUE);

    // 枠
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(0, 0, 0), FALSE);

    // HP数値
    DrawFormatString(barX + 8, barY + 2, GetColor(255,255,255), "HP: %.0f / %.0f", hp, maxHP);

#ifdef _DEBUG
	if (m_pCamera)
	{
		// デバッグカメラの位置とターゲットを取得
		const VECTOR pos = m_pCamera->GetPos();
		const VECTOR tgt = m_pCamera->GetTarget();
		float fov = m_pCamera->GetFOV();

		// サブカメラ用描画領域
		int subW = 320, subH = 180; // サブウィンドウサイズ
		int subX = Game::kScreenWidth - subW;
		int subY = Game::kScreenHeigth - subH;

		// サブカメラ用描画先サーフェス作成
		int subScreen = MakeScreen(subW, subH, true);
		SetDrawScreen(subScreen);
		ClearDrawScreen();

		// サブカメラで描画
		m_pDebugCamera->SetCameraToDxLib();

		// ここでフィールド・プレイヤーモデル・当たり判定を描画 
		DrawField(); // フィールド描画
		MV1DrawModel(m_modelHandle); // プレイヤーモデル描画

		// プレイヤーカプセル当たり判定のデバッグ表示
		VECTOR capA, capB;
		float radius;
		GetCapsuleInfo(capA, capB, radius);
		DebugUtil::DrawCapsule(
			capA,
			capB,
			radius,
			16,
			0xff00ff,
			false
		);

		// メイン画面に戻す
		SetDrawScreen(DX_SCREEN_BACK);

		// サブ画面を右下に転送
		DrawGraph(subX, subY, subScreen, false);

		// サーフェス解放
		DeleteGraph(subScreen);

		// 標準出力
		printf("[DebugCamera] Pos:(%.1f, %.1f, %.1f)  Target:(%.1f, %.1f, %.1f)  FOV:%.1f\n",
			pos.x, pos.y, pos.z, tgt.x, tgt.y, tgt.z, fov * 180.0f / DX_PI_F);
	}
#endif

}

// フィールドの描画
void Player::DrawField()
{
	SetUseLighting(false);

	const int gridSize = 100;
	const int fieldSize = 800;

	unsigned int color1 = GetColor(80, 80, 80);
	unsigned int color2 = GetColor(100, 100, 100);

	for (int z = static_cast<int>(-fieldSize * 0.5f); z < static_cast<int>(fieldSize * 0.5f); z += gridSize)
	{
		for (int x = static_cast<int>(-fieldSize * 0.5f); x < static_cast<int>(fieldSize * 0.5f); x += gridSize)
		{
			VECTOR cubeCenter = VGet(x + gridSize * 0.5f, 0, z + gridSize * 0.5f);

			VECTOR cubeSize = VGet(gridSize, 10, gridSize);

			unsigned int color = ((x / gridSize + z / gridSize) % 2 == 0) ? color1 : color2;

			DrawCube3D(
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y - 80, cubeCenter.z - cubeSize.z * 0.5f),
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y - 80 + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f),
				color,
				color,
				true
			);
		}
	}

	SetUseLighting(true);
}

// ダメージを受ける処理
void Player::TakeDamage(float damage)
{
	m_health -= damage; // ダメージを適用
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
}

// 弾の取得
std::vector<Bullet>& Player::GetBullets()
{
	return m_bullets;
}

// プレイヤーがショット可能かどうか
bool Player::HasShot()
{
	bool shot = m_hasShot;
	m_hasShot = false; // 状態をリセット
	return shot; // 撃ったかどうかを返す
}

void Player::Shoot(std::vector<Bullet>& bullets)
{
    //// クールダウン中なら発射しない
    //if (m_shotCooldownTimer > 0)
    //{
    //    return;
    //}

    //// 弾薬がないなら発射しない
    //if (m_ammoCount <= 0)
    //{
    //    return;
    //}

    // カメラの位置とカメラが向いている方向の遠い目標点を取得
    // m_pCameraがGetPos()メソッドを持つことを前提としています。
    // もしコンパイルエラーが発生する場合、Cameraクラスの実装をご確認ください。
    VECTOR cameraPos = m_pCamera->GetPos();
    VECTOR cameraLookDir = GetGunRot(); // カメラの現在の向き
    float targetDistance = 1000.0f; // 十分に遠い距離（例: 1000単位）
    VECTOR targetPoint = VAdd(cameraPos, VScale(cameraLookDir, targetDistance));

    // 銃の発射位置から目標点への方向ベクトルを計算
    VECTOR gunPos = GetGunPos();
    VECTOR bulletDirection = VNorm(VSub(targetPoint, gunPos));

    // 弾丸を発射
    bullets.emplace_back(gunPos, bulletDirection); // 新しい bulletDirection を使用
    m_ammo--;
 //   m_pEffect->PlaySound(Effect::SoundType::Shot);
//    m_pEffect->SpawnEffect(Effect::EffectType::MuzzleFlash, gunPos, bulletDirection); // エフェクトの向きも更新

    // 発射クールダウンを設定
    //m_shotCooldownTimer = kShotCooldown;

    // アニメーション関連
    ChangeAnime("Shoot", false); // 射撃アニメーションを再生
  //  SetAnimSpeed(kShotAnimSpeed);
}

// アニメーションのアタッチ
void Player::AttachAnime(AnimData& data, const char* animName, bool isLoop) 
{
	// アニメーションのインデックスを取得
	int index = MV1GetAnimIndex(m_modelHandle, animName); 

	// モデルアニメーションをアタッチ
	data.attachNo = MV1AttachAnim(m_modelHandle, index, -1, false); 
	data.count  = 0.0f;	  // アニメーションのカウントを初期化
	data.isLoop = isLoop; // ループフラグを設定
	data.isEnd  = false;  // アニメーションが終了したか
}

// アニメーションの更新
void Player::UpdateAnime(AnimData& data) 
{
	if (data.attachNo == -1) return; // アタッチされていない場合は何もしない

	float animSpeed = 1.0f;

	data.count += animSpeed;

	// アニメーションの総時間を取得
	const float totalTime = MV1GetAttachAnimTotalTime(m_modelHandle, data.attachNo); 
	
	// ループアニメーションの場合
	if (data.isLoop)
	{
		// アニメーションのカウントが総時間を超えた場合、ループさせる
		while (data.count > totalTime)  
		{
			data.count -= totalTime; // 総時間を引いてループ
		}
	}
	else
	{
		if (data.count > totalTime)
		{
			data.count = totalTime; // アニメーションのカウントを総時間に制限
			data.isEnd = true;      // アニメーションが終了したフラグを立てる
		}
	}

	// アニメーションの時間を設定
	MV1SetAttachAnimTime(m_modelHandle, data.attachNo, data.count); 
}

// アニメーションのブレンドを更新
void Player::UpdateAnimeBlend() 
{
	// アタッチされていない場合は何もしない
	if (m_nextAnimData.attachNo == -1 && m_prevAnimData.attachNo == -1) return; 

	// アニメーションのブレンド率を更新
	m_animBlendRate += 1.0f / 8.0f; 

	// ブレンド率が1.0を超えたら
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f; // 1.0に制限
	}

	// 前のアニメーションのブレンド率を設定
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate); 
	// 次のアニメーションのブレンド率を設定
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate); 
}

// アニメーションの変更
void Player::ChangeAnime(const char* animName, bool isLoop) 
{
	// 前のアニメーションを解除
	MV1DetachAnim(m_modelHandle, m_prevAnimData.attachNo);

	// 前のアニメーションデータを保存
	m_prevAnimData = m_nextAnimData; 

	// 次のアニメーションをアタッチ
	AttachAnime(m_nextAnimData, animName, isLoop); 

	// アニメーションのブレンド率をリセット
	m_animBlendRate = 0.0f; 

	// 前のアニメーションのブレンド率を設定
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate); 
	// 次のアニメーションのブレンド率を設定
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

// 銃の位置を取得
VECTOR Player::GetGunPos() const 
{
	// モデルのオフセットと回転を計算
	VECTOR modelOffset		  = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ); // モデルのオフセット
	MATRIX rotYaw			  = MGetRotY(m_pCamera->GetYaw());                     // カメラのヨー回転
	MATRIX rotPitch			  = MGetRotX(-m_pCamera->GetPitch());	               // カメラのピッチ回転
	MATRIX modelRot			  = MMult(rotPitch, rotYaw);						   // モデルの回転行列を計算
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);				   // オフセットを回転
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset);			   // モデルの位置とオフセットを組み合わせて銃の位置を計算

	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ); // 銃のオフセット
	VECTOR gunPos    = VTransform(gunOffset, modelRot);			    // 銃のオフセットを回転

	// 銃の位置を計算して返す
	return VAdd(modelPosition, gunPos); 
}

// 銃の向きを取得
VECTOR Player::GetGunRot() const
{
	// この関数はカメラの純粋な向きを返すものとして維持します
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}

// タックル情報を取得
Player::TackleInfo Player::GetTackleInfo() const
{
	TackleInfo info;
	info.isTackling = m_isTackling;
	if (m_isTackling)
	{
		float tackleHeight = kTackleHitHeight;
		info.tackleId = m_tackleId; // タックルIDをセット

		// プレイヤーの体の中心位置
		VECTOR bodyCenter = m_modelPos;
		bodyCenter.y += kModelOffsetY;

		// プレイヤーの前面中心（体の中心から前方へkTackleHitRangeだけ進める）
		VECTOR frontCenter = VAdd(bodyCenter, VScale(m_tackleDir, kTackleHitRange));

		// カプセルの中心軸を前面中心の上下に伸ばす
		VECTOR capA = VAdd(frontCenter, VGet(0, -tackleHeight * 0.5f, 0));
		VECTOR capB = VAdd(frontCenter, VGet(0, tackleHeight * 0.5f, 0));

		info.capA = capA;
		info.capB = capB;
		info.radius = kTackleHitRadius;
		info.damage = kTackleDamage;
	}
	return info;
}

// カプセル情報を取得
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
	// プレイヤーの体の中心を基準に、縦長のカプセルを想定
	constexpr float kCapsuleHeight = 100.0f; // プレイヤーの身長
	constexpr float kCapsuleRadius = 80.0f;  // プレイヤーの体の半径

	VECTOR center = m_modelPos;
	center.y += 60.0f; // 足元から腰～胸あたりを中心に

	capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	radius = kCapsuleRadius;
}

void Player::AddHp(float value)
{
	m_health += value; // 体力を加算
	if (m_health > m_maxHealth)
	{
		m_health = m_maxHealth; // 最大体力を超えないように制限
	}

	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
}
