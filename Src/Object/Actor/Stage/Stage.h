#pragma once
#include <DxLib.h>

class Stage
{

public:

	static constexpr int MAGIC_CIRCLE_WIDTH = 1500;
	static constexpr int MAGIC_CIRCLE_HEIGHT = 1500;


	Stage(float x ,float y);
	~Stage(void);

	void Init(void);
	void Load(void);
	void LoadEnd(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// ステージモデルのハンドルID
	int GetModelId(void);

private:

	// メンバ変数
	VECTOR pos_;         // 中心座標
	float angle_;        // 回転角度
	float scale_;        // 基本スケール
	int imageHandle_;    // 魔法陣画像ハンドル

};

