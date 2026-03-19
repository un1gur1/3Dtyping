#pragma once
#include "../AttackBase.h"

class MeteorAttack : public AttackBase {
public:
	MeteorAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime = 0.0f);
	~MeteorAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

private:
	float fallSpeed_ = 0.0f;
	bool impacted_ = false;

	// エフェクト管理用
	bool warningPlayed_ = false;
	bool attackExecuted_ = false;
	int effectPlayingId_ = -1;
};