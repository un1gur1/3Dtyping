#pragma once
#include "../AttackBase.h"

class MeteorAttack : public AttackBase {
public:
	MeteorAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);
	~MeteorAttack() override = default;

	void Update() override;
	void Draw() override;
	void Execute() override;

private:
	float fallSpeed_ = 0.0f;
	bool impacted_ = false;
};