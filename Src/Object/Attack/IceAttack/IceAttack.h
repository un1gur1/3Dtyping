#pragma once
#include "../AttackBase.h"

class IceAttack : public AttackBase {
public:
	IceAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);
	~IceAttack() override = default;

	void Update() override;
	void Draw() override;
	void Execute() override; // Õ“Ë‚ÉŒÄ‚Î‚ê‚é‘z’è
};