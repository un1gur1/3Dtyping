#pragma once
#include <DxLib.h>

class Stage
{

public:

	Stage(void);
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

	// ステージモデルのハンドルID
	int modelId_;

	// ステージモデルの位置
	VECTOR pos_;

};

