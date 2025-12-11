#pragma once
#include "../AttackBase.h"

class RangedAttack : public AttackBase {
public:
    RangedAttack(const VECTOR& pos, const VECTOR& vel, int damage);

    void Update() override;
    void Draw() override;

private:
    float lifeTime_ = 10.0f; // ïbêîÇ»Ç«
};
