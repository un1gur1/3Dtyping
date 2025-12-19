#pragma once
#include "../AttackBase.h"
#include "../../Actor/ActorBase.h"
#include "../../../Common/UiManager.h"
#include <vector>

class ThunderAttack : public AttackBase
{
public:
    // 1‚Â‚ÌƒOƒŠƒbƒh‚É‘Î‚µ‚Ä1”­‚¸‚Â’e‚ğ”­Ë‚·‚éİŒv
    ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);

    void Update() override;
    void Draw() override;
    void DrawWarning() override;
    void Execute() override;

    BulletType GetBulletType() const override { return BulletType::PLAYER; }


private:
    struct ThunderBullet {
        VECTOR pos;
        VECTOR vel;
        bool isActive = true;
        int gridIndex;
        float elapsed = 0.0f;
    };

    std::vector<ThunderBullet> bullets_; // ”­Ë’†‚Ì’e
    float warningTime_ = 1.0f; // —\’›ŠÔi•bj
    float elapsed_ = 0.0f;
    bool executed_ = false;
    bool bulletFired_ = false;
    float bulletLifeTime_ = 1.0f; // ’e‚Ì¶‘¶ŠÔi•bj
};
