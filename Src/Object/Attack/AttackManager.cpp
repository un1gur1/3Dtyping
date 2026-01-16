#include "AttackManager.h"
#include "../Actor/ActorBase.h"
#include "../Attack/Magic/ThunderAttack.h"
#include "../../Application.h"
#include "AttackBase.h"
#include <fstream>
#include <sstream>
#include <random>
#include <functional> 

AttackManager::AttackManager() {
    ReloadCommands();


}

AttackManager::~AttackManager() {
    Clear();
}

void AttackManager::Add(AttackBase* attack) {
    attacks_.push_back(attack);
}

void AttackManager::UpdateAll(const std::vector<ActorBase*>& targets) {
    for (auto* attack : attacks_) {
        if (!attack || !attack->IsAlive()) continue;
        attack->Update();

        for (auto* target : targets) {
            if (!target || !target->GetisCollision()) continue;
            if (attack->GetShooter() == target) continue;

            // ブロードフェーズ: グリッド判定
            if (attack->collisionType_ == AttackBase::CollisionType::Grid || attack->collisionType_ == AttackBase::CollisionType::Both) {
                // 例: グリッド座標が一致していれば詳細判定へ
                if (attack->GetTargetGridIdx() == target->gridPos_.x && attack->GetTargetGridIdx() == target->gridPos_.z) {
                    // ローフェーズ: 球体判定
                    if (attack->collisionType_ == AttackBase::CollisionType::Both || attack->collisionType_ == AttackBase::CollisionType::Sphere) {
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
                            Application::GetInstance()->ShakeScreen(5, 30, true, true);


                            break;
                        }
                    }
                    else {
                        // グリッドのみで判定
                        target->ApplyDamage(attack->GetDamage());
                        attack->Kill();
                        Application::GetInstance()->ShakeScreen(5, 30, true, true);


                        break;
                    }
                }
            }
            else if (attack->collisionType_ == AttackBase::CollisionType::Sphere) {
                // 球体判定のみ
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
                    Application::GetInstance()->ShakeScreen(5, 30, true, true);


                    break;
                }
            }
        }
    }
}




void AttackManager::DrawAll() {
    for (auto* attack : attacks_) {
        if (attack && attack->IsAlive()) {
            attack->Draw();
        }
    }
}

void AttackManager::Clear() {
    for (auto* attack : attacks_) {
        delete attack;
    }
    attacks_.clear();
}




std::string AttackManager::RegisterUltimateCommand(const std::string& commandString, int minLength) {
    if (commandString.length() < minLength) return "";
    for (const auto& pair : registeredCommands_) {
        if (pair.first == commandString) return "";
    }
    ++commandIdCounter_;
    char buf[32];
    sprintf_s(buf, "PLAYER_ULT_%04d", commandIdCounter_);
    std::string commandId = buf;

    // ハッシュ値生成
    std::size_t hashValue = std::hash<std::string>{}(commandString);

    // ダメージと速度をハッシュ値から決定
    int damage = 50 + (hashValue % 151); // 50～200
    float speed = 10.0f + ((hashValue / 151) % 31); // 10.0～40.0

    UltimateCommandData data;
    data.commandId = commandId;
    data.damage = damage;
    data.speed = speed;
    ultimateCommandDataMap_[commandId] = data;

    registeredCommands_.emplace_back(commandString, commandId);

    std::ofstream ofs("Data/CSV/Ultimate.csv", std::ios::app);
    ofs << commandString << "," << commandId << "," << damage << "," << speed << std::endl;

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

std::vector<std::string> AttackManager::GetUltimateCommandNames() const {
    std::vector<std::string> names;
    for (const auto& pair : registeredCommands_) {
        names.push_back(pair.first); // コマンド名
    }
    return names;
}
