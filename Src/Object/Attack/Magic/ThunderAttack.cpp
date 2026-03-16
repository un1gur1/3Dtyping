#include "ThunderAttack.h"
#include "../../../Common/Grid.h"
#include <DxLib.h>
#include <random>
#include <algorithm>
#include <cstdio> // std::fprintf用
#include "EffekseerForDXLib.h"
#include "../../Actor/Player/Player.h"
#include "../../Actor/ActorBase.h"
#include "../../../Application.h"
#include "../../../Common/UiManager.h"



ThunderAttack::ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
}

void ThunderAttack::Update()
{
    if (!isAlive_) return;

    // フレーム経過
    elapsed_ += 1.0f / 60.0f;

    // 生成されたら即 Execute
    if (!executed_) {
        Execute();
        executed_ = true;
    }

    // 弾の移動・寿命管理（自由落下）
    for (auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        bullet.pos.y += bullet.vel.y * (1.0f / 60.0f);
        bullet.elapsed += 1.0f / 60.0f;

        // 着地判定（地面 y<=0 と仮定）
        if (bullet.isActive && bullet.pos.y <= 0.0f) {

            // 着弾時のダメージ判定
            for (auto* tgt : targets_) {
                if (!tgt || !tgt->GetisCollision()) continue;
                const VECTOR tpos = tgt->GetPos();
                float dx = tpos.x - bullet.pos.x;
                float dy = tpos.y - bullet.pos.y;
                float dz = tpos.z - bullet.pos.z;
                float distSq = dx * dx + dy * dy + dz * dz;
                float radiusSum = 100.0f + tgt->GetCapsuleRadius();
                if (distSq < radiusSum * radiusSum) {
                    tgt->ApplyDamage(damage_);
                    Application::GetInstance()->ShakeScreen(5, 30, true, true);
                }
            }

            // ==========================================================
            // ★ 着弾エフェクトを再生（Effekseer 3D）
            // ==========================================================
            if (!s_thunderEffectTried) {
                s_thunderEffectTried = true;

                // ★ 3Dマネージャーが存在するか確認
                if (GetEffekseer3DManager() != nullptr) {
                    const char* path = "Data/Image/efe2/thunder.efk";
                    s_thunderEffectHandle = LoadEffekseerEffect(path, 1.0f);
                    if (s_thunderEffectHandle == -1) {
                        std::fprintf(stderr, "ThunderAttack: LoadEffekseerEffect failed for %s\n", path);
                    }
                }
                else {
                    std::fprintf(stderr, "ThunderAttack: Effekseer 3D manager not available\n");
                }
            }

            if (s_thunderEffectHandle != -1) {
                // ★ 3Dエフェクトとして再生
                int ph = PlayEffekseer3DEffect(s_thunderEffectHandle);
                if (ph != -1) {
                    // ★ 3D空間の地面の座標にセット
                    VECTOR groundPos = bullet.pos;
                    groundPos.y = 5.0f;

                    SetPosPlayingEffekseer3DEffect(ph, groundPos.x, groundPos.y, groundPos.z);
                    SetScalePlayingEffekseer3DEffect(ph, 15, 15, 15);
                }
                else {
                    std::fprintf(stderr, "ThunderAttack: PlayEffekseer3DEffect returned -1\n");
                }
            }
            // ==========================================================

            // 着弾後は非アクティブ化
            bullet.isActive = false;
            bullet.elapsed = 0.0f;
        }

        if (bullet.elapsed > bulletLifeTime_) bullet.isActive = false;
    }

    // 全弾消滅で生存フラグを落とす
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
    // 弾を描画（弾ごとの位置で描く）
    for (const auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        DrawSphere3D(bullet.pos, 30.0f, 16, GetColor(255, 200, 50), GetColor(255, 200, 50), true);

        // 地面エフェクト（小さい円）
        VECTOR ground = bullet.pos;
        ground.y = 0.0f;
        DrawSphere3D(ground, 60.0f, 24, GetColor(255, 180, 80), GetColor(0, 0, 0), true);
    }
}

void ThunderAttack::DrawWarning()
{
    // ワーニングは無効（要件より）
}

void ThunderAttack::Execute()
{
    strikePositions_.clear();
    strikeGridIndices_.clear();

    if (!isPlayer_) {
        // 敵が発射した落雷: 事前に SetPos() された座標を使う
        strikePositions_.push_back(pos_);
        strikeGridIndices_.push_back(AttackBase::CalcGridIndex(pos_, isPlayer_));
    }
    else {
        // プレイヤーが発射した落雷: 敵の頭上へ落とす
        for (auto* a : targets_) {
            if (!a) continue;
            if (!a->IsEnemy()) continue;
            VECTOR p = a->GetPos();
            p.y += 150.0f; // 敵の頭上
            int gidx = AttackBase::CalcGridIndex(p, false);
            strikePositions_.push_back(p);
            strikeGridIndices_.push_back(gidx);
        }

        // 敵がいなければフォールバック
        if (strikePositions_.empty()) {
            VECTOR fallback = pos_;
            if (fallback.x == 0.0f && fallback.y == 0.0f && fallback.z == 0.0f) {
                fallback = Grid::GetWorldPosFromIndex(targetGridIdx_, isPlayer_);
            }
            strikePositions_.push_back(fallback);
            strikeGridIndices_.push_back(AttackBase::CalcGridIndex(fallback, isPlayer_));
        }
    }

    // グリッド状態を Attack に変更して、上空から落とす弾を生成
    for (size_t i = 0; i < strikePositions_.size(); ++i) {
        const VECTOR& target = strikePositions_[i];
        int gridIdx = strikeGridIndices_[i];
        UIManager::GetInstance().SetGridState(gridIdx, Grid::GridState::Attack, isPlayer_);

        ThunderBullet bullet;
        // 上空から落とす
        bullet.pos = target;
        bullet.pos.y = 300.0f; // 高さ
        bullet.vel = { 0.0f, -600.0f, 0.0f }; // 下方向速度
        bullet.gridIndex = gridIdx;
        bullet.isActive = true;
        bullet.elapsed = 0.0f;
        bullets_.push_back(bullet);
    }
}