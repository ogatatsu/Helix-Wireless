/*
  The MIT License (MIT)

  Copyright (c) 2018 ogatatsu.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "keymap.h"
#include "Command.h"
#include "config.h"
#include "keycode.h"

/*------------------------------------------------------------------*/
/*  define short name command
 *------------------------------------------------------------------*/
// no operation
static Command *const NOP = NoOperation;

// normal key
static inline Command *NK(uint8_t keycode) { return (new NormalKey(keycode)); }

// modifier key
static inline Command *MO(Modifier modifier) { return (new ModifierKey(modifier)); }

// combination key
static inline Command *CK(Modifier modifier, uint8_t keycode) { return (new CombinationKey(modifier, keycode)); }

// modifier or keytap
static inline Command *MT(Modifier modifier, uint8_t keycode) { return (new ModifierTap(modifier, keycode)); }

// modifier or oneshot modifier
static inline Command *OSM(Modifier modifier) { return (new OneShotModifier(modifier)); }

// divide layer
template <typename... Commands>
static inline Command *L(Command *first, Commands... rest) {
    Command **arg = new Command *[LAYER_SIZE] { first, rest... };
    return (new DivideLayer(arg));
}

// transparent (_ * 7)
#define _______ (Transparent)

// layer or keytap
static inline Command *LT(uint8_t layerNumber, uint8_t keycode) { return (new LayerTap(layerNumber, keycode)); }

// toggle layer (alternate)
static inline Command *TL(uint8_t layerNumber) { return (new ToggleLayer(layerNumber)); }

// switch layer (momentary)
static inline Command *SL(uint8_t layerNumber) { return (new SwitchLayer(layerNumber)); }

// layer or oneshot layer
static inline Command *OSL(uint8_t layerNumber) { return (new OneShotLayer(layerNumber)); }

// detect multi press
template <typename... Commands>
static inline Command *MP(Command *first, Commands... rest) {
    Command **arg = new Command *[1 + sizeof...(rest)]{first, rest...};
    return (new DetectMultiPress(arg, 1 + sizeof...(rest)));
}

// tap or press
static inline Command *TOP(uint ms, Command *tapCommand, Command *pressCommand) { return (new TapOrPress(ms, tapCommand, pressCommand)); }

// consumer controll
static inline Command *CC(UsageCode usageCode) { return (new ConsumerControll(usageCode)); }

// mouse move
static inline Command *MS_MOV(int8_t x, int8_t y) { return (new MouseMove(x, y)); }

// mouse speed
static inline Command *MS_SPD(int percent) { return (new MouseSpeed(percent)); }

// mouse scroll
static inline Command *MS_SCR(int8_t scroll) { return (new MouseScroll(scroll)); }

// mouse pan
static inline Command *MS_PAN(int8_t pan) { return (new MousePan(pan)); }

// mouse click
static inline Command *MS_CLK(MouseButton button) { return (new MouseClick(button)); }

// switch sequence mode
static Command *const SEQ_MODE = SwitchSequenceMode;

// reset ble connection
static Command *const RESET = ResetConnection;

// macro
template <typename... MacroCommands>
static inline Command *MACRO(Macro::MacroCommand *first, MacroCommands... rest) {
    Macro::MacroCommand **arg = new Macro::MacroCommand *[1 + sizeof...(rest)]{first, rest...};
    return (new Macro(arg, 1 + sizeof...(rest)));
}

static inline Macro::MacroCommand *D(uint8_t keycode) { return (new Macro::DownKey(keycode)); }
static inline Macro::MacroCommand *D(Modifier modifier) { return (new Macro::DownModifier(modifier)); }
static inline Macro::MacroCommand *U(uint8_t keycode) { return (new Macro::UpKey(keycode)); }
static inline Macro::MacroCommand *U(Modifier modifier) { return (new Macro::UpModifier(modifier)); }
static inline Macro::MacroCommand *W(uint delay) { return (new Macro::Wait(delay)); }
#define T(keycode_or_modifier) D(keycode_or_modifier), W(TAP_SPEED), U(keycode_or_modifier), W(TAP_SPEED)

/*------------------------------------------------------------------*/
/*  define keymap
 *------------------------------------------------------------------*/
struct Key {
    uint8_t id;
    Command *command;
};

struct SimultaneousKey {
    uint8_t ids[MAX_SIMULTANEOUS_PRESS_COUNT];
    Command *command;
    uint idsLength;
};

struct SequenceKey {
    uint8_t ids[MAX_SEQUENCE_COUNT];
    Command *command;
    uint idsLength;
};

/* ID
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |   1  |   2  |   3  |   4  |   5  |   6  |             |   7  |   8  |   9  |  10  |  11  |  12  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  13  |  14  |  15  |  16  |  17  |  18  |             |  19  |  20  |  21  |  22  |  23  |  24  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  25  |  26  |  27  |  28  |  29  |  30  |             |  31  |  32  |  33  |  34  |  35  |  36  |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |  37  |  38  |  39  |  40  |  41  |  42  |  43  |  44  |  45  |  46  |  47  |  48  |  49  |  50  |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |  51  |  52  |  53  |  54  |  55  |  56  |  57  |  58  |  59  |  60  |  61  |  62  |  63  |  64  |
   * `-------------------------------------------------------------------------------------------------'
   */

/* Layer 0 (qwerty)
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |   `  |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  | Del  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * | Tab  |   Q  |   W  |   E  |   R  |   T  |             |   Y  |   U  |   I  |   O  |   P  | Bksp |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * | Ctrl |   A  |   S  |   D  |   F  |   G  |             |   H  |   J  |   K  |   L  |   ;  |  '   |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * | Shift|   Z  |   X  |   C  |   V  |   B  |   [  |   ]  |   N  |   M  |   ,  |   .  |   /  |Enter |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |  NOP | Esc  | Alt  | GUI  | EISU |Lower |Space |Space |Raise | KANA | Left | Down |  Up  |Right |
   * `-------------------------------------------------------------------------------------------------'
   */

/* Layer 1 (Lower)
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |   ~  |   !  |   @  |   #  |   $  |   %  |             |   ^  |   &  |   *  |   (  |   )  |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |   ~  |   !  |   @  |   #  |   $  |   %  |             |   ^  |   &  |   *  |   (  |   )  |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |  F1  |  F2  |  F3  |  F4  |  F5  |             |  F6  |   _  |   +  |   {  |   }  |  |   |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |      |  F7  |  F8  |  F9  |  F10 |  F11 |  (   |   )  |  F12 |      |      | Home | End  |      |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      | Next | Vol- | Vol+ | Play |
   * `-------------------------------------------------------------------------------------------------'
   */

/* Layer 2 (Raise)
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |   `  |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  | Bksp |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |   `  |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  | Del  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |  F1  |  F2  |  F3  |  F4  |  F5  |             |  F6  |   -  |   =  |   [  |   ]  |  \   |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |      |  F7  |  F8  |  F9  |  F10 |  F11 |      |      |  F12 |      |      |PageDn|PageUp|      |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      | Next | Vol- | Vol+ | Play |
   * `-------------------------------------------------------------------------------------------------'
   */

static const int LOWER = 1;
static const int RAISE = 2;

static Key keymap[] = {
    {1, L(NK(_GRAVE), CK(_SHIFT, _GRAVE), NK(_GRAVE))},
    {2, L(NK(_1), CK(_SHIFT, _1), NK(_1))},
    {3, L(NK(_2), CK(_SHIFT, _2), NK(_2))},
    {4, L(NK(_3), CK(_SHIFT, _3), NK(_3))},
    {5, L(NK(_4), CK(_SHIFT, _4), NK(_4))},
    {6, L(NK(_5), CK(_SHIFT, _5), NK(_5))},
    {7, L(NK(_6), CK(_SHIFT, _6), NK(_6))},
    {8, L(NK(_7), CK(_SHIFT, _7), NK(_7))},
    {9, L(NK(_8), CK(_SHIFT, _8), NK(_8))},
    {10, L(NK(_9), CK(_SHIFT, _9), NK(_9))},
    {11, L(NK(_0), CK(_SHIFT, _0), NK(_0))},
    {12, L(NK(_DELETE), _______, NK(_BACKSPACE))},

    {13, L(NK(_TAB), CK(_SHIFT, _GRAVE), NK(_GRAVE))},
    {14, L(NK(_Q), CK(_SHIFT, _1), NK(_1))},
    {15, L(NK(_W), CK(_SHIFT, _2), NK(_2))},
    {16, L(NK(_E), CK(_SHIFT, _3), NK(_3))},
    {17, L(NK(_R), CK(_SHIFT, _4), NK(_4))},
    {18, L(NK(_T), CK(_SHIFT, _5), NK(_5))},
    {19, L(NK(_Y), CK(_SHIFT, _6), NK(_6))},
    {20, L(NK(_U), CK(_SHIFT, _7), NK(_7))},
    {21, L(NK(_I), CK(_SHIFT, _8), NK(_8))},
    {22, L(NK(_O), CK(_SHIFT, _9), NK(_9))},
    {23, L(NK(_P), CK(_SHIFT, _0), NK(_0))},
    {24, L(NK(_BACKSPACE), _______, NK(_DELETE))},

    {25, L(MO(_CTRL), _______, _______)},
    {26, L(NK(_A), NK(_F1), NK(_F1))},
    {27, L(NK(_S), NK(_F2), NK(_F2))},
    {28, L(NK(_D), NK(_F3), NK(_F3))},
    {29, L(NK(_F), NK(_F4), NK(_F4))},
    {30, L(NK(_G), NK(_F5), NK(_F5))},
    {31, L(NK(_H), NK(_F6), NK(_F6))},
    {32, L(NK(_J), CK(_SHIFT, _MINUS), NK(_MINUS))},
    {33, L(NK(_K), CK(_SHIFT, _EQUAL), NK(_EQUAL))},
    {34, L(NK(_L), CK(_SHIFT, _BRACKET_LEFT), NK(_BRACKET_LEFT))},
    {35, L(NK(_SEMICOLON), CK(_SHIFT, _BRACKET_RIGHT), NK(_BRACKET_RIGHT))},
    {36, L(NK(_QUOTE), CK(_SHIFT, _BACKSLASH), NK(_BACKSLASH))},

    {37, L(MO(_SHIFT), _______, _______)},
    {38, L(NK(_Z), NK(_F7), NK(_F7))},
    {39, L(NK(_X), NK(_F8), NK(_F8))},
    {40, L(NK(_C), NK(_F9), NK(_F9))},
    {41, L(NK(_V), NK(_F10), NK(_F10))},
    {42, L(NK(_B), NK(_F11), NK(_F11))},
    {43, L(NK(_BRACKET_LEFT), CK(_SHIFT, _9), _______)},
    {44, L(NK(_BRACKET_RIGHT), CK(_SHIFT, _0), _______)},
    {45, L(NK(_N), NK(_F12), NK(_F12))},
    {46, L(NK(_M), _______, _______)},
    {47, L(NK(_COMMA), _______, _______)},
    {48, L(NK(_PERIOD), NK(_HOME), NK(_PAGE_DOWN))},
    {49, L(NK(_SLASH), NK(_END), NK(_PAGE_UP))},
    {50, L(NK(_ENTER), _______, _______)},

    {51, NOP},
    {52, L(NK(_ESCAPE), _______, _______)},
    {53, L(MO(_ALT), _______, _______)},
    {54, L(MO(_GUI), _______, _______)},
    {55, L(NK(_INT5), _______, _______)},
    {56, SL(LOWER)},
    {57, L(NK(_SPACE), _______, _______)},
    {58, L(NK(_SPACE), _______, _______)},
    {59, SL(RAISE)},
    {60, L(NK(_INT4), _______)},
    {61, L(NK(_ARROW_LEFT), CC(_NEXT), CC(_NEXT))},
    {62, L(NK(_ARROW_DOWN), CC(_VOLUME_UP), CC(_VOLUME_UP))},
    {63, L(NK(_ARROW_UP), CC(_VOLUME_DOWN), CC(_VOLUME_DOWN))},
    {64, L(NK(_ARROW_RIGHT), CC(_PLAY_PAUSE), CC(_PLAY_PAUSE))},
};

static SimultaneousKey simultaneousKeymap[] = {
    {{51, 56, 59}, TOP(2000, NOP, RESET)},
};

static SequenceKey sequenceKeymap[] = {

};

/*------------------------------------------------------------------*/
/*  define function
 *------------------------------------------------------------------*/
static uint validLength(const uint8_t ids[], uint maxLength) {
    int i = 0;
    for (; i < maxLength; i++) {
        if (ids[i] == 0) {
            break;
        }
    }
    return i;
}

void initKeymap(BLEHidAdafruit &blehid) {
    Command::init(blehid);

    for (int i = 0; i < arrcount(simultaneousKeymap); i++) {
        simultaneousKeymap[i].idsLength = validLength(simultaneousKeymap[i].ids, MAX_SIMULTANEOUS_PRESS_COUNT);
    }
    for (int i = 0; i < arrcount(sequenceKeymap); i++) {
        sequenceKeymap[i].idsLength = validLength(sequenceKeymap[i].ids, MAX_SEQUENCE_COUNT);
    }
}

// idsがsequenceKeymapの定義とマッチするか調べる
// =>0 マッチしない
// =>1 部分マッチ
// =>2 完全にマッチ、完全にマッチした場合はmatchedにマッチしたSequenceKeyを入れて返す
static uint matchSequence(const uint8_t ids[], uint len, SequenceKey **matched) {
    for (int i = 0; i < arrcount(sequenceKeymap); i++) {
        uint minLen = min(len, sequenceKeymap[i].idsLength);
        if (memcmp(ids, sequenceKeymap[i].ids, minLen) == 0) {
            if (len == sequenceKeymap[i].idsLength) {
                *matched = &sequenceKeymap[i];
                return 2;
            }
            return 1;
        }
    }
    return 0;
}

void applyToKeymap(const UInt8Set &ids) {
    static UInt8Set prevIDs, pressedInMatchModeIDs;
    static uint8_t seq[MAX_SEQUENCE_COUNT];
    static uint seqLen = 0;
    static SequenceKey *matched;

    // SEQ_MODE_MATCH内で押されたIDのリリースを監視する
    if (pressedInMatchModeIDs.count() != 0) {
        // １つ前のIDs - 現在のIDs = リリースされたIDs
        UInt8Set releaseIDs = prevIDs - ids;
        pressedInMatchModeIDs -= releaseIDs;
    }

    // apply to normal keymap
    for (int i = 0; i < arrcount(keymap); i++) {
        // SEQ_MODE_MATCH内で押されたIDなら何もしない
        if (pressedInMatchModeIDs.contains(keymap[i].id)) {
            continue;
        }
        // IDが押されているかを取得
        bool pressed = ids.contains(keymap[i].id);
        // SEQ_MODE_MATCHの時はリリースのみ許可する
        if ((pressed == true) && (sequenceModeState == SEQ_MODE_MATCH)) {
            continue;
        }
        // コマンドに現在の状態を適用する
        keymap[i].command->apply(pressed);
    }

    // apply to simultaneous keymap
    for (int i = 0; i < arrcount(simultaneousKeymap); i++) {
        // SEQ_MODE_MATCH内で押されたIDが含まれていたら何もしない
        if (pressedInMatchModeIDs.containsAny(simultaneousKeymap[i].ids, simultaneousKeymap[i].idsLength)) {
            continue;
        }
        // 全てのIDが押されているかを取得
        bool pressed = ids.containsAll(simultaneousKeymap[i].ids, simultaneousKeymap[i].idsLength);
        // SEQ_MODE_MATCHの時はリリースのみ許可する
        if ((pressed == true) && (sequenceModeState == SEQ_MODE_MATCH)) {
            continue;
        }
        // コマンドに現在の状態を適用する
        simultaneousKeymap[i].command->apply(pressed);
    }

    // apply to sequence keymap
    if (sequenceModeState == SEQ_MODE_START) {
        // マッチングをするのは次の入力から
        sequenceModeState = SEQ_MODE_MATCH;
    } else if (sequenceModeState == SEQ_MODE_MATCH) {
        // 現在のIDs - １つ前のIDs = 新しく押されたIDs
        UInt8Set newPressIDs = ids - prevIDs;

        uint len = newPressIDs.count();
        uint8_t buf[len];
        newPressIDs.toArray(buf);
        // SEQ_MODE_MATCHに移行してから押された全てのIDを入れておく
        int i = 0;
        while ((seqLen < MAX_SEQUENCE_COUNT) && (i < len)) {
            seq[seqLen++] = buf[i++];
        }
        // SEQ_MODE_MATCHに移行してから押された全てのIDとsequenceKeymapを比較してIDの順番がマッチする定義があるか調べる
        int matchResult = matchSequence(seq, seqLen, &matched);
        if (matchResult == 0) { // マッチしなければシーケンスモードを解除
            seqLen = 0;
            sequenceModeState = SEQ_MODE_DISABLE;
        } else if (matchResult == 1) { // 部分マッチならば何もしない
            // pass
        } else if (matchResult == 2) { // 完全マッチしたらコマンドを実行してSEQ_MODE_KEY_RELEASEに移行
            seqLen = 0;
            matched->command->apply(true);
            sequenceModeState = SEQ_MODE_KEY_RELEASE;
        }
        // SEQ_MODE_MATCH内で押されたIDは１回リリースされるまではコマンドを実行しない
        // そのためリリースを監視する必要があるので追加していく
        pressedInMatchModeIDs |= newPressIDs;

    } else if (sequenceModeState == SEQ_MODE_KEY_RELEASE) {
        // SEQ_MODE_MATCHで実行したコマンドを解除するためにキーアップを監視する
        // 最後のキーがリリースされたら解除する
        if (ids.contains(matched->ids[matched->idsLength - 1]) == false) {
            matched->command->apply(false);
            sequenceModeState = SEQ_MODE_DISABLE;
        }
    }
    prevIDs = ids;
}
