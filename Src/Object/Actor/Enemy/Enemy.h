#pragma once
#include "../ActorBase.h"

class Player;

class Enemy : public ActorBase
{

public:

	// 視野の広さ
	static constexpr float VIEW_RANGE = 600.0f;

	// 視野角
	static constexpr float VIEW_ANGLE = 60.0f;

	// 聴覚の広さ
	static constexpr float HEARING_RANGE = 400.0f;

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		MAX,
	};

public:
	// コンストラクタ
	Enemy(Player* player);

	// デストラクタ
	~Enemy(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

private:

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;

	// 視野描画
	void DrawViewRange(void);

private:

	// プレイヤー
	Player* player_;
	
	// 検知フラグ(視野)
	bool isNoticeView_;

	// 検知フラグ(聴覚)
	bool isNoticeHearing_;

	// 索敵
	void Search(void);
};
