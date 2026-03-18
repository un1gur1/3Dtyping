#pragma once
#include <DxLib.h>
#include <vector>
#include <string>

class Loading {
private:
    static constexpr int MIN_LOAD_TIME = 90; // 少し長めの1.5秒に設定（Tipsを読ませるため）

public:
    Loading();
    ~Loading();

    void Init(void);
    void Load(void);
    void Update(void);
    void Draw(void);
    void Release(void);

    void StartAsyncLoad(void);
    void EndAsyncLoad(void);

    bool IsLoading(void) const { return isLoading_; }

private:
    int handle_;
    bool isLoading_;
    int loadTimer_;

    // 演出用の変数
    float angle_;       // ぐるぐる回るアイコン用
    int dotCount_;      // "LOADING..." のドット数
    int initialLoadNum_;// 最初に読み込む総数（進捗計算用）

    // Tips管理
    std::vector<std::string> tips_;
    int currentTipIdx_;
};