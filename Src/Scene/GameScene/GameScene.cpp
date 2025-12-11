#include "GameScene.h"

#include <DxLib.h>

#include "../../Application.h"
#include "../../Camera/Camera.h"
#include "../../Common/Grid.h"

#include "../../Object/Actor/ActorBase.h"
#include "../../Object/Actor/Player/Player.h"
#include "../../Object/Actor/Enemy/Enemy.h"

#include "../../Object/Actor/Stage/Stage.h"

GameScene::GameScene(void)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{

	// カメラの初期化
	camera_->Init();

	// ステージ初期化
	stage_->Init();

	grid_->Init();


	// 全てのアクターを初期化
	for (auto actor : allActor_)
	{
		// 初期化
		actor->Init();
	}
}

void GameScene::Load(void)
{
	attackManager_ = new AttackManager();			// 攻撃管理の生成


	// 生成処理
	camera_ = new Camera();					// カメラの生成
	stage_ = new Stage();					// ステージの生成
	grid_ = new Grid();						// グリッドの生成

	Player* player_ = new Player(camera_);	// プレイヤーの生成
	Enemy* enemy_ = new Enemy(player_);		// 敵の生成

	// アクター配列に入れる
	allActor_.push_back(player_);
	allActor_.push_back(enemy_);

	// カメラモード変更
	camera_->SetFollow(player_);
	camera_->ChangeMode(Camera::MODE::FOLLOW);

	player_->SetAttackManager(attackManager_);

	// ステージの読み込み
	stage_->Load();

	// 全てのアクターを読み込み
	for (auto actor : allActor_)
	{
		// 読み込み
		actor->Load();
	}


}

void GameScene::LoadEnd(void)
{
	// カメラの初期化
	camera_->Init();

	// ステージ初期化
	stage_->LoadEnd();

	// 全てのアクターを読み込み後
	for (auto actor : allActor_)
	{
		// 読み込み
		actor->LoadEnd();
	}
}

void GameScene::Update(void)
{
	// カメラの更新
	camera_->Update();

	// ステージ更新
	stage_->Update();

	// 全てのアクターを回す
	for (auto actor : allActor_)
	{
		// 更新処理
		actor->Update();

		// 当たり判定を取るか？
		if (actor)
		{
			// 当たり判定
			//FieldCollision(actor);
			WallCollision(actor);
		}
	}
	// 攻撃管理の更新
	if (attackManager_) {
		attackManager_->UpdateAll();
	}
}

void GameScene::Draw(void)
{

	// グリッド描画
	grid_->Draw();

	// カメラの描画更新
	camera_->SetBeforeDraw();

	// ステージ描画
	stage_->Draw();


	// 全てのアクターを回す
	for (auto actor : allActor_)
	{
		// 更新処理
		actor->Draw();
	}

	// 攻撃管理の描画
	if (attackManager_) {
		attackManager_->DrawAll();
	}
}

void GameScene::Release(void)
{

	grid_->Release();
	delete grid_;

	// ステージ解放
	stage_->Release();
	delete stage_;
	
	delete attackManager_;
	attackManager_ = nullptr;
	// 全てのアクターを回す
	for (auto actor : allActor_)
	{
		// 更新処理
		actor->Release();
		delete actor;
	}

	// 配列をクリア
	allActor_.clear();

	delete camera_;
	camera_ = nullptr;

}

// ステージの床とプレイヤーの衝突
void GameScene::FieldCollision(ActorBase* actor)
{
	// 座標を所得
	VECTOR actorPos = actor->GetPos();

	// 線分の上座標
	VECTOR startPos = actorPos;
	startPos.y = actorPos.y + 10.0f;

	// 線分の下座標
	VECTOR endPos = actorPos;
	endPos.y = actorPos.y - 10.0f;

	// ステージのモデルを取得
	int modelId = stage_->GetModelId();

	// 線分とモデルの衝突判定
	MV1_COLL_RESULT_POLY res =
		MV1CollCheck_Line(modelId, -1, startPos, endPos);

	// ステージに当たっているか？
	if (res.HitFlag)
	{
		// 当たった場所に戻す
		actor->CollisionStage(res.HitPosition);
	}
}

void GameScene::WallCollision(ActorBase* actor)
{
	// 座標を取得
	VECTOR pos = actor->GetPos();

	// カプセルの座標
	VECTOR capStartPos = VAdd(pos, actor->GetStartCapsulePos());
	VECTOR capEndPos = VAdd(pos, actor->GetEndCapsulePos());

	// カプセルとの当たり判定
	auto hits = MV1CollCheck_Capsule
	(
		stage_->GetModelId(),			// ステージのモデルID
		-1,								// ステージ全てのポリゴンを指定
		capStartPos,					// カプセルの上
		capEndPos,						// カプセルの下
		actor->GetCapsuleRadius()		// カプセルの半径
	);

	// 衝突したポリゴン全ての検索
	for (int i = 0; i < hits.HitNum; i++)
	{
		// ポリゴンを1枚に分割
		auto hit = hits.Dim[i];

		// ポリゴン検索を制限(全てを検索すると重いので)
		for (int tryCnt = 0; tryCnt < 10; tryCnt++)
		{
			// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
			int pHit = HitCheck_Capsule_Triangle
			(
				capStartPos,					// カプセルの上
				capEndPos,						// カプセルの下
				actor->GetCapsuleRadius(),		// カプセルの半径
				hit.Position[0],				// ポリゴン1
				hit.Position[1],				// ポリゴン2
				hit.Position[2]					// ポリゴン3
			);

			// カプセルとポリゴンが当たっていた
			if (pHit)
			{
				// 当たっていたので座標をポリゴンの法線方向に移動させる
				pos = VAdd(pos, VScale(hit.Normal, 1.0f));

				// 球体の座標も移動させる
				capStartPos = VAdd(capStartPos, VScale(hit.Normal, 1.0f));
				capEndPos = VAdd(capEndPos, VScale(hit.Normal, 1.0f));

				// 複数当たっている可能性があるので再検索
				continue;
			}
		}
	}
	// 検出したポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

	// 計算した場所にアクターを戻す
	actor->CollisionStage(pos);
}
