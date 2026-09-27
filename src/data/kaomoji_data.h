#pragma once
#include <cstddef>

namespace data {

struct KaomojiEntry {
    const char* text;
    const char* label;
    const char* category;
};

// Curated compile-time kaomoji table — no network required.
// NOLINTBEGIN(modernize-avoid-c-arrays)
inline constexpr KaomojiEntry kKaomoji[] = {
    // ── Happy ────────────────────────────────────────────────────────────
    {"(^_^)",               "happy",                    "Happy"},
    {"(^o^)",               "joyful",                   "Happy"},
    {"(≧▽≦)",              "overjoyed",                "Happy"},
    {"(ﾉ◕ヮ◕)ﾉ",           "excited",                  "Happy"},
    {"(✿◠‿◠)",             "cheerful",                 "Happy"},
    {"ヽ(^o^)ノ",           "hooray",                   "Happy"},
    {"(◡‿◡✿)",             "content",                  "Happy"},
    {"(*^▽^*)",             "delighted",                "Happy"},
    {"(。♥‿♥。)",           "blissful",                 "Happy"},
    {"(＾▽＾)",             "grinning",                 "Happy"},
    {"(。◕‿◕。)",           "cheerful face",            "Happy"},
    {"(*≧ω≦*)",             "gleeful",                  "Happy"},
    {"(^▽^)",               "smiling",                  "Happy"},
    {"☆*:.｡.o(≧▽≦)o.｡.:*☆", "ecstatic",              "Happy"},
    {"(ﾉ^ヮ^)ﾉ",           "overjoyed raising hands",  "Happy"},

    // ── Sad ──────────────────────────────────────────────────────────────
    {"(T_T)",               "crying",                   "Sad"},
    {"(._.) ",              "downcast",                 "Sad"},
    {"(。•́︿•̀。)",          "sad face",                 "Sad"},
    {"(╥_╥)",               "weeping",                  "Sad"},
    {"(ó_ò)",               "unhappy",                  "Sad"},
    {"(っ˘̩╭╮˘̩)っ",         "sobbing",                  "Sad"},
    {"(ノ_<。)",             "dejected",                 "Sad"},
    {"(π_π)",               "very sad",                 "Sad"},
    {"(｡•́︿•̀｡)",           "pouty",                    "Sad"},
    {"(-_-)",               "gloomy",                   "Sad"},
    {"(´；ω；`)",           "on the verge of crying",   "Sad"},
    {"(；∀；)",              "embarrassed and sad",      "Sad"},

    // ── Angry ────────────────────────────────────────────────────────────
    {"(╯°□°）╯︵ ┻━┻",      "table flip",               "Angry"},
    {"(ಠ_ಠ)",               "disapproval",              "Angry"},
    {"(¬_¬)",               "annoyed",                  "Angry"},
    {"( ̄へ ̄)",              "hmph",                     "Angry"},
    {"(╬ ಠ益ಠ)",            "furious",                  "Angry"},
    {"ٹ(╬ʘ益ʘ╬)۶",         "enraged",                  "Angry"},
    {"(ง'̀-'́)ง",            "ready to fight",           "Angry"},
    {"(ノ°益°)ノ",            "slams down",               "Angry"},
    {"٩(ఠ益ఠ)۶",            "very angry",               "Angry"},
    {"凸(ಠ_ಠ)凸",           "double middle finger",     "Angry"},

    // ── Surprised ────────────────────────────────────────────────────────
    {"(°o°)",               "surprised",                "Surprised"},
    {"(☉_☉)",               "wide-eyed",                "Surprised"},
    {"(⊙_⊙)",               "stunned",                  "Surprised"},
    {"Σ(°ロ°)",              "gasping",                  "Surprised"},
    {"( ; ロ ; )",           "shocked",                  "Surprised"},
    {"(゜o゜)",              "oh my",                    "Surprised"},
    {"Σ(っ °Д °;)っ",        "panic",                    "Surprised"},
    {"(⊙ω⊙)",               "astonished",               "Surprised"},
    {"⊙.☉",                 "staring",                  "Surprised"},
    {"(✧ω✧)",               "sparkly eyes",             "Surprised"},
    {"∑(O_O;)",              "alarm",                    "Surprised"},

    // ── Love ─────────────────────────────────────────────────────────────
    {"(◕‿◕)♡",              "loving",                   "Love"},
    {"(♡°▽°♡)",             "in love",                  "Love"},
    {"(づ｡◕‿‿◕｡)づ",         "giving a hug",             "Love"},
    {"♡\\(^‿^\\)",           "hearts",                   "Love"},
    {"(≧◡≦)",               "adoring",                  "Love"},
    {"(●♡∀♡)",              "loving eyes",              "Love"},
    {"(/◕ヮ◕)/  ❣",          "celebrating love",         "Love"},
    {"(♥ω♥*)",              "fond",                     "Love"},
    {"(人*´∀｀)",            "warmth",                   "Love"},
    {"(*´∀｀*)",             "sweet affection",          "Love"},

    // ── Cute / Animals ───────────────────────────────────────────────────
    {"=^.^=",               "cat",                      "Cute"},
    {"(>^_^)>",             "hugging",                  "Cute"},
    {"/\\_/\\",             "cat crouching",            "Cute"},
    {"(ฅ^•ﻌ•^ฅ)",           "cat pawing",               "Cute"},
    {"UwU",                 "cute",                     "Cute"},
    {"OwO",                 "curious",                  "Cute"},
    {">_<",                 "squeamish",                "Cute"},
    {"(•ω•)",               "cute neutral",             "Cute"},
    {"(´・ω・`)",            "pouting cute",             "Cute"},
    {"（´。• ᵕ •。｀）",     "baby face",                "Cute"},
    {"(ᵔᴥᵔ)",               "puppy",                    "Cute"},
    {"(●´ω｀●)",             "shy cute",                 "Cute"},

    // ── Sleepy / Tired ───────────────────────────────────────────────────
    {"(-_-)zzz",            "sleeping",                 "Tired"},
    {"(¬‿¬)",               "sly",                      "Tired"},
    {"(￣_￣)zzz",           "napping",                  "Tired"},
    {"(-.-)zzZZ",           "dozing off",               "Tired"},
    {"(ᴗ˳ᴗ)",               "content and sleepy",       "Tired"},

    // ── Misc / Actions ───────────────────────────────────────────────────
    {"¯\\_(ツ)_/¯",          "shrug",                    "Misc"},
    {"┬─┬ノ(ò_óノ)",         "fixing the table",         "Misc"},
    {"(/≧▽≦)/",             "celebration",              "Misc"},
    {"(งツ)ว",              "flexing",                  "Misc"},
    {"≧◔◡◔≦",               "pleased",                  "Misc"},
    {"(⌐■_■)",              "cool",                     "Misc"},
    {"≖‿≖",                 "smug",                     "Misc"},
    {"(ᗒᗨᗕ)",               "goofy grin",               "Misc"},
    {"( •_•)>⌐■-■",         "putting on sunglasses",    "Misc"},
    {"(҂◡_◡) ᕤ",            "tough guy",                "Misc"},
    {"ヾ(⌐■_■)ノ♪",          "cool and musical",         "Misc"},
    {"〜(꒪꒳꒪)〜",            "dizzy wandering",          "Misc"},
};
// NOLINTEND(modernize-avoid-c-arrays)

inline constexpr std::size_t kKaomojiCount = std::size(kKaomoji);

} // namespace data
