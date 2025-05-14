#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include <cmath>
#include <cassert>

namespace
{
	// アニメーション名
	const char* const kIdleAnimName = "Pistol_IDLE"; // 待機
	const char* const kShotAnimName = "Pistol_FIRE"; // 弾を撃つ
	const char* const kWalkAnimName = "Pistol_WALK"; // 歩く
	const char* const kRunAnimName  = "Pistol_RUN";  // 走る

	constexpr float kMoveSpeed = 2.0f; // 歩く速度
	constexpr float kRunSpeed  = 5.0f; // 走る速度

	// プレイヤーモデルのオフセット
	constexpr float kModelOffsetX = 4.0f;  // X軸オフセット
	constexpr float kModelOffsetY = 30.0f; // Y軸オフセット
	constexpr float kModelOffsetZ = 40.0f; // Z軸オフセット

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// 銃のオフセット
	constexpr float kGunOffsetX = 10.0f;  // X軸オフセット
	constexpr float kGunOffsetY = 88.0f;  // Y軸オフセット
	constexpr float kGunOffsetZ = 200.0f; // Z軸オフセット
}

Player::Player() :
	m_modelHandle(-1),
	m_shootSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pCamera(std::make_shared<Camera>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false)
{
	// モデルの読み込み
	m_modelHandle = MV1LoadModel("data/image/player.mv1");

	// 弾を撃つSEを読み込む
	m_shootSEHandle = LoadSoundMem("data/sound/SE/GunShot.mp3");
	assert(m_shootSEHandle != -1);

	// プレイヤーモデルの初期回転を設定 (Z-方向を向く)
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));
}

Player::~Player()
{
	// モデルの削除
	MV1DeleteModel(m_modelHandle);
	// SEの削除
	DeleteSoundMem(m_shootSEHandle);
}

void Player::Init()
{
	// カメラの初期化
	m_pCamera->Init();

	// アニメーションのアタッチ
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate;
}

void Player::Update()
{
	// カメラの位置をプレイヤーの位置に基づいて更新
	m_pCamera->SetPlayerPos(m_modelPos);

	// カメラの更新
	m_pCamera->Update();

	UpdateAnime(m_prevAnimData);
	UpdateAnime(m_nextAnimData);
	UpdateAnimeBlend();

	// モデルの位置をカメラの前方に設定
	VECTOR modelOffset        = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw             = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch           = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot			  = MMult(rotPitch, rotYaw);
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset);

	// モデルの位置を更新
	MV1SetPosition(m_modelHandle, modelPosition);

	// プレイヤーモデルの回転を更新
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f));

	// 弾の発射
	if (Mouse::IsTriggerLeft())
	{
		Shoot();
		ChangeAnime(kShotAnimName, false);
	}

	// 移動状態・走り状態の判定
	const bool isRunning = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT);
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

	// ショットアニメーション終了時の復帰
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

	m_isMoving = isMoving;     
	m_isWasRunning = isRunning; 
}
void Player::Draw()
{
	// モデルの描画
	MV1DrawModel(m_modelHandle);

	// フィールドの描画
	DrawField();
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
				VGet(cubeCenter.x - cubeSize.x * 0.5f, cubeCenter.y, cubeCenter.z - cubeSize.z * 0.5f), // 最小座標
				VGet(cubeCenter.x + cubeSize.x * 0.5f, cubeCenter.y + cubeSize.y, cubeCenter.z + cubeSize.z * 0.5f), // 最大座標
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
	VECTOR gunDir = GetGunRot();

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
	data.count += 1.0f;

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
	// 銃の位置をプレイヤーモデルの位置に基づいて設定
	VECTOR gunOffset = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
	MATRIX rotYaw    = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch  = MGetRotX(-m_pCamera->GetPitch());
	MATRIX gunRot    = MMult(rotPitch, rotYaw);
	return VAdd(m_modelPos, VTransform(gunOffset, gunRot));
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
