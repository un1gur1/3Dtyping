#pragma once
#include "../SceneBase.h"	
#include <string>
class ResultScene : public SceneBase
{
public:

	ResultScene(void);				// コンストラクタ
	~ResultScene(void) override;		// デストラクタ

	void Init(void)		override;	// 初期化
	void Load(void)		override;	// 読み込み
	void LoadEnd(void)	override;	// 読み込み後の処理
	void Update(void)	override;	// 更新
	void Draw(void)		override;	// 描画
	void Release(void)	override;	// 解放



private:
	int handle_;
};
