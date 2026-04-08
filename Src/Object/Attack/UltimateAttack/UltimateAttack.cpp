#include "UltimateAttack.h"
#include <DxLib.h>
#include <cstdlib>
#include <vector>
#include <filesystem>
#include "../../../Common/EffectManager.h"
#include <windows.h> // OutputDebugString 用（デバッグログ）

// 候補エフェクトキー（必要なら増減・ファイルパス調整してください）
const std::vector<std::string> UltimateAttack::kEffectKeys_ = {
    "dark", "meteo", "sword", "cold", "water", "ken", "tornerd", "thunder", "heal"
};

UltimateAttack::UltimateAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTimer_)
{
    // 事前ロード（存在しないファイルは EffectManager が失敗するのでここでログしておく）
    for (const auto& key : kEffectKeys_) {
        const std::string path = std::string("Data/Image/efe3/") + key + ".efk";
        if (!std::filesystem::exists(path)) {
            std::string msg = "UltimateAttack: effect file NOT FOUND: " + path + "\n";
            OutputDebugStringA(msg.c_str());
        }
        EffectManager::GetInstance().Load(key, path);
    }
}

UltimateAttack::~UltimateAttack() {
    // 念のため破棄時に残っているエフェクトを停止
    StopEffects();
}

void UltimateAttack::StopEffects() {
    if (!effectPlayingIds_.empty()) {
        for (int id : effectPlayingIds_) {
            if (id != -1) EffectManager::GetInstance().Stop(id);
        }
        effectPlayingIds_.clear();
    }
}

void UltimateAttack::Update() {
    AttackBase::Update();

    // 生存中はエフェクト位置を攻撃位置に追従させる
    if (!effectPlayingIds_.empty() && isAlive_) {
        for (int id : effectPlayingIds_) {
            if (id != -1) EffectManager::GetInstance().SetPos(id, pos_);
        }
    }

    // 攻撃が終わったらエフェクトを停止してクリア
    if (!isAlive_ && !effectPlayingIds_.empty()) {
        StopEffects();
    }
}

void UltimateAttack::Draw() {
    // 必要ならデバッグ用球を表示（既存と同じ）
    DrawSphere3D(pos_, 30.0f, 16, GetColor(255, 100, 100), GetColor(200, 122, 15), true);
}

void UltimateAttack::DrawWarning() {
    // 未使用（必要なら警告エフェクトなど追加）
}

void UltimateAttack::Execute() {
    // 既存エフェクトが残っていたら先に止める
    StopEffects();

    // 単一エフェクトをランダムに選んで再生（まずは同時生成は行わない）
    const std::string& key = kEffectKeys_[rand() % kEffectKeys_.size()];
    const std::string path = std::string("Data/Image/efe3/") + key + ".efk";

    // ファイル存在は事前にチェックしているが、念のためログ出力
    if (!std::filesystem::exists(path)) {
        std::string msg = "UltimateAttack::Execute - effect file missing: " + path + "\n";
        OutputDebugStringA(msg.c_str());
    }

    int eid = EffectManager::GetInstance().Play(key, pos_);
    if (eid == -1) {
        // 再生失敗の原因追跡用ログ
        std::string msg = "UltimateAttack: Play failed for key=" + key + " path=" + path + "\n";
        OutputDebugStringA(msg.c_str());
    }
    else {
        // 再生成功：位置追従用に保持
        effectPlayingIds_.push_back(eid);
        // スケールは攻撃に合わせて調整
        EffectManager::GetInstance().SetScale(eid, 100.0f);
    }

    // ここで既存の究極攻撃本体の処理（弾生成など）があれば追記してください。
}