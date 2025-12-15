#include "AttackBase.h"
#include "../Actor/ActorBase.h"

AttackBase::AttackBase(ActorBase* shooter)
    : 
    pos_{ 0,0,0 }, 
    vel_{ 0,0,0 },
    damage_(1), 
    isAlive_(true),
    shooter_(shooter)

{
    if (shooter_) {
        if (shooter_->IsPlayer()) bulletType_ = BulletType::PLAYER;
        else if (shooter_->IsEnemy()) bulletType_ = BulletType::ENEMY;
        else bulletType_ = BulletType::ENEMY;
    }
    else {
        bulletType_ = BulletType::ENEMY;
    }
    isAlive_ = true;
}

AttackBase::~AttackBase()
{
}

