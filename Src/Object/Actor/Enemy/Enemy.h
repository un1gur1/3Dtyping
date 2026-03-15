#pragma once
#include "../ActorBase.h"
#include <vector>
#include <string>

class Player;
class AttackManager;
class UlitimateAttack;

class Enemy : public ActorBase
{
public:
	// --- 定数 (UPPER_SNAKE_CASE) ---
	static constexpr float VIEW_RANGE = 600.0f;
	static constexpr float VIEW_ANGLE = 60.0f;
	static constexpr float HEARING_RANGE = 400.0f;

	// --- アニメーション ---
	enum class ANIM_TYPE {
		IDLE,
		WALK,
		SHOT,
		MAX,
	};

	// --- コンボ・アクション定義 ---
	enum class EnemyAction {
		NONE,
		ATTACK_NEAR,
		ATTACK_RANGE,
		THUNDER,
		ULTIMATE
	};

	// --- 攻撃の発射予定データ ---
	struct PlannedAttack {
		enum class Kind { RANGED, THUNDER, ULTIMATE } kind;
		VECTOR pos;
		VECTOR velocity;
		int damage = 0;
		std::vector<VECTOR> thunderPositions;
	};

public:
	Enemy(Player* player);
	~Enemy(void) override;

	void Update(void) override;
	void Draw(void) override; // ※親クラスの仮想関数に依存するため non-const

	void ChangeState(ActorState state) override;
	ActorState GetState() const override;

	void SetAttackManager(AttackManager* manager) { attackManager_ = manager; }
	void OnAttackCancelled();

private:
	// --- ActorBase オーバーライド ---
	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;
	void ApplyDamage(const int damage) override;
	void AddStun(const int value) override;
	void OnStunned() override;
	bool IsDead() const override;
	bool IsPlayer() const override { return false; }
	bool IsEnemy() const override { return true; }

	// --- 詠唱（タイピング）システム ---
	void StartTypingUltimate(const std::string& command);
	void UpdateTypingUltimate(const float deltaTime);
	void PreparePlannedAttackData();

	// --- コンボ管理システム ---
	void StartNextComboStep();
	bool IsMyAttackAlive() const;

public:
	// ※UIManager連携用メンバ変数 (lowerCamelCase_)
	std::string typingCommand_;
	float typingElapsed_ = 0.0f;
	float typingWait_ = 0.0f;

	std::vector<PlannedAttack> plannedAttacks_;

private:
	// --- 基本ステータス・参照 ---
	ActorState state_ = ActorState::IDLE;
	float stateTimer_ = 0.0f;
	Player* player_ = nullptr;
	AttackManager* attackManager_ = nullptr;

	bool isAttacking_ = false;
	float moveTime_ = 0.0f;
	float moveCenterX_ = 0.0f;

	// --- コンボ・クールダウン制御 ---
	std::vector<std::vector<EnemyAction>> comboSets_;
	int activeComboIndex_ = -1;
	int comboStep_ = 0;

	bool waitingForAttackFinish_ = false;
	float attackCooldown_ = 0.5f;
	float attackCooldownTimer_ = 0.0f;

	// --- HP閾値定数 ---
	static constexpr float HP_HIGH = 0.7f;
	static constexpr float HP_LOW = 0.3f;
};