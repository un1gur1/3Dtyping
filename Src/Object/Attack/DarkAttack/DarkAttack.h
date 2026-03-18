#pragma once
#include "../AttackBase.h"

class DarkAttack : public AttackBase {
public:
	DarkAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);
	~DarkAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override; 
	void Execute() override;

	BulletType GetBulletType() const override { return BulletType::PLAYER; } 
};