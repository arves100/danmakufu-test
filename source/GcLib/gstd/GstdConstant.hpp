#ifndef __GSTD_GAMESTDCONSTANT__
#define __GSTD_GAMESTDCONSTANT__

//標準関数対応表
//http://www1.kokusaika.jp/advisory/org/ja/win32_unicode.html

// MSVC warning pragmas (msvc specific, TODO: wipe later and migrate to cmake)
#ifdef _MSC_VER
#pragma warning(disable : 4786) //STL Warning抑止
#pragma warning(disable : 4018) //signed と unsigned の数値を比較
#pragma warning(disable : 4244) //double' から 'float' に変換
#pragma warning(disable : 4503) //

#pragma warning(disable : 4302) // 切り詰めます。
#pragma warning(disable : 4305) // 'double' から 'FLOAT' へ切り詰めます。
#pragma warning(disable : 4819) //ファイルは、現在のコード ページ (932) で表示できない文字を含んでいます。データの損失を防ぐために、ファイルを Unicode 形式で保存してください。
#pragma warning(disable : 4996) //This function or variable may be unsafe.
#endif

// https://docs.microsoft.com/en-us/windows/win32/winprog/enabling-strict
#if defined(_WIN32) && !defined(STRICT)
#define STRICT 1 
#endif

// C STD Library
#include <cstdlib>
#include <cwchar>
#include <cstdint>
#include <climits>
#include <cmath>
#include <cstdlib>

// C++ STL
#include <algorithm>
#include <bitset>
#include <exception>
#include <stdexcept>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#ifdef _WIN32

// Windows main inclusions

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1 // Exclude uncommon stuff from Windows API
#endif
#ifndef NOMINMAX
#define NOMINMAX 1 // pls nt
#endif

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

// for acm
#include <mmreg.h> 
#include <msacm.h>

// Visual C++ memory leak detection
#if defined(_DEBUG) && defined(_MSC_VER)
	#define MSCV_LEAK_DETECTION 1
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define new ::new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#else

// POSIX main inclusions

#define MSCV_LEAK_DETECTION 0

#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h> // MIN, MAX

// Define macros for compatibility with Windows code

#define _MAX_PATH PATH_MAX
#define MAX_PATH _MAX_PATH

#define ZeroMemory(dst, size) memset(dst, 0, size)
#define Sleep sleep
#define _vsnprintf vsnprintf

// Type definitions for compatibility with Windows code
typedef int errno_t;

#endif // defined(_WIN32)


#define _MAX(a,b) ((a > b) ? (a) : (b))
#define _MIN(a,b) ((a < b) ? (a) : (b))

#ifdef __GNUC__
	#define _int64 long long
#endif // defined(__GNUC__)


// Simple Direct Library 2 main inclusion
#include <SDL.h>

#endif
