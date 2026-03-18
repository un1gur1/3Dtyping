#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "../ActorBase.h"

class Camera;
class AttackManager;

class Player : public ActorBase
{
public:
	// アニメーション種別
	enum class ANIM_TYPE {
		IDLE, WALK, WALK_LOOP, ATTACK2, ATTACK3, BOXING,
		FALLING, FLYING, JUMP, JUMP_ATTACK, JUMP_HOVER,
		JUMPING, JUMP_RISING, SHOT, THROW, VICTORY, WARP_POSE, MAX,
	};

	// コマンド種別
	enum class CommandType {
		MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
		MOVE_UP_RIGHT, MOVE_UP_LEFT, MOVE_DOWN_RIGHT, MOVE_DOWN_LEFT,
		DODGE, MOVE_RANDOM, SHOOT, ATTACK, UNKNOWN
	};

	Player(Camera* camera);
	~Player(void) override;

	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// オーバーライド（ステータス関連）
	void ApplyDamage(int damage) override;
	void AddStun(int value) override;
	void OnStunned() override;
	bool IsDead() const override;
	void ChangeState(ActorState state) override;
	ActorState GetState() const override;
	bool IsPlayer() const override { return true; }
	bool IsEnemy() const override { return false; }

	// セッター・ゲッター
	void SetAttackManager(AttackManager* manager) { attackManager_ = manager; }
	void SetEnemyList(std::vector<ActorBase*>* list) { enemyList_ = list; }
	std::vector<ActorBase*>* GetEnemyList() const { return enemyList_; }
	std::vector<std::string> GetNormalCommandNames() const;

private:
	// 内部処理
	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;
	void Move(void) override;

	void LoadMoveWordDict(const std::string& path);
	CommandType StringToCommandType(const std::string& str);

	// ★完全統一：すべての魔法・攻撃を生成するファクトリー関数
	void ExecuteMagic(const std::string& magicType, int damage, float speed);

private:
	// --- 依存オブジェクト ---
	Camera* camera_ = nullptr;
	AttackManager* attackManager_ = nullptr;
	std::vector<ActorBase*>* enemyList_ = nullptr;

	// --- ステータス・状態 ---
	ActorState state_ = ActorState::IDLE;
	bool isAttacking_ = false;

	// --- 通常コマンド入力 ---
	int keyInputHandle_ = -1;
	bool isInputActive_ = false;
	char inputBuf_[128] = { 0 };
	std::string inputText_;

	// ★コマンド辞書（種類と属性を分けて記憶）
	std::unordered_map<std::string, CommandType> commandMap_;
	std::unordered_map<std::string, std::string> magicTypeMap_;

	// --- 必殺技登録入力 ---
	bool isRegisteringUltimate_ = false;
	int registerKeyInputHandle_ = -1;
	char registerInputBuf_[128] = { 0 };
};