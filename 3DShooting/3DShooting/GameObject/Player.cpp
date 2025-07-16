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
#include "CapsuleCollider.h"
#include "TransformDataLoader.h"
#include <cmath>
#include <cassert>
#include <algorithm>

namespace
{
	// モデルのオフセット
	constexpr float kModelOffsetX = 80.0f; 
	constexpr float kModelOffsetY = 20.0f;
	constexpr float kModelOffsetZ = 60.0f;

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// 銃のオフセット
	constexpr float kGunOffsetX = -50.0f;
	constexpr float kGunOffsetY = 50;
	constexpr float kGunOffsetZ = 0;

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
	constexpr float kTackleHitRange    = 250.0f; // タックルの前方有効距離
	constexpr float kTackleHitRadius   = 250.0f; // タックルの横幅(半径)
	constexpr float kTackleHitHeight   = 100.0f; // タックルの高さ

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f; // カプセルコライダーの高さ
	constexpr float kCapsuleRadius = 50.0f;  // カプセルコライダーの半径
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_pDebugCamera(std::make_shared<Camera>()),
	//m_pEffect(std::make_shared<Effect>()),
	m_pEnemy(std::make_shared<EnemyNormal>()),
	m_pBodyCollider(std::make_shared<CapsuleCollider>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
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
	m_damageEffectAlpha(0.0f),
	m_damageEffectTimer(0.0f),
	m_healEffectAlpha(0.0f),
	m_healEffectTimer(0.0f)
{
	// プレイヤーモデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/AR_M.mv1");
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
	// CSVからPlayerのTransform情報を取得
	auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
	for (const auto& data : dataList) 
	{
		if (data.name == "Player") 
		{
			m_pos = data.pos;
			m_modelPos = data.pos;
			m_scale = data.scale;
			m_health = data.hp;
			m_maxHealth = data.hp;
			m_moveSpeed = data.speed;
			m_tackleCooldownMax = data.tackleCooldown;
			m_tackleSpeed = data.tackleSpeed;
			m_tackleDamage = data.tackleDamage;
			m_runSpeed = data.runSpeed;
			m_initialAmmo = data.initialAmmo;
			m_bulletPower = data.bulletPower;
			MV1SetScale(m_modelHandle, data.scale);
			MV1SetRotationXYZ(m_modelHandle, data.rot);
			break;
		}
	}
	m_pCamera->Init(); // カメラの初期化
	//m_pEffect->Init(); // エフェクトの初期化

	m_animBlendRate = kAnimBlendRate; // アニメーションのブレンド率を設定

	// CSVの初期弾薬数を反映
	m_ammo = m_initialAmmo;
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	// プレイヤーの位置をカメラに設定
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update(); // カメラの更新
	//m_pEffect->Update(); // エフェクトの更新

	//UpdateAnime(m_prevAnimData); // 前のアニメーションデータを更新
	//UpdateAnime(m_nextAnimData); // 次のアニメーションデータを更新
	//UpdateAnimeBlend();		   // アニメーションのブレンドを更新

	// プレイヤーのカプセルコライダーを毎フレーム更新
	VECTOR center = m_modelPos;
	center.y += 60.0f; // 足元から腰～胸あたりを中心に
	VECTOR capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	VECTOR capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	m_pBodyCollider->SetSegment(capA, capB);
	m_pBodyCollider->SetRadius(kCapsuleRadius);

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
		m_tackleCooldown = m_tackleCooldownMax; // クールタイム開始
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
			m_modelPos = VAdd(m_modelPos, VScale(m_tackleDir, m_tackleSpeed));

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
				enemy->Update(m_bullets, tackleInfo, *this, enemyList);
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
			enemy->Update(m_bullets, tackleInfo, *this, enemyList);
		}
	
		// 弾の更新
		Bullet::UpdateBullets(m_bullets, m_modelPos);

		// 走るキー入力
		const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
		bool isRunning = wantRun; // 走っているかどうかのフラグ

		float moveSpeed = isRunning ? m_runSpeed : m_moveSpeed; // 移動速度の設定
		bool isMoving = false; // 移動中かどうかのフラグ

		// 移動方向の初期化
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

	SetFontSize(ammoFontSize);
	DrawFormatString(ammoX, ammoY, 0xffff50, "AMMO: %d", m_ammo);
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

	//m_pEffect->Draw(); // エフェクトの描画

	// タックルクールタイムゲージ
	const int tackleGaugeX = 10;
	const int tackleGaugeY = 50;
	const int tackleGaugeWidth = 200;
	const int tackleGaugeHeight = 16;

	// 枠
	DrawBox(tackleGaugeX - 1, tackleGaugeY - 1, tackleGaugeX + tackleGaugeWidth + 1, tackleGaugeY + tackleGaugeHeight + 1, 0x5050C8, false);

	// ゲージ本体
	float tackleRate = 1.0f - (m_tackleCooldown / static_cast<float>(m_tackleCooldownMax));
	int tackleFilledWidth = static_cast<int>(tackleGaugeWidth * tackleRate);
	DrawBox(tackleGaugeX, tackleGaugeY, tackleGaugeX + tackleFilledWidth, tackleGaugeY + tackleGaugeHeight, 0x50B4ff, true);

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

    // 最大HP
    const float maxHP = 100.0f; 
    float hp = m_health;
    if (hp < 0) hp = 0;
    if (hp > maxHP) hp = maxHP;

    // HP割合
    float hpRate = hp / maxHP;

    // 背景
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, 0x505050, true);

    // HPバー本体
    DrawBox(barX, barY, barX + static_cast<int>(barWidth * hpRate), barY + barHeight, 0xff4040, true);

    // 枠
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, 0x000000, false);

    // HP数値
    DrawFormatString(barX + 8, barY + 2, 0xffffff, "HP: %.0f / %.0f", hp, maxHP);

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
		MV1DrawModel(m_modelHandle); // プレイヤーモデル描画

		// プレイヤーカプセル当たり判定のデバッグ表示
		//auto playerCol = GetBodyCollider();
		//DebugUtil::DrawCapsule(
		//	playerCol->GetSegmentA(),
		//	playerCol->GetSegmentB(),
		//	playerCol->GetRadius(),
		//	16,
		//	0xff00ff,
		//	false
		//);

		// メイン画面に戻す
		SetDrawScreen(DX_SCREEN_BACK);

		// サブ画面を右下に転送
		//DrawGraph(subX, subY, subScreen, false);

		// サーフェス解放
		DeleteGraph(subScreen);

		// 標準出力
		//printf("[DebugCamera] Pos:(%.1f, %.1f, %.1f)  Target:(%.1f, %.1f, %.1f)  FOV:%.1f\n",
			//pos.x, pos.y, pos.z, tgt.x, tgt.y, tgt.z, fov * 180.0f / DX_PI_F);
	}
#endif

	// ダメージエフェクト描画（赤）
	DrawEffectFeedback(m_damageEffect);

	// 回復エフェクト描画(緑)
	DrawEffectFeedback(m_healEffect);

    // 弾薬エフェクト描画(オレンジ)
    DrawEffectFeedback(m_ammoEffect);
}

void Player::DrawEffectFeedback(Player::EffectFeedback& effect)
{
    if (effect.timer > 0.0f && effect.alpha > 0.0f)
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW / 2;
        int centerY = screenH / 2;
        float maxDistance = sqrtf((float)(screenW * screenW + screenH * screenH)) * 0.5f;
        float edgeWidth = maxDistance * 0.4f;
        const int stepSize = 8;
        for (int y = 0; y < screenH; y += stepSize)
        {
            for (int x = 0; x < screenW; x += stepSize)
            {
                float distanceFromCenter = sqrtf((float)((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)));
                float distanceFromEdge = maxDistance - distanceFromCenter;
                float edgeIntensity = 0.0f;
                if (distanceFromEdge < edgeWidth)
                {
                    edgeIntensity = 1.0f - (distanceFromEdge / edgeWidth);
                }
                int alpha = static_cast<int>(effect.alpha * 180 * edgeIntensity);
                if (alpha > 0)
                {
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                    DrawBox(x, y, x + stepSize, y + stepSize, GetColor(effect.colorR, effect.colorG, effect.colorB), TRUE);
                }
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        // エフェクトの減衰処理
        effect.alpha -= 1.0f / effect.duration;
        if (effect.alpha < 0.0f) effect.alpha = 0.0f;
        effect.timer -= 1.0f;
    }
}

// ダメージを受ける処理
void Player::TakeDamage(float damage)
{
	m_health -= damage; // ダメージを適用
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
	// ダメージエフェクトを発動
	m_damageEffect.Trigger(30.0f, 255, 0, 0); // 赤
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
    // 画面中央の座標を取得
    float screenCenterX, screenCenterY;
    GetCameraScreenCenter(&screenCenterX, &screenCenterY);
    
    // 画面中央の3D座標を計算（カメラから一定距離の点）
    VECTOR cameraPos = m_pCamera->GetPos();
    VECTOR cameraForward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
    float rayDistance = 1000.0f; // 十分に遠い距離
    VECTOR screenCenter3D = VAdd(cameraPos, VScale(cameraForward, rayDistance));
    
    // 銃の発射位置を取得
    VECTOR gunPos = GetGunPos();
    
    // 銃の位置から画面中央の3D座標への方向ベクトルを計算
    VECTOR bulletDirection = VNorm(VSub(screenCenter3D, gunPos));
    
    // 弾丸を発射
    bullets.emplace_back(gunPos, bulletDirection, m_bulletPower);
    m_ammo--;

	// カメラシェイクを発生
	if (m_pCamera)
	{
		m_pCamera->Shake(8.0f, 8); // 強さ・フレーム数
	}
    
    // アニメーション関連
    ChangeAnime("Shoot", false); // 射撃アニメーションを再生
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

std::shared_ptr<CapsuleCollider> Player::GetBodyCollider() const
{
	return m_pBodyCollider;
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
		info.damage = m_tackleDamage;
	}
	return info;
}

// カプセル情報を取得
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
	// m_pBodyCollider から直接取得
	capA   = m_pBodyCollider->GetSegmentA();
	capB   = m_pBodyCollider->GetSegmentB();
	radius = m_pBodyCollider->GetRadius();
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
    // 回復時にエフェクトを発動
    m_healEffect.Trigger(45.0f, 0, 255, 0); // 緑
}

void Player::AddAmmo(int value)
{
    m_ammo += value;
    if (m_ammo < 0) m_ammo = 0;
    // 弾薬取得時にエフェクトを発動
    m_ammoEffect.Trigger(45.0f, 255, 128, 0); // オレンジ
}
