#ifndef __GSTD_GAMESTDCONSTANT__
#define __GSTD_GAMESTDCONSTANT__

//標準関数対応表
//http://www1.kokusaika.jp/advisory/org/ja/win32_unicode.html

//pragma
#pragma warning(disable : 4786) //STL Warning抑止
#pragma warning(disable : 4018) //signed と unsigned の数値を比較
#pragma warning(disable : 4244) //double' から 'float' に変換
#pragma warning(disable : 4503) //

#pragma warning(disable : 4302) // 切り詰めます。
#pragma warning(disable : 4305) // 'double' から 'FLOAT' へ切り詰めます。
#pragma warning(disable : 4819) //ファイルは、現在のコード ページ (932) で表示できない文字を含んでいます。データの損失を防ぐために、ファイルを Unicode 形式で保存してください。
#pragma warning(disable : 4996) //This function or variable may be unsafe.

//define
#ifndef STRICT
#define STRICT 1
#endif

//std
#include <cstdlib>
#include <cwchar>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

//Windows
#include <windows.h>
#include <windowsx.h>

#include <commctrl.h>
#include <mmsystem.h>
#include <pdh.h>
#include <process.h>
#include <shlwapi.h>
#include <wingdi.h>

#include <mlang.h>
#include <psapi.h>

#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new ::new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <SDL.h>

#endif
