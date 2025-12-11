#pragma once
#include <string>
#include <DxLib.h>
class AnimationController;

class ActorBase
{
public:

	ActorBase(void);			// コンストラクタ
	virtual ~ActorBase(void);	// デストラクタ

	void Init(void);			// 初期化
	void Load(void);			// 読み込み
	void LoadEnd(void);			// 読み込み後
	virtual void Update(void);	// 更新
	virtual void Draw(void);	// 描画
	virtual void Release(void);	// 解放

	// 座標を取得
	const VECTOR& GetPos(void) const { return pos_; }

	// カプセルの当たり判定を取得
	const VECTOR& GetStartCapsulePos(void) const { return startCapsulePos_; }
	const VECTOR& GetEndCapsulePos(void) const { return endCapsulePos_; }
	const float& GetCapsuleRadius(void) const { return capsuleRadius_; }

	const bool& GetisCollision(void) const { return isCollision_; }

	// ステージと衝突
	void CollisionStage(const VECTOR& pos);

	struct GridPos
	{
		int x;
		int z;
	};
	static constexpr int GRID_WIDTH = 100;
	static constexpr int GRID_HEIGHT = 100;
	VECTOR logicPos_;   // ロジック（ゲーム進行用）座標
	VECTOR drawPos_;    // 描画（表示用）座標
	GridPos gridPos_;      // 現在のグリッド座標
	GridPos targetGridPos_;// 目標グリッド座標
	bool isMovingOnGrid_ = false; // グリッド移動中フラグ
	float gridSize_ = 200.0f;     // 1マスの大きさ

	void MoveToGrid(const GridPos& target);
protected:

	// アニメーション制御
	AnimationController* animationController_;

	// アニメーション種別
	int animType_;

	// モデル情報
	int modelId_;
	VECTOR pos_;
	VECTOR angle_;
	VECTOR localAngle_;
	VECTOR scale_;

	// カプセルの当たり判定座標
	VECTOR startCapsulePos_;
	VECTOR endCapsulePos_;
	float capsuleRadius_;

	// 当たり判定を取るか
	bool isCollision_;

	// パッド入力値の補間
	VECTOR preInputDir_;

	// 移動方向
	VECTOR moveDir_;

	// ジャンプ力
	float jumpPow_;

	// リソースロード
	virtual void InitLoad(void) = 0;

	// 大きさ、回転、座標の初期化
	virtual void InitTransform(void) = 0;

	// アニメーションの初期化
	virtual void InitAnimation(void) = 0;

	// 初期化後の個別処理
	virtual void InitPost(void) = 0;

	// 移動処理
	virtual void Move(void);

	// 遅延回転処理
	void DelayRotate(void);

};
