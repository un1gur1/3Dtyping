#pragma once
#include "../AttackBase.h"
#include <vector>
#include <string>

class UltimateAttack : public AttackBase
{
public:
    // targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter
    UltimateAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_);
    ~UltimateAttack() override;

    void Update() override;
    void Draw() override;
    void DrawWarning() override;
    void Execute() override;

    BulletType GetBulletType() const override { return BulletType::PLAYER; }

private:
    // 再生中のエフェクトハンドル保持
    std::vector<int> effectPlayingIds_;

    // 使用候補キー（コンストラクタでロード）
    static const std::vector<std::string> kEffectKeys_;

    // 再生中エフェクトを安全に停止してクリアするヘルパ
    void StopEffects();
};