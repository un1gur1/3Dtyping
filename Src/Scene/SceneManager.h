#pragma once

class SceneBase;
class Loading;

class SceneManager
{

public:

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME
	};

public:
	// シングルトン（生成・取得・削除）
	static void CreateInstance(void) { if (instance_ == nullptr) { instance_ = new SceneManager(); } };
	static SceneManager* GetInstance(void) { return instance_; };
	static void DeleteInstance(void) { if (instance_ != nullptr) { delete instance_; instance_ = nullptr; } }

private:

	// 静的インスタンス
	static SceneManager* instance_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);
	// デストラクタも同様
	~SceneManager(void);

	// コピー・ムーブ操作を禁止
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

	// 下記をコンパイルエラーさせるため 上記を追加
	// SceneManager copy = *SceneManager::GetInstance();
	// SceneManager copied(*SceneManager::GetInstance());
	// SceneManager moved = std::move(*SceneManager::GetInstance());

public:

	void Init(void);	// 初期化
	void Init3D(void);	// 3Dの初期化
	void Update(void);	// 更新
	void Draw(void);	// 描画
	void Delete(void);	// リソースの破棄

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void) { return sceneId_; };

	// ゲーム終了
	void GameEnd(void) { isGameEnd_ = true; }

	// ゲーム終了取得
	bool GetGameEnd(void) { return isGameEnd_; }

private:

	// 各種シーン
	SceneBase* scene_;

	// ロード画面
	Loading* load_;

	// シーンID
	SCENE_ID sceneId_;

	// ゲーム終了
	bool isGameEnd_;
};