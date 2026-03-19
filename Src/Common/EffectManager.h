#pragma once
#include <string>
#include <unordered_map>
#include <DxLib.h>
#include<EffekseerForDXLib.h>
class EffectManager {
public:
    // シングルトン（どこからでも EffectManager::GetInstance() で呼べる）
    static EffectManager& GetInstance() {
        static EffectManager instance;
        return instance;
    }

    // ==========================================
    // リソース管理
    // ==========================================
    // エフェクトの読み込み（倍率指定も可能）
    void Load(const std::string& key, const std::string& filePath, float magnification = 1.0f);

    // 全てのエフェクトデータを解放（シーン切り替え時に呼ぶ）
    void Clear();

    // ==========================================
    // 再生・停止制御
    // ==========================================
    // 再生（戻り値として「再生中のID」が返ってくるので、それを変数に保存しておく）
    int Play(const std::string& key, const VECTOR& pos);

    // 特定のエフェクトを強制停止
    void Stop(int playingId);

    // そのエフェクトがまだ再生中かどうかを確認する（消滅判定に便利！）
    bool IsPlaying(int playingId);

    // ==========================================
    // リアルタイム操作（再生中のエフェクトを操る）
    // ==========================================
    // 位置を更新（移動する魔法の弾に追従させる）
    void SetPos(int playingId, const VECTOR& pos);

    // 大きさを変更（1.0fが標準。ダメージ量に応じて巨大化させる等）
    void SetScale(int playingId, float scale);
    void SetScale(int playingId, const VECTOR& scale); // XYZ別々に伸ばす用

    // 回転を変更（剣の軌跡や、飛んでいく魔法の向きを合わせる）
    void SetRotation(int playingId, const VECTOR& rot);

    // 再生速度を変更（1.0fが標準。0.5fでスローモーション、2.0fで倍速！）
    void SetSpeed(int playingId, float speed);

    // ==========================================
    // 2Dエフェクト用（タイトル画面の花火など）
    // ==========================================
    int Play2D(const std::string& key, const VECTOR& pos);
    void SetScale2D(int playingId, float scale);
    void SetColor2D(int playingId, int r, int g, int b, int a);
    bool IsPlaying2D(int playingId);
    void Stop2D(int playingId);

private:
    EffectManager() = default;
    ~EffectManager() { Clear(); }

    // 読み込んだエフェクトの「大元データ」を記憶する辞書
    std::unordered_map<std::string, int> effectResourceMap_;
};