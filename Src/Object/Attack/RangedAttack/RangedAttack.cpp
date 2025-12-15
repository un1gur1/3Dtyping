#include "RangedAttack.h"
#include <DxLib.h>

RangedAttack::RangedAttack(const VECTOR& pos, const VECTOR& vel, int damage, ActorBase* shooter)
    : 
    shooter_(shooter),
	AttackBase(shooter)
{
    pos_ = pos;
    vel_ = vel;
    damage_ = damage;
    isAlive_ = true;
}

void RangedAttack::Update() {
    // ˆÚ“®
    pos_.x += vel_.x;
    pos_.y += vel_.y;
    pos_.z += vel_.z;
    lifeTime_ -= 1.0f / 60.0f;
    if (lifeTime_ <= 0.0f) {
        isAlive_ = false;
    }
}

void RangedAttack::Draw() {
    DrawSphere3D(pos_, 100.0f, 16, GetColor(255, 100, 100), GetColor(255, 100, 100), false);
}
