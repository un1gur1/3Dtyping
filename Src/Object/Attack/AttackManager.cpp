#include "AttackManager.h"
#include "../Actor/ActorBase.h"
#include <fstream>
AttackManager::AttackManager() {}

AttackManager::~AttackManager() {
    Clear();
}

void AttackManager::Add(AttackBase* attack) {
    attacks_.push_back(attack);
}

// AttackManager.cpp
void AttackManager::UpdateAll(const std::vector<ActorBase*>& targets) {

    // 1. ’e‚Æ’e‚Ì‘ŠE”»’è
    for (size_t i = 0; i < attacks_.size(); ++i) {
        AttackBase* a = attacks_[i];
        if (!a || !a->IsAlive()) continue;
        for (size_t j = i + 1; j < attacks_.size(); ++j) {
            AttackBase* b = attacks_[j];
            if (!b || !b->IsAlive()) continue;
            // ‹——£”»’èi‹…‘Ì“¯mj
            const VECTOR& apos = a->GetPos();
            const VECTOR& bpos = b->GetPos();
            float dx = apos.x - bpos.x;
            float dy = apos.y - bpos.y;
            float dz = apos.z - bpos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float radiusSum = 100.0f * 2; // ’e‚Ì”¼Œa“¯mi‰¼‚É—¼•û100.0f‚Æ‚·‚éj
            if (distSq < radiusSum * radiusSum) {
                a->Kill();
                b->Kill();
            }
        }
    }
	// 2. ’e‚ÆƒLƒƒƒ‰ƒNƒ^[‚Ì“–‚½‚è”»’è
    for (auto* attack : attacks_) {
        if (!attack || !attack->IsAlive()) continue;
        attack->Update();

        for (auto* target : targets) {
            if (!target || !target->GetisCollision()) continue;
            if (attack->GetShooter() == target) continue;

            if (attack->GetBulletType() == AttackBase::BulletType::PLAYER && !target->IsEnemy()) continue;
            if (attack->GetBulletType() == AttackBase::BulletType::ENEMY && !target->IsPlayer()) continue;

            // ‹——£”»’èi‹…‘Ì“¯mj
            const VECTOR& apos = attack->GetPos();
            const VECTOR& tpos = target->GetPos();
            float dx = apos.x - tpos.x;
            float dy = apos.y - tpos.y;
            float dz = apos.z - tpos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float radiusSum = 100.0f + target->GetCapsuleRadius();
            if (distSq < radiusSum * radiusSum) {
                target->ApplyDamage(attack->GetDamage());
                attack->Kill();
                break;
            }
        }
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

std::string AttackManager::RegisterUltimateCommand(const std::string& commandString) {
    // ˆêˆÓID¶¬
    ++commandIdCounter_;
    char buf[32];
    sprintf_s(buf, "PLAYER_ULT_%04d", commandIdCounter_);
    std::string commandId = buf;

    // “o˜^
    registeredCommands_.emplace_back(commandString, commandId);

    // •K—v‚È‚çCSVƒtƒ@ƒCƒ‹‚É‚à’Ç‹L
    std::ofstream ofs("Data/CSV/Word.csv", std::ios::app);
    ofs << commandString << "," << commandId << std::endl;

    return commandId;
}
