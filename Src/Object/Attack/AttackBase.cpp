#include "AttackBase.h"

AttackBase::AttackBase()
    : pos_{ 0,0,0 }, vel_{ 0,0,0 }, damage_(1), isAlive_(true)
{
}

AttackBase::~AttackBase()
{
}
