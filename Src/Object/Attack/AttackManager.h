#pragma once
#include <vector>
#include <string>
#include "AttackBase.h"
class ActorBase;
class AttackManager {
public:
    AttackManager();
    ~AttackManager();

    // 攻撃の追加
    void Add(AttackBase* attack);

    // 全攻撃の更新・描画
    void UpdateAll(const std::vector<ActorBase*>& targets);
    void DrawAll();

    std::string RegisterUltimateCommand(const std::string& commandString);

    // 全攻撃の取得
    const std::vector<AttackBase*>& GetAttacks() const { return attacks_; }

    // 全削除
    void Clear();

private:
    std::vector<AttackBase*> attacks_; // 生ポインタで管理

    int commandIdCounter_ = 0;
    std::vector<std::pair<std::string, std::string>> registeredCommands_;

};