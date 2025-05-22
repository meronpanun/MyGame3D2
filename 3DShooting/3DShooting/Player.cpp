#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include <cmath>
#include <cassert>

namespace
{
	// アニメーション名
	const char* const kIdleAnimName   = "Pistol_IDLE";   // 待機
	const char* const kShotAnimName   = "Pistol_FIRE";   // 弾を撃つ
	const char* const kWalkAnimName   = "Pistol_WALK";   // 歩く
	const char* const kRunAnimName    = "Pistol_RUN";    // 走る
	const char* const kReloadAnimName = "Pistol_RELOAD"; // リロード

	constexpr float kMoveSpeed = 2.0f; // 歩く速度
	constexpr float kRunSpeed  = 5.0f; // 走る速度

	// プレイヤーモデルのオフセット
	constexpr float kModelOffsetX = 2.0f;  // X軸オフセット
	constexpr float kModelOffsetY = 30.0f; // Y軸オフセット
	constexpr float kModelOffsetZ = 25.0f; // Z軸オフセット

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// 銃のオフセット
	constexpr float kGunOffsetX = 10.0f; // X軸オフセット
	constexpr float kGunOffsetY = 58.0f; // Y軸オフセット
	constexpr float kGunOffsetZ = 12.0f;  // Z軸オフセット

	// スタミナ関連定数
	constexpr float kStaminaMax        = 100.0f; // 最大スタミナ
	constexpr float kStaminaRunCost    = 0.8f;   // 走る時の1フレームあたり消費量
	constexpr float kStaminaRecover    = 0.5f;   // 歩き/停止時の1フレームあたり回復量
	constexpr float kStaminaRunRecover = 30.0f;  // 走れるようになるスタミナ値

	// リロード関連定数
	constexpr int kInitialAmmo = 700; // 初期弾薬数
	constexpr int kMaxAmmo     = 10;  // 最大弾薬数
	constexpr int kReloadTime  = 80;  // リロード時間

	// 右下表示用のオフセット
	constexpr int kMarginX    = 20; // X軸オフセット
	constexpr int kMarginY    = 20; // Y軸オフセット
	constexpr int kFontHeight = 20; // フォント高さ

	// 盾のオフセット
	constexpr float kShieldScreenOffsetX = -30.0f; 
	constexpr float kShieldScreenOffsetY = 10.0f; 
	constexpr float kShieldScreenOffsetZ = 40.0f; 

}

Player::Player() :
	m_modelHandle(-1),
	m_shieldHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_pEffect(std::make_shared<Effect>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
	m_stamina(kStaminaMax),
	m_isCanRun(true),
	m_ammo(kInitialAmmo),
	m_maxAmmo(kMaxAmmo),
	m_isReloading(false),
	m_reloadTimer(0),
	m_shotCooldown(0)
{
	// モデルの読み込み
	m_modelHandle = MV1LoadModel("data/image/Player.mv1");
	assert(m_modelHandle != -1);

	// 盾のモデルを読み込む
	m_shieldHandle = MV1LoadModel("data/image/Shield.mv1");
	assert(m_shieldHandle != -1);

	// 弾を撃つSEを読み込む
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);

	// プレイヤーモデルの初期回転を設定
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));

	// 盾の初期回転を設定
	MV1SetRotationXYZ(m_shieldHandle, VGet(0.0f, 0.0f, 0.0f));
	// 盾の大きさを設定
	MV1SetScale(m_shieldHandle, VGet(0.2f, 0.2f, 0.2f));
}

Player::~Player()
{
	// モデルの削除
	MV1DeleteModel(m_modelHandle);
	// 盾のモデルの削除
	MV1DeleteModel(m_shieldHandle);

	// SEの削除
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	// カメラの初期化
	m_pCamera->Init();

	// エフェクトの初期化
	m_pEffect->Init();

	// アニメーションのアタッチ
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate;
}

void Player::Update()
{
	// カメラの位置をプレイヤーの位置に基づいて更新
	m_pCamera->SetPlayerPos(m_modelPos);

	// カメラの位置を盾の位置に基づいて更新
	m_pCamera->SetShieldPos(m_modelPos);

	// カメラの更新
	m_pCamera->Update();

	// エフェクトの更新
	m_pEffect->Update();

	UpdateAnime(m_prevAnimData);
	UpdateAnime(m_nextAnimData);
	UpdateAnimeBlend();

	// モデルの位置をカメラの前方に設定
	VECTOR modelOffset    = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw         = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch       = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot		  = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// モデルの位置を更新
	MV1SetPosition(m_modelHandle, modelPos);

	// プレイヤーモデルの回転を更新
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	// 盾モデルの位置をカメラの前方に設定
	VECTOR shieldOffset	   = VGet(kShieldScreenOffsetX, kShieldScreenOffsetY, kShieldScreenOffsetZ);
	MATRIX shieldRotYaw    = MGetRotY(m_pCamera->GetYaw());
	MATRIX shieldRotPitch  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX shieldModelRot  = MMult(shieldRotPitch, shieldRotYaw);
	VECTOR rotShieldOffset = VTransform(shieldOffset, shieldModelRot);
	VECTOR shieldPos       = VAdd(m_modelPos, rotShieldOffset);

	// 盾の位置を更新
	MV1SetPosition(m_shieldHandle, shieldPos);

	// 盾モデルの回転を更新
	MV1SetRotationXYZ(m_shieldHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	// クールタイム減算
	if (m_shotCooldown > 0)
	{
		m_shotCooldown--;
	}

	// リロード処理
	if (m_isReloading) 
	{
		m_reloadTimer--;
		if (m_reloadTimer <= 0)
		{
			// 補充できる弾数を計算
			int need = kInitialAmmo - m_ammo;
			if (need > 0)
			{
				// 補充できる弾数
				int reloadAmount = (m_maxAmmo < need) ? m_maxAmmo : need; 
				m_ammo += reloadAmount;    // 弾数を増やす
				m_maxAmmo -= reloadAmount; // 残弾数を減らす

				// 最大弾数を超えないようにする
				if (m_maxAmmo < 0) m_maxAmmo = 0;
			}
			m_isReloading = false;
		}
	}
	else
	{
		// 弾発射
		if (Mouse::IsTriggerLeft() && m_ammo > 0 && m_shotCooldown == 0)
		{
			Shoot();
			m_ammo--;
			m_shotCooldown = 10; // 10フレーム間隔で発射可能
			ChangeAnime(kShotAnimName, false);
		}
		// Rキーでリロード
		if (CheckHitKey(KEY_INPUT_R) && m_ammo < kInitialAmmo && m_maxAmmo > 0)
		{
			Reload();
			ChangeAnime(kReloadAnimName, false);
		}
	}

	// 移動状態・走り状態の判定
	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
	bool isRunning = false;
	if (wantRun && m_isCanRun && m_stamina > 0.0f) 
	{
		isRunning = true;
	}
	float moveSpeed = isRunning ? kRunSpeed : kMoveSpeed;
	
	bool isMoving = false; 

	// 移動ベクトルを合成
	VECTOR moveDir = VGet(0, 0, 0);

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

	// ベクトルが0でなければ正規化して移動
	if (moveDir.x != 0.0f || moveDir.z != 0.0f) {
		float len = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		moveDir.x /= len;
		moveDir.z /= len;
		m_modelPos = VAdd(m_modelPos, VScale(moveDir, moveSpeed));
		isMoving = true;
	}

	// スタミナ処理
	if (isRunning && isMoving) 
	{
		m_stamina -= kStaminaRunCost;
		if (m_stamina < 0.0f) m_stamina = 0.0f;
	}
	else 
	{
		m_stamina += kStaminaRecover;
		if (m_stamina > kStaminaMax) m_stamina = kStaminaMax;
	}

	// 走れるかどうかの判定
	if (m_stamina <= 0.0f) 
	{
		m_isCanRun = false;
	}
	else if (!m_isCanRun && m_stamina >= kStaminaRunRecover) 
	{
		m_isCanRun = true;
	}

	// リロード中はアニメーション切り替えを行わない
	if (!m_isReloading)
	{
		if (m_nextAnimData.isEnd)
		{
			if (m_isMoving)
			{
				ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true);
			}
			else
			{
				ChangeAnime(kIdleAnimName, true);
			}
		}

		// アニメーション切り替え
		// 移動開始
		if (isMoving && !m_isMoving)
		{
			ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		}
		// 移動終了
		else if (!isMoving && m_isMoving)
		{
			ChangeAnime(kIdleAnimName, true);
		}
		// 移動中に走り⇔歩きが切り替わった場合
		else if (isMoving && m_isMoving && (isRunning != m_isWasRunning))
		{
			ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
		}
	}

	m_isMoving = isMoving;     
	m_isWasRunning = isRunning; 
}
void Player::Draw()
{
	// モデルの描画
	MV1DrawModel(m_modelHandle);
	
	// 盾モデルの描画
	MV1DrawModel(m_shieldHandle);

	// エフェクトの描画
	m_pEffect->Draw();

	// フィールドの描画
	DrawField();

	// 画面サイズ取得
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	const int bgWidth = 160;
	const int bgHeight = 48;
	int bgX = screenW - kMarginX - bgWidth;
	int bgY = screenH - kMarginY - bgHeight;

	// 半透明の背景板
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // 0～255で透明度指定
	DrawBox(bgX, bgY, bgX + bgWidth, bgY + bgHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 弾数表示
	int ammoX = bgX + 12;
	int ammoY = bgY + 8;
	DrawFormatString(ammoX, ammoY, 0xFFFFFF, "Ammo: %d / %d", m_ammo, m_maxAmmo);

	// リロード中表示
	if (m_isReloading)
	{
		DrawFormatString(ammoX, ammoY + kFontHeight, 0xFF4444, "Reloading...");
	}

	// スタミナ表示
	DrawFormatString(10, 30, 0x000000, "Stamina: %.1f", m_stamina);
	
	// スタミナゲージ描画
	const int gaugeX = 10;
	const int gaugeY = 50;
	const int gaugeWidth = 200;
	const int gaugeHeight = 16;

	// ゲージの枠
	DrawBox(gaugeX - 1, gaugeY - 1, gaugeX + gaugeWidth + 1, gaugeY + gaugeHeight + 1, GetColor(0x80, 0x80, 0x80), FALSE);

	// ゲージの中身（スタミナ割合で幅を決定）
	float staminaRate = m_stamina / kStaminaMax;
	int filledWidth = static_cast<int>(gaugeWidth * staminaRate);

	// ゲージ本体（緑→赤に変化させる）
	int r = static_cast<int>((1.0f - staminaRate) * 255);
	int g = static_cast<int>(staminaRate * 255);
	int gaugeColor = GetColor(r, g, 0);

	DrawBox(gaugeX, gaugeY, gaugeX + filledWidth, gaugeY + gaugeHeight, gaugeColor, TRUE);
}

void Player::DrawField()
{
	// ライティングを無効化
	SetUseLighting(false);

	// グリッドサイズとフィールド範囲の設定
	const int gridSize = 100;  // グリッド1マスのサイズ
	const int fieldSize = 800; // フィールドの範囲（-400～400）

	// 地味な色2色を設定
	unsigned int color1 = GetColor(80, 80, 80);  // 地味な色1
	unsigned int color2 = GetColor(100, 100, 100); // 地味な色2

	// 地面の描画
	for (int z = -fieldSize * 0.5f; z < fieldSize * 0.5f; z += gridSize)
	{
		for (int x = -fieldSize * 0.5f; x < fieldSize * 0.5f; x += gridSize)
		{
			// 立方体の中心座標
			VECTOR cubeCenter = VGet(x + gridSize * 0.5f, 0, z + gridSize * 0.5f);

			// 立方体のサイズ
			VECTOR cubeSize = VGet(gridSize, 10, gridSize);

			// 色を交互に切り替え
			unsigned int color = ((x / gridSize + z / gridSize) % 2 == 0) ? color1 : color2;

			// 立方体を描画
			DrawCube3D(
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y - 80, cubeCenter.z - cubeSize.z * 0.5f), // 最小座標
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y - 80 + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f), // 最大座標
				color,
				color,
				true // 塗りつぶし
			);
		}
	}

	// ライティングを再度有効化
	SetUseLighting(true);
}

void Player::Shoot()
{
	// 銃の位置を取得
	VECTOR gunPos = GetGunPos();

	// 銃の向きをオイラー角（ラジアン）で算出
	float rotX = -m_pCamera->GetPitch(); // X軸（上下）
	float rotY = m_pCamera->GetYaw();    // Y軸（左右）
	float rotZ = 0.0f;                   // Z軸（ロールは不要）

	// マズルフラッシュエフェクト再生
	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	// 弾を撃つSEを再生
	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);
}

void Player::AttachAnime(AnimData& data, const char* animName, bool isLoop)
{
	// アタッチしたいアニメーション番号を取得
	int index = MV1GetAnimIndex(m_modelHandle, animName);

	// モデルアニメーションをアタッチ
	data.attachNo = MV1AttachAnim(m_modelHandle, index, -1, false);
	// アニメーションカウンタ初期化
	data.count = 0.0f;
	// アニメーションのループ設定
	data.isLoop = isLoop;
	// アニメーションが終了したか
	data.isEnd = false;
}

void Player::UpdateAnime(AnimData& data)
{
	// アニメーションがアタッチされていない場合は何もしない
	if (data.attachNo == -1) return;

	// アニメーションの更新
	float animSpeed = 1.0f;
	// リロードアニメーション中は2倍速
	if (m_isReloading && data.attachNo == m_nextAnimData.attachNo) {
		animSpeed = 2.0f; // 2倍速
	}
	data.count += animSpeed;
	// 現在再生中のアニメーションの総時間を取得
	const float totalTime = MV1GetAttachAnimTotalTime(m_modelHandle, data.attachNo);

	// アニメーションの設定によってループさせるか最後のフレームで停止するかを判定
	if (data.isLoop)
	{
		// アニメーションをループさせる
		while (data.count > totalTime)
		{
			data.count -= totalTime;
		}
	}
	else
	{
		// 最後のフレームで停止する
		if (data.count > totalTime)
		{
			data.count = totalTime;
			data.isEnd = true;
		}
	}
	// 進行させたいアニメーションをモデルに適用する
	MV1SetAttachAnimTime(m_modelHandle, data.attachNo, data.count);
}

// アニメーションブレンド率の更新
void Player::UpdateAnimeBlend()
{
	//両方にアニメが設定されていない場合は変化させない
	if (m_nextAnimData.attachNo == -1 && m_prevAnimData.attachNo == -1) return;

	m_animBlendRate += 1.0f / 8.0f;
	if (m_animBlendRate > 1.0f)
	{
		m_animBlendRate = 1.0f;
	}

	// m_animBlendRateをアニメーションに適用する
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

void Player::ChangeAnime(const char* animName, bool isLoop)
{
	//一つ前のデータは今後管理できなくなるのであらかじめデタッチしておく
	MV1DetachAnim(m_modelHandle, m_prevAnimData.attachNo);

	// 現在再生中のアニメーションは一つ古いデータ扱いになる
	m_prevAnimData = m_nextAnimData;

	// 新たにショットのアニメーションをアタッチする
	AttachAnime(m_nextAnimData, animName, isLoop);

	// アニメーションブレンド率を初期化
	m_animBlendRate = 0.0f;

	// m_animBlendRateをアニメーションに適用する
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimData.attachNo, 1.0f - m_animBlendRate);
	MV1SetAttachAnimBlendRate(m_modelHandle, m_nextAnimData.attachNo, m_animBlendRate);
}

VECTOR Player::GetGunPos() const
{
   // モデルの実際の描画位置を算出
	VECTOR modelOffset = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw      = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch    = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot    = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset);

	// 銃のオフセットをモデルの描画位置から算出
	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
	VECTOR gunPos    = VTransform(gunOffset, modelRot);
	return VAdd(modelPosition, gunPos);
}

VECTOR Player::GetGunRot() const
{
	// 銃の方向を計算
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}

void Player::Reload()
{
	m_isReloading = true;
	m_reloadTimer = kReloadTime;
}
