#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "../../Utility/MatrixUtility.h"
#include "../Common/AnimationController.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
{ 
	animationController_ = nullptr;

	animType_ = 0;

	modelId_ = -1;
	pos_ = { 0.0f,0.0f,0.0f };
	angle_ = { 0.0f,0.0f,0.0f };
	localAngle_ = { 0.0f,0.0f,0.0f };
	scale_ = { 0.0f,0.0f,0.0f };

	startCapsulePos_ = { 0.0f,0.0f,0.0f };
	endCapsulePos_ = { 0.0f,0.0f,0.0f };
	capsuleRadius_ = 0.0f;

	preInputDir_ = { 0.0f,0.0f,0.0f };

	moveDir_ = { 0.0f,0.0f,0.0f };
	jumpPow_ = 0.0f;
	isCollision_ = false;
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::Init(void)
{
	// Transform初期化
	InitTransform();

	// アニメーションの初期化
	InitAnimation();

	// 初期化後の個別処理
	InitPost();
}

void ActorBase::Load(void)
{
	// リソースロード
	InitLoad();
}

void ActorBase::LoadEnd(void)
{
	// 初期化
	Init();
}

void ActorBase::Update(void)
{

	

	// プレイヤーの遅延回転処理
	DelayRotate();

	// 行列の合成(子, 親と指定すると親⇒子の順に適用される)
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);

	// 回転行列をモデルに反映
	MV1SetRotationMatrix(modelId_, mat);


	// 重力(加速度を速度に加算していく)
	//jumpPow_ -= 0.8f;
	
	// プレイヤーの座標に移動量(速度、ジャンプ力)を加算する
	pos_.y += jumpPow_;

	// 描画座標をロジック座標にLerpで近づける
	const float LERP_SPEED = 0.2f;
	drawPos_.x += (logicPos_.x - drawPos_.x) * LERP_SPEED;
	drawPos_.y += (logicPos_.y - drawPos_.y) * LERP_SPEED;
	drawPos_.z += (logicPos_.z - drawPos_.z) * LERP_SPEED;


	// プレイヤーの移動処理
	Move();

	// モデルに座標を設定する
	MV1SetPosition(modelId_, drawPos_);

	// アニメーションの更新
	animationController_->Update();
}

void ActorBase::Draw(void)
{
	MV1DrawModel(modelId_);

	//DrawSphere3D(
	//	VAdd(pos_,startCapsulePos_),
	//	capsuleRadius_,
	//	16,
	//	0x00ff00,
	//	0x00ff00,
	//	false
	//);

	//DrawSphere3D(
	//	VAdd(pos_, endCapsulePos_),
	//	capsuleRadius_,
	//	16,
	//	0x00ff00,
	//	0x00ff00,
	//	false
	//);
}

void ActorBase::Release(void)
{
	MV1DeleteModel(modelId_);
	delete animationController_;
}

void ActorBase::CollisionStage(const VECTOR& pos)
{
	// 衝突判定に指定座標に押し戻す
	pos_ = pos;
	jumpPow_ = 0.0f;
}

void ActorBase::MoveToGrid(const GridPos& target)
{
	//// 境界チェック (GRID_WIDTH, GRID_HEIGHTは定数または設定値として定義されている前提)
	//// プレイヤーグリッドの範囲は 0 から GRID_SIZE - 1 まで
	//if (target.x < -GRID_WIDTH || target.x >= GRID_WIDTH ||
	//	target.z < -GRID_HEIGHT || target.z >= GRID_HEIGHT)
	//{
	//	// 範囲外であれば移動をキャンセルし、フラグを立てない
	//	return;
	//}

	targetGridPos_ = target;
	isMovingOnGrid_ = true;
}

void ActorBase::Move(void)
{
}

void ActorBase::DelayRotate(void)
{
	// 移動方向から角度に変換する
	float goal = atan2f(moveDir_.x, moveDir_.z);

	// 常に最短経路で補間
	angle_.y = AsoUtility::LerpAngle(angle_.y, goal, 0.2f);
}
