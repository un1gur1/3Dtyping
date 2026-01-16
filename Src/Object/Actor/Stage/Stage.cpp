#include "Stage.h"
#include <DxLib.h>
#include "../../../Application.h"

Stage::Stage(float x, float y)
{
	pos_ = { x, y, 0.0f };
	angle_ = 0.0f;
	scale_ = 0.;
	imageHandle_ = LoadGraph("Data/Model/Stage/magic_circle.png");

}

Stage::~Stage(void)
{
}

void Stage::Init(void)
{
	//// モデルの位置
	//pos_ = { 0.0f, 80.0f, 0.0f };

	//// モデルの位置を設定
	//MV1SetPosition(modelId_, pos_);

	//// マテリアルの数を取得
	//int num = MV1GetMaterialNum(modelId_);
	//for (int i = 1; i < num; i++)
	//{
	//	// 0は地面なので、1から設定する
	//	MV1SetMaterialEmiColor(modelId_, i, GetColorF(0.2f, 0.2f, 0.2f, 1.0f));
	//}

	//// 衝突判定情報の構築
	//MV1SetupCollInfo(modelId_, -1);
}

void Stage::Load(void)
{
	// ステージモデルの読み込み
	//modelId_ = MV1LoadModel((Application::PATH_MODEL + "Stage/Stage.mv1").c_str());
}

void Stage::LoadEnd(void)
{
	Init();
}

void Stage::Update(void)
{
}

void Stage::Draw(void)
{
	// ステージモデル描画
	//MV1DrawModel(modelId_);
	DrawRotaGraph3(
		static_cast<int>(pos_.x), static_cast<int>(pos_.y), // 描画位置
		MAGIC_CIRCLE_WIDTH / 2, MAGIC_CIRCLE_HEIGHT / 2,    // 回転の中心（画像の真ん中）
		scale_,
		scale_ * 0.25,
		angle_,
		imageHandle_,
		true,
		false
	);

}

void Stage::Release(void)
{
}

int Stage::GetModelId(void)
{
	return imageHandle_;
}
