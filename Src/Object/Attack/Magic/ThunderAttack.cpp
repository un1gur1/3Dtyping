#include "ThunderAttack.h"
#include "../../../Common/Grid.h"
#include <DxLib.h>
#include <random>
#include <algorithm>
#include "../../Actor/Player/Player.h"
#include "../../Actor/ActorBase.h"
#include "../../../Application.h"

ThunderAttack::ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
}

void ThunderAttack::Update()
{
    if (!isAlive_) return;

    // フレーム経過（用途が無くなった warning 用だが残しておく）
    elapsed_ += 1.0f / 60.0f;

    // ワーニング無しのため、生成されたら即 Execute
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
            // 着弾時のダメージ判定（targets_ に対して判定）
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
            // 着弾エフェクトを少し残して非アクティブ化
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
    // no-op
}

void ThunderAttack::Execute()
{
    // strikePositions_ を構築するロジック:
    // - 敵の攻撃（isPlayer_ == false）：プレイヤーの現在地を主要ターゲットにし、残りはランダム
    // - プレイヤーの攻撃（isPlayer_ == true）：渡された targets_ の中の敵の位置のみをターゲットにする

    strikePositions_.clear();
    strikeGridIndices_.clear();

    if (!isPlayer_) {
        // 敵が発射した落雷: プレイヤー位置 + ランダム
        ActorBase* player = nullptr;
        for (auto* a : targets_) {
            if (a && a->IsPlayer()) { player = a; break; }
        }
        if (player) {
            VECTOR p = player->GetPos();
            p.y += 150.0f; // 頭上寄せ
            strikePositions_.push_back(p);
            strikeGridIndices_.push_back(AttackBase::CalcGridIndex(p, isPlayer_));
        }
        else {
            // フォールバック: 自身（shooter_）の位置を主要ターゲットにする
            if (shooter_) {
                VECTOR p = shooter_->GetPos();
                p.y += 150.0f;
                strikePositions_.push_back(p);
                strikeGridIndices_.push_back(AttackBase::CalcGridIndex(p, isPlayer_));
            }
        }

        // 追加ランダムスポットを選ぶ（グリッド交点 -800..800 step 400）
        std::mt19937 mt{ static_cast<unsigned int>(std::random_device{}()) };
        std::uniform_int_distribution<int> dist(-2, 2); // -2,-1,0,1,2 -> *400

        int attempts = 0;
        while ((int)strikePositions_.size() < strikeCount_ && attempts < 64) {
            int gx = dist(mt) * 400;
            int gz = dist(mt) * 400;
            VECTOR p = { static_cast<float>(gx), 0.0f, static_cast<float>(gz) };
            int gidx = AttackBase::CalcGridIndex(p, isPlayer_);
            bool dup = false;
            for (int existing : strikeGridIndices_) if (existing == gidx) { dup = true; break; }
            if (!dup) {
                strikePositions_.push_back(p);
                strikeGridIndices_.push_back(gidx);
            }
            attempts++;
        }
    }
    else {
        // プレイヤーが発射した落雷: targets_ の敵の位置のみ
        for (auto* a : targets_) {
            if (!a) continue;
            if (!a->IsEnemy()) continue;
            VECTOR p = a->GetPos();
            p.y += 150.0f; // 敵の頭上
            int gidx = AttackBase::CalcGridIndex(p, false);
            strikePositions_.push_back(p);
            strikeGridIndices_.push_back(gidx);
        }

        // もし敵がいなければ、targetGridIdx_ または pos_ を使うフォールバック
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
        // 上空から落とす（高さは少しずらす）
        bullet.pos = target;
        bullet.pos.y = 300.0f + static_cast<float>(i * 40);
        bullet.vel = { 0.0f, -600.0f, 0.0f }; // 下方向速度
        bullet.gridIndex = gridIdx;
        bullet.isActive = true;
        bullet.elapsed = 0.0f;
        bullets_.push_back(bullet);
    }
}