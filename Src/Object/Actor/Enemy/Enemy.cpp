#include "Enemy.h"

#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../Common/AnimationController.h"

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

	//ActorBase::Draw();

	// 視野描画
	//DrawViewRange();
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

	// 角度から方向に変換する
	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };

	// 行列の合成(子, 親と指定すると親⇒子の順に適用される)
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);

	// 回転行列をモデルに反映
	MV1SetRotationMatrix(modelId_, mat);

	// モデルの位置設定
	pos_ = { 0.0f, -400.0f, 600.0f };
	scale_ = { 10,6,6 };
	MV1SetPosition(modelId_, pos_);
	MV1SetScale(modelId_, scale_);
	// 当たり判定を作成
	startCapsulePos_ = { 0.0f,110,0.0f };
	endCapsulePos_ = { 0.0f,30.0f,0.0f };
	capsuleRadius_ = 20.0f;

	// 当たり判定を取るか
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

void Enemy::DrawViewRange(void)
{
	float viewRad = AsoUtility::Deg2RadF(VIEW_ANGLE);

	// 向き角度から方向を取得する
	MATRIX mat = MGetIdent();
	mat = MatrixUtility::GetMatrixRotateXYZ(angle_);

	// 前方方向
	VECTOR forward = VTransform(AsoUtility::DIR_F, mat);

	// 右側方向
	MATRIX rightMat = MMult(mat, MGetRotY(AsoUtility::Deg2RadF(VIEW_ANGLE)));
	VECTOR right = VTransform(AsoUtility::DIR_F, rightMat);

	// 左側方向
	MATRIX leftMat = MMult(mat, MGetRotY(AsoUtility::Deg2RadF(-VIEW_ANGLE)));
	VECTOR left = VTransform(AsoUtility::DIR_F, leftMat);

	// 自分の位置
	VECTOR pos0 = pos_;

	// 正面の位置
	VECTOR pos1 = VAdd(pos0, VScale(forward, VIEW_RANGE));

	// 正面から反時計回り
	VECTOR pos2 = VAdd(pos0, VScale(left, VIEW_RANGE));

	// 正面から時計回り
	VECTOR pos3 = VAdd(pos0, VScale(right, VIEW_RANGE));

	// 視野
	pos0.y = pos1.y = pos2.y = pos3.y = 10.0f;
	DrawTriangle3D(pos0, pos2, pos1, 0xdd77dd, true);
	DrawTriangle3D(pos0, pos1, pos3, 0xdd77dd, true);
	DrawLine3D(pos0, pos1, 0x000000);
	DrawLine3D(pos0, pos2, 0x000000);
	DrawLine3D(pos0, pos3, 0x000000);

	// 聴覚
	DrawCone3D(
		VAdd(pos_, { 0.0f, 0.0f, 0.0f }),
		VAdd(pos_, { 0.0f, 1.0f, 0.0f }),
		HEARING_RANGE, 10, 0xffff7f, 0xffff7f, true);
}

void Enemy::Search(void)
{
	// 検知フラグリセット
	isNoticeView_ = false;
	isNoticeHearing_ = false;

	// プレイヤーの座標を取得
	VECTOR pPos = player_->GetPos();

	// エネミーからプレイヤーまでのベクトル
	VECTOR diff = VSub(pPos, pos_);

	// 視野範囲に入っているか判断(ピタゴラスの定理)
	float distance = std::pow(diff.x, 2.0f) + std::pow(diff.z, 2.0f);
	if (distance <= (std::pow(VIEW_RANGE, 2.0f)))
	{
		// エネミーから見たプレイヤーの角度を求める
		float dot = VDot(VNorm(moveDir_), VNorm(diff));
		float angle = acosf(dot);

		// 視野角をラジアンに変換
		const float viewRad = AsoUtility::Deg2RadF(VIEW_ANGLE);

		// 視野各内に入っているか判断
		if (angle <= viewRad)
		{
			isNoticeView_ = true;
		}
	}

	// 聴覚
	if (distance <= (std::pow(HEARING_RANGE, 2.0f)))
	{
		isNoticeHearing_ = true;
	}
}
