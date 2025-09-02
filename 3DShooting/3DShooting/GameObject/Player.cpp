#include "Player.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "EffekseerForDXLib.h"
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
	// 銃のオフセット
	constexpr float kGunOffsetX = 80.0f; 
	constexpr float kGunOffsetY = 20.0f;
	constexpr float kGunOffsetZ = 60.0f;

	// アニメーションのブレンド率
	constexpr float kAnimBlendRate = 1.0f; 

	// マズルフラッシュエフェクトのオフセット
	constexpr float kMuzzleFlashEffectOffsetX = -20.0f;
	constexpr float kMuzzleFlashEffectOffsetY = 30.0f;
	constexpr float kMuzzleFlashEffectOffsetZ = 80.0f;

	// UI関連
	constexpr int kMarginX    = 20; 
	constexpr int kMarginY    = 20;
	constexpr int kFontHeight = 20;

	// 重力とジャンプ関連
	constexpr float kGravity   = 0.35f; // 重力の強さ
	constexpr float kJumpPower = 7.0f;  // ジャンプの初速
	constexpr float kGroundY   = 0.0f;  // 地面のY座標

	// タックル関連
	constexpr int   kTackleDuration  = 20;     // タックル持続フレーム数
	constexpr float kTackleHitRange  = 250.0f; // タックルの前方有効距離
	constexpr float kTackleHitRadius = 250.0f; // タックルの横幅（半径）
	constexpr float kTackleHitHeight = 100.0f; // タックルの高さ
	// タックルクールタイムゲージ
	constexpr int kTackleGaugeX		 = 10;  // タックルクールタイムゲージのX座標
	constexpr int kTackleGaugeY		 = 50;  // タックルクールタイムゲージのY座標
	constexpr int kTackleGaugeWidth  = 200; // タックルクールタイムゲージの幅
	constexpr int kTackleGaugeHeight = 16;  // タックルクールタイムゲージの高さ

	// カプセルコライダーのサイズ
	constexpr float kCapsuleHeight = 100.0f; // カプセルコライダーの高さ
	constexpr float kCapsuleRadius = 50.0f;  // カプセルコライダーの半径

	// 集中線関連
	constexpr int   kNumLines			 = 18;    // 集中線の本数
	constexpr float kMaxScreenRadiusRate = 0.95f; // 集中線の最大長さ（画面対角の何割まで伸ばすか）
	constexpr float kLenRatio			 = 0.75f; // 集中線の標準長さ（0.0〜1.0、1.0で最大）
	constexpr int   kBaseThickness		 = 200;   // 集中線の根元（外側）の太さ
	constexpr int   kTipThickness		 = 4;     // 集中線の先端（中心側）の太さ
	constexpr float kAppear			     = 0.2f;  // 集中線が伸びるアニメーションの比率（0.0〜1.0）
	constexpr float kVanish				 = 0.2f;  // 集中線が消えるアニメーションの比率（0.0〜1.0）
	constexpr float kOverRatioScale		 = 1.2f;  // 消える時の根元の外側へのスケール（1.0で画面端、1.2で完全に外）

	constexpr float kShootRate = 10.0f; // 1秒あたりの発射回数

	// X,Z座標の移動範囲制限
	constexpr float kLimitMoveX = 2800.0f;
	constexpr float kLimitMoveZ = 2800.0f;
}

Player::Player() :
	m_modelHandle(-1),
	m_swordHandle(-1),
	m_shootSEHandle(-1),
	m_playerHitSEHandle(-1),
	m_tackleSEHandle(-1),
	m_recoverySEHandle(-1),
	m_ammoItemSEHandle(-1),
	m_modelPos(VGet(0, 0, 0)),
	m_pEffect(std::make_shared<Effect>()),
	m_pCamera(std::make_shared<Camera>()),
	m_pDebugCamera(std::make_shared<Camera>()),
	m_pEnemy(std::make_shared<EnemyNormal>()),
	m_pBodyCollider(std::make_shared<CapsuleCollider>()),
	m_animBlendRate(0.0f),
	m_isMoving(false),
	m_isWasRunning(false),
	m_pos(VGet(0, 0, 0)),
	m_health(100.0f),
	m_healthBarAnim(100.0f),
	m_healthBarAnimTimer(0.0f),
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
	m_healEffectTimer(0.0f),
	m_ammoEffectAlpha(0.0f),
	m_ammoEffectTimer(0.0f),
	m_shootCooldown(0.0f),
	m_shootCooldownTimer(0.0f),
	m_shootRate(kShootRate),
	m_isInvincible(false),
	m_isInfiniteAmmo(false),
	m_tackleCooldownMax(0.0f),
	m_tackleSpeed(0.0f),
	m_tackleDamage(0.0f),
	m_isSwordAnimating(false),
	m_swordAnimTimer(0.0f),
	m_swordAnimDuration(0.05f)
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
	m_playerHitSEHandle = LoadSoundMem("data/sound/SE/PlayerHit.mp3");
	assert(m_playerHitSEHandle != -1);
	m_tackleSEHandle = LoadSoundMem("data/sound/SE/Tackle.mp3");
	assert(m_tackleSEHandle != -1);
	m_recoverySEHandle = LoadSoundMem("data/sound/SE/RecoveryItem.mp3");
	assert(m_recoverySEHandle != -1);
	m_ammoItemSEHandle = LoadSoundMem("data/sound/SE/AmmoItem.mp3");
	assert(m_ammoItemSEHandle != -1);
}

Player::~Player()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_swordHandle);

	// SEの解放
	DeleteSoundMem(m_shootSEHandle);
	DeleteSoundMem(m_playerHitSEHandle);
	DeleteSoundMem(m_tackleSEHandle);
	DeleteSoundMem(m_recoverySEHandle);
	DeleteSoundMem(m_ammoItemSEHandle);
}

void Player::Init()
{
	// CSVからPlayerのTransform情報を取得
	auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
	for (const auto& data : dataList) 
	{
		if (data.name == "Player") 
		{
			m_pos				= data.pos;
			m_modelPos			= data.pos;
			m_scale				= data.scale;
			m_health			= data.hp;
			m_maxHealth			= data.hp;
			m_moveSpeed		    = data.speed;
			m_tackleCooldownMax = data.tackleCooldown;
			m_tackleSpeed		= data.tackleSpeed;
			m_tackleDamage		= data.tackleDamage;
			m_runSpeed			= data.runSpeed;
			m_initialAmmo		= data.initialAmmo;
			m_bulletPower		= data.bulletPower;
			MV1SetScale(m_modelHandle, data.scale);
			MV1SetRotationXYZ(m_modelHandle, data.rot);
			break;
		}
	}

	m_pEffect->Init(); // エフェクトの初期化
	m_pCamera->Init(); // カメラの初期化

	m_shootCooldown = 1.0f / m_shootRate; // 発射クールタイムを設定

	m_animBlendRate = kAnimBlendRate; // アニメーションのブレンド率を設定

	// CSVの初期弾薬数を反映
	m_ammo = m_initialAmmo;
}

void Player::Update(const std::vector<EnemyBase*>& enemyList)
{
	unsigned char keyState[256];
	GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

	// クールタイムタイマー減算
	if (m_shootCooldownTimer > 0.0f) 
	{
		m_shootCooldownTimer -= 1.0f / 60.0f;
		if (m_shootCooldownTimer < 0.0f) m_shootCooldownTimer = 0.0f;
	}

	// プレイヤーの位置をカメラに設定
	m_pCamera->SetPlayerPos(m_modelPos);

	// 剣のアニメーションタイマー更新
	if (m_isSwordAnimating)
	{
		m_swordAnimTimer += 1.0f / 60.0f;
		if (m_swordAnimTimer >= m_swordAnimDuration)
		{
			m_isSwordAnimating = false;
			m_swordAnimTimer = 0.0f;
		}
	}

	m_pCamera->Update(); // カメラの更新
	m_pEffect->Update(); // エフェクトの更新

	// プレイヤーのカプセルコライダーを毎フレーム更新
	VECTOR center = m_modelPos;
	center.y += 60.0f; // 足元から腰～胸あたりを中心に
	VECTOR capA = VAdd(center, VGet(0, -kCapsuleHeight * 0.5f, 0));
	VECTOR capB = VAdd(center, VGet(0, kCapsuleHeight * 0.5f, 0));
	m_pBodyCollider->SetSegment(capA, capB);
	m_pBodyCollider->SetRadius(kCapsuleRadius);

	// モデルの位置と回転を更新
	VECTOR modelOffset	  = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ);
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

	// マウスの左クリックで射撃（タックル中は射撃不可）
	if (!m_isTackling && Mouse::IsPressLeft() && (m_ammo > 0 || m_isInfiniteAmmo) && m_shootCooldownTimer <= 0.0f)
	{
		Shoot(m_bullets); // 弾を発射

		// 弾薬無限モードでない場合のみ弾薬を減らす
		if (!m_isInfiniteAmmo)
		{
			m_ammo--; // 弾薬を減らす
		}

		m_shootCooldownTimer = m_shootCooldown; // クールタイムリセット
	}

	// 地面にいるかどうかの判定
	bool isOnGround = (m_modelPos.y <= kGroundY + 0.01f);

	// 右クリックでタックル開始
	if (!m_isTackling && m_tackleCooldown <= 0 && Mouse::IsTriggerRight())
	{
		m_isTackling = true;
		m_isSwordAnimating = true; // 剣のアニメーション開始
		m_swordAnimTimer = 0.0f;   // タイマーリセット

		PlaySoundMem(m_tackleSEHandle, DX_PLAYTYPE_BACK); // タックルSE再生
		m_tackleFrame = kTackleDuration;
		m_tackleCooldown = m_tackleCooldownMax; // クールタイム開始
		m_tackleId++; // タックルごとにIDを更新

		// カメラの向きで3D正規化ベクトルを作成
		float yaw   = m_pCamera->GetYaw();
		float pitch = m_pCamera->GetPitch();

		// タックル方向を計算
		m_tackleDir = VGet(
			cosf(pitch) * sinf(yaw),
			sinf(pitch),
			cosf(pitch) * cosf(yaw)
		);

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
		}
		// タックル中は他の移動・ジャンプを無効化
		return;
	}

	// 各敵に更新処理を行うためのタックル情報を作成
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

	// X,Z座標の移動範囲制限
	m_modelPos.x = std::clamp(m_modelPos.x, -kLimitMoveX, kLimitMoveX);
	m_modelPos.z = std::clamp(m_modelPos.z, -kLimitMoveZ, kLimitMoveZ);

	m_isMoving = isMoving;      // 移動中の状態を更新
	m_isWasRunning = isRunning; // 走っている状態を更新

	// Head Bobbing状態をカメラに設定
	if (m_pCamera)
	{
		m_pCamera->SetHeadBobbingState(isMoving, isRunning);
	}

	std::copy(std::begin(keyState), std::end(keyState), std::begin(m_prevKeyState));

	// HPバーアニメーション（ダメージ分を徐々に減らす）
	if (m_healthBarAnim > m_health) 
	{
		float animSpeed = 1.5f; // 減少速度（大きいほど速い）
		m_healthBarAnim -= animSpeed;
		if (m_healthBarAnim < m_health) m_healthBarAnim = m_health;
	} 
	else 
	{
		m_healthBarAnim = m_health;
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

	// 弾薬無限モードの場合は「∞」を表示
	if (m_isInfiniteAmmo)
	{
		DrawFormatString(ammoX, ammoY, 0xffff50, "AMMO: ∞");
	}
	else
	{
		DrawFormatString(ammoX, ammoY, 0xffff50, "AMMO: %d", m_ammo);
	}

	SetFontSize(16); // フォントサイズを元に戻す

	/*剣の描画*/
	// 画面サイズに応じてスケーリング
	float baseScreenW = 640.0f;
	float baseScreenH = 480.0f;
	float scaleW = screenW / baseScreenW;
	float scaleH = screenH / baseScreenH;
	float scaleAvg = (scaleW + scaleH) * 0.5f;

	// カメラオフセット設定
	VECTOR totalCameraOffset = VGet(0, 0, 0);
	if (m_pCamera)
	{
		VECTOR shakeOffset   = m_pCamera->GetShakeOffset();
		VECTOR headBobOffset = m_pCamera->GetHeadBobOffset();
		totalCameraOffset    = VAdd(shakeOffset, headBobOffset);
	}
	VECTOR swordCamPos = VGet(0, 0, -35.0f * scaleAvg);
	swordCamPos.x += totalCameraOffset.x;
	swordCamPos.y += totalCameraOffset.y;
	VECTOR swordCamTarget = VGet(totalCameraOffset.x * 0.3f, totalCameraOffset.y * 0.3f, 0);
	SetCameraPositionAndTarget_UpVecY(swordCamPos, swordCamTarget);

	// 描画するかどうかのフラグ
	bool shouldDrawSword = (m_tackleCooldown <= 0) || m_isSwordAnimating;
	if (!m_isSwordAnimating && m_tackleCooldown > 0) 
	{
		shouldDrawSword = false; // クールダウン中は非表示
	}

	if (shouldDrawSword)
	{
		// 待機状態の剣の位置と回転を定義
		VECTOR waitPos = VGet(-20.0f * scaleW, -30.0f * scaleH, -10.0f);
		VECTOR waitRotVec = VGet(0.0f, 30.0f, 0.0f); // Y軸回転を0度に変更

		// 待機状態の基本となる変換行列を作成 (回転 * 平行移動)
		MATRIX matWaitRot  = MGetRotY(waitRotVec.y);
		MATRIX matWaitPos  = MGetTranslate(waitPos);
		MATRIX waitMatrix  = MMult(matWaitRot, matWaitPos);
		MATRIX finalMatrix = waitMatrix;

		if (m_isSwordAnimating)
		{
			float animProgress = m_swordAnimTimer / m_swordAnimDuration;
			// イージング（滑らかな動き）
			animProgress = -0.5f * (cosf(DX_PI_F * animProgress) - 1.0f);

			// 回転角度を計算 (左 -> 右)
			float startAngle = 25.0f * (DX_PI_F / 180.0f);
			float endAngle = 180.0f * (DX_PI_F / 180.0f);
			float currentAngle = startAngle + (endAngle - startAngle) * animProgress;

			// モデルのローカル座標におけるピボット（手持ち部分）の位置
			VECTOR pivotOffset = VGet(0.0f, 0.0f, -20.0f);

			// 行列を使ってピボット回転を実装
			MATRIX matPivotTrans = MGetTranslate(VScale(pivotOffset, -1.0f));
			MATRIX matRotate = MGetRotY(currentAngle);
			MATRIX matPivotTransInv = MGetTranslate(pivotOffset);

			// 最終的な行列をMMultで計算
			// finalMatrix = (待機時の行列) -> (ピボットを原点へ) -> (回転) -> (ピボットを戻す)
			MATRIX tempMat1 = MMult(waitMatrix, matPivotTrans);
			MATRIX tempMat2 = MMult(tempMat1, matRotate);
			finalMatrix = MMult(tempMat2, matPivotTransInv);
		}

		// モデルに最終的な変換行列を適用
		MV1SetMatrix(m_swordHandle, finalMatrix);
		MV1SetScale(m_swordHandle, VGet(0.5f * scaleAvg, 0.5f * scaleAvg, 0.5f * scaleAvg));
		MV1DrawModel(m_swordHandle);
	}

	// メインカメラに戻す
	m_pCamera->SetCameraToDxLib();

	m_pEffect->Draw(); // エフェクトの描画

	// 枠
	DrawBox(kTackleGaugeX - 1, kTackleGaugeY - 1, kTackleGaugeX + kTackleGaugeWidth + 1, kTackleGaugeY + kTackleGaugeHeight + 1, 0x5050C8, false);

	// ゲージ本体
	float tackleRate = 1.0f - (m_tackleCooldown / static_cast<float>(m_tackleCooldownMax));
	int tackleFilledWidth = static_cast<int>(kTackleGaugeWidth * tackleRate);
	DrawBox(kTackleGaugeX, kTackleGaugeY, kTackleGaugeX + tackleFilledWidth, kTackleGaugeY + kTackleGaugeHeight, 0x50B4ff, true);

	// テキスト
	DrawFormatString(kTackleGaugeX, kTackleGaugeY - 18, 0xFFFFFF, "Tackle Cooldown");
	if (m_tackleCooldown > 0) 
	{
		DrawFormatString(kTackleGaugeX + kTackleGaugeWidth + 10, kTackleGaugeY, 0xFF8080, "WAIT");
	}
	else 
	{
		DrawFormatString(kTackleGaugeX + kTackleGaugeWidth + 10, kTackleGaugeY, 0x80FF80, "READY");
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

    float hpAnim = m_healthBarAnim;

    if (hpAnim < 0) hpAnim = 0;
    if (hpAnim > maxHP) hpAnim = maxHP;

    // HP割合
    float hpRate = hp / maxHP;
    float hpAnimRate = hpAnim / maxHP;

    // 背景
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, 0x505050, true);

    // ダメージ分（アニメーション中の減少分）
    if (hpAnim > hp) {
        int animStart = barX + static_cast<int>(barWidth * hpRate);
        int animEnd   = barX + static_cast<int>(barWidth * hpAnimRate);
        DrawBox(animStart, barY, animEnd, barY + barHeight, 0xFFD700, true);
    }
    // HPバー本体
    DrawBox(barX, barY, barX + static_cast<int>(barWidth * hpRate), barY + barHeight, 0xff4040, true);

    // 枠
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, 0x000000, false);

    // HP数値
    DrawFormatString(barX + 8, barY + 2, 0xffffff, "HP: %.0f / %.0f", hp, maxHP);

	// ダメージエフェクト描画（赤）
	DrawEffectFeedback(m_damageEffect);

	// 回復エフェクト描画(緑)
	DrawEffectFeedback(m_healEffect);

    // 弾薬エフェクト描画(オレンジ)
    DrawEffectFeedback(m_ammoEffect);

	// タックル中は集中線エフェクトを描画
	if (m_isTackling)
	{
		DrawTackleLines();
	}
}

void Player::DrawTackleLines()
{
    int screenW = Game::kScreenWidth;
    int screenH = Game::kScreenHeigth;
    int centerX = screenW * 0.5f;
    int centerY = screenH * 0.5f;
    // 画面中心から端までの最大半径を計算
    float maxScreenRadius = sqrtf((float)(screenW * screenW + screenH * screenH)) * kMaxScreenRadiusRate;
    // タックル進行度（0.0:開始→1.0:終了）
    float progress = 1.0f - (float)m_tackleFrame / (float)kTackleDuration;
    // 集中線の明るさ（進行度に応じて変化）
    float intensity = 0.3f + progress * 0.7f;

    for (int i = 0; i < kNumLines; ++i)
    {
        // 放射状に等間隔で角度を決定
        float baseAngle = (float)i / kNumLines * 2.0f * DX_PI_F;
        float angle = baseAngle;

        // 線の長さ（全て同じ）
        float lenRatio = kLenRatio;

        // 描画範囲（根元・先端の進行度）
        float drawStart = 0.0f;
        float drawEnd   = 1.0f;

        // 根元・先端の半径を計算
        float startRadius = maxScreenRadius * (1.0f - lenRatio * drawStart);
        float endRadius = maxScreenRadius * (1.0f - lenRatio * drawEnd);

        // 伸びるアニメーション
        if (progress < kAppear)
        {
            drawEnd = progress / kAppear;
            endRadius = maxScreenRadius * (1.0f - lenRatio * drawEnd);
        }
        // 消えるアニメーション（根元が外側に抜けていく）
        else if (progress > 1.0f - kVanish)
        {
            drawStart = (progress - (1.0f - kVanish)) / kVanish;
            float overRatio = drawStart * kOverRatioScale;
            startRadius = maxScreenRadius * (1.0f - lenRatio * drawStart + overRatio * lenRatio);
        }

        // 根元・先端の座標を計算
        int x1 = centerX + (int)(startRadius * cosf(angle));
        int y1 = centerY + (int)(startRadius * sinf(angle));
        int x2 = centerX + (int)(endRadius * cosf(angle));
        int y2 = centerY + (int)(endRadius * sinf(angle));

        // 太さ（根元が太く、先端が細い）
        int baseThickness = kBaseThickness;
        int tipThickness = kTipThickness;
        float perpAngle = angle + DX_PI_F * 0.5f;
        float baseHalf = baseThickness * 0.5f;
        float tipHalf = tipThickness * 0.5f;

        // 四隅の座標を計算（台形/三角形のため）
        int bx1 = x1 + (int)(baseHalf * cosf(perpAngle));
        int by1 = y1 + (int)(baseHalf * sinf(perpAngle));
        int bx2 = x1 - (int)(baseHalf * cosf(perpAngle));
        int by2 = y1 - (int)(baseHalf * sinf(perpAngle));
        int tx1 = x2 + (int)(tipHalf * cosf(perpAngle));
        int ty1 = y2 + (int)(tipHalf * sinf(perpAngle));
        int tx2 = x2 - (int)(tipHalf * cosf(perpAngle));
        int ty2 = y2 - (int)(tipHalf * sinf(perpAngle));

        // アルファブレンドで白色の集中線を描画
        int alpha = (int)(255 * intensity);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawTriangle(bx1, by1, bx2, by2, tx1, ty1, 0xffffff, true);
        DrawTriangle(bx2, by2, tx1, ty1, tx2, ty2, 0xffffff, true);
    }
    // ブレンドモードを元に戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Player::DrawEffectFeedback(Player::EffectFeedback& effect)
{
    if (effect.timer > 0.0f && effect.alpha > 0.0f)
    {
        int screenW, screenH;
        GetScreenState(&screenW, &screenH, nullptr);
        int centerX = screenW * 0.5f;
        int centerY = screenH * 0.5f;
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
                    DrawBox(x, y, x + stepSize, y + stepSize, GetColor(effect.colorR, effect.colorG, effect.colorB), true);
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
	if (m_isInvincible) return; // 無敵モード中はダメージを受けない

	m_health -= damage; // ダメージを適用
	if (m_health < 0.0f)
	{
		m_health = 0.0f; // 体力が負にならないように制限
	}
	// HPバーアニメーション用タイマーをリセット
	m_healthBarAnimTimer = 0.0f;
	// ダメージエフェクトを発動
	m_damageEffect.Trigger(30.0f, 255, 0, 0); 
	// 被弾SEを再生
	PlaySoundMem(m_playerHitSEHandle, DX_PLAYTYPE_BACK);

	// カメラシェイクを発生
	if (m_pCamera)
	{
		m_pCamera->Shake(5.0f, 15);
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
    // 画面中央(カメラ中心)からレティクル方向へ発射
    VECTOR cameraPos = m_pCamera->GetPos();
    VECTOR cameraForward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));

	VECTOR gunPos = GetGunPos();
	VECTOR gunDir = GetGunRot();

    // 画面中央から出ているように見せるため、カメラ前方に小さくオフセット
    constexpr float kCameraMuzzleOffset = 10.0f;
    VECTOR spawnPos = VAdd(cameraPos, VScale(cameraForward, kCameraMuzzleOffset));

    // 弾丸を発射（起点: 画面中央、方向: レティクル方向）
    bullets.emplace_back(spawnPos, cameraForward, m_bulletPower);
    //bullets.emplace_back(gunPos, gunDir, m_bulletPower);

	float rotX = -m_pCamera->GetPitch();
	float rotY = m_pCamera->GetYaw();
	float rotZ = 0.0f;

	if (m_pEffect)
	{
		m_pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
	}

	// カメラシェイクを発生
	if (m_pCamera)
	{
		m_pCamera->Shake(6.0f, 8); // 強さ・フレーム数
	}
}

// 銃の位置を取得
VECTOR Player::GetGunPos() const
{
	// モデルのオフセットと回転を計算
	VECTOR modelOffset        = VGet(kGunOffsetX, kGunOffsetY, kGunOffsetZ); // モデルのオフセット
	MATRIX rotYaw             = MGetRotY(m_pCamera->GetYaw());        // カメラのヨー回転
	MATRIX rotPitch           = MGetRotX(-m_pCamera->GetPitch());	  // カメラのピッチ回転
	MATRIX modelRot           = MMult(rotPitch, rotYaw);			  // モデルの回転行列を計算
	VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);	  // オフセットを回転
	VECTOR modelPosition      = VAdd(m_modelPos, rotatedModelOffset); // モデルの位置とオフセットを組み合わせて銃の位置を計算

	VECTOR gunOffset = VGet(kMuzzleFlashEffectOffsetX, kMuzzleFlashEffectOffsetY, kMuzzleFlashEffectOffsetZ); // マズルフラッシュのオフセット
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
		bodyCenter.y += kGunOffsetY;

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
    m_healEffect.Trigger(45.0f, 0, 255, 0);
}

void Player::AddAmmo(int value)
{
    m_ammo += value;
    if (m_ammo < 0) m_ammo = 0;
    // 弾薬取得時にエフェクトを発動
    m_ammoEffect.Trigger(45.0f, 255, 128, 0);
}
