#include "RangedAttack.h"
#include <DxLib.h>

RangedAttack::RangedAttack(const VECTOR& pos, const VECTOR& vel, int damage)
{
    pos_ = pos;
    vel_ = vel;
    damage_ = damage;
    isAlive_ = true;
}

void RangedAttack::Update() {
    pos_ = VAdd(pos_, vel_);
    lifeTime_ -= 1.0f / 60.0f; // 60FPS‘z’è
    if (lifeTime_ <= 0) {
        isAlive_ = false;
    }
    // •Ç‚â“G‚Æ‚Ì“–‚½‚è”»’è‚Í‚±‚±‚Å’Ç‰Á
}

void RangedAttack::Draw() {
    DrawSphere3D(pos_, 10.0f, 16, GetColor(255, 100, 100), GetColor(255, 100, 100), false);
}
