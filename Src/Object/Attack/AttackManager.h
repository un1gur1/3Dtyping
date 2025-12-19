#pragma once
#include <vector>
#include <string>
#include <map>
#include "AttackBase.h"
class ActorBase;
class AttackManager {
public:

    // 必殺技コマンド情報
    struct UltimateCommandData {
        std::string commandId;
        int damage;
        float speed;
    };

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
    void LoadCommandsFromCSV(const std::string& path);
    void ReloadCommands();

    std::map<std::string, UltimateCommandData> ultimateCommandDataMap_;


    std::vector<std::pair<std::string, std::string>> registeredCommands_;

private:
    std::vector<AttackBase*> attacks_; // 生ポインタで管理

    int commandIdCounter_ = 0;

};