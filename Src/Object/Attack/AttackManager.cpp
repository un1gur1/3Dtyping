#include "AttackManager.h"

AttackManager::AttackManager() {}

AttackManager::~AttackManager() {
    Clear();
}

void AttackManager::Add(AttackBase* attack) {
    attacks_.push_back(attack);
}

void AttackManager::UpdateAll() {
    for (auto* attack : attacks_) {
        if (attack) attack->Update();
    }
}

void AttackManager::DrawAll() {
    for (auto* attack : attacks_) {
        if (attack) attack->Draw();
    }
}

void AttackManager::Clear() {
    for (auto* attack : attacks_) {
        delete attack;
    }
    attacks_.clear();
}
