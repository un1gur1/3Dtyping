#pragma once

class Loading
{

private:
	// 最低でもロード画面を表示する時間
	static constexpr int MIN_LOAD_TIME = 60;	// 60fps(1秒) * x

public:
	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	Loading();
	~Loading();

	void Init(void);		// 初期化
	void Load(void);		// 読み込み
	void Update(void);		// 更新
	void Draw(void);		// 描画
	void Release(void);		// 解放

	void StartAsyncLoad(void);	// 非同期ロードの開始
	void EndAsyncLoad(void);	// 非同期ロードの終了

	// ロード中かを返す。
	bool IsLoading(void) { return isLoading_; }

private:

	// 画像ハンドル
	int handle_;

	// 座標
	float posX_;	// X座標
	float posY_;	// Y座標

	// ロード中の判定用
	bool isLoading_;

	// 最低でもロード画面を表示する時間の範囲
	int loadTimer_;
};
