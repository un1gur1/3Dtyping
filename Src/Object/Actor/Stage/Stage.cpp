#include "Stage.h"
#include <DxLib.h>
#include "../../../Application.h"

Stage::Stage(void)
{
	modelId_ = -1;
	pos_ = { 0.0f, 0.0f, 0.0f };
}

Stage::~Stage(void)
{
}

void Stage::Init(void)
{
	// モデルの位置
	pos_ = { 0.0f, 80.0f, 0.0f };

	// モデルの位置を設定
	MV1SetPosition(modelId_, pos_);

	// マテリアルの数を取得
	int num = MV1GetMaterialNum(modelId_);
	for (int i = 1; i < num; i++)
	{
		// 0は地面なので、1から設定する
		MV1SetMaterialEmiColor(modelId_, i, GetColorF(0.2f, 0.2f, 0.2f, 1.0f));
	}

	// 衝突判定情報の構築
	MV1SetupCollInfo(modelId_, -1);
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


}

void Stage::Release(void)
{
	MV1DeleteModel(modelId_);
}

int Stage::GetModelId(void)
{
	return modelId_;
}
