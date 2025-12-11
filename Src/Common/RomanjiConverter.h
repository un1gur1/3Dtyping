#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class RomanjiConverter {
public:
    RomanjiConverter();
    // 1文字ずつ追加して変換
    void addInput(char c);
    // 変換済みひらがな取得
    const std::string& getOutput() const;
    // バッファクリア
    void clear();
    // 入力バッファ取得
    const std::string& getBuffer() const;
    // テスト用: 一括変換
    std::string convert(const std::string& roman);

private:
    std::string inputBuffer_;
    std::string outputString_;
    // ローマ字→ひらがな辞書（長い順優先）
    std::vector<std::pair<std::string, std::string>> romanDict_;
    // 促音判定
    bool isConsonant(char c) const;
    // 辞書初期化
    void initDict();
    // バッファ変換
    void tryConvert();
};

class TypingJudge {
public:
    // お手本と入力済みひらがなを比較
    static int getMatchPos(const std::string& target, const std::string& input);
    // 完成判定
    static bool isComplete(const std::string& target, const std::string& input);
    // ミス判定
    static bool isMiss(const std::string& target, const std::string& input);
};
