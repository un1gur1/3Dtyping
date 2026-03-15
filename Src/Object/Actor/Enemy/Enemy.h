#pragma once
#include "../ActorBase.h"
#include <vector>
class Player;
class AttackManager;
class UlitimateAttack;

class Enemy : public ActorBase
{

public:

	// ����̍L��
	static constexpr float VIEW_RANGE = 600.0f;

	// ����p
	static constexpr float VIEW_ANGLE = 60.0f;

	// ���o�̍L��
	static constexpr float HEARING_RANGE = 400.0f;

	// �A�j���[�V�������
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		SHOT,
		MAX,
	};



public:
	// �R���X�g���N�^
	Enemy(Player* player);

	// �f�X�g���N�^
	~Enemy(void) override;

	// �X�V
	void Update(void) override;

	// �`��
	void Draw(void) override;

	void OnAttackCancelled();

	void ChangeState(ActorState state)override;

	ActorState GetState() const override;

	void SetAttackManager(AttackManager* manager) { attackManager_ = manager; }

	std::string typingCommand_; // ���^�C�s���O���̃R�}���h
	float typingElapsed_ = 0.0f;
	float typingWait_ = 0.0f;
	void StartTypingUltimate(const std::string& command);
	void UpdateTypingUltimate(float deltaTime);
private:

	// ���\�[�X���[�h
	void InitLoad(void) override;

	// �傫���A��]�A���W�̏�����
	void InitTransform(void) override;

	// �A�j���[�V�����̏�����
	void InitAnimation(void) override;

	// ��������̌ʏ���
	void InitPost(void) override;

	//// ����`��
	//void DrawViewRange(void);

	// �I�[�o�[���C�h
	void ApplyDamage(int damage) override;
	void AddStun(int value) override;
	void OnStunned() override;
	bool IsDead() const override;
	bool IsPlayer() const override { return false; }
	bool IsEnemy() const override { return true; }



private:

	ActorState state_ = ActorState::IDLE;
	float stateTimer_ = 0.0f; // �N�[���_�E����r���A�Ђ�ݗp
	Player* player_;
	// �U�����̊Ǘ��p
	bool attackRegistered_ = false;
	AttackManager* attackManager_ = nullptr;
	UlitimateAttack* ultimateAttack_ = nullptr;

	// ���m�t���O(����)
	bool isNoticeView_;

	// ���m�t���O(���o)
	bool isNoticeHearing_;

	bool isAttacking_ = false;
	float moveTime_ = 0.0f; // �ړ��p�^�C�}�[
	float moveCenterX_ = 0.0f; // ���S���W
public:
	// コンボ関連（追加）
	enum class EnemyAction {
		NONE,
		ATTACK_NEAR,
		ATTACK_RANGE,
		THUNDER,   // 特殊近接（落雷）
		ULTIMATE
	};

	// コンボセット群（HP状態別に選択する）
	std::vector<std::vector<EnemyAction>> comboSets_;
	int activeComboIndex_ = -1;
	int comboStep_ = 0;
	bool comboExecuting_ = false;         // コンボ実行中フラグ
	bool comboActionInProgress_ = false; // 現在のコンボアクションが進行中か

	// HP状態判定用閾値
	static constexpr float HP_HIGH = 0.7f;
	static constexpr float HP_LOW = 0.3f;

	//// ���G
	//void Search(void);
};