#include "Enemy.h"
#include <random>

#include "../../Attack/AttackManager.h"
#include "../../Attack/AttackBase.h"
#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../Common/AnimationController.h"
#include "../../Attack/RangedAttack/RangedAttack.h"

#include "../Player/Player.h"

Enemy::Enemy(Player* player)
{
	player_ = player;
}

Enemy::~Enemy(void)
{
}

void Enemy::Update(void)
{
	ActorBase::Update();

	switch (state_) {
	case ActorState::IDLE:
		stateTimer_ -= 1.0f / 60.0f; // 60FPS想定
		if (stateTimer_ <= 0.0f) {
			// HP閾値でULTIMATE強制遷移
			if (hp_ < maxHp_ * 0.3f) {
				ChangeState(ActorState::ULTIMATE);

				break;
			}
			// 確率抽選
			float r = 0.2;
			if (r < 0.3f) {
				ChangeState(ActorState::ATTACK_NEAR);

			}
			else if (r < 0.8f) {
				ChangeState(ActorState::ATTACK_RANGE);
			}
			else {
				ChangeState(ActorState::ULTIMATE);
			}
		}
		break;
	case ActorState::ATTACK_NEAR:
		if (!attackRegistered_) {
			VECTOR playerPos = player_->GetPos();

			// 発射開始位置（Yだけ上げる）
			VECTOR launchPos = pos_;
			launchPos.y += 00.0f;

			// launchPos から playerPos への方向ベクトル
			VECTOR toPlayer = {
				playerPos.x - launchPos.x,
				playerPos.y - launchPos.y,
				playerPos.z - launchPos.z
			};
			float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
			VECTOR dir = { 0, 0, 0 };
			if (len > 0.0f) {
				dir.x = toPlayer.x / len;
				dir.y = toPlayer.y / len;
				dir.z = toPlayer.z / len;
			}
			VECTOR velocity = { dir.x * 20.0f, dir.y * 20.0f, dir.z * 20.0f };

			attackManager_->Add(new RangedAttack(
				launchPos,    // 発射開始位置
				velocity,     // プレイヤーの場所に向かう速度
				10,
				this
			));
			attackRegistered_ = true;
			stateTimer_ = 1.0f;
		}
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
		}
		break;



	case ActorState::ATTACK_RANGE:
		if (!attackRegistered_) {
			// プレイヤーの座標を取得
			VECTOR playerPos = player_->GetPos();
			// 敵からプレイヤーへの方向ベクトル
			VECTOR toPlayer = {
				playerPos.x - pos_.x,
				playerPos.y - pos_.y,
				playerPos.z - pos_.z
			};
			// 正規化
			float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
			VECTOR dir = { 0, 0, 0 };
			if (len > 0.0f) {
				dir.x = toPlayer.x / len;
				dir.y = toPlayer.y / len;
				dir.z = toPlayer.z / len;
			}
			// 速度を設定（例: 20.0f）
			VECTOR velocity = { dir.x * 20.0f, dir.y * 20.0f, dir.z * 20.0f };

			attackManager_->Add(new RangedAttack(
				pos_,        // 発射位置
				velocity,    // プレイヤー方向の速度
				10,          // ダメージ
				this         // 発射者
			));
			attackRegistered_ = true;
			stateTimer_ = 1.0f; // 例
		}
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
		}
		break;

	case ActorState::ULTIMATE:
		if (!attackRegistered_) {
			// プレイヤーの座標を取得
			VECTOR playerPos = player_->GetPos();
			// 敵からプレイヤーへの方向ベクトル
			VECTOR toPlayer = {
				playerPos.x - pos_.x,
				playerPos.y - pos_.y,
				playerPos.z - pos_.z
			};
			// 正規化
			float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
			VECTOR dir = { 0, 0, 0 };
			if (len > 0.0f) {
				dir.x = toPlayer.x / len;
				dir.y = toPlayer.y / len;
				dir.z = toPlayer.z / len;
			}
			// 速度を設定（例: 20.0f）
			VECTOR velocity = { dir.x * 20.0f, dir.y * 20.0f, dir.z * 20.0f };

			attackManager_->Add(new RangedAttack(
				pos_,        // 発射位置
				velocity,    // プレイヤー方向の速度
				10,          // ダメージ
				this         // 発射者
			));
			attackRegistered_ = true;
			stateTimer_ = 1.0f; // 例
		}
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
		}
		break;

	case ActorState::STUN:
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
		}
		break;
	}


	//// 索敵
	//Search();
	//if (isNoticeView_)
	//{
	//	// プレイヤーの座標取得
	//	VECTOR playerPos = player_->GetPos();

	//	// 敵からプレイヤーへのベクトル
	//	VECTOR toPlayer = VSub(playerPos, pos_);

	//	// XZ平面の距離
	//	float dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
	//	if (dist > 0.0f)
	//	{
	//		// 方向ベクトル正規化
	//		VECTOR dir = { toPlayer.x / dist, 0.0f, toPlayer.z / dist };

	//		// 速度
	//		const float ENEMY_SPEED = 2.0f;

	//		// 移動
	//		pos_.x += dir.x * ENEMY_SPEED;
	//		pos_.z += dir.z * ENEMY_SPEED;

	//		// プレイヤー方向に回転
	//		float targetAngleY = atan2f(dir.x, dir.z);
	//		angle_.y = AsoUtility::LerpAngle(angle_.y, targetAngleY, 0.2f);

	//	}
	//}
	// 
    // モデルの位置を反映
	MV1SetPosition(modelId_, pos_);
	// 歩くアニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}

void Enemy::Draw(void)
{
	//if (isNoticeView_)
	//{
	//	// 視野範囲内：ディフューズカラーを赤色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(1.0f, 0.0f, 0.0f, 1.0f));
	//}
	//else if (isNoticeHearing_)
	//{
	//	// 聴覚範囲内：ディフューズカラーを黄色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(1.0f, 1.0f, 0.0f, 1.0f));
	//}
	//else
	//{
	//	// 視野範囲外：ディフューズカラーを灰色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
	//}

	ActorBase::Draw();

	// 視野描画
	//DrawViewRange();
	// --- カプセル当たり判定のデバッグ表示 ---
	VECTOR start = VAdd(pos_, startCapsulePos_);
	VECTOR end = VAdd(pos_, endCapsulePos_);
	float radius = capsuleRadius_;
	DrawCapsule3D(start, end, radius, 16, GetColor(0, 255, 0), false,false);

	DrawFormatString(700, 10, 0xFFFFFF, "HP: %d / %d", GetHp(), GetMaxHp());
}

void Enemy::InitLoad(void)
{
	// モデルの読み込み
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Player/Player.mv1").c_str());
}

void Enemy::InitTransform(void)
{
	// モデルの角度
	angle_ = { 0.0f, AsoUtility::Deg2RadF(90), 0.0f };
	localAngle_ = { 0.0f, AsoUtility::Deg2RadF(270.0f), 0.0f };

	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);
	MV1SetRotationMatrix(modelId_, mat);

	// グリッド外に配置（例: X=-500, Y=-400, Z=1000）
	//pos_ = { 0.0f, -2000.0f, 4000.0f };
	pos_ = { 0.0f, -0, 800 };

	// ボスサイズに拡大（例: X=30, Y=18, Z=18）
	//scale_ = { 30, 18, 18 };
	scale_ = { 1, 1, 1 };
	MV1SetPosition(modelId_, pos_);
	MV1SetScale(modelId_, scale_);

	// 当たり判定も大きく
	//startCapsulePos_ = { 0.0f, 2500, 0.0f };
	//endCapsulePos_ = { 0.0f, 90.0f, 0.0f };
	//capsuleRadius_ = 400.0f;

	startCapsulePos_ = { 0.0f,110,0.0f };
	endCapsulePos_ = { 0.0f,30.0f,0.0f };
	capsuleRadius_ = 20.0f;

	isCollision_ = true;
}


void Enemy::InitAnimation(void)
{
	// モデルアニメーション制御の初期化
	animationController_ = new AnimationController(modelId_);

	// アニメーションの追加
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::IDLE), 0.5f, Application::PATH_MODEL + "Player/Idle.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::WALK), 0.5f, Application::PATH_MODEL + "Player/Walk.mv1");

	// 初期アニメーションの再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}

void Enemy::InitPost(void)
{
	isNoticeView_ = false;
	isNoticeHearing_ = false;
}

//void Enemy::DrawViewRange(void)
//{
//	float viewRad = AsoUtility::Deg2RadF(VIEW_ANGLE);
//
//	// 向き角度から方向を取得する
//	MATRIX mat = MGetIdent();
//	mat = MatrixUtility::GetMatrixRotateXYZ(angle_);
//
//	// 前方方向
//	VECTOR forward = VTransform(AsoUtility::DIR_F, mat);
//
//	// 右側方向
//	MATRIX rightMat = MMult(mat, MGetRotY(AsoUtility::Deg2RadF(VIEW_ANGLE)));
//	VECTOR right = VTransform(AsoUtility::DIR_F, rightMat);
//
//	// 左側方向
//	MATRIX leftMat = MMult(mat, MGetRotY(AsoUtility::Deg2RadF(-VIEW_ANGLE)));
//	VECTOR left = VTransform(AsoUtility::DIR_F, leftMat);
//
//	// 自分の位置
//	VECTOR pos0 = pos_;
//
//	// 正面の位置
//	VECTOR pos1 = VAdd(pos0, VScale(forward, VIEW_RANGE));
//
//	// 正面から反時計回り
//	VECTOR pos2 = VAdd(pos0, VScale(left, VIEW_RANGE));
//
//	// 正面から時計回り
//	VECTOR pos3 = VAdd(pos0, VScale(right, VIEW_RANGE));
//
//	// 視野
//	pos0.y = pos1.y = pos2.y = pos3.y = 10.0f;
//	DrawTriangle3D(pos0, pos2, pos1, 0xdd77dd, true);
//	DrawTriangle3D(pos0, pos1, pos3, 0xdd77dd, true);
//	DrawLine3D(pos0, pos1, 0x000000);
//	DrawLine3D(pos0, pos2, 0x000000);
//	DrawLine3D(pos0, pos3, 0x000000);
//
//	// 聴覚
//	DrawCone3D(
//		VAdd(pos_, { 0.0f, 0.0f, 0.0f }),
//		VAdd(pos_, { 0.0f, 1.0f, 0.0f }),
//		HEARING_RANGE, 10, 0xffff7f, 0xffff7f, true);
//}
//void Enemy::Search(void)
//{
//	// 検知フラグリセット
//	isNoticeView_ = false;
//	isNoticeHearing_ = false;
//
//	// プレイヤーの座標を取得
//	VECTOR pPos = player_->GetPos();
//
//	// エネミーからプレイヤーまでのベクトル
//	VECTOR diff = VSub(pPos, pos_);
//
//	// 視野範囲に入っているか判断(ピタゴラスの定理)
//	float distance = std::pow(diff.x, 2.0f) + std::pow(diff.z, 2.0f);
//	if (distance <= (std::pow(VIEW_RANGE, 2.0f)))
//	{
//		// エネミーから見たプレイヤーの角度を求める
//		float dot = VDot(VNorm(moveDir_), VNorm(diff));
//		float angle = acosf(dot);
//
//		// 視野角をラジアンに変換
//		const float viewRad = AsoUtility::Deg2RadF(VIEW_ANGLE);
//
//		// 視野各内に入っているか判断
//		if (angle <= viewRad)
//		{
//			isNoticeView_ = true;
//		}
//	}
//
//	// 聴覚
//	if (distance <= (std::pow(HEARING_RANGE, 2.0f)))
//	{
//		isNoticeHearing_ = true;
//	}
//}

void Enemy::ApplyDamage(int damage) {
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
	// ボス用の追加処理（例：エフェクト、SE、ひるみゲージ加算など）
}

void Enemy::AddStun(int value) {
	stunGauge_ += value;
	if (stunGauge_ > maxStunGauge_) stunGauge_ = maxStunGauge_;
	// ボス用の追加処理
}

void Enemy::OnStunned() {
	// ボスがひるんだ時の処理
	// 例：ステートをSTUNに変更、エフェクト再生など
}

bool Enemy::IsDead() const {
	return hp_ <= 0;
}


void Enemy::ChangeState(ActorState state)
{
	state_ = state;
	attackRegistered_ = false;
	switch (state_) {
	case ActorState::IDLE:
		stateTimer_ = 3.0f; // 待機時間例
		break;
	case ActorState::ATTACK_NEAR:
		// 近接攻撃準備
		break;
	case ActorState::ATTACK_RANGE:

		break;
	case ActorState::ULTIMATE:
		// 必殺技準備
		break;
	case ActorState::STUN:
		stateTimer_ = 2.0f; // ひるみ時間例
		break;
	}
}

ActorBase::ActorState Enemy::GetState() const
{
	return ActorState();
}




