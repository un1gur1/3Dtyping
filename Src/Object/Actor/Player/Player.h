#pragma once
#include <vector>
#include <map> 
#include <string>
#include <utility>
#include <algorithm>
#include <DxLib.h>
#include "../ActorBase.h"
#include "../../../Common/RomanjiConverter.h"

class Camera;
class RomanjiConverter;
class AttackManager;
class RangedAttack;

class Player : public ActorBase
{

public:

	
	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,         // Idle.mv1
		WALK,         // FastRun.mv1, 
		WALK_LOOP,    // Walk.mv1
		ATTACK2,      // Attack2.mv1
		ATTACK3,      // Attack3.mv1
		BOXING,       // Boxing.mv1
		FALLING,      // Falling.mv1
		FLYING,       // Flying.mv1
		JUMP,         // Jump.mv1
		JUMP_ATTACK,  // JumpAttack.mv1
		JUMP_HOVER,   // JumpHover.mv1
		JUMPING,      // Jumping.mv1
		JUMP_RISING,  // JumpRising.mv1
		SHOT,         // Shot.mv1
		THROW,        // Throw.mv1
		VICTORY,      // Victory.mv1
		WARP_POSE,    // WarpPose.mv1
		// 既存のPUNCHなども残す場合はここに
		MAX,
	};

	// コンストラクタ
	Player(Camera* camera);

	// デストラクタ
	~Player(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

	std::map<std::string, std::pair<int, int>> moveWordDict_;
	std::string inputText_;
	char inputBuf_[128] = { 0 }; // 入力中バッファ
	void LoadMoveWordDict(const std::string& path);	
	RomanjiConverter romanjiConverter_;
	std::string hiraText_;    // 変換後ひらがな
	int keyInputHandle_ = -1; // 入力欄ハンドル
	bool isInputActive_ = false; // 入力中フラグ
	bool isFirstInputFrame_ = true;
	void SetAttackManager(AttackManager* manager) { attackManager_ = manager; }

	enum class CommandType {
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP_RIGHT,
		MOVE_UP_LEFT,
		MOVE_DOWN_RIGHT,
		MOVE_DOWN_LEFT,
		DODGE,
		MOVE_RANDOM,
		SHOOT,
		MOVE,
		ATTACK,
		UNKNOWN
	};
	CommandType StringToCommandType(const std::string& str);
	std::unordered_map<std::string, CommandType> commandMap_;

	// オーバーライド
	void ApplyDamage(int damage) override;
	void AddStun(int value) override;
	void OnStunned() override;
	bool IsDead() const override;
	void ChangeState(ActorState state)override;
	ActorState GetState() const override;
	bool IsPlayer() const override { return true; }
	bool IsEnemy() const override { return false; }

private:
	bool isBulletFired_ = false; // 弾発射フラグ追加

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;

	// 移動処理
	void Move(void) override;

	bool isAttacking_ = false;

	ActorState state_ = ActorState::IDLE;

private:
	bool isRegisteringUltimate_ = false;
	char registerInputBuf_[128] = { 0 };
	int registerKeyInputHandle_ = -1;
	// カメラ
	Camera* camera_;
	RomanjiConverter* cov_;
	AttackManager* attackManager_;
	RangedAttack* rangedAttack_;


};
