#pragma once
#include "../AttackBase.h"

class RangedAttack : public AttackBase {
public:
    RangedAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);

    void Update() override;
    void Draw() override;
    void DrawWarning() override;
    void Execute() override;

	BulletType GetBulletType() const override { return BulletType::PLAYER; }
};
