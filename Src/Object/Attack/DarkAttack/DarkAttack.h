#pragma once
#include "../AttackBase.h"

class DarkAttack : public AttackBase {
public:
	// 遅延タイマー対応
	DarkAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime = 0.0f);
	~DarkAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	// 敵・味方両対応
	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

private:
	// エフェクト管理用
	bool warningPlayed_ = false;
	bool attackExecuted_ = false;
	int effectPlayingId_ = -1;
};