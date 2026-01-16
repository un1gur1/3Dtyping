#pragma once
#include <DxLib.h>
#include "../../Common/UiManager.h"
#include"../Actor/ActorBase.h"
class ActorBase;
class AttackBase {
public:
    // targetGridIdx: 攻撃対象グリッド番号
    // isPlayer: プレイヤー側かどうか
    // velocity: 初期速度
    // lifeTime: 寿命（秒）
    AttackBase(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);

    virtual ~AttackBase();

    // 共通の更新処理
    virtual void Update();

    // 派生で描画処理を実装
    virtual void Draw() = 0;

    // 予兆（警告）描画
    virtual void DrawWarning() = 0;

    // 攻撃本体（例：弾の発生・ダメージ判定など）
    virtual void Execute() = 0;

    // 位置取得・設定
    const VECTOR& GetPos() const { return pos_; }
    void SetPos(const VECTOR& pos) { pos_ = pos; }

    // ダメージ値取得
    int GetDamage() const { return damage_; }

    // 生存フラグ
    bool IsAlive() const { return isAlive_; }
    void Kill() { isAlive_ = false; }

    ActorBase* GetShooter() const { return shooter_; }

    int GetTargetGridIdx() const { return targetGridIdx_; }

    bool IsPlayerSide() const { return isPlayer_; }

    enum class BulletType {
        PLAYER,
        ENEMY
    };

    virtual BulletType GetBulletType() const = 0; // 純粋仮想関数として宣言
    enum class CollisionType {
        Sphere,
        Grid,
        Both
    };
    static int CalcGridIndex(const VECTOR& pos, bool isPlayer);

    CollisionType collisionType_ = CollisionType::Sphere;

protected:
    VECTOR pos_;      // 位置
    VECTOR vel_;      // 速度
    int damage_;      // ダメージ
    bool isAlive_;    // 生存フラグ
    float lifeTime_;  // 寿命（秒）
    int targetGridIdx_; // 攻撃対象グリッド番号
    bool isPlayer_;      // プレイヤー側かどうか
    ActorBase* shooter_;
    
};