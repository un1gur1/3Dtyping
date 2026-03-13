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

// ヘルパー: UTF-8 の先頭バイトからコードポイントのバイト長を返す（簡易判定）
static int Utf8CharLen(unsigned char lead) {
    if ((lead & 0x80) == 0x00) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 1; // 異常時は1で進める
}

// 1文字ずつ追加して変換
void RomanjiConverter::addInput(char c) {
    inputBuffer_ += c;
    tryConvert();
}

std::string RomanjiConverter::convert(const std::string& roman) {
    clear();

    // 全角数字マップ（UTF-8）
    static const std::string fullwidthDigits[10] = {
        "０", "１", "２", "３", "４", "５", "６", "７", "８", "９"
    };

    size_t i = 0;
    while (i < roman.size()) {
        unsigned char uc = static_cast<unsigned char>(roman[i]);

        if (uc < 128) {
            // ASCII処理

            // 数字 → 全角数字へ
            if (uc >= '0' && uc <= '9') {
                tryConvert();
                // 変換しきれなかったバッファ（単独の子音など）がある場合は強制的に出力してクリア
                if (!inputBuffer_.empty()) {
                    outputString_.append(inputBuffer_);
                    inputBuffer_.clear();
                }
                outputString_.append(fullwidthDigits[uc - '0']);
                ++i;
                continue;
            }

            // ハイフン '-' を長音符「ー」へ
            if (uc == '-') {
                tryConvert();
                // 変換しきれなかったバッファがある場合は強制的に出力してクリア
                if (!inputBuffer_.empty()) {
                    outputString_.append(inputBuffer_);
                    inputBuffer_.clear();
                }
                outputString_.append("ー");
                ++i;
                continue;
            }

            // 英字・アポストロフィ(') はローマ字バッファへ（小文字化して追加）
            if (std::isalpha(uc) || uc == '\'') {
                unsigned char lowered = static_cast<unsigned char>(std::tolower(uc));
                addInput(static_cast<char>(lowered));
                ++i;
                continue;
            }

            // その他のASCII記号
            tryConvert();
            if (!inputBuffer_.empty()) {
                outputString_.append(inputBuffer_);
                inputBuffer_.clear();
            }
            outputString_.push_back(static_cast<char>(uc));
            ++i;
        }
        else {
            // 非ASCII (UTF-8) シーケンス
            int len = Utf8CharLen(uc);
            if (i + len <= roman.size()) {
                // ★UTF-8文字を追加する前に、溜まっているバッファを処理する
                tryConvert();
                if (!inputBuffer_.empty()) {
                    // 変換しきれないアルファベットが残っていればそのままくっつける
                    outputString_.append(inputBuffer_);
                    inputBuffer_.clear();
                }

                outputString_.append(roman.substr(i, len));
                i += len;
                continue;
            }
            else {
                // 不正な末端シーケンス
                tryConvert();
                if (!inputBuffer_.empty()) {
                    outputString_.append(inputBuffer_);
                    inputBuffer_.clear();
                }
                outputString_.append(roman.substr(i));
                break;
            }
        }
    }

    // 残ったローマ字バッファを最終フラッシュ
    tryConvert();
    if (!inputBuffer_.empty()) {
        outputString_.append(inputBuffer_);
        inputBuffer_.clear();
    }

    return outputString_;
}// 促音判定用
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
        // 長い順に辞書マッチ（長い順に並んだ辞書で対応）
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
    // 長い順で網羅的に列挙（拗音・小書き・外来音も含む）
    std::vector<std::pair<std::string, std::string>> dict = {
        // 4文字（小書き/拗音の特殊表記）
        {"ltsu","っ"}, {"xtsu","っ"}, {"tcha","っちゃ"}, {"tchu","っちゅ"}, {"tcho","っちょ"},
        // 3文字（主要な拗音・複合）
        {"kya","きゃ"}, {"kyu","きゅ"}, {"kyo","きょ"},
        {"gya","ぎゃ"}, {"gyu","ぎゅ"}, {"gyo","ぎょ"},
        {"sha","しゃ"}, {"shu","しゅ"}, {"sho","しょ"},
        {"sya","しゃ"}, {"syu","しゅ"}, {"syo","しょ"},
        {"ja","じゃ"}, {"ju","じゅ"}, {"jo","じょ"},
        {"jya","じゃ"}, {"jyu","じゅ"}, {"jyo","じょ"},
        {"cha","ちゃ"}, {"chu","ちゅ"}, {"cho","ちょ"},
        {"tya","ちゃ"}, {"tyu","ちゅ"}, {"tyo","ちょ"},
        {"cya","ちゃ"}, {"cyu","ちゅ"}, {"cyo","ちょ"},
        {"nya","にゃ"}, {"nyu","にゅ"}, {"nyo","にょ"},
        {"hya","ひゃ"}, {"hyu","ひゅ"}, {"hyo","ひょ"},
        {"bya","びゃ"}, {"byu","びゅ"}, {"byo","びょ"},
        {"pya","ぴゃ"}, {"pyu","ぴゅ"}, {"pyo","ぴょ"},
        {"mya","みゃ"}, {"myu","みゅ"}, {"myo","みょ"},
        {"rya","りゃ"}, {"ryu","りゅ"}, {"ryo","りょ"},
        {"fya","ふゃ"}, {"fyu","ふゅ"}, {"fyo","ふょ"},
        {"vya","ゔゃ"}, {"vyu","ゔゅ"}, {"vyo","ゔょ"},
        // 2文字 y-row with vowel-like combos
        {"kwi","くぃ"}, {"kwe","くぇ"}, {"gwi","ぐぃ"}, {"gwe","ぐぇ"},
        // 3文字: additional foreign sounds
        {"she","しぇ"}, {"je","じぇ"}, {"jea","じぇあ"}, {"che","ちぇ"},
        {"tsa","つぁ"}, {"tsi","つぃ"}, {"tse","つぇ"}, {"tso","つぉ"},
        {"dja","ぢゃ"}, {"dju","ぢゅ"}, {"djo","ぢょ"},
        // 2文字（濁音拗音含む） - many are covered below as 2-letter entries
        // 小書き拡張（3文字/2文字）
        {"xya","ゃ"}, {"xyu","ゅ"}, {"xyo","ょ"},
        {"lya","ゃ"}, {"lyu","ゅ"}, {"lyo","ょ"},
        {"xtu","っ"}, {"xtsu","っ"}, {"ltu","っ"},
        {"xa","ぁ"}, {"xi","ぃ"}, {"xu","ぅ"}, {"xe","ぇ"}, {"xo","ぉ"},
        {"la","ぁ"}, {"li","ぃ"}, {"lu","ぅ"}, {"le","ぇ"}, {"lo","ぉ"},

        // 2文字（外来音・濁音・半濁音・拗音代替）
        {"ka","か"}, {"ki","き"}, {"ku","く"}, {"ke","け"}, {"ko","こ"},
        {"ga","が"}, {"gi","ぎ"}, {"gu","ぐ"}, {"ge","げ"}, {"go","ご"},
        {"sa","さ"}, {"si","し"}, {"shi","し"}, {"su","す"}, {"se","せ"}, {"so","そ"},
        {"za","ざ"}, {"zi","じ"}, {"ji","じ"}, {"zu","ず"}, {"ze","ぜ"}, {"zo","ぞ"},
        {"ta","た"}, {"ti","ち"}, {"chi","ち"}, {"tu","つ"}, {"tsu","つ"}, {"te","て"}, {"to","と"},
        {"da","だ"}, {"di","ぢ"}, {"du","づ"}, {"de","で"}, {"do","ど"},
        {"na","な"}, {"ni","に"}, {"nu","ぬ"}, {"ne","ね"}, {"no","の"},
        {"ha","は"}, {"hi","ひ"}, {"fu","ふ"}, {"hu","ふ"}, {"he","へ"}, {"ho","ほ"},
        {"ba","ば"}, {"bi","び"}, {"bu","ぶ"}, {"be","べ"}, {"bo","ぼ"},
        {"pa","ぱ"}, {"pi","ぴ"}, {"pu","ぷ"}, {"pe","ぺ"}, {"po","ぽ"},
        {"ma","ま"}, {"mi","み"}, {"mu","む"}, {"me","め"}, {"mo","も"},
        {"ya","や"}, {"yu","ゆ"}, {"yo","よ"},
        {"ra","ら"}, {"ri","り"}, {"ru","る"}, {"re","れ"}, {"ro","ろ"},
        {"wa","わ"}, {"wo","を"}, {"nn","ん"}, {"n'","ん"},
        {"n","ん"},

        // vowels
        {"a","あ"}, {"i","い"}, {"u","う"}, {"e","え"}, {"o","お"},

        // small combinations and common English-origin sounds
        {"fa","ふぁ"}, {"fi","ふぃ"}, {"fe","ふぇ"}, {"fo","ふぉ"},
        {"va","ゔぁ"}, {"vi","ゔぃ"}, {"vu","ゔ"}, {"ve","ゔぇ"}, {"vo","ゔぉ"},
        {"qa","くぁ"}, {"qi","くぃ"}, {"qe","くぇ"}, {"qo","くぉ"},
        {"kwa","くぁ"}, {"gwa","ぐぁ"},
        {"she","しぇ"}, {"je","じぇ"}, {"je","じぇ"},
        {"tyu","ちゅ"}, {"dzu","づ"}, {"dji","ぢ"}, {"dju","ぢゅ"},

        // wi/we (古い表記)
        {"wi","うぃ"}, {"we","うぇ"}, {"wu","うぅ"},

        // common combos for foreign sounds
        {"xa","ぁ"}, {"xi","ぃ"}, {"xu","ぅ"}, {"xe","ぇ"}, {"xo","ぉ"},
        {"wi","うぃ"}, {"we","うぇ"}, {"wo","を"},

        // others: map single letters already included, but ensure coverage for 'y' combinations
        {"by","びゃ"}, {"byu","びゅ"}, {"byo","びょ"},
        {"my","みゃ"}, {"myu","みゅ"}, {"myo","みょ"},
        {"py","ぴゃ"}, {"pyu","ぴゅ"}, {"pyo","ぴょ"},
        {"sy","しゃ"}, {"syu","しゅ"}, {"syo","しょ"},
        {"ty","ちゃ"}, {"tyu","ちゅ"}, {"tyo","ちょ"},

        // fallback common two-letter combos (ensure no missing basic pairs)
        {"ka","か"}, {"ki","き"}, {"ku","く"}, {"ke","け"}, {"ko","こ"},
        {"sa","さ"}, {"su","す"}, {"se","せ"}, {"so","そ"},
        {"ta","た"}, {"te","て"}, {"to","と"},
        {"na","な"}, {"ni","に"}, {"nu","ぬ"}, {"ne","ね"}, {"no","の"},
        {"ha","は"}, {"hi","ひ"}, {"he","へ"}, {"ho","ほ"},
        {"ma","ま"}, {"mi","み"}, {"mu","む"}, {"me","め"}, {"mo","も"},
        {"ya","や"}, {"yu","ゆ"}, {"yo","よ"},
        {"ra","ら"}, {"ri","り"}, {"ru","る"}, {"re","れ"}, {"ro","ろ"},
    };

    // 重複を避けつつ、より長いキーを優先するため長さ順にソートする
    std::sort(dict.begin(), dict.end(), [](const auto& a, const auto& b) {
        if (a.first.size() != b.first.size()) return a.first.size() > b.first.size();
        return a.first < b.first;
        });
    // 辞書を設定
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