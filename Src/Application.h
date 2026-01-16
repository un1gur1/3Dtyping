#pragma once
#include <string>

// クラスの前方宣言
class FpsControl;

class Application
{

public:

	// スクリーンサイズ
	static constexpr int SCREEN_SIZE_X = 1980;
	static constexpr int SCREEN_SIZE_Y = 1080;

	// データパス関連
	//-------------------------------------------
	static const std::string PATH_DATA;
	static const std::string PATH_IMAGE;
	static const std::string PATH_MODEL;
	static const std::string PATH_EFFECT;
	//-------------------------------------------

public:
	// シングルトン（生成・取得・削除）
	static void CreateInstance(void) { if (instance_ == nullptr) { instance_ = new Application(); } }
	static Application* GetInstance(void) { return instance_; }
	static void DeleteInstance(void) { if (instance_ != nullptr) delete instance_; instance_ = nullptr; }

private:
	// 静的インスタンス
	static Application* instance_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	Application(void);

	// デストラクタも同様
	~Application(void);

	// コピー・ムーブ操作を禁止
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	// 下記をコンパイルエラーさせるため 上記を追加
	// Application copy = *Application::GetInstance();
	// Application copied(*Application::GetInstance());
	// Application moved = std::move(*Application::GetInstance());

public:

	void Init(void);		// 初期化
	void Run(void);			// ゲームループの開始
	void Delete(void);		// リソースの破棄
	
	bool IsInitFail(void) const;	// 初期化成功／失敗の判定
	bool IsReleaseFail(void) const;	// 解放成功／失敗の判定

	void ShakeScreen(int power, int duration, bool isShakeX = true, bool isShakeY = true);	// 画面を揺らす
	void StopShakeScreen(void);
	int shakePower_ = 0;	// 画面揺らしの強さ
	int shakeDuration_ = 0;	// 画面揺らしの持続時間
	int shakeOffsetX_ = 0;	// 画面揺らしのXオフセット
	int shakeOffsetY_ = 0;	// 画面揺らしのYオフセット
	bool isShakeX_;			// X方向の揺らしを有効にするか
	bool isShakeY_;			// Y方向の揺らしを有効にするか

private:

	// 初期化失敗
	bool isInitFail_;

	// 解放失敗
	bool isReleaseFail_;

	// FPS
	FpsControl* fps_;
};
