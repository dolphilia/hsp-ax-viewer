# axファイルのフォーマット

HSPオブジェクトファイルであるax形式ファイルには次のデータが格納されています。

1. AXヘッダー情報
2. オプション領域: Option Info(OPTBUF)
3. コード領域: Code Segment(CS)
4. データ領域: Data Segment(DS)
5. ラベル情報: Object Temp(OT):
6. 行番号情報: Debug Info(DINFO)
7. ライブラリ情報: Lib Info(LINFO)
8. 関数情報: Func Info(FINFO)
9. モジュール情報: Module Info(MINFO)
10. 関数情報2: Func Info2(FINFO2): 
11. HSP3プラグイン情報: HPI Info(HPIBUF)

## データへのアクセス

2〜11のデータにはAXヘッダー情報に格納されているオフセットを参照することで位置を特定することができます。

## 1. AXヘッダ情報

AXヘッダー情報はHSPHED構造体で定義されています。

hsp3struct.hから抜粋:

```c
// HSP3.0 ヘッダー構造体
typedef struct HSPHED {
    char	h1;         // H
    char	h2;         // S
    char	h3;         // P
    char	h4;         // 3
    int		version;    // バージョン番号の情報
    int		max_val;    // VALオブジェクトの最大数
    int		allsize;    // 合計ファイルサイズ
    
    int		pt_cs;      // コード領域のオフセット
    int		max_cs;     // コード領域のサイズ
    int		pt_ds;      // データ領域のオフセット
    int		max_ds;     // データ領域のサイズ
    
    int		pt_ot;      // ラベル情報のオフセット
    int		max_ot;     // ラベル情報のサイズ
    int		pt_dinfo;   // 行番号情報のオフセット
    int		max_dinfo;  // 行番号情報のサイズ
    
    int		pt_linfo;   // ライブラリ情報のオフセット(2.3)
    int		max_linfo;  // ライブラリ情報のサイズ(2.3)
    int		pt_finfo;   // 関数情報のオフセット(2.3)
    int		max_finfo;  // 関数情報のサイズ(2.3)
    
    int		pt_minfo;   // モジュール情報のオフセット(2.5)
    int		max_minfo;  // モジュール情報のサイズ(2.5)
    int		pt_finfo2;  // 関数情報のオフセット2(2.5)
    int		max_finfo2; // 関数情報のサイズ2(2.5)
    
    int		pt_hpidat;  // HPIデータのオフセット(3.0)
    short	max_hpi;    // HPIデータのサイズ(3.0)
    short	max_varhpi; // 変数型プラグインの数(3.0)
    int		bootoption; // 起動オプション
    int		runtime;    // ランタイム名のオフセット
    
    //		HSP3.5 エクストラヘッダー構造体
    //
    int		pt_sr;      // オプション領域のオフセット
    int		max_sr;     // オプション領域のサイズ
    int		pt_exopt;   // 追加オプション領域のオフセット (3.6)
    int		max_exopt;  // 追加オプション領域のサイズ (3.6)
} HSPHED;
```

`h1`〜`h4`には`'H','S','P','3'`の文字コードが一文字ずつ入ります。`version`にはコードジェネレーターのバージョンが格納されています。例えば3.6の場合は`0x0306`となります。

`pt_cs`のような値は該当するデータの位置を示すファイル先頭からのオフセットです。`max_cs`のような値はデータのサイズを表しています。

`bootoption`には以下のフラグが格納されます。

```c
#define HSPHED_BOOTOPT_DEBUGWIN 1  // 起動時にデバッグウインドウを表示する
#define HSPHED_BOOTOPT_WINHIDE 2   // 起動時ウインドウ非表示
#define HSPHED_BOOTOPT_DIRSAVE 4   // 起動時カレントディレクトリ変更なし
#define HSPHED_BOOTOPT_SAVER 0x100 // スクリーンセーバー

#define HSPHED_BOOTOPT_RUNTIME 0x1000   // 動的ランタイムを有効にする
#define HSPHED_BOOTOPT_NOMMTIMER 0x2000 // マルチメディアタイマーを無効にする
#define HSPHED_BOOTOPT_NOGDIP 0x4000    // GDI+による描画を無効にする
#define HSPHED_BOOTOPT_FLOAT32 0x8000   // 実数を32bit floatとして処理する
#define HSPHED_BOOTOPT_ORGRND 0x10000   // 標準の乱数発生を使用する
```

例：

```c
hsphed.bootoption |= HSPHED_BOOTOPT_RUNTIME;
```

動的ランタイムを有効にした場合は`runtime`にランタイム名のオフセットが格納されます。

## データの格納場所

- オプション領域: コンパイル時に指定されたランタイムなどのオプション情報が格納されています。
- コード領域: HSPの中間コードが格納されています。
- データ領域: 文字列や実数などのデータが格納されています。
- ラベル情報: ラベルに対応するコード領域の位置が格納されています。
- 行番号情報: 行番号に対応するコード領域の位置が格納されています。
- ライブラリ情報: DLLなどの外部呼び出しに関する情報が格納されています。
- 関数情報1と2: モジュールやDLL定義等の構造に関する情報が格納されています。
- モジュール情報: モジュールに関する情報が格納されています。
- HSPプラグイン情報: HSP3拡張プラグインに関する情報が格納されています。

HSPプラグイン情報のデータはHPIDAT構造体の配列として書き込まれます。

```c
typedef struct HPIDAT {
    short flag;     // フラグ情報
    short option;
    int   libname;  // ファイル名インデックス (データ領域)
    int   funcname; // 関数名インデックス (データ領域)
    int   p_libptr; // ライブラリハンドル
} HPIDAT;
```

`ibname`はDLL名、`funcname`は初期化関数名を示すデータ領域へのオフセットになります。`p_libptr`はランタイム実行時に書き込むための予約領域です。

HPIDAT構造体はHSP3拡張プラグインのためのものです。HSP2.x互換のプラグインは外部DLL呼び出し扱いとなるためHPIDATには含まれません。

## hsp3中間コードフォーマット

中間コードには2種類の形式が存在します。

32ビットコード:

```
│ EXFLG_3(0) │ EXFLG_2 │ EXFLG_1 │ EXFLG_0 │ Type │  +  | Code(16bit) |
```

48ビットコード:

```
│ EXFLG_3(1) │ EXFLG_2 │ EXFLG__1 │ EXFLG_0 │ Type │  +  | Code(32bit) │
```

それぞれ前半は５つのデータを合わせて16ビット、後半は16ビットか32ビットとなります。中間コードは16ビット(short値)が基本的な単位となります。

### 前半のデータ

前半のデータはマクロで定義されています。

```c
#define EXFLG_0 0x1000
#define EXFLG_1 0x2000
#define EXFLG_0 0x4000
#define EXFLG_3 0x8000
#define CSTYPE 0x0fff
```

それぞれのデータと値は次の通りです。

| データ | 値 | 説明 |
| ---- | ---- | ---- |
| Type | bit0～bit11 (0～0xfff) | 格納されるコードの種類 |
| EXFLG_0 | bit12 (0x1000) | パラメーターが演算や変数を含まない単一の値であることを示すフラグ |
| EXFLG_1 | bit13 (0x2000) | 行頭であることを示すフラグ(1ならば行頭となる) |
| EXFLG_2 | bit14 (0x4000) | パラメーターの区切りであることを示すフラグ(1ならば区切り) |
| EXFLG_3 | bit15 (0x8000) | 48ビットコードであることを示すフラグ |

Typeの値には次のマクロが使用されます。

| Type | マクロ | 種類 |
| ---- | ---- | ---- |
|  0 | TYPE_MARK      | 記号(code=文字コード) |
|  1 | TYPE_VAR       | ユーザー定義変数(code=変数ID) |
|  2 | TYPE_STRING    | 文字列(code=DSオフセット) |
|  3 | TYPE_DNUM      | 実数値(code=DSオフセット) |
|  4 | TYPE_INUM      | 整数値(code=値) |
|  5 | TYPE_STRUCT    | モジュール変数・構造体(code=minfoID) |
|  6 | TYPE_XLABEL    | 未使用 |
|  7 | TYPE_LABEL     | ラベル名(code=OTオフセット) |
|  8 | TYPE_INTCMD    | HSP内部(コア)命令(code=コマンドID) |
|  9 | TYPE_EXTCMD    | HSP拡張(機種依存)命令(code=コマンドID) |
| 10 | TYPE_EXTSYSVAR | HSP拡張(機種依存)システム変数(code=コマンドID) |
| 11 | TYPE_CMPCMD    | 比較命令(code=コマンドID) |
| 12 | TYPE_MODCMD    | ユーザー拡張命令・関数(code=コマンドID) |
| 13 | TYPE_INTFUNC   | HSP内部(コア)関数(code=コマンドID) |
| 14 | TYPE_SYSVAR    | HSP内部(コア)システム変数(code=コマンドID) |
| 15 | TYPE_PROGCMD   | プログラム制御命令(code=コマンドID) |
| 16 | TYPE_DLLFUNC   | 外部DLL拡張命令・関数(code=コマンドID) |
| 17 | TYPE_DLLCTRL   | 拡張DLLコントロールコマンド(code=コマンドID) |
| 18 | TYPE_USERDEF   | HSP3拡張プラグインコマンド(code=コマンドID) |

TYPE_USERDEFタイプ以降は、プラグインによって拡張された命令などで使用されていきます。(Type値の最大は、0x1fffになります)


### 後半のデータ

Codeの値が0x0000～0xffff(符号なし)までの場合はサイズ削減のために32ビットコードで格納されます。Codeの値が0x10000以上の場合は48ビットコードとなり32bitフルに格納されます。


## hsp3中間コード展開(パラメーター)

hsp3中間コードは命令に続いてパラメーターの記述が行なわれます。パラメーターは逆ポーランド法によりスタックを用いて演算が行なわれます。

実際にパラメーター演算時に行なう手順は以下の通りです。

- パラメーターが変数や固定値だった場合は値をスタックに積みます。(TYPE_VAR、TYPE_INUM等)
- パラメーターが演算子だった場合はスタックに積まれている２つの要素の演算を行ない、結果をスタックに積みます。(TYPE_MARK)

演算子として解釈されるCode値は、以下の通りです。

| Code値 | マクロ名 | 内容 |
| ---- | ---- | ---- 
|  0  | CALCCODE_ADD  | 加算(+) |
|  1  | CALCCODE_SUB  | 減算(-) |
|  2  | CALCCODE_MUL  | 積算(*) |
|  3  | CALCCODE_DIV  | 除算(/) |
|  4  | CALCCODE_MOD  | 余剰(\\) |
|  5  | CALCCODE_AND  | 論理積(&) |
|  6  | CALCCODE_OR   | 論理和(|) |
|  7  | CALCCODE_XOR  | 排他的論理和(\^) |
|  8  | CALCCODE_EQ   | 同一比較(=) |
|  9  | CALCCODE_NE   | 非同一比較(!) |
| 10  | CALCCODE_GT   | より大きい(<) |
| 11  | CALCCODE_LT   | より小さい(>) |
| 12  | CALCCODE_GTEQ | 以上(<=) |
| 13  | CALCCODE_LTEQ | 以下(>=) |
| 14  | CALCCODE_RR   | ビットシフト右(>>) |
| 15  | CALCCODE_LR   | ビットシフト左(<<) |

特殊なケースとしてCode値に0x3f(63)が使用されることがあります。これはパラメーターの省略を行なった場合にダミーのコードとして挿入されるものです。

例えば「1,,2」のような省略を含むパラメーターの指定時には、「1」「ダミー」「2」のような形でダミーが挿入されます。


## hsp3中間コード展開(変数)

中間コードで`TYPE_VAR`のタイプを持つパラメーターは変数の指定を示しています。このタイプのCode値は、変数IDを示します。変数IDは、ランタイムが持つコンテキスト(HSPCTX構造体)内の、`mem_var`メンバが保持する変数情報配列の要素になります。つまり、`hspctx->mem_var[0]`がID0の変数になります。

また、配列の指定が行なわれている場合には、直後に`TYPE_MARK`タイプで'('のCodeを持つ中間コードが続きます。その後、配列要素がパラメーターと同様に展開され、最後に`TYPE_MARK`タイプで')'のCodeを持つ中間コードが現われます。

行頭(最初の中間コード)に、`TYPE_VAR`のタイプを持つコードがある場合は、変数代入を示します。変数に続いて、代入演算子が`TYPE_VAR`のタイプを持つパラメーターとして指定されます。代入演算子のコードは、パラメーターと同様のマクロ`(CALCCODE_*)`を利用することが可能です。


## hsp3中間コード展開(比較)

中間コードで、`TYPE_CMPCMD`のタイプを持つコードは、比較命令を示しています。このタイプのCode値は、0の場合if命令を、1の場合else命令を示します。比較命令の場合に限り、中間コードの直後に16bitのオフセットが埋め込まれています。オフセットは、比較命令が正しくなかった時(if命令以降をスキップする場合)のCS内ジャンプ先が相対値として記録されています。


## Debug Info(DINFO)詳細

Debug Info(DINFO)猟奇には、スクリプトのデバッグに関する情報が格納されています。主に、ランタイム上でエラーの表示や、デバッグウィンドウ上での表示補助に利用されます。(DINFOのデータは、HSP2.xのデバッグデータの上位互換になっています。)Debug Info(DINFO)は、リリース時(exeファイル作成時)のオブジェクトには含まれません。

DINFOのデータは符号なし8bitデータの列が続いています。

| 値 | 内容 |
| ---- | ---- |
| 255 | データ列の終端 |
| 254 | ソースファイル指定(*1) |
| 253 | 変数名データ指定(*2) |
| 252 | 次の16bit値が、次行までのCSオフセット |
| その他 | 次行までのCSオフセット |

*1
次の24bit値がソースファイル名を示すデータ領域のオフセットとなる。
さらに続く16bit値が参照される行番号の値となる。
データ領域のオフセットが0の場合は行番号のみの指定となる。

*2
次の24bit値が変数名へのデータ領域のオフセットとなる。
さらに続く16bit値はダミー値として0が格納されている。
このデータは、変数IDの順にDebug Info(DINFO)領域内に連続して設定されている。

## 付録: 予約キーワードのtyepとコード

HSPに標準で設定されている予約キーワードのtypeとコードは以下の通りです。

hspcmd.cppから抜粋:

```c
char 	s_rec[1]= "", *hsp_prestr[] =
{
	//
	//	label check
	//	  | opt value
	//	  | |
	//	  | | type
	//	  | | | keyword
	//	  | | |  |
	//	  | | |  |
	//	"$000 8 goto",
	//
	s_rec,

	//	program control commands (ver3.0)

	"$000 15 goto",
	"$001 15 gosub",
	"$002 15 return",
	"$003 15 break",
	"$004 15 repeat",
	"$005 15 loop",
	"$006 15 continue",
	"$007 15 wait",
	"$008 15 await",

	"$009 15 dim",
	"$00a 15 sdim",
//	"$00d 15 skiperr",				// delete
	"$00b 15 foreach",				// (ver3.0)
//	"$00c 15 eachchk",				// (ver3.0) hidden
	"$00d 15 dimtype",				// (ver3.0)
	"$00e 15 dup",
	"$00f 15 dupptr",				// (ver3.0)

	"$010 15 end",
	"$011 15 stop",
	"$012 15 newmod",				// (ver3.0)
//	"$013 15 setmod",				// (ver3.0)
	"$014 15 delmod",				// (ver3.0)
//	"$015 15 alloc",				// (changed 3.0)
	"$016 15 mref",					// (ver2.5)
	"$017 15 run",
	"$018 15 exgoto",				// ver2.6
	"$019 15 on",					// ver2.6
	"$01a 15 mcall",				// (ver3.0)
	"$01b 15 assert",				// (ver3.0)
	"$01c 15 logmes",				// (ver3.0)
	"$01d 15 newlab",				// (ver3.2)
	"$01e 15 resume",				// (ver3.2)
	"$01f 15 yield",				// (ver3.2)
	"$020 15 strexchange",			// (ver3.6)

//	"$015 15 logmode",				// (ver2.55)
//	"$016 15 logmes",				// (ver2.55)

	//	enhanced command (ver2.6)

	"$10000 8 onexit",
	"$10001 8 onerror",
	"$10002 8 onkey",
	"$10003 8 onclick",
	"$10004 8 oncmd",

	"$011 8 exist",
	"$012 8 delete",
	"$013 8 mkdir",
	"$014 8 chdir",

	"$015 8 dirlist",
	"$016 8 bload",
	"$017 8 bsave",
	"$018 8 bcopy",
	"$019 8 memfile",				// (changed on ver2.6*)

	//	no macro command (ver2.61)
	//
	"$000 11 if",
	"$001 11 else",

	//	normal commands

	"$01a 8 poke",
	"$01b 8 wpoke",
	"$01c 8 lpoke",
	"$01d 8 getstr",
	"$01e 8 chdpm",					// (3.0)
	"$01f 8 memexpand",				// (3.0)
	"$020 8 memcpy",				// (ver2.55)
	"$021 8 memset",				// (ver2.55)

	"$022 8 notesel",				// (changed on ver2.55)
	"$023 8 noteadd",				// (changed on ver2.55)
	"$024 8 notedel",				// (changed on ver2.55)
	"$025 8 noteload",				// (changed on ver2.6*)
	"$026 8 notesave",				// (changed on ver2.6*)
	"$027 8 randomize",				// (changed on ver3.0)
	"$028 8 noteunsel",				// (changed on ver3.0)
	"$029 8 noteget",				// (changed on ver2.55)
	"$02a 8 split",					// (3.2)
	"$02b 8 strrep",				// (3.4)
	"$02c 8 setease",				// (3.4)
	"$02d 8 sortval",				// (3.5)
	"$02e 8 sortstr",				// (3.5)
	"$02f 8 sortnote",				// (3.5)
	"$030 8 sortget",				// (3.5)

	//	enhanced command (ver2.2)

	"$10000 9 button",

	"$001 9 chgdisp",
	"$002 9 exec",
	"$003 9 dialog",

//	"$007 9 palfade",				// delete
//	"$009 9 palcopy",				// delete

	"$008 9 mmload",
	"$009 9 mmplay",
	"$00a 9 mmstop",
	"$00b 9 mci",

	"$00c 9 pset",
	"$00d 9 pget",
	"$00e 9 syscolor",				// (ver3.0)

	"$00f 9 mes",
	"$00f 9 print",
	"$010 9 title",
	"$011 9 pos",
	"$012 9 circle",				// (ver3.0)
	"$013 9 cls",
	"$014 9 font",
	"$015 9 sysfont",
	"$016 9 objsize",
	"$017 9 picload",
	"$018 9 color",
	"$019 9 palcolor",
	"$01a 9 palette",
	"$01b 9 redraw",
	"$01c 9 width",
	"$01d 9 gsel",
	"$01e 9 gcopy",
	"$01f 9 gzoom",
	"$020 9 gmode",
	"$021 9 bmpsave",

//	"$022 9 text",					// delete

	"$022 9 hsvcolor",				// (ver3.0)
	"$023 9 getkey",

	"$024 9 listbox",
	"$025 9 chkbox",
	"$026 9 combox",

	"$027 9 input",
	"$028 9 mesbox",
	"$029 9 buffer",
	"$02a 9 screen",
	"$02b 9 bgscr",

	"$02c 9 mouse",
	"$02d 9 objsel",
	"$02e 9 groll",
	"$02f 9 line",

	"$030 9 clrobj",
	"$031 9 boxf",

	//	enhanced command (ver2.3)

	"$032 9 objprm",
	"$033 9 objmode",
	"$034 9 stick",
//	"$041 9 objsend",				// delete
	"$035 9 grect",					// (ver3.0)
	"$036 9 grotate",				// (ver3.0)
	"$037 9 gsquare",				// (ver3.0)
	"$038 9 gradf",					// (ver3.2)
	"$039 9 objimage",				// (ver3.2)
	"$03a 9 objskip",				// (ver3.2)
	"$03b 9 objenable",				// (ver3.2)
	"$03c 9 celload",				// (ver3.2)
	"$03d 9 celdiv",				// (ver3.2)
	"$03e 9 celput",				// (ver3.2)

	"$03f 9 gfilter",				// (ver3.5)
	"$040 9 setreq",				// (ver3.5)
	"$041 9 getreq",				// (ver3.5)
	"$042 9 mmvol",					// (ver3.5)
	"$043 9 mmpan",					// (ver3.5)
	"$044 9 mmstat",				// (ver3.5)
	"$045 9 mtlist",				// (ver3.5)
	"$046 9 mtinfo",				// (ver3.5)
	"$047 9 devinfo",				// (ver3.5)
	"$048 9 devinfoi",				// (ver3.5)
	"$049 9 devprm",				// (ver3.5)
	"$04a 9 devcontrol",			// (ver3.5)
	"$04b 9 httpload",				// (ver3.5)
	"$04c 9 httpinfo",				// (ver3.5)
	"$05d 9 gmulcolor",				// (ver3.5)
	"$05e 9 setcls",				// (ver3.5)
	"$05f 9 celputm",				// (ver3.5)

	"$05c 9 celbitmap",				// (ver3.6)
	"$04d 9 objcolor",				// (ver3.6)
	"$04e 9 rgbcolor",				// (ver3.6)
	"$04f 9 viewcalc",				// (ver3.6)
	"$050 9 layerobj",				// (ver3.6)

	//	enhanced command (ver3.0)

	"$000 17 newcom",
	"$001 17 querycom",
	"$002 17 delcom",
	"$003 17 cnvstow",
	"$004 17 comres",
	"$005 17 axobj",
	"$006 17 winobj",
	"$007 17 sendmsg",
	"$008 17 comevent",
	"$009 17 comevarg",
	"$00a 17 sarrayconv",
	//"$00b 17 variantref",
	"$00c 17 cnvstoa",				// (ver3.5)

	"$100 17 callfunc",
	"$101 17 cnvwtos",
	"$102 17 comevdisp",
	"$103 17 libptr",
	"$104 17 cnvatos",				// (ver3.5)

	//	3.0 system vals

	"$000 14 system",
	"$001 14 hspstat",
	"$002 14 hspver",
	"$003 14 stat",
	"$004 14 cnt",
	"$005 14 err",
	"$006 14 strsize",
	"$007 14 looplev",					// (ver2.55)
	"$008 14 sublev",					// (ver2.55)
	"$009 14 iparam",					// (ver2.55)
	"$00a 14 wparam",					// (ver2.55)
	"$00b 14 lparam",					// (ver2.55)
	"$00c 14 refstr",
	"$00d 14 refdval",					// (3.0)

	//	3.0 internal function
	"$000 13 int",
	"$001 13 rnd",
	"$002 13 strlen",
	"$003 13 length",					// (3.0)
	"$004 13 length2",					// (3.0)
	"$005 13 length3",					// (3.0)
	"$006 13 length4",					// (3.0)
	"$007 13 vartype",					// (3.0)
	"$008 13 gettime",
	"$009 13 peek",
	"$00a 13 wpeek",
	"$00b 13 lpeek",
	"$00c 13 varptr",					// (3.0)
	"$00d 13 varuse",					// (3.0)
	"$00e 13 noteinfo",					// (3.0)
	"$00f 13 instr",
	"$010 13 abs",						// (3.0)
	"$011 13 limit",					// (3.0)
	"$012 13 getease",					// (3.4)
	"$013 13 notefind",					// (3.5)
	"$014 13 varsize",					// (3.5)

	//	3.0 string function
	"$100 13 str",
	"$101 13 strmid",
	"$103 13 strf",
	"$104 13 getpath",
	"$105 13 strtrim",					// (3.2)

	//	3.0 math function
	"$180 13 sin",
	"$181 13 cos",
	"$182 13 tan",
	"$183 13 atan",
	"$184 13 sqrt",
	"$185 13 double",
	"$186 13 absf",
	"$187 13 expf",
	"$188 13 logf",
	"$189 13 limitf",
	"$18a 13 powf",						// (3.3)
	"$18b 13 geteasef",					// (3.4)

	//	3.0 external sysvar,function

	"$000 10 mousex",
	"$001 10 mousey",
	"$002 10 mousew",					// (3.0)
	"$003 10 hwnd",						// (3.0)
	"$004 10 hinstance",				// (3.0)
	"$005 10 hdc",						// (3.0)

	"$100 10 ginfo",
	"$101 10 objinfo",
	"$102 10 dirinfo",
	"$103 10 sysinfo",

	"$ffffffff 5 thismod",

	"*"
};
```

・更新履歴

	2011/05/12 ver0.2

		HSP3.3に合わせて一部改訂。

	2007/11/01 ver0.1

		最初のバージョン。


・連絡先

OpenHSPに関するお問い合わせ、感想、ご意見などは以下のメールアドレスまでお寄せ下さい。
メールの返信につきましては、時間がかかる場合がありますので予めご了承下さい。

        Homepage: http://www.onionsoft.net/openhsp/
                  http://hsp.tv/
        e-mail  : onitama@onionsoft.net

-------------------------------------------------------------------------------
                                                HSP users manual / end of file 
-------------------------------------------------------------------------------
