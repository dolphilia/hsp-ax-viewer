// $ clang hex.c IEEE754_binary_encoder/float.c -lm -o hex
// $ ./hex

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "IEEE754_binary_encoder/float.h"
#include "sjis_to_utf8/table_sjis.h"
#include "sjis_to_utf8/table_utf8.h"

#define HSPHED_BOOTOPT_DEBUGWIN  1        // 起動時にデバッグウインドウを表示する
#define HSPHED_BOOTOPT_WINHIDE   2        // 起動時ウインドウ非表示
#define HSPHED_BOOTOPT_DIRSAVE   4        // 起動時カレントディレクトリ変更なし
#define HSPHED_BOOTOPT_SAVER     0x100    // スクリーンセーバー
#define HSPHED_BOOTOPT_RUNTIME   0x1000   // 動的ランタイムを有効にする
#define HSPHED_BOOTOPT_NOMMTIMER 0x2000   // マルチメディアタイマーを無効にする
#define HSPHED_BOOTOPT_NOGDIP    0x4000   // GDI+による描画を無効にする
#define HSPHED_BOOTOPT_FLOAT32   0x8000   // 実数を32bit floatとして処理する
#define HSPHED_BOOTOPT_ORGRND    0x10000  // 標準の乱数発生を使用する

#define EXFLG_0 0x1000
#define EXFLG_1 0x2000
#define EXFLG_2 0x4000
#define EXFLG_3 0x8000
#define CSTYPE  0x0fff

// command type
#define TYPE_MARK       0 // 記号(code = 文字コード)
#define TYPE_VAR        1 // ユーザー定義変数(code = 変数ID)
#define TYPE_STRING     2 // 文字列(code = DSオフセット)
#define TYPE_DNUM       3 // 実数値(code = DSオフセット)
#define TYPE_INUM       4 // 整数値(code = 値)
#define TYPE_STRUCT     5 // モジュール変数・構造体(code = minfoID)
#define TYPE_XLABEL     6 // 未使用
#define TYPE_LABEL      7 // ラベル名(code = OTオフセット)
#define TYPE_INTCMD     8 // 内蔵命令(code = コマンドID)
#define TYPE_EXTCMD     9 // 拡張命令(code = コマンドID)
#define TYPE_EXTSYSVAR 10 // 拡張システム変数(code = コマンドID)
#define TYPE_CMPCMD    11 // 比較命令(code = コマンドID)
#define TYPE_MODCMD    12 // ユーザー命令/関数(code = コマンドID)
#define TYPE_INTFUNC   13 // 内蔵関数(code = コマンドID)
#define TYPE_SYSVAR    14 // 内蔵システム変数(code = コマンドID)
#define TYPE_PROGCMD   15 // プログラム制御命令(code = コマンドID)
#define TYPE_DLLFUNC   16 // DLL拡張命令/関数(code = コマンドID)
#define TYPE_DLLCTRL   17 // DLLコントロール命令(code = コマンドID)
#define TYPE_USERDEF   18 // HSP3拡張プラグイン命令(code = コマンドID)

#define TYPE_ERROR     -1
#define TYPE_CALCERROR -2

enum {
    CALCCODE_ADD = 0,
    CALCCODE_SUB,
    CALCCODE_MUL,
    CALCCODE_DIV,
    CALCCODE_MOD,
    CALCCODE_AND,
    CALCCODE_OR,
    CALCCODE_XOR,
    CALCCODE_EQ,
    CALCCODE_NE,
    CALCCODE_GT,
    CALCCODE_LT,
    CALCCODE_GTEQ,
    CALCCODE_LTEQ,
    CALCCODE_RR,
    CALCCODE_LR,
    CALCCODE_MAX
};

char* type_name_str[] = {
    "記号",
    "ユーザー変数",
    "文字列",
    "実数値",
    "整数値",
    "モジュール変数等",
    "未使用",
    "ラベル",
    "内蔵命令",
    "拡張命令",
    "拡張システム変数",
    "比較命令",
    "ユーザー命令関数",
    "内蔵関数",
    "内蔵システム変数",
    "制御命令",
    "DLL拡張命令関数",
    "DLL系命令",
    "HSP3拡張プラグイン命令",
    "",
};

// TYPE_INTCMD == 8
// HSP内部(コア)命令(code=コマンドID)
char* type_builtin_command[] = {
    "onexit",    //  0 | 0x00
    "onerror",   //  1 | 0x01
    "onkey",     //  2 | 0x02
    "onclick",   //  3 | 0x03
    "oncmd",     //  4 | 0x04
    "",          //  5 | 0x05
    "",          //  6 | 0x06
    "",          //  7 | 0x07
    "",          //  8 | 0x08
    "",          //  9 | 0x09
    "",          // 10 | 0x0A
    "",          // 11 | 0x0B
    "",          // 12 | 0x0C
    "",          // 13 | 0x0D
    "",          // 14 | 0x0E
    "",          // 15 | 0x0F
    "",          // 16 | 0x10
    "exist",     // 17 | 0x11
    "delete",    // 18 | 0x12
    "mkdir",     // 19 | 0x13
    "chdir",     // 20 | 0x14
    "dirlist",   // 21 | 0x15
    "bload",     // 22 | 0x16
    "bsave",     // 23 | 0x17
    "bcopy",     // 24 | 0x18
    "memfile",   // 25 | 0x19
    "poke",      // 26 | 0x1A
    "wpoke",     // 27 | 0x1B
    "lpoke",     // 28 | 0x1C
    "getstr",    // 29 | 0x1D
    "chdpm",     // 30 | 0x1E
    "memexpand", // 31 | 0x1F
    "memcpy",    // 32 | 0x20
    "memset",    // 33 | 0x21
    "notesel",   // 34 | 0x22
    "noteadd",   // 35 | 0x23
    "notedel",   // 36 | 0x24
    "noteload",  // 37 | 0x25
    "notesave",  // 38 | 0x26
    "randomize", // 39 | 0x27
    "noteunsel", // 40 | 0x28
    "noteget",   // 41 | 0x29
    "split",     // 42 | 0x2A
    "strrep",    // 43 | 0x2B
    "setease",   // 44 | 0x2C
    "sortval",   // 45 | 0x2D
    "sortstr",   // 46 | 0x2E
    "sortnote",  // 47 | 0x2F
    "sortget",   // 48 | 0x30
};

// TYPE_EXTCMD == 9
// HSP拡張(機種依存)命令(code=コマンドID)
char* type_extend_command[] = {
    "button",     //  0 | 0x00 (?)
    "chgdisp",    //  1 | 0x01
    "exec",       //  2 | 0x02
    "dialog",     //  3 | 0x03
    "",           //  4 | 0x04
    "",           //  5 | 0x05
    "",           //  6 | 0x06
    "",           //  7 | 0x07
    "mmload",     //  8 | 0x08
    "mmplay",     //  9 | 0x09
    "mmstop",     // 10 | 0x0A
    "mci",        // 11 | 0x0B
    "pset",       // 12 | 0x0C
    "pget",       // 13 | 0x0D
    "syscolor",   // 14 | 0x0E
    "mes",        // 15 | 0x0F | == print
    "title",      // 16 | 0x10
    "pos",        // 17 | 0x11
    "circle",     // 18 | 0x12
    "cls",        // 19 | 0x13
    "font",       // 20 | 0x14
    "sysfont",    // 21 | 0x15
    "objsize",    // 22 | 0x16
    "picload",    // 23 | 0x17
    "color",      // 24 | 0x18
    "palcolor",   // 25 | 0x19
    "palette",    // 26 | 0x1A
    "redraw",     // 27 | 0x1B
    "width",      // 28 | 0x1C
    "gsel",       // 29 | 0x1D
    "gcopy",      // 30 | 0x1E
    "gzoom",      // 31 | 0x1F
    "gmode",      // 32 | 0x20
    "bmpsave",    // 33 | 0x21
    "hsvcolor",   // 34 | 0x22
    "getkey",     // 35 | 0x23
    "listbox",    // 36 | 0x24
    "chkbox",     // 37 | 0x25
    "combox",     // 38 | 0x26
    "input",      // 39 | 0x27
    "mesbox",     // 40 | 0x28
    "buffer",     // 41 | 0x29
    "screen",     // 42 | 0x2A
    "bgscr",      // 43 | 0x2B
    "mouse",      // 44 | 0x2C
    "objsel",     // 45 | 0x2D
    "groll",      // 46 | 0x2E
    "line",       // 47 | 0x2F
    "clrobj",     // 48 | 0x30
    "boxf",       // 49 | 0x31
    "objprm",     // 50 | 0x32
    "objmode",    // 51 | 0x33
    "stick",      // 52 | 0x34
    "grect",      // 53 | 0x35
    "grotate",    // 54 | 0x36
    "gsquare",    // 55 | 0x37
    "gradf",      // 56 | 0x38
    "objimage",   // 57 | 0x39
    "objskip",    // 58 | 0x3A
    "objenable",  // 59 | 0x3B
    "celload",    // 60 | 0x3C
    "celdiv",     // 61 | 0x3D
    "celput",     // 62 | 0x3E
    "gfilter",    // 63 | 0x3F
    "setreq",     // 64 | 0x40
    "getreq",     // 65 | 0x41
    "mmvol",      // 66 | 0x42
    "mmpan",      // 67 | 0x43
    "mmstat",     // 68 | 0x44
    "mtlist",     // 69 | 0x45
    "mtinfo",     // 70 | 0x46
    "devinfo",    // 71 | 0x47
    "devinfoi",   // 72 | 0x48
    "devprm",     // 73 | 0x49
    "devcontrol", // 74 | 0x4A
    "httpload",   // 75 | 0x4B
    "httpinfo",   // 76 | 0x4C
    "objcolor",   // 77 | 0x4D
    "rgbcolor",   // 78 | 0x4E
    "viewcalc",   // 79 | 0x4F
    "layerobj",   // 80 | 0x50
    "",           // 81 | 0x51
    "",           // 82 | 0x52
    "",           // 83 | 0x53
    "",           // 84 | 0x54
    "",           // 85 | 0x55
    "",           // 86 | 0x56
    "",           // 87 | 0x57
    "",           // 88 | 0x58
    "",           // 89 | 0x59
    "",           // 90 | 0x5A
    "",           // 91 | 0x5B
    "celbitmap",  // 92 | 0x5C
    "gmulcolor",  // 93 | 0x5D
    "setcls",     // 94 | 0x5E
    "celputm",    // 95 | 0x5F
};

// 10
char* type_extra_system_var[] = {
    "mousex",    // 0x000
    "mousey",    // 0x001
    "mousew",    // 0x002
    "hwnd",      // 0x003
    "hinstance", // 0x004
    "hdc",       // 0x005
    "","","","","","","","","","", // 0x06 - 0x0F
    "","","","","","","","","","","","","","","","", // 0x10 - 0x1F
    "","","","","","","","","","","","","","","","", // 0x20 - 0x2F
    "","","","","","","","","","","","","","","","", // 0x30 - 0x3F
    "","","","","","","","","","","","","","","","", // 0x40 - 0x4F
    "","","","","","","","","","","","","","","","", // 0x50 - 0x5F
    "","","","","","","","","","","","","","","","", // 0x60 - 0x6F
    "","","","","","","","","","","","","","","","", // 0x70 - 0x7F
    "","","","","","","","","","","","","","","","", // 0x80 - 0x8F
    "","","","","","","","","","","","","","","","", // 0x90 - 0x9F
    "","","","","","","","","","","","","","","","", // 0xA0 - 0xAF
    "","","","","","","","","","","","","","","","", // 0xB0 - 0xBF
    "","","","","","","","","","","","","","","","", // 0xC0 - 0xCF
    "","","","","","","","","","","","","","","","", // 0xD0 - 0xDF
    "","","","","","","","","","","","","","","","", // 0xE0 - 0xEF
    "","","","","","","","","","","","","","","","", // 0xF0 - 0xFF
    "ginfo",
	"objinfo",
	"dirinfo",
	"sysinfo",
};

// 11
char* type_compare_command[] = {
    "if",   // 0 | 0x00
    "else", // 1 | 0x01
};

// 13
char* type_builtin_func[] = {
    "int",      //  0 | 0x000
    "rnd",      //  1 | 0x001
    "strlen",   //  2 | 0x002
    "length",   //  3 | 0x003
    "length2",  //  4 | 0x004
    "length3",  //  5 | 0x005
    "length4",  //  6 | 0x006
    "vartype",  //  7 | 0x007
    "gettime",  //  8 | 0x008
    "peek",     //  9 | 0x009
    "wpeek",    // 10 | 0x00A
    "lpeek",    // 11 | 0x00B
    "varptr",   // 12 | 0x00C
    "varuse",   // 13 | 0x00D
    "noteinfo", // 14 | 0x00E
    "instr",    // 15 | 0x00F
    "abs",      // 16 | 0x010
    "limit",    // 17 | 0x011
    "getease",  // 18 | 0x012
    "notefind", // 19 | 0x013
    "varsize",  // 20 | 0x014
    "","","","","","","","","","","","", // 0x15 - 0x 1F
    "","","","","","","","","","","","","","","","", // 0x20 - 0x2F
    "","","","","","","","","","","","","","","","", // 0x30 - 0x3F
    "","","","","","","","","","","","","","","","", // 0x40 - 0x4F
    "","","","","","","","","","","","","","","","", // 0x50 - 0x5F
    "","","","","","","","","","","","","","","","", // 0x60 - 0x6F
    "","","","","","","","","","","","","","","","", // 0x70 - 0x7F
    "","","","","","","","","","","","","","","","", // 0x80 - 0x8F
    "","","","","","","","","","","","","","","","", // 0x90 - 0x9F
    "","","","","","","","","","","","","","","","", // 0xA0 - 0xAF
    "","","","","","","","","","","","","","","","", // 0xB0 - 0xBF
    "","","","","","","","","","","","","","","","", // 0xC0 - 0xCF
    "","","","","","","","","","","","","","","","", // 0xD0 - 0xDF
    "","","","","","","","","","","","","","","","", // 0xE0 - 0xEF
    "","","","","","","","","","","","","","","","", // 0xF0 - 0xFF
    "str",      //  | 0x0100
    "strmid",   //  | 0x0101
    "strf",     //  | 0x0103
    "getpath",  //  | 0x0104
    "strtrim",  //  | 0x0105
    "","","","","","","","","","","", // 0x106 - 0x10F
    "","","","","","","","","","","","","","","","", // 0x0110 - 0x011F
    "","","","","","","","","","","","","","","","", // 0x0120 - 0x012F
    "","","","","","","","","","","","","","","","", // 0x0130 - 0x013F
    "","","","","","","","","","","","","","","","", // 0x0140 - 0x014F
    "","","","","","","","","","","","","","","","", // 0x0150 - 0x015F
    "","","","","","","","","","","","","","","","", // 0x0160 - 0x016F
    "","","","","","","","","","","","","","","","", // 0x0170 - 0x017F
    "sin",      //  | 0x0180
    "cos",      //  | 0x0181
    "tan",      //  | 0x0182
    "atan",     //  | 0x0183
    "sqrt",     //  | 0x0184
    "double",   //  | 0x0185
    "absf",     //  | 0x0186
    "expf",     //  | 0x0187
    "logf",     //  | 0x0188
    "limitf",   //  | 0x0189
    "powf",     //  | 0x018A
    "geteasef", //  | 0x018B
};

// 14
char* type_builtin_var[] = {
	"system",  //  0 | 0x00
	"hspstat", //  1 | 0x01
	"hspver",  //  2 | 0x02
	"stat",    //  3 | 0x03
	"cnt",     //  4 | 0x04
	"err",     //  5 | 0x05
	"strsize", //  6 | 0x06
	"looplev", //  7 | 0x07
	"sublev",  //  8 | 0x08
	"iparam",  //  9 | 0x09
	"wparam",  // 10 | 0x0a
	"lparam",  // 11 | 0x0b
	"refstr",  // 12 | 0x0c
	"refdval", // 13 | 0x0d
};

// 15
char* type_program_command[] = {
    "goto",        //  0 | 0x00
    "gosub",       //  1 | 0x01
    "return",      //  2 | 0x02
    "break",       //  3 | 0x03
    "repeat",      //  4 | 0x04
    "loop",        //  5 | 0x05
    "continue",    //  6 | 0x06
    "wait",        //  7 | 0x07
    "await",       //  8 | 0x08
    "dim",         //  9 | 0x09
    "sdim",        // 10 | 0x0A
    "foreach",     // 11 | 0x0B
    "",            // 12 | 0x0C
    "dimtype",     // 13 | 0x0D
    "dup",         // 14 | 0x0E
    "dupptr",      // 15 | 0x0F
    "end",         // 16 | 0x10
    "stop",        // 17 | 0x11
    "newmod",      // 18 | 0x12
    "",            // 19 | 0x13
    "delmod",      // 20 | 0x14
    "",            // 21 | 0x15
    "mref",        // 22 | 0x16
    "run",         // 23 | 0x17
    "exgoto",      // 24 | 0x18
    "on",          // 25 | 0x19
    "mcall",       // 26 | 0x1A
    "assert",      // 27 | 0x1B
    "logmes",      // 28 | 0x1C
    "newlab",      // 29 | 0x1D
    "resume",      // 30 | 0x1E
    "yield",       // 31 | 0x1F
    "strexchange", // 32 | 0x20
};

// 17
char* type_library_control[] = {
    "newcom",     //   1 | 0x00
    "querycom",   //   2 | 0x01
    "delcom",     //   3 | 0x02
    "cnvstow",    //   4 | 0x03
    "comres",     //   5 | 0x04
    "axobj",      //   6 | 0x05
    "winobj",     //   7 | 0x06
    "sendmsg",    //   8 | 0x07
    "comevent",   //   9 | 0x08
    "comevarg",   //  10 | 0x09
    "sarrayconv", //  11 | 0x0A
    "",           //  12 | 0x0B
    "cnvstoa",    //  13 | 0x0C
    "",           //  14 | 0x0D
    "",           //  15 | 0x0E
    "",           //  16 | 0x0F
    "","","","","","","","","","","","","","","","", // 0x10 - 0x1F
    "","","","","","","","","","","","","","","","", // 0x20 - 0x2F
    "","","","","","","","","","","","","","","","", // 0x30 - 0x3F
    "","","","","","","","","","","","","","","","", // 0x40 - 0x4F
    "","","","","","","","","","","","","","","","", // 0x50 - 0x5F
    "","","","","","","","","","","","","","","","", // 0x60 - 0x6F
    "","","","","","","","","","","","","","","","", // 0x70 - 0x7F
    "","","","","","","","","","","","","","","","", // 0x80 - 0x8F
    "","","","","","","","","","","","","","","","", // 0x90 - 0x9F
    "","","","","","","","","","","","","","","","", // 0xA0 - 0xAF
    "","","","","","","","","","","","","","","","", // 0xB0 - 0xBF
    "","","","","","","","","","","","","","","","", // 0xC0 - 0xCF
    "","","","","","","","","","","","","","","","", // 0xD0 - 0xDF
    "","","","","","","","","","","","","","","","", // 0xE0 - 0xEF
    "","","","","","","","","","","","","","","","", // 0xF0 - 0xFF
    "callfunc",   // 256 | 0x0100
    "cnvwtos",    // 257 | 0x0101
    "comevdisp",  // 258 | 0x0102
    "libptr",     // 259 | 0x0103
    "cnvatos",    // 260 | 0x0104
};


// HSP3.0 ヘッダー構造体
typedef struct HSPHED {
    char    h1;         // H
    char    h2;         // S
    char    h3;         // P
    char    h4;         // 3
    int32_t version;    // バージョン番号の情報
    int32_t max_val;    // VALオブジェクトの最大数
    int32_t allsize;    // 合計ファイルサイズ
    int32_t pt_cs;      // コード領域のオフセット
    int32_t max_cs;     // コード領域のサイズ
    int32_t pt_ds;      // データ領域のオフセット
    int32_t max_ds;     // データ領域のサイズ
    int32_t pt_ot;      // ラベル情報のオフセット
    int32_t max_ot;     // ラベル情報のサイズ
    int32_t pt_dinfo;   // 行番号情報のオフセット
    int32_t max_dinfo;  // 行番号情報のサイズ
    int32_t pt_linfo;   // ライブラリ情報のオフセット(2.3)
    int32_t max_linfo;  // ライブラリ情報のサイズ(2.3)
    int32_t pt_finfo;   // 関数情報のオフセット(2.3)
    int32_t max_finfo;  // 関数情報のサイズ(2.3)
    int32_t pt_minfo;   // モジュール情報のオフセット(2.5)
    int32_t max_minfo;  // モジュール情報のサイズ(2.5)
    int32_t pt_finfo2;  // 関数情報のオフセット2(2.5)
    int32_t max_finfo2; // 関数情報のサイズ2(2.5)
    int32_t pt_hpidat;  // HPIデータのオフセット(3.0)
    short   max_hpi;    // HPIデータのサイズ(3.0)
    short   max_varhpi; // 変数型プラグインの数(3.0)
    int32_t bootoption; // 起動オプション
    int32_t runtime;    // ランタイム名のオフセット
    int32_t pt_sr;      // オプション領域のオフセット
    int32_t max_sr;     // オプション領域のサイズ
    int32_t pt_exopt;   // 追加オプション領域のオフセット (3.6)
    int32_t max_exopt;  // 追加オプション領域のサイズ (3.6)
} HSPHED;

typedef struct print_data_t {
    bool extra_flag_0;
    bool extra_flag_1;
    bool extra_flag_2;
    bool extra_flag_3;
    int  code_type;
    char code_name[1024];
    bool is_result_str;
    bool is_result_int;
    bool is_result_int2;
    bool is_result_double;
    char result_str[1024];
    int  result_int;
    int  result_int2;
    double result_double;
} print_data_t;

void println(const char* str, ...) {
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    printf("\n");
    va_end(args);
}

void set_array_from_str(char* array, char* src) {
    int i;
    for (i = 0; i < (int)strlen(src); i++) {
        array[i] = src[i];
    }
    array[i] = '\0';
}

int16_t hex_to_int16(uint8_t hex_1, uint8_t hex_2) {
    uint16_t ret = 0;
    ret += hex_2;
    ret = ret << 8;
    ret += hex_1;
    return (int16_t)ret;
}

int32_t hex_to_int24(uint8_t hex_1, uint8_t hex_2, uint8_t hex_3, uint8_t hex_4) {
    uint32_t ret = 0;
    ret += hex_4;
    ret = ret << 8;
    ret += hex_3;
    ret = ret << 8;
    ret += hex_2;
    ret = ret >> 6;
    return (int32_t)ret;
}

int32_t hex_to_int32(uint8_t hex_1, uint8_t hex_2, uint8_t hex_3, uint8_t hex_4) {
    uint32_t ret = 0;
    ret += hex_4;
    ret = ret << 8;
    ret += hex_3;
    ret = ret << 8;
    ret += hex_2;
    ret = ret << 8;
    ret += hex_1;
    return (int32_t)ret;
}

int16_t hex_to_int16_index(uint8_t* data, int32_t* index) {
    int16_t ret = hex_to_int16(data[*index], data[*index + 1]);
    *index += 2;
    return ret;
}

int32_t hex_to_int24_index(uint8_t* data, int32_t* index) {
    int32_t ret = hex_to_int24(data[*index], data[*index + 1], data[*index + 2], data[*index + 3]);
    *index += 3;
    return ret;
}

int32_t hex_to_int32_index(uint8_t* data, int32_t* index) {
    int32_t ret = hex_to_int32(data[*index], data[*index + 1], data[*index + 2], data[*index + 3]);
    *index += 4;
    return ret;
}

char hex_to_char_index(uint8_t* data, int32_t* index) {
    char ret = (char)data[*index];
    *index += 1;
    return ret;
}

FILE* file_open_read(char* filename) {
    FILE* file_ptr = fopen(filename, "r");
    if (file_ptr == NULL) {
        println("ファイルオープンに失敗しました"); // ファイルオープンエラーの処理
        exit(0);
    }
    return file_ptr;
}

void file_seek_end(FILE* file_ptr) {
    bool is_error = fseek(file_ptr, 0, SEEK_END);
    if (is_error) { // ファイルサイズの取得
        println("fseek(fp, 0, SEEK_END)に失敗しました"); // fseek エラーの処理
        exit(0);
    }
}

void file_seek_set(FILE* file_ptr) {
    bool is_error = fseek(file_ptr, 0L, SEEK_SET);
    if (is_error) {
        println("fseek(fp, 0, SEEK_SET)に失敗しました"); // fseek エラーの処理
        exit(0);
    }
}

int32_t get_file_length(FILE* file_ptr) {
    int32_t length = 0;
    file_seek_end(file_ptr);
    length = ftell(file_ptr);
    file_seek_set(file_ptr);
    return length;
}

uint8_t* memory_alloc(int32_t size) {
    uint8_t* memory = (unsigned char*)malloc(size);
    if (memory == NULL) {
        println("メモリ割り当てに失敗しました"); // メモリ割り当てエラーの処理
        exit(0);
    }
    return memory;
}

void file_read(FILE* file_ptr, uint8_t* data, int32_t size) {
    bool is_error = fread(data, 1, size, file_ptr) < size;
    if (is_error) {
        println("ファイルの読み取りに失敗しました"); // ファイル読み取りエラーの処理
        exit(0);
    }
}

uint8_t* get_file_raw_data(char* filename, int32_t* size) {
    FILE *file_ptr;
    uint8_t* raw_data;
    file_ptr = file_open_read(filename);
    *size = get_file_length(file_ptr);
    raw_data = memory_alloc(*size); // ファイル全体を格納するメモリを割り当てる 
    file_read(file_ptr, raw_data, *size);
    fclose(file_ptr);
    return raw_data;
}

void set_hsp_header(uint8_t* data, HSPHED* hsp_header) {
    int index = 0;
    hsp_header->h1 = hex_to_char_index(data, &index);
    hsp_header->h2 = hex_to_char_index(data, &index);
    hsp_header->h3 = hex_to_char_index(data, &index);
    hsp_header->h4 = hex_to_char_index(data, &index);
    hsp_header->version = hex_to_int32_index(data, &index);
    hsp_header->max_val = hex_to_int32_index(data, &index);
    hsp_header->allsize = hex_to_int32_index(data, &index);
    hsp_header->pt_cs = hex_to_int32_index(data, &index);
    hsp_header->max_cs = hex_to_int32_index(data, &index);
    hsp_header->pt_ds = hex_to_int32_index(data, &index);
    hsp_header->max_ds = hex_to_int32_index(data, &index);
    hsp_header->pt_ot = hex_to_int32_index(data, &index);
    hsp_header->max_ot = hex_to_int32_index(data, &index);
    hsp_header->pt_dinfo = hex_to_int32_index(data, &index);
    hsp_header->max_dinfo = hex_to_int32_index(data, &index);
    hsp_header->pt_linfo = hex_to_int32_index(data, &index);
    hsp_header->max_linfo = hex_to_int32_index(data, &index);
    hsp_header->pt_finfo = hex_to_int32_index(data, &index);
    hsp_header->max_finfo = hex_to_int32_index(data, &index);
    hsp_header->pt_minfo = hex_to_int32_index(data, &index);
    hsp_header->max_minfo = hex_to_int32_index(data, &index);
    hsp_header->pt_finfo2 = hex_to_int32_index(data, &index);
    hsp_header->max_finfo2 = hex_to_int32_index(data, &index);
    hsp_header->pt_hpidat = hex_to_int32_index(data, &index);
    hsp_header->max_hpi = hex_to_int16_index(data, &index);
    hsp_header->max_varhpi = hex_to_int16_index(data, &index);
    hsp_header->bootoption = hex_to_int32_index(data, &index);
    hsp_header->runtime = hex_to_int32_index(data, &index);
    hsp_header->pt_sr = hex_to_int32_index(data, &index);
    hsp_header->max_sr = hex_to_int32_index(data, &index);
    hsp_header->pt_exopt = hex_to_int32_index(data, &index);
    hsp_header->max_exopt = hex_to_int32_index(data, &index);
}

void print_hex_raw_data(uint8_t* data, int32_t size) {
    println("[binary]");
    printf("   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    printf("---+------------------------------------------------\n");
    printf("00 | ");
    int line = 0;
    for(int i = 0; i < size; i++) { // バイナリを表示する
        printf("%02X ",data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n%02X | ", line + 1);
            line++;
        }
    }
    puts("");
}

void print_hsp_header(HSPHED* hsp_header) {
    println("[header]");
    println("h1         = %c", hsp_header->h1);
    println("h2         = %c", hsp_header->h2);
    println("h3         = %c", hsp_header->h3);
    println("h4         = %c", hsp_header->h4);
    println("version    = 0x%x", hsp_header->version);
    println("allsize    = %d", hsp_header->allsize);
    println("pt_cs      = %d", hsp_header->pt_cs);
    println("max_cs     = %d", hsp_header->max_cs);
    println("pt_ds      = %d", hsp_header->pt_ds);
    println("max_ds     = %d", hsp_header->max_ds);
    println("pt_ot      = %d", hsp_header->pt_ot);
    println("max_ot     = %d", hsp_header->max_ot);
    println("pt_dinfo   = %d", hsp_header->pt_dinfo);
    println("max_dinfo  = %d", hsp_header->max_dinfo);
    println("pt_linfo   = %d", hsp_header->pt_linfo);
    println("max_linfo  = %d", hsp_header->max_linfo);
    println("pt_finfo   = %d", hsp_header->pt_finfo);
    println("max_finfo  = %d", hsp_header->max_finfo);
    println("pt_minfo   = %d", hsp_header->pt_minfo);
    println("max_minfo  = %d", hsp_header->max_minfo);
    println("pt_finfo2  = %d", hsp_header->pt_finfo2);
    println("max_finfo2 = %d", hsp_header->max_finfo2);
    println("pt_hpidat  = %d", hsp_header->pt_hpidat);
    println("max_hpi    = %d", hsp_header->max_hpi);
    println("max_varhpi = %d", hsp_header->max_varhpi);
    println("bootoption = %d", hsp_header->bootoption);
    println("runtime    = %d", hsp_header->runtime);
    println("pt_sr      = %d", hsp_header->pt_sr);
    println("max_sr     = %d", hsp_header->max_sr);
    println("pt_exopt   = %d", hsp_header->pt_exopt);
    println("max_exopt  = %d", hsp_header->max_exopt);
}

void print_bootoption(HSPHED* hsp_header) {
    println("[bootoption]");
    printf("HSPHED_BOOTOPT_DEBUGWIN  = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_DEBUGWIN) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_WINHIDE   = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_WINHIDE) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_DIRSAVE   = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_DIRSAVE) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_SAVER     = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_SAVER) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_RUNTIME   = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_RUNTIME) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_NOMMTIMER = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_NOMMTIMER) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_NOGDIP    = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_NOGDIP) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_FLOAT32   = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_FLOAT32) {
        println("true");
    } else {
        println("false");
    }
    printf("HSPHED_BOOTOPT_ORGRND    = ");
    if (hsp_header->bootoption & HSPHED_BOOTOPT_ORGRND) {
        println("true");
    } else {
        println("false");
    }
}

void print_code_segment_first(print_data_t* print_data, int16_t code_segment_first) {
    if (code_segment_first & EXFLG_0) {
        print_data->extra_flag_0 = true;
    } else {
        print_data->extra_flag_0 = false;
    }
    if (code_segment_first & EXFLG_1) {
        print_data->extra_flag_1 = true;
    } else {
        print_data->extra_flag_1 = false;
    }
    if (code_segment_first & EXFLG_2) {
        print_data->extra_flag_2 = true;
    } else {
        print_data->extra_flag_2 = false;
    }
    if (code_segment_first & EXFLG_3) {
        print_data->extra_flag_3 = true;
    } else {
        print_data->extra_flag_3 = false;
    }
}

void if_type_mark32(print_data_t* print_data, int32_t code_segment_second) {
    println("文字コード = %x", code_segment_second);
}

void if_type_mark16(print_data_t* print_data, int16_t code_segment_second) {
    switch(code_segment_second) {
        case CALCCODE_ADD:
            set_array_from_str(print_data->result_str, "+");
            break;
        case CALCCODE_SUB:
            set_array_from_str(print_data->result_str, "-");
            break;
        case CALCCODE_MUL:
            set_array_from_str(print_data->result_str, "*");
            break;
        case CALCCODE_DIV:
            set_array_from_str(print_data->result_str, "/");
            break;
        case CALCCODE_MOD:
            set_array_from_str(print_data->result_str, "\\");
            break;
        case CALCCODE_AND:
            set_array_from_str(print_data->result_str, "&");
            break;
        case CALCCODE_OR:
            set_array_from_str(print_data->result_str, "|");
            break;
        case CALCCODE_XOR:
            set_array_from_str(print_data->result_str, "^");
            break;
        case CALCCODE_EQ:
            set_array_from_str(print_data->result_str, "=");
            break;
        case CALCCODE_NE:
            set_array_from_str(print_data->result_str, "!");
            break;
        case CALCCODE_GT:
            set_array_from_str(print_data->result_str, ">");
            break;
        case CALCCODE_LT:
            set_array_from_str(print_data->result_str, "<");
            break;
        case CALCCODE_GTEQ:
            set_array_from_str(print_data->result_str, ">=");
            break;
        case CALCCODE_LTEQ:
            set_array_from_str(print_data->result_str, "<=");
            break;
        case CALCCODE_RR:
            set_array_from_str(print_data->result_str, ">>");
            break;
        case CALCCODE_LR:
            set_array_from_str(print_data->result_str, "<<");
            break;
        case CALCCODE_MAX:
            set_array_from_str(print_data->result_str, "MAX");
            break;
        case 40:
            set_array_from_str(print_data->result_str, "(");
            break;
        case 41:
            set_array_from_str(print_data->result_str, ")");
            break;
        case 63:
            set_array_from_str(print_data->result_str, "dammy");
            break;
        default:
            set_array_from_str(print_data->result_str, "other");
    }
    print_data->is_result_str = true;
}

void if_type_mark(print_data_t* print_data, bool is_code_48bit, int16_t second16, int32_t second32) {
    if (is_code_48bit) { // 48bitコードか
        if_type_mark32(print_data, second32);    
    } else {
        if_type_mark16(print_data, second16);
    }
}

void if_type_var(print_data_t* print_data, bool is_code_48bit, int16_t second16, int32_t second32) {
    if (is_code_48bit) { // 48bitコードか
        print_data->result_int = second32;
    } else {
        print_data->result_int = second16;
    }
    print_data->is_result_int = true;
}

// 1バイトのutf8コードをchar配列に格納する
void utf8_1byte_to_char(char* str,uint32_t utf8_code) {
    uint32_t tmp_utf8 = utf8_code;
    str[1] = '\0';
    str[0] = (uint8_t)tmp_utf8;
}

// 2バイトのutf8コードをchar配列に格納する
void utf8_2byte_to_char(char* str, uint32_t utf8_code) {
    uint32_t tmp_utf8 = utf8_code;
    str[2] = '\0';
    str[1] = (uint8_t)tmp_utf8;
    tmp_utf8 = tmp_utf8 >> 8;
    str[0] = (uint8_t)tmp_utf8;
}

// 3バイトのutf8コードをchar配列に格納する
void utf8_3byte_to_char(char* str, uint32_t utf8_code) {
    uint32_t tmp_utf8 = utf8_code;
    str[3] = '\0';
    str[2] = (uint8_t)tmp_utf8;
    tmp_utf8 = tmp_utf8 >> 8;
    str[1] = (uint8_t)tmp_utf8;
    tmp_utf8 = tmp_utf8 >> 8;
    str[0] = (uint8_t)tmp_utf8;
}

void utf8_to_char(char* str, uint32_t utf8_code) {
    if (utf8_code < 0x80) {
        utf8_1byte_to_char(str, utf8_code);
    } else if (utf8_code >= 0xc0 && utf8_code <= 0xe0) {
        utf8_1byte_to_char(str, utf8_code);
    } else if (utf8_code >= 0xe0 && utf8_code <= 0xf0) {
        utf8_1byte_to_char(str, utf8_code);
    } else if (utf8_code >= 0xf0 && utf8_code <= 0xf8) {
        utf8_1byte_to_char(str, utf8_code);
    } else if (utf8_code >= 0xf8 && utf8_code <= 0xfc) {
        utf8_1byte_to_char(str, utf8_code);
    } else if (utf8_code >= 0xfc && utf8_code <= 0xfe) {
        utf8_1byte_to_char(str, utf8_code);
    } else {
        utf8_3byte_to_char(str, utf8_code);
    }
}

int search_sjis_index(uint32_t* sjis_table, uint32_t sjis_code) { //指定したSJISコードにマッチする位置を返す
    int table_len = sizeof(table_sjis) / sizeof(uint32_t);
    for (int i = 0; i < table_len; i++) {
        if (sjis_code == sjis_table[i]) {
            return i;
        }
    }
    return '?';
}

void print_utf8_from_sjis(print_data_t* print_data, uint32_t* sjis_table, uint32_t sjis_code, int* str_index) {
    int offset = search_sjis_index(sjis_table, sjis_code);
    if (offset) {
        char str[4] = "";
        utf8_to_char(str, table_utf8[offset]);
        int i;
        for (i = 0; i < (int)strlen(str); i++) {
            print_data->result_str[*str_index + i] = str[i];
        }
        *str_index += i;
    } else { // マッチしなかった場合は変換せずに格納する
        char str[4] = "";
        utf8_to_char(str, sjis_code);
    }
}

uint32_t get_2byte_from_raw_data(uint8_t* data, int offset) {
    uint32_t code = 0;
    code += data[offset + 0]; // ２バイト分流し込む
    code = code << 8;
    code += data[offset + 1];
    return code;
}

void print_sjis_data(print_data_t* print_data, uint8_t* data, int32_t size) {
    int str_index = 0;
    uint32_t* sjis_table = table_sjis;
    for(int offset = 0; offset < size; ) {
        if ((data[offset] < 0x80) ||
            (data[offset] >= 0xA1 && data[offset] <= 0xDF)) { // 1バイト目が0x81未満なら１バイト文字
            uint32_t sjis_code = 0;
            sjis_code += data[offset]; // ２バイト分流し込む
            offset += 1;
            print_utf8_from_sjis(print_data, sjis_table, sjis_code, &str_index);
        } else { // 1バイト目が0x81以上なら２バイト文字
            uint32_t sjis_code = 0;
            sjis_code = get_2byte_from_raw_data(data, offset);
            offset += 2;
            print_utf8_from_sjis(print_data, sjis_table, sjis_code, &str_index);
        }
    }
    print_data->result_str[str_index] = '\0';
    print_data->is_result_str = true;
}

void if_type_string(print_data_t* print_data, bool is_code_48bit, uint8_t* data, int32_t* code_index, HSPHED* hsp_header) {
    if (is_code_48bit) { // 48bitコードか
        uint32_t code;
        int32_t i = *code_index - 4;
        code = data[i + 3];
        code = code << 8;
        code += data[i + 2];
        code = code << 8;
        code += data[i + 1];
        code = code << 8;
        code += data[i + 0];
        uint8_t* str = memory_alloc(hsp_header->max_ds);
        int32_t count;
        for (count = 0; count < hsp_header->max_ds; count++) {
            str[count] = data[hsp_header->pt_ds + count + code];
            if (str[count] == '\0') {
                break;
            }
        }
        print_sjis_data(print_data, str, count);
        free(str);
    } else {
        uint32_t code = 0;
        int32_t i = *code_index - 2;
        code = data[i + 1];
        code = code << 8;
        code += data[i + 0];
        uint8_t* str = memory_alloc(hsp_header->max_ds);
        int32_t count;
        for (count = 0; count < hsp_header->max_ds; count++) {
            str[count] = data[hsp_header->pt_ds + count + code];
            if (str[count] == '\0') {
                break;
            }
        }
        print_sjis_data(print_data, str, count);
        free(str);
    }
}

void if_type_dnum(print_data_t* print_data, bool is_code_48bit, uint8_t* data, int32_t* code_index) {
    if (is_code_48bit) { // 48bitコードか
        uint32_t code = 0;
        int32_t i = *code_index - 4;
        code = data[i + 3];
        code = code << 8;
        code += data[i + 2];
        code = code << 8;
        code += data[i + 1];
        code = code << 8;
        code += data[i + 0];
    } else {
        uint16_t code = 0;
        int32_t i = *code_index - 2;
        code = data[i + 1];
        code = code << 8;
        code += data[i + 0];
        if (code == 0) {
            puts("");
            return;
        }
        int32_t offset = 552 + (int)code;
        char binary[8];
        for (int p = 0; p < 8; p++) {
            binary[p] = data[offset + (7 - p)];
        }
        double decode = IEE754_binary64_decode( binary );
        print_data->is_result_double = true;
        print_data->result_double = decode;
    } 
}

void if_type_inum(print_data_t* print_data, bool is_code_48bit, uint8_t* data, int32_t* code_index) {
    if (is_code_48bit) { // 48bitコードか
        int32_t code = 0;
        int32_t i = *code_index - 3;
        code = data[i + 3];
        code = code << 8;
        code += data[i + 2];
        code = code << 8;
        code += data[i + 1];
        code = code << 8;
        code += data[i + 0];
        print_data->result_int = code;
    } else {
        uint16_t code = 0;
        int32_t i = *code_index - 2;
        code = data[i + 1];
        code = code << 8;
        code += data[i + 0];
        print_data->result_int = code;
    }
    print_data->is_result_int = true;
}

void if_type_default(print_data_t* print_data, bool is_code_48bit, int16_t second16, int32_t second32) {
    if (is_code_48bit) { // 48bitコードか
        print_data->result_int = second32;
    } else {
        print_data->result_int = second16;
    }
    print_data->is_result_int = true;
}

void print_code_segment_second(print_data_t* print_data, int16_t code_segment_first, uint8_t* data, int32_t* code_index, HSPHED* hsp_header, int* loop_count) {
    int16_t code_segment_second16 = 0;
    int32_t code_segment_second32 = 0;
    bool is_code_48bit = code_segment_first & EXFLG_3;

    if (is_code_48bit) { // 48bitコードか
        code_segment_second32 = hex_to_int24_index(data, code_index);
    } else {
        code_segment_second16 = hex_to_int16_index(data, code_index);
    }

    int code_type = code_segment_first & CSTYPE;
    print_data->code_type = code_type;

    print_data->is_result_str = false;
    print_data->is_result_int = false;
    print_data->is_result_int2 = false;
    print_data->is_result_double = false;
    set_array_from_str(print_data->code_name, "");

    switch(code_type) {
        case TYPE_MARK: // 記号
            if_type_mark(print_data, is_code_48bit, code_segment_second16, code_segment_second32);
            break;
        case TYPE_VAR: // ユーザー変数
            if_type_var(print_data, is_code_48bit, code_segment_second16, code_segment_second32);
            break;
        case TYPE_STRING: // 文字列
            if_type_string(print_data, is_code_48bit, data, code_index, hsp_header);
            break;
        case TYPE_DNUM: // 実数値
            if_type_dnum(print_data, is_code_48bit, data, code_index);
            break;
        case TYPE_INUM: // 整数値
            if_type_inum(print_data, is_code_48bit, data, code_index);
            if (is_code_48bit) {
                hsp_header->pt_cs+=2;
                *loop_count += 2;
            }
            break;
        case TYPE_STRUCT: // モジュール変数等
            print_data->is_result_int = true;
            print_data->result_int = code_segment_second16;
            print_data->is_result_int2 = true;
            print_data->result_int2 = data[*code_index - 3];
            break;
        case TYPE_XLABEL: // 未使用
            break;
        case TYPE_LABEL: // ラベル
            print_data->is_result_int = true;
            print_data->result_int = code_segment_second16;
            break;
        case TYPE_INTCMD: // 内蔵命令
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_builtin_command[code_segment_second16]);
            break;
        case TYPE_EXTCMD: // 拡張命令
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_extend_command[code_segment_second16]);
            break;
        case TYPE_EXTSYSVAR: // 拡張システム変数
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_extra_system_var[code_segment_second16]);
            hsp_header->pt_cs+=4;
            *loop_count += 4;
            break;
        case TYPE_CMPCMD: // 比較命令
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_compare_command[code_segment_second16]);
            hsp_header->pt_cs+=2;
            *loop_count += 2;
            break;
        case TYPE_MODCMD: // ユーザー命令関数
            print_data->is_result_int = true;
            print_data->result_int = code_segment_second16;
            break;
        case TYPE_INTFUNC: // 内蔵関数
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_builtin_func[code_segment_second16]);
            break;
        case TYPE_SYSVAR: // 内蔵システム変数
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_builtin_var[code_segment_second16]);
            hsp_header->pt_cs+=4;
            *loop_count += 4;
            break;
        case TYPE_PROGCMD: // 制御命令
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_program_command[code_segment_second16]);
            break;
        case TYPE_DLLFUNC: // DLL拡張命令関数
            println("DLL拡張命令関数");
            break;
        case TYPE_DLLCTRL: // DLL系命令
            print_data->is_result_str = true;
            set_array_from_str(print_data->result_str, type_library_control[code_segment_second16]);
            break;
        case TYPE_USERDEF: // HSP3拡張プラグイン命令
            println("HSP3拡張プラグイン命令");
            break;
        default:
            if_type_default(print_data, is_code_48bit, code_segment_second16, code_segment_second32);
            hsp_header->pt_cs+=2;
            *loop_count += 2;
    }
}

void print_code_segment(print_data_t* print_data, uint8_t* data, HSPHED* hsp_header, int* code_index, int* loop_count) {
    int16_t code_segment_first = hex_to_int16_index(data, code_index);
    print_code_segment_first(print_data, code_segment_first);
    print_code_segment_second(print_data, code_segment_first, data, code_index, hsp_header, loop_count);
 }

char* sample_basic[] = {
    "hsp_ax_sample/basic/arraynote.ax",
    "hsp_ax_sample/basic/atan_grect.ax",
    "hsp_ax_sample/basic/aviplay.ax",
    "hsp_ax_sample/basic/bmpsave.ax",
    "hsp_ax_sample/basic/calcsheet.ax",
    "hsp_ax_sample/basic/cdplay.ax",
    "hsp_ax_sample/basic/chkbox.ax",
    "hsp_ax_sample/basic/clock.ax",
    "hsp_ax_sample/basic/clock2.ax",
    "hsp_ax_sample/basic/dirinfo.ax",
    "hsp_ax_sample/basic/dirlist.ax",
    "hsp_ax_sample/basic/editor.ax",
    "hsp_ax_sample/basic/emes.ax",
    "hsp_ax_sample/basic/filedialog.ax",
    "hsp_ax_sample/basic/fonts.ax",
    "hsp_ax_sample/basic/getpath.ax",
    "hsp_ax_sample/basic/gradf.ax",
    "hsp_ax_sample/basic/grect.ax",
    "hsp_ax_sample/basic/groll.ax",
    "hsp_ax_sample/basic/grotate.ax",
    "hsp_ax_sample/basic/groupbox.ax",
    "hsp_ax_sample/basic/gsel.ax",
    "hsp_ax_sample/basic/gsquare.ax",
    "hsp_ax_sample/basic/gstyle.ax",
    "hsp_ax_sample/basic/hsv.ax",
    "hsp_ax_sample/basic/htcopy1.ax",
    "hsp_ax_sample/basic/htcopy2.ax",
    "hsp_ax_sample/basic/line.ax",
    "hsp_ax_sample/basic/memcpy.ax",
    "hsp_ax_sample/basic/memfile.ax",
    "hsp_ax_sample/basic/memnote.ax",
    "hsp_ax_sample/basic/menusample.ax",
    "hsp_ax_sample/basic/mesinfo.ax",
    "hsp_ax_sample/basic/mouse.ax",
    "hsp_ax_sample/basic/mouse2.ax",
    "hsp_ax_sample/basic/note1.ax",
    "hsp_ax_sample/basic/note2.ax",
    "hsp_ax_sample/basic/noteadd.ax",
    "hsp_ax_sample/basic/objimage.ax",
    "hsp_ax_sample/basic/objmode.ax",
    "hsp_ax_sample/basic/omedetai.ax",
    "hsp_ax_sample/basic/onerror.ax",
    "hsp_ax_sample/basic/onexit.ax",
    "hsp_ax_sample/basic/onkey.ax",
    "hsp_ax_sample/basic/picfont.ax",
    "hsp_ax_sample/basic/picload.ax",
    "hsp_ax_sample/basic/star.ax",
    "hsp_ax_sample/basic/starmove.ax",
    "hsp_ax_sample/basic/stars.ax",
    "hsp_ax_sample/basic/stick.ax",
    "hsp_ax_sample/basic/str.ax",
    "hsp_ax_sample/basic/strf.ax",
    "hsp_ax_sample/basic/strmid.ax",
    "hsp_ax_sample/basic/switch.ax",
    "hsp_ax_sample/basic/sysinfo.ax",
};

int main (void) {
    uint8_t* ax_raw_data;
    int32_t ax_size;
    HSPHED hsp_header;
    print_data_t print_data;

    ax_raw_data = get_file_raw_data(sample_basic[3], &ax_size);

    puts("");
    print_hex_raw_data(ax_raw_data, ax_size);

    set_hsp_header(ax_raw_data, &hsp_header);

    puts("");
    print_hsp_header(&hsp_header);

    puts("");
    print_bootoption(&hsp_header);

    puts("");
    int loop_count;
    int code_segment_offset = hsp_header.pt_cs;
    int32_t code_index = hsp_header.pt_cs; // コード領域のオフセット
    for (loop_count = 0; loop_count < hsp_header.max_cs; loop_count += 4) {
        code_index = hsp_header.pt_cs; // コード領域のオフセット
        printf("%04X",hsp_header.pt_cs - code_segment_offset);
        printf(" | ");
        print_code_segment(&print_data, ax_raw_data, &hsp_header, &code_index, &loop_count);
        printf("%d ", print_data.extra_flag_0);
        printf("%d ", print_data.extra_flag_1);
        printf("%d ", print_data.extra_flag_2);
        printf("%d", print_data.extra_flag_3);
        printf(" | ");
        printf("%02d", print_data.code_type);
        printf(" | ");
        if (print_data.code_type <= 18) {
            printf("%s", type_name_str[print_data.code_type]);
        }
        else {
            printf("unknown");
        }
        if (print_data.is_result_int) {
            printf(" | ");
            printf("%d", print_data.result_int);
        }
        if (print_data.is_result_str) {
            printf(" | ");
            printf("%s", print_data.result_str);
        }
        if (print_data.is_result_int2) {
            printf(" | ");
            printf("%d", print_data.result_int2);
        }
        puts("");
        hsp_header.pt_cs += 4;
    }

    puts("");
    puts("[デバッグ用ラベル情報]");
    for(int i = 0; i < hsp_header.max_ot; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_ot + i + 0],
            ax_raw_data[hsp_header.pt_ot + i + 1],
            ax_raw_data[hsp_header.pt_ot + i + 2],
            ax_raw_data[hsp_header.pt_ot + i + 3]
        );
        printf("%X ", x);
        // printf("%X %X %X %X ", ax_raw_data[hsp_header.pt_ot + i + 0],
        //     ax_raw_data[hsp_header.pt_ot + i + 1],
        //     ax_raw_data[hsp_header.pt_ot + i + 2],
        //     ax_raw_data[hsp_header.pt_ot + i + 3]
        // );
    }

    println("\n");
    puts("[行番号情報]");
    for(int i = 0; i < hsp_header.max_dinfo; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_dinfo + i + 0],
            ax_raw_data[hsp_header.pt_dinfo + i + 1],
            ax_raw_data[hsp_header.pt_dinfo + i + 2],
            ax_raw_data[hsp_header.pt_dinfo + i + 3]
        );
        //printf("%X ", x);
        printf("%X %X %X %X ", ax_raw_data[hsp_header.pt_dinfo + i + 0],
            ax_raw_data[hsp_header.pt_dinfo + i + 1],
            ax_raw_data[hsp_header.pt_dinfo + i + 2],
            ax_raw_data[hsp_header.pt_dinfo + i + 3]
        );
    }

    println("\n");
    puts("[ライブラリ情報]");
    for(int i = 0; i < hsp_header.max_linfo; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_linfo + i + 0],
            ax_raw_data[hsp_header.pt_linfo + i + 1],
            ax_raw_data[hsp_header.pt_linfo + i + 2],
            ax_raw_data[hsp_header.pt_linfo + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[関数情報]");
    for(int i = 0; i < hsp_header.max_finfo; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_finfo + i + 0],
            ax_raw_data[hsp_header.pt_finfo + i + 1],
            ax_raw_data[hsp_header.pt_finfo + i + 2],
            ax_raw_data[hsp_header.pt_finfo + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[モジュール情報]");
    for(int i = 0; i < hsp_header.max_minfo; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_minfo + i + 0],
            ax_raw_data[hsp_header.pt_minfo + i + 1],
            ax_raw_data[hsp_header.pt_minfo + i + 2],
            ax_raw_data[hsp_header.pt_minfo + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[関数情報2]");
    for(int i = 0; i < hsp_header.max_finfo2; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_finfo2 + i + 0],
            ax_raw_data[hsp_header.pt_finfo2 + i + 1],
            ax_raw_data[hsp_header.pt_finfo2 + i + 2],
            ax_raw_data[hsp_header.pt_finfo2 + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[HPIデータ]");
    for(int i = 0; i < hsp_header.max_hpi; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_hpidat + i + 0],
            ax_raw_data[hsp_header.pt_hpidat + i + 1],
            ax_raw_data[hsp_header.pt_hpidat + i + 2],
            ax_raw_data[hsp_header.pt_hpidat + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[オプション領域]");
    for(int i = 0; i < hsp_header.max_sr; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_sr + i + 0],
            ax_raw_data[hsp_header.pt_sr + i + 1],
            ax_raw_data[hsp_header.pt_sr + i + 2],
            ax_raw_data[hsp_header.pt_sr + i + 3]
        );
        printf("%X ", x);
    }

    println("\n");
    puts("[追加オプション領域]");
    for(int i = 0; i < hsp_header.max_exopt; i += 4) {
        int32_t x = hex_to_int32(
            ax_raw_data[hsp_header.pt_exopt + i + 0],
            ax_raw_data[hsp_header.pt_exopt + i + 1],
            ax_raw_data[hsp_header.pt_exopt + i + 2],
            ax_raw_data[hsp_header.pt_exopt + i + 3]
        );
        printf("%X ", x);
    }

    free(ax_raw_data); // メモリを解放する
    return 0;
}