#pragma once
#include "../AttackBase.h"

class HealAttack : public AttackBase {
public:
	// damage ˆø”‚Í‰ñ•œ—Ê‚Æ‚µ‚Äˆµ‚¤i³•‰‚ÍİŒvŸ‘æj
	HealAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int healAmount, ActorBase* shooter);
	~HealAttack() override = default;

	void Update() override;
	void Draw() override;
	void Execute() override;
};