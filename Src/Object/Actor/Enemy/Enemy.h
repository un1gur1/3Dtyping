#pragma once
#include "../ActorBase.h"

class Player;
class AttackManager;
class UlitimateAttack;	

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
		SHOT,
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

	void OnAttackCancelled();

	void ChangeState(ActorState state)override;

	ActorState GetState() const override;

	void SetAttackManager(AttackManager* manager) { attackManager_ = manager; }

	std::string typingCommand_; // 今タイピング中のコマンド
	float typingElapsed_ = 0.0f;
	float typingWait_ = 0.0f;
	void StartTypingUltimate(const std::string& command);
	void UpdateTypingUltimate(float deltaTime);
private:

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;

	//// 視野描画
	//void DrawViewRange(void);

	// オーバーライド
	void ApplyDamage(int damage) override;
	void AddStun(int value) override;
	void OnStunned() override;
	bool IsDead() const override;
	bool IsPlayer() const override { return false; }
	bool IsEnemy() const override { return true; }



private:

	ActorState state_ = ActorState::IDLE;
	float stateTimer_ = 0.0f; // クールダウンや詠唱、ひるみ用
	Player* player_;
	// 攻撃中の管理用
	bool attackRegistered_ = false;
	AttackManager* attackManager_ = nullptr;
	UlitimateAttack* ultimateAttack_ = nullptr;
	
	// 検知フラグ(視野)
	bool isNoticeView_;

	// 検知フラグ(聴覚)
	bool isNoticeHearing_;

	bool isAttacking_ = false;	
	float moveTime_ = 0.0f; // 移動用タイマー
	float moveCenterX_ = 0.0f; // 中心座標

	//// 索敵
	//void Search(void);
};
