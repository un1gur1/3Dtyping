#pragma once
#include "../AttackBase.h"
#include "../../Actor/ActorBase.h"
#include "../../../Common/UiManager.h"
#include <vector>

class ThunderAttack : public AttackBase
{
public:
    // 1回の攻撃で複数スポットに落雷
    ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);

    void Update() override;
    void Draw() override;
    void DrawWarning() override;
    void Execute() override;

    BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

    void SetTargets(const std::vector<ActorBase*>& targets) {
        targets_ = targets;
    }
private:
    struct ThunderBullet {
        VECTOR pos;
        VECTOR vel;
        bool isActive = true;
        int gridIndex;
        float elapsed = 0.0f;
    };

    std::vector<ThunderBullet> bullets_; // 発射された弾
    float warningTime_ = 1.0f; // 予兆時間
    float elapsed_ = 0.0f;
    bool executed_ = false;
    bool bulletFired_ = false;
    float bulletLifeTime_ = 1.0f; // 弾の寿命

    // 静的キャッシュ：エフェクトリソースハンドル
    int s_thunderEffectHandle = -1;
    bool s_thunderEffectTried = false;

    int s_thunderWarningEffectHandle;
    bool s_thunderWarningEffectTried;
    std::vector<int> warningEffectPlayHandles_;

    // 生成する落雷スポット（予兆時に決定して Execute と共有）
    std::vector<VECTOR> strikePositions_;
    std::vector<int> strikeGridIndices_;
    int strikeCount_ = 3; // デフォルト: 主要1 + 2ランダム
    std::vector<ActorBase*> targets_;
};