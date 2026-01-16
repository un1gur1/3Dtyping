#include "GameScene.h"

#include <DxLib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "../../Application.h"
#include "../../Camera/Camera.h"
#include "../../Common/Grid.h"

#include "../../Object/Actor/ActorBase.h"
#include "../../Object/Actor/Player/Player.h"
#include "../../Object/Actor/Enemy/Enemy.h"

#include "../../Object/Actor/Stage/Stage.h"

#include"../../Common/UiManager.h"
#include"../SceneManager.h"
#include"../ResultScene/ResultScene.h"
#include"../../Input/InputManager.h"

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
	UIManager::GetInstance().InitGrids();

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

	skyDomeModelId_ = MV1LoadModel("Data/Model/Stage/Skydome.mv1");
	if (skyDomeModelId_ != -1) {
    int materialNum = MV1GetMaterialNum(skyDomeModelId_);
    for (int i = 0; i < materialNum; ++i) {
        COLOR_F color = MV1GetMaterialDifColor(skyDomeModelId_, i);
        // 0.5倍で暗くする（値はお好みで調整）
        color.r *= 0.5f;
        color.g *= 0.5f;
        color.b *= 0.5f;
        MV1SetMaterialDifColor(skyDomeModelId_, i, color);
    }
}
	// 生成処理
	camera_ = new Camera();					// カメラの生成
	stage_ = new Stage(100,100);					// ステージの生成
	Grid* grid_ = new Grid();						// グリッドの生成
	Player* player_ = new Player(camera_);	// プレイヤーの生成
	Enemy* enemy_ = new Enemy(player_);		// 敵の生成

	// アクター配列に入れる
	allActor_.push_back(player_);
	allActor_.push_back(enemy_);

	// カメラモード変更
	camera_->SetFollow(player_);
	camera_->ChangeMode(Camera::MODE::FOLLOW);


	player_->SetAttackManager(attackManager_);
	enemy_->SetAttackManager(attackManager_);

	// ステージの読み込み
	stage_->Load();

	// 全てのアクターを読み込み
	for (auto actor : allActor_)
	{
		// 読み込み
		actor->Load();
	}
	player_->SetEnemyList(&allActor_);

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

	if (attackManager_) {
		std::vector<std::string> ultimateCmds = attackManager_->GetUltimateCommandNames();
		UIManager::GetInstance().SetUltimateCommandList(ultimateCmds);
	}
	Player* player = nullptr;
	for (auto actor : allActor_) {
		if (actor && actor->IsPlayer()) player = static_cast<Player*>(actor);
	}
	if (player) {
		std::vector<std::string> normalCmds = player->GetNormalCommandNames();
		UIManager::GetInstance().SetNormalCommandList(normalCmds);
	}
}
void GameScene::Update(void)
{
	// ポーズ切り替え
	if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_TAB)) {
		if (pauseState_ == PauseMenuState::None) {
			pauseState_ = PauseMenuState::Pause;
			pauseCursor_ = 0;
			isPause_ = true;
		}
		else {
			pauseState_ = PauseMenuState::None;
			isPause_ = false;
		}
	}

	// ポーズ中の操作
	if (isPause_) {
		// ポーズメニューの操作のみ許可
		if (pauseState_ == PauseMenuState::Pause) {
			if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_UP)) {
				pauseCursor_ = (pauseCursor_ + 2) % 3;
			}
			if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_DOWN)) {
				pauseCursor_ = (pauseCursor_ + 1) % 3;
			}
			if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_RETURN)) {
				if (pauseCursor_ == 1) {
					// タイトルに戻る
					SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::TITLE);
				}
				else if (pauseCursor_ == 2) {
					// ゲーム終了
					DxLib_End(); // DxLibの終了
					exit(0);     // プロセス終了
				}
			}
			// UIManagerにカーソル位置を渡す
			UIManager::GetInstance().SetPauseCursor(pauseCursor_);
		}
		// UIManagerはポーズ状態で更新
		UIManager::GetInstance().Update(UIManager::UIState::Pause);
		return; // ここでゲームロジックを完全停止
	}

	// --- ここから下は通常時のみ動く ---
	// カメラの更新
	camera_->Update();

	// ステージ更新
	stage_->Update();
	UIManager::GetInstance().Update(UIManager::UIState::Normal);

	// 全てのアクターを回す
	for (auto actor : allActor_)
	{
		actor->Update();
		if (actor)
		{
			WallCollision(actor);
		}
	}
	// 攻撃管理の更新
	if (attackManager_) {
		attackManager_->UpdateAll(allActor_);
	}

	// 以下、HPやシーン遷移などの処理も通常時のみ
	Player* player = nullptr;
	Enemy* enemy = nullptr;
	for (auto actor : allActor_) {
		if (actor && actor->IsPlayer()) player = static_cast<Player*>(actor);
		if (actor && actor->IsEnemy()) enemy = static_cast<Enemy*>(actor);
	}
	if (player) {
		UIManager::GetInstance().SetPlayerStatus(player->GetHp(), player->GetMaxHp());
	}
	if (enemy) {
		UIManager::GetInstance().SetEnemyStatus(enemy->GetHp(), enemy->GetMaxHp(), "BOSS");
	}

	bool isEnemyDead = false;
	for (auto actor : allActor_) {
		if (actor && actor->IsEnemy()) {
			if (actor->GetHp() <= 0) {
				isEnemyDead = true;
				break;
			}
		}
	}
	if (isEnemyDead) {
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::RESULT_WIN);
	}
	else if (player && player->GetHp() <= 0) {
		Application::GetInstance()->ShakeScreen(5, 30, true, true);
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::RESULT_LOSE);
	}
}

void GameScene::Draw(void)
{
	//SetBackgroundColor(180, 180, 180);
	//SetBackgroundColor(255, 255, 255);


	// ドーム型背景モデルの描画（カメラより前に描画）
	if (skyDomeModelId_ != -1) {
		MV1SetPosition(skyDomeModelId_, VGet(0.0f, 0.0f, 0.0f)); // 原点に設置
		MV1SetScale(skyDomeModelId_, VGet(30.0f, 30.0f, 30.0f)); // 必要に応じて拡大
		MV1DrawModel(skyDomeModelId_);
	}
	// カメラの描画更新
	camera_->SetBeforeDraw();
	camera_->DrawDebug();
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

	// UI描画
	if (pauseState_ == PauseMenuState::Pause) {
		UIManager::GetInstance().Draw(UIManager::UIState::Pause);
	}
	else {
		UIManager::GetInstance().Draw(UIManager::UIState::Normal);
	}

}

void GameScene::Release(void)
{


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
	if (skyDomeModelId_ != -1) {
		MV1DeleteModel(skyDomeModelId_);
		skyDomeModelId_ = -1;
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
