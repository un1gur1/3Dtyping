#pragma once
#include "../AttackBase.h"

class SwordAttack : public AttackBase {
public:
	SwordAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);
	~SwordAttack() override = default;

	void Update() override;
	void Draw() override;
	void Execute() override;
};