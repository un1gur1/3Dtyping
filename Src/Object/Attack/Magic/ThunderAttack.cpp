#include "ThunderAttack.h"
#include "../../../Common/Grid.h"
#include <DxLib.h>

ThunderAttack::ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
    // ’e‚Í‚Ü‚¾”­Ë‚µ‚È‚¢i—\’›Œã‚ÉExecute‚Å”­Ëj
}

void ThunderAttack::Update()
{
    if (!isAlive_) return;

    elapsed_ += 1.0f / 60.0f;

    if (!executed_ && elapsed_ < warningTime_) {
        DrawWarning();
        // —\’›’†‚ÍAttackBase‚ÌUpdate‚ÍŒÄ‚Î‚È‚¢
        return;
    }

    if (!executed_) {
        Execute(); // —\’›I—¹Œã‚É’e”­Ë
        executed_ = true;
    }

    // ’e‚ÌˆÚ“®Eõ–½ŠÇ—
    for (auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        bullet.pos.y += bullet.vel.y * (1.0f / 60.0f);
        bullet.elapsed += 1.0f / 60.0f;
        if (bullet.elapsed > bulletLifeTime_) bullet.isActive = false;
    }

    // ‘S’eÁ–Å‚Å¶‘¶ƒtƒ‰ƒO‚ğ—‚Æ‚·
    bool anyActive = false;
    for (const auto& bullet : bullets_) {
        if (bullet.isActive) anyActive = true;
    }
    if (!anyActive) {
        isAlive_ = false;
    }
}

void ThunderAttack::Draw()
{
    for (const auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        DrawSphere3D(pos_, 100.0f, 16, GetColor(255, 100, 100), GetColor(255, 100, 100), false);
    }
}

void ThunderAttack::DrawWarning()
{
    // —\’›: ‘ÎÛƒOƒŠƒbƒh‚ğŒxF‚É
    UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Warning, isPlayer_);
}

void ThunderAttack::Execute()
{
    // UŒ‚F‚É•ÏX
    UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Attack, isPlayer_);

    // ’e‚ğ”­Ë
    ThunderBullet bullet;
    bullet.pos = Grid::GetWorldPosFromIndex(targetGridIdx_, isPlayer_);
    bullet.pos.y = 0.0f; // ‰º‚©‚ç”­Ë
    bullet.vel = { 0.0f, 20.0f, 0.0f }; // ã•ûŒü
    bullet.gridIndex = targetGridIdx_;
    bullet.isActive = true;
    bullet.elapsed = 0.0f;
    bullets_.push_back(bullet);
}
