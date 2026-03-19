#include "EffectManager.h"

void EffectManager::Load(const std::string& key, const std::string& filePath, float magnification) {
    // 既に読み込み済みのキーならスキップ（二重読み込み防止）
    if (effectResourceMap_.find(key) != effectResourceMap_.end()) return;

    // Effekseerデータのロード
    int handle = LoadEffekseerEffect(filePath.c_str(), magnification);
    if (handle != -1) {
        effectResourceMap_[key] = handle;
    }
    else {
        // 読み込み失敗時のログ（パス間違いに気づきやすくなります）
        AppLogAdd("【エラー】エフェクトの読み込みに失敗: %s\n", filePath.c_str());
    }
}

void EffectManager::Clear() {
    // 辞書に登録された全てのエフェクトをメモリから解放
    for (auto& pair : effectResourceMap_) {
        DeleteEffekseerEffect(pair.second);
    }
    effectResourceMap_.clear();
}

int EffectManager::Play(const std::string& key, const VECTOR& pos) {
    // 辞書にないエフェクトを再生しようとしたら失敗（-1）を返す
    if (effectResourceMap_.find(key) == effectResourceMap_.end()) return -1;

    // 再生して、その固有の「再生ID」を取得
    int playingId = PlayEffekseer3DEffect(effectResourceMap_[key]);

    // 初期位置を設定
    if (playingId != -1) {
        SetPosPlayingEffekseer3DEffect(playingId, pos.x, pos.y, pos.z);
    }


    return playingId;
}

void EffectManager::Stop(int playingId) {
    if (playingId != -1) {
        StopEffekseer3DEffect(playingId);
    }
}

bool EffectManager::IsPlaying(int playingId) {
    if (playingId == -1) return false;
    return IsEffekseer3DEffectPlaying(playingId) == true;
}
void EffectManager::SetPos(int playingId, const VECTOR& pos) {
    if (playingId != -1) {
        SetPosPlayingEffekseer3DEffect(playingId, pos.x, pos.y, pos.z);
    }
}

void EffectManager::SetScale(int playingId, float scale) {
    if (playingId != -1) {
        SetScalePlayingEffekseer3DEffect(playingId, scale, scale, scale);
    }
}

void EffectManager::SetScale(int playingId, const VECTOR& scale) {
    if (playingId != -1) {
        SetScalePlayingEffekseer3DEffect(playingId, scale.x, scale.y, scale.z);
    }
}

void EffectManager::SetRotation(int playingId, const VECTOR& rot) {
    if (playingId != -1) {
        SetRotationPlayingEffekseer3DEffect(playingId, rot.x, rot.y, rot.z);
    }
}

void EffectManager::SetSpeed(int playingId, float speed) {
    if (playingId != -1) {
        SetSpeedPlayingEffekseer3DEffect(playingId, speed);
    }
}

// ==========================================
// 2Dエフェクト用実装
// ==========================================
int EffectManager::Play2D(const std::string& key, const VECTOR& pos) {
    if (effectResourceMap_.find(key) == effectResourceMap_.end()) return -1;
    int playingId = PlayEffekseer2DEffect(effectResourceMap_[key]);
    if (playingId != -1) {
        SetPosPlayingEffekseer2DEffect(playingId, pos.x, pos.y, pos.z);
    }
    return playingId;
}

void EffectManager::SetScale2D(int playingId, float scale) {
    if (playingId != -1) SetScalePlayingEffekseer2DEffect(playingId, scale, scale, scale);
}

void EffectManager::SetColor2D(int playingId, int r, int g, int b, int a) {
    if (playingId != -1) SetColorPlayingEffekseer2DEffect(playingId, r, g, b, a);
}

bool EffectManager::IsPlaying2D(int playingId) {
    if (playingId == -1) return false;
    return IsEffekseer2DEffectPlaying(playingId) == TRUE; // 再生中ならTRUE
}

void EffectManager::Stop2D(int playingId) {
    if (playingId != -1) StopEffekseer2DEffect(playingId);
}