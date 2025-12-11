#include "RomanjiConverter.h"
#include <iostream>
#include <algorithm>

RomanjiConverter::RomanjiConverter() {
    initDict();
    clear();
}

void RomanjiConverter::clear() {
    inputBuffer_.clear();
    outputString_.clear();
}

const std::string& RomanjiConverter::getOutput() const {
    return outputString_;
}

const std::string& RomanjiConverter::getBuffer() const {
    return inputBuffer_;
}

// 1文字ずつ追加して変換
void RomanjiConverter::addInput(char c) {
    inputBuffer_ += c;
    tryConvert();
}

// 一括変換（テスト用）
std::string RomanjiConverter::convert(const std::string& roman) {
    clear();
    for (char c : roman) addInput(c);
    return outputString_;
}

// 促音判定用
bool RomanjiConverter::isConsonant(char c) const {
    return std::string("bcdfghjklmnpqrstvwxyz").find(c) != std::string::npos;
}

// バッファ変換（3→2→1文字の順でマッチ）
void RomanjiConverter::tryConvert() {
    while (!inputBuffer_.empty()) {
        // 促音（っ）判定
        if (inputBuffer_.size() >= 2) {
            char c1 = inputBuffer_[0], c2 = inputBuffer_[1];
            if (isConsonant(c1) && c1 == c2 && c1 != 'n') {
                outputString_ += "っ";
                inputBuffer_.erase(0, 1);
                continue;
            }
        }
        // 撥音（ん）判定
        if (inputBuffer_.size() >= 2) {
            if (inputBuffer_.substr(0, 2) == "nn" || inputBuffer_.substr(0, 2) == "n'") {
                outputString_ += "ん";
                inputBuffer_.erase(0, 2);
                continue;
            }
            if (inputBuffer_[0] == 'n' && isConsonant(inputBuffer_[1]) && inputBuffer_[1] != 'y') {
                outputString_ += "ん";
                inputBuffer_.erase(0, 1);
                continue;
            }
        }
        if (inputBuffer_.size() >= 1 && inputBuffer_[0] == 'n' && inputBuffer_.size() == 1) {
            // n単独は保留
            break;
        }
        // 長い順に辞書マッチ（3→2→1文字）
        bool matched = false;
        for (const auto& kv : romanDict_) {
            const std::string& roma = kv.first;
            if (inputBuffer_.size() >= roma.size() &&
                inputBuffer_.substr(0, roma.size()) == roma) {
                outputString_ += kv.second;
                inputBuffer_.erase(0, roma.size());
                matched = true;
                break;
            }
        }
        if (!matched) break;
    }
}

// --- 完全ローマ字→ひらがな辞書（長い順優先） ---
void RomanjiConverter::initDict() {
    romanDict_.clear();
    // 拗音・小書き・特殊・濁音・五十音・促音・撥音など網羅
    // 長い順（例: kyou, kyo, kya, shi, etc...）
    // 例示: ここでは一部のみ記載、実際は200行以上に拡張してください
    std::vector<std::pair<std::string, std::string>> dict = {
        // 拗音・小書き
        {"kya", "きゃ"}, {"kyu", "きゅ"}, {"kyo", "きょ"},
        {"sha", "しゃ"}, {"shu", "しゅ"}, {"sho", "しょ"},
        {"cha", "ちゃ"}, {"chu", "ちゅ"}, {"cho", "ちょ"},
        {"nya", "にゃ"}, {"nyu", "にゅ"}, {"nyo", "にょ"},
        {"hya", "ひゃ"}, {"hyu", "ひゅ"}, {"hyo", "ひょ"},
        {"mya", "みゃ"}, {"myu", "みゅ"}, {"myo", "みょ"},
        {"rya", "りゃ"}, {"ryu", "りゅ"}, {"ryo", "りょ"},
        {"gya", "ぎゃ"}, {"gyu", "ぎゅ"}, {"gyo", "ぎょ"},
        {"bya", "びゃ"}, {"byu", "びゅ"}, {"byo", "びょ"},
        {"pya", "ぴゃ"}, {"pyu", "ぴゅ"}, {"pyo", "ぴょ"},
        {"ja", "じゃ"}, {"ju", "じゅ"}, {"jo", "じょ"},
        // 小書き
        {"xa", "ぁ"}, {"xi", "ぃ"}, {"xu", "ぅ"}, {"xe", "ぇ"}, {"xo", "ぉ"},
        {"xya", "ゃ"}, {"xyu", "ゅ"}, {"xyo", "ょ"}, {"xtsu", "っ"}, {"xtu", "っ"},
        {"la", "ぁ"}, {"li", "ぃ"}, {"lu", "ぅ"}, {"le", "ぇ"}, {"lo", "ぉ"},
        {"lya", "ゃ"}, {"lyu", "ゅ"}, {"lyo", "ょ"}, {"ltu", "っ"}, {"ltsu", "っ"},
        // 促音（っ）→ tryConvertで自動生成
        // 撥音（ん）→ tryConvertで自動生成
        // 特殊
        {"shi", "し"}, {"si", "し"}, {"chi", "ち"}, {"ti", "ち"}, {"tsu", "つ"}, {"tu", "つ"},
        {"fu", "ふ"}, {"hu", "ふ"}, {"ji", "じ"}, {"zi", "じ"}, {"di", "ぢ"}, {"du", "づ"},
        {"wi", "うぃ"}, {"we", "うぇ"}, {"wo", "を"}, {"va", "ゔぁ"}, {"vi", "ゔぃ"}, {"vu", "ゔ"}, {"ve", "ゔぇ"}, {"vo", "ゔぉ"},
        // 五十音
        {"a", "あ"}, {"i", "い"}, {"u", "う"}, {"e", "え"}, {"o", "お"},
        {"ka", "か"}, {"ki", "き"}, {"ku", "く"}, {"ke", "け"}, {"ko", "こ"},
        {"sa", "さ"}, {"su", "す"}, {"se", "せ"}, {"so", "そ"},
        {"ta", "た"}, {"te", "て"}, {"to", "と"},
        {"na", "な"}, {"ni", "に"}, {"nu", "ぬ"}, {"ne", "ね"}, {"no", "の"},
        {"ha", "は"}, {"hi", "ひ"}, {"he", "へ"}, {"ho", "ほ"},
        {"ma", "ま"}, {"mi", "み"}, {"mu", "む"}, {"me", "め"}, {"mo", "も"},
        {"ya", "や"}, {"yu", "ゆ"}, {"yo", "よ"},
        {"ra", "ら"}, {"ri", "り"}, {"ru", "る"}, {"re", "れ"}, {"ro", "ろ"},
        {"wa", "わ"}, {"wo", "を"}, {"n", "ん"},
        // 濁音
        {"ga", "が"}, {"gi", "ぎ"}, {"gu", "ぐ"}, {"ge", "げ"}, {"go", "ご"},
        {"za", "ざ"}, {"ji", "じ"}, {"zu", "ず"}, {"ze", "ぜ"}, {"zo", "ぞ"},
        {"da", "だ"}, {"di", "ぢ"}, {"du", "づ"}, {"de", "で"}, {"do", "ど"},
        {"ba", "ば"}, {"bi", "び"}, {"bu", "ぶ"}, {"be", "べ"}, {"bo", "ぼ"},
        {"pa", "ぱ"}, {"pi", "ぴ"}, {"pu", "ぷ"}, {"pe", "ぺ"}, {"po", "ぽ"},
        // その他（200行以上に拡張してください）
        // ...（省略: 実際は全パターン網羅）
    };
    // 長い順でソート
    std::sort(dict.begin(), dict.end(), [](const auto& a, const auto& b) {
        return a.first.size() > b.first.size();
        });
    romanDict_ = dict;
}

// --- 判定システム ---
int TypingJudge::getMatchPos(const std::string& target, const std::string& input) {
    int len = std::min(target.size(), input.size());
    for (int i = 0; i < len; ++i) {
        if (target[i] != input[i]) return i;
    }
    return len;
}
bool TypingJudge::isComplete(const std::string& target, const std::string& input) {
    return target == input;
}
bool TypingJudge::isMiss(const std::string& target, const std::string& input) {
    int pos = getMatchPos(target, input);
    return pos < input.size();
}
