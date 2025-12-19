#include "AttackManager.h"
#include "../Actor/ActorBase.h"
#include <fstream>
#include <sstream>
#include <random>

AttackManager::AttackManager() {
    ReloadCommands();


}

AttackManager::~AttackManager() {
    Clear();
}

void AttackManager::Add(AttackBase* attack) {
    attacks_.push_back(attack);
}

// AttackManager.cpp
void AttackManager::UpdateAll(const std::vector<ActorBase*>& targets) {

    // 1. 弾と弾の相殺判定
    for (size_t i = 0; i < attacks_.size(); ++i) {
        AttackBase* a = attacks_[i];
        if (!a || !a->IsAlive()) continue;
        for (size_t j = i + 1; j < attacks_.size(); ++j) {
            AttackBase* b = attacks_[j];
            if (!b || !b->IsAlive()) continue;
            // 距離判定（球体同士）
            const VECTOR& apos = a->GetPos();
            const VECTOR& bpos = b->GetPos();
            float dx = apos.x - bpos.x;
            float dy = apos.y - bpos.y;
            float dz = apos.z - bpos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            float radiusSum = 100.0f * 2; // 弾の半径同士（仮に両方100.0fとする）
            if (distSq < radiusSum * radiusSum) {
                a->Kill();
                b->Kill();
            }
        }
    }
	// 2. 弾とキャラクターの当たり判定
    for (auto* attack : attacks_) {
        if (!attack || !attack->IsAlive()) continue;
        attack->Update();

        for (auto* target : targets) {
            if (!target || !target->GetisCollision()) continue;
            if (attack->GetShooter() == target) continue;

            if (attack->GetBulletType() == AttackBase::BulletType::PLAYER && !target->IsEnemy()) continue;
            if (attack->GetBulletType() == AttackBase::BulletType::ENEMY && !target->IsPlayer()) continue;

            // 距離判定（球体同士）
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
    // 一意ID生成
    ++commandIdCounter_;
    char buf[32];
    sprintf_s(buf, "PLAYER_ULT_%04d", commandIdCounter_);
    std::string commandId = buf;

    // ランダム値生成
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_int_distribution<int> damageDist(50, 200);      // 例: 50～200ダメージ
    std::uniform_real_distribution<float> speedDist(10.0f, 40.0f); // 例: 10.0～40.0 速度

    int randomDamage = damageDist(mt);
    float randomSpeed = speedDist(mt);

    // コマンドデータを保存
    UltimateCommandData data;
    data.commandId = commandId;
    data.damage = randomDamage;
    data.speed = randomSpeed;
    ultimateCommandDataMap_[commandId] = data;

    // 登録
    registeredCommands_.emplace_back(commandString, commandId);

    // 必要ならCSVファイルにも追記
    std::ofstream ofs("Data/CSV/Ultimate.csv", std::ios::app);
    ofs << commandString << "," << commandId << "," << randomDamage << "," << randomSpeed << std::endl;

    return commandId;
}

void AttackManager::LoadCommandsFromCSV(const std::string& path) {
    ultimateCommandDataMap_.clear();
    registeredCommands_.clear(); // 追加: 既存の登録もクリア
    std::ifstream file(path);
    std::string line;
    int maxId = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);

        std::string command, commandId, damageStr, speedStr;
        if (!std::getline(iss, command, ',')) continue;
        if (!std::getline(iss, commandId, ',')) continue;
        if (!std::getline(iss, damageStr, ',')) continue;
        if (!std::getline(iss, speedStr, ',')) continue;

        UltimateCommandData data;
        data.commandId = commandId;
        if (damageStr.empty()) continue;
        try {
            data.damage = std::stoi(damageStr);
        }
        catch (const std::exception&) {
            continue;
        }

        data.speed = std::stof(speedStr);
        ultimateCommandDataMap_[commandId] = data;

        // 追加: コマンドも登録
        registeredCommands_.emplace_back(command, commandId);

        // 連番最大値の更新
        size_t pos = commandId.rfind('_');
        if (pos != std::string::npos) {
            int idNum = std::stoi(commandId.substr(pos + 1));
            if (idNum > maxId) maxId = idNum;
        }
    }
    commandIdCounter_ = maxId;
}



void AttackManager::ReloadCommands() {
    LoadCommandsFromCSV("Data/CSV/Ultimate.csv");
}

