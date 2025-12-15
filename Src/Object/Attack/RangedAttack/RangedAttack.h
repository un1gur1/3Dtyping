#pragma once
#include "../AttackBase.h"

class ActorBase;
class RangedAttack : public AttackBase {
public:
    RangedAttack(const VECTOR& pos, const VECTOR& vel, int damage, ActorBase* shooter);

    void Update() override;
    void Draw() override;

    ActorBase* GetShooter() const { return shooter_; }

private:
    float lifeTime_ = 10.0f; // ïbêîÇ»Ç«
    ActorBase* shooter_;
};
