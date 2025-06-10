#include "Player.h"
#include "DxLib.h"
#include "Game.h" 
#include "Mouse.h"
#include "Camera.h"
#include "Effect.h"	
#include "Bullet.h"
#include "SceneMain.h"
#include "EnemyBase.h"
#include <cmath>
#include <cassert>

namespace
{
	// アニメーション名
	const char* const kIdleAnimName = "Pistol_IDLE"; // 待機
	const char* const kShotAnimName = "Pistol_FIRE"; // 発射
	const char* const kWalkAnimName = "Pistol_WALK"; // 歩行
	const char* const kRunAnimName  = "Pistol_RUN";  // 走行
	const char* const kJumpAnimName = "Pistol_JUMP"; // ジャンプ

	constexpr float kMoveSpeed = 2.0f; // 移動速度
	constexpr float kRunSpeed  = 5.0f; // 走る速度

	// モデルのオフセット
	constexpr float kModelOffsetX = 2.0f; 
	constexpr float kModelOffsetY = 30.0f;
	constexpr float kModelOffsetZ = 25.0f;

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// 銃のオフセット
	constexpr float kGunOffsetX = 10.0f;
	constexpr float kGunOffsetY = 58.0f;
	constexpr float kGunOffsetZ = 12.0f;

	// スタミナ関連
	constexpr float kStaminaMax        = 100.0f;
	constexpr float kStaminaRunCost	   = 0.8f;
	constexpr float kStaminaRecover    = 0.5f;
	constexpr float kStaminaRunRecover = 30.0f;

	// 初期弾薬数
	constexpr int kInitialAmmo = 700;

	// UI関連
	constexpr int kMarginX    = 20; 
	constexpr int kMarginY    = 20;
	constexpr int kFontHeight = 20;

	// 重力とジャンプ関連
	constexpr float kGravity   = 0.35f; // 重力の強さ
	constexpr float kJumpPower = 7.0f;  // ジャンプの初速
	constexpr float kGroundY   = 0.0f;  // 地面のY座標
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
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
	m_shotCooldown(0),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_isJumping(false),
	m_jumpVelocity(0.0f),
	m_hasShot(false),
	m_lockOnTargetId(-1),
	m_isLockOn(false)
{
	// プレイヤーモデルの読み込み
	m_modelHandle = MV1LoadModel("data/model/Player.mv1");
	assert(m_modelHandle != -1);

	// シールドの読み込み
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

	// モデルの初期位置と回転
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, 0.0f, 0.0f));

	// アニメーションデータの初期化
	AttachAnime(m_nextAnimData, kIdleAnimName, true);
	m_animBlendRate = kAnimBlendRate; // アニメーションのブレンド率を設定
}

void Player::Update()
{
	// プレイヤーの位置をカメラに設定
	m_pCamera->SetPlayerPos(m_modelPos);

	m_pCamera->Update(); // カメラの更新
	m_pEffect->Update(); // エフェクトの更新

	UpdateAnime(m_prevAnimData); // 前のアニメーションデータを更新
	UpdateAnime(m_nextAnimData); //	次のアニメーションデータを更新
	UpdateAnimeBlend();			 // アニメーションのブレンドを更新

	// モデルの位置と回転を更新
	VECTOR modelOffset    = VGet(kModelOffsetX, kModelOffsetY, kModelOffsetZ);
	MATRIX rotYaw         = MGetRotY(m_pCamera->GetYaw());
	MATRIX rotPitch	      = MGetRotX(-m_pCamera->GetPitch());
	MATRIX modelRot       = MMult(rotPitch, rotYaw);
	VECTOR rotModelOffset = VTransform(modelOffset, modelRot);
	VECTOR modelPos       = VAdd(m_modelPos, rotModelOffset);

	// モデルの位置を設定
	MV1SetPosition(m_modelHandle, modelPos); 

	// モデルの回転を設定
	MV1SetRotationXYZ(m_modelHandle, VGet(m_pCamera->GetPitch(), m_pCamera->GetYaw() + DX_PI_F, 0.0f)); 

	// ショットクールダウンがある場合
	if (m_shotCooldown > 0) 
	{
		m_shotCooldown--;
	}

	// マウスの左クリックで射撃
	if (Mouse::IsTriggerLeft() && m_ammo > 0 && m_shotCooldown == 0)  
	{
		Shoot();  // 射撃処理
		m_ammo--; // 弾薬を減らす
		m_shotCooldown = 10; // ショットクールダウンを設定
		ChangeAnime(kShotAnimName, false); // 発射アニメーションに変更
	}

	// マウスの右クリックでロックオンの切り替え
	//if (Mouse::IsPressRight()) 
	//{
	//	m_isLockOn = true;
	//}
	//else
	//{
	//	m_isLockOn = false;
	//}

	// 弾の更新
	Bullet::UpdateBullets(m_bullets);

	// 走るキー入力
	const bool wantRun = CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT); 
	bool isRunning = false; // 走っているかどうかのフラグ

	// 走る条件
	if (wantRun && m_isCanRun && m_stamina > 0.0f) 
	{
		isRunning = true;
	}
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

	// 地面にいるか判定
	bool isOnGround = (m_modelPos.y <= kGroundY + 0.01f);

	// スペースキーでジャンプ
	if (CheckHitKey(KEY_INPUT_SPACE) && isOnGround && !m_isJumping)
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

	// 走っていて移動中なら
	if (isRunning && isMoving) 
	{
		m_stamina -= kStaminaRunCost; // スタミナを減らす

		// スタミナが0未満にならないようにする
		if (m_stamina < 0.0f) m_stamina = 0.0f; 
	}
	else
	{
		m_stamina += kStaminaRecover; // スタミナを回復

		// スタミナの上限を設定
		if (m_stamina > kStaminaMax) m_stamina = kStaminaMax; 
	}

	// スタミナが0以下なら走れない
	if (m_stamina <= 0.0f) 
	{
		m_isCanRun = false; // 走れない状態にする
	}
	else if (!m_isCanRun && m_stamina >= kStaminaRunRecover) // スタミナが回復したら
	{
		m_isCanRun = true; // 走れる状態に戻す
	}

	// アニメーションが終了したら
	if (m_nextAnimData.isEnd)  
	{
		if (m_isMoving)
		{
			// 移動アニメーションに変更
			ChangeAnime(m_isWasRunning ? kRunAnimName : kWalkAnimName, true); 
		}
		else
		{
			// 待機アニメーションに変更
			ChangeAnime(kIdleAnimName, true);
		}
	}

	// 移動中で前回は移動していなかった場合
	if (isMoving && !m_isMoving)
	{
		ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true);
	}
	else if (!isMoving && m_isMoving) 
	{
		ChangeAnime(kIdleAnimName, true);
	}
	else if (isMoving && m_isMoving && (isRunning != m_isWasRunning)) 
	{
		ChangeAnime(isRunning ? kRunAnimName : kWalkAnimName, true); 
	}

	m_isMoving	   = isMoving;  // 移動中の状態を更新
	m_isWasRunning = isRunning; // 走っている状態を更新
}
void Player::Draw()
{
	// プレイヤーモデルの描画
	MV1DrawModel(m_modelHandle); 

	// 弾の描画
	Bullet::DrawBullets(m_bullets);

	int screenW = Game::kScreenWidth;
	int screenH = Game::kScreenHeigth;

	float shieldScreenX = -25.0f;
	float shieldScreenY = -30;
	float shieldScreenZ = 55.0f;

	VECTOR camPos = VGet(0, 0, -shieldScreenZ); // カメラの位置
	VECTOR camTgt = VGet(0, 0, 0);			    // カメラのターゲット位置
	SetCameraPositionAndTarget_UpVecY(camPos, camTgt); 

	// 剣の位置
	MV1SetPosition(m_swordHandle, VGet(shieldScreenX, shieldScreenY, 0.0f));

	MV1SetRotationXYZ(m_swordHandle, VGet(0.0f, 200.0f, 0.0f)); // 剣の回転
	MV1SetScale(m_swordHandle, VGet(0.5f, 0.5f, 0.5f));		    // 剣のスケール

	// 剣の描画
	MV1DrawModel(m_swordHandle);

	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw(); // エフェクトの描画

	DrawField(); // フィールドの描画

	const int bgWidth = 160;
	const int bgHeight = 48;
	int bgX = screenW - kMarginX - bgWidth;
	int bgY = screenH - kMarginY - bgHeight;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(bgX, bgY, bgX + bgWidth, bgY + bgHeight, GetColor(0, 0, 0), false);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int ammoX = bgX + 12;
	int ammoY = bgY + 8;
	DrawFormatString(ammoX, ammoY, 0xFFFFFF, "Ammo: %d", m_ammo); 

	DrawFormatString(10, 30, 0x000000, "Stamina: %.1f", m_stamina);

	const int gaugeX = 10;
	const int gaugeY = 50;
	const int gaugeWidth = 200;
	const int gaugeHeight = 16;

	DrawBox(gaugeX - 1, gaugeY - 1, gaugeX + gaugeWidth + 1, gaugeY + gaugeHeight + 1, GetColor(0x80, 0x80, 0x80), false);

	float staminaRate = m_stamina / kStaminaMax;
	int filledWidth = static_cast<int>(gaugeWidth * staminaRate);

	int r = static_cast<int>((1.0f - staminaRate) * 255);
	int g = static_cast<int>(staminaRate * 255);
	int gaugeColor = GetColor(r, g, 0);

	DrawBox(gaugeX, gaugeY, gaugeX + filledWidth, gaugeY + gaugeHeight, gaugeColor, false);

	// ロックオン中なら画面中央に赤い円を描画
	if (m_isLockOn)
	{
		// 画面中央に赤い円を描画
		int centerX = Game::kScreenWidth * 0.5f;
		int centerY = Game::kScreenHeigth * 0.5f;
		int radius = 40; // 円の半径
		int color = GetColor(255, 64, 64);
		int thickness = 4; // 円の太さ
		DrawCircle(centerX, centerY, radius, color, false, thickness);
	}
}

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

bool Player::HasShot()
{
	bool shot = m_hasShot;
	m_hasShot = false; // 状態をリセット
	return shot; // 撃ったかどうかを返す
}

void Player::Shoot()
{
	// 銃の位置を取得
	VECTOR gunPos = GetGunPos();
	VECTOR gunDir = GetGunRot();

	// 弾を生成
	m_bullets.emplace_back(gunPos, gunDir, 10.0f);

	float rotX = -m_pCamera->GetPitch();
	float rotY = m_pCamera->GetYaw();
	float rotZ = 0.0f;

	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	// 弾を撃つSEを再生
	PlaySoundMem(m_shootSEHandle, DX_PLAYTYPE_BACK);

	m_hasShot = true; // 弾を撃ったフラグを立てる
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

// 銃の回転を取得
VECTOR Player::GetGunRot() const 
{
	return VGet(
		cosf(m_pCamera->GetPitch()) * sinf(m_pCamera->GetYaw()),
		sinf(m_pCamera->GetPitch()),
		cosf(m_pCamera->GetPitch()) * cosf(m_pCamera->GetYaw())
	);
}
