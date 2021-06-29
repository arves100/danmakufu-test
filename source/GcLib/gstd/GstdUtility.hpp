#ifndef __GSTD_UTILITIY__
#define __GSTD_UTILITIY__

#include "GstdConstant.hpp"


namespace gstd {

//================================================================
//DebugUtility (Windows/MSVC only memory leak dump)
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
class DebugUtility {
public:
	static void DumpMemoryLeaksOnExit();
};
#endif

//================================================================
//StringUtility
class StringUtility {
public:
	static std::string ConvertWideToMulti(std::wstring const& wstr, int codeMulti = 932);
	static std::wstring ConvertMultiToWide(std::string const& str, int codeMulti = 932);

	//----------------------------------------------------------------
	static std::vector<std::string> Split(std::string str, std::string delim);
	static void Split(std::string str, std::string delim, std::vector<std::string>& res);
	static std::string Format(const char* str, ...);

	static size_t CountCharacter(std::string& str, char c);
	static size_t CountCharacter(std::vector<char>& str, char c);
	static int ToInteger(std::string const& s);
	static double ToDouble(std::string const& s);
	static std::string Replace(std::string& source, std::string pattern, std::string placement);
	static std::string ReplaceAll(std::string& source, std::string pattern, std::string placement, size_t replaceCount = INT_MAX, size_t start = 0, size_t end = 0);
	static std::string Slice(std::string const& s, size_t length);
	static std::string Trim(const std::string& str);
};

//================================================================
//ErrorUtility
class ErrorUtility {
public:
	enum {
		ERROR_FILE_NOTFOUND,
		ERROR_PARSE,
		ERROR_END_OF_FILE,
		ERROR_OUTOFRANGE_INDEX,
	};

public:
#ifdef _WIN32
	static std::string GetLastErrorMessage(DWORD error);
#else
	static std::string GetLastErrorMessage(errno_t error);
#endif
	static std::string GetLastErrorMessage();
	static std::string GetErrorMessage(int type);
	static std::string GetFileNotFoundErrorMessage(std::string path);
	static std::string GetParseErrorMessage(int line, std::string what);
	static std::string GetParseErrorMessage(std::string path, int line, std::string what);
};

//================================================================
//Math
const double PAI = 3.14159265358979323846;
class Math {
public:
	inline static double DegreeToRadian(double angle) { return angle * PAI / 180; }
	inline static double RadianToDegree(double angle) { return angle * 180 / PAI; }

	static void InitializeFPU();

	static double Round(double val) { return floorl(val + 0.5); }
};

//================================================================
//ByteOrder
class ByteOrder {
public:
	enum {
		ENDIAN_LITTLE,
		ENDIAN_BIG,
	};

public:
	static void Reverse(void* buf, size_t size);
};

//================================================================
//Sort
class SortUtility {
public:
	template <class BidirectionalIterator, class Predicate>
	static void CombSort(BidirectionalIterator first,
		BidirectionalIterator last,
		Predicate pr)
	{
		int gap = static_cast<int>(std::distance(first, last));
		if (gap < 1)
			return;

		BidirectionalIterator first2 = last;
		bool swapped = false;
		do {
			int newgap = (gap * 10 + 3) / 13;
			if (newgap < 1)
				newgap = 1;
			if (newgap == 9 || newgap == 10)
				newgap = 11;
			std::advance(first2, newgap - gap);
			gap = newgap;
			swapped = false;
			for (BidirectionalIterator target1 = first, target2 = first2;
				 target2 != last;
				 ++target1, ++target2) {
				if (pr(*target2, *target1)) {
					std::iter_swap(target1, target2);
					swapped = true;
				}
			}
		} while ((gap > 1) || swapped);
	}
};

//================================================================
//PathProperty
class PathProperty {
public:
	static std::string GetFileDirectory(std::string path)
	{
#ifdef _WIN32
		wchar_t pDrive[_MAX_PATH];
		wchar_t pDir[_MAX_PATH];
		_wsplitpath(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), pDrive, pDir, nullptr, nullptr);
		return StringUtility::ConvertWideToMulti(pDrive, CP_UTF8) + StringUtility::ConvertWideToMulti(pDir, CP_UTF8);
#else
		return ""; // TODO: Linux
#endif
	}

	static std::string GetDirectoryName(std::string path)
	{
		//ディレクトリ名を返す
		std::string dir = GetFileDirectory(path);
		dir = StringUtility::ReplaceAll(dir, "\\", "/");
		std::vector<std::string> strs = StringUtility::Split(dir, "/");
		return strs[strs.size() - 1];
	}

	static std::string GetFileName(std::string path)
	{
#ifdef _WIN32
		wchar_t pFileName[_MAX_PATH];
		wchar_t pExt[_MAX_PATH];
		_wsplitpath(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), nullptr, nullptr, pFileName, pExt);
		return StringUtility::ConvertWideToMulti(pFileName, CP_UTF8) + StringUtility::ConvertWideToMulti(pExt, CP_UTF8);
#else
		return ""; // TODO: Linux
#endif
	}

	static std::string GetDriveName(std::string path)
	{
#ifdef _WIN32
		wchar_t pDrive[_MAX_PATH];
		_wsplitpath(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), pDrive, nullptr, nullptr, nullptr);
		return StringUtility::ConvertWideToMulti(pDrive, CP_UTF8);
#else
		return ""; // TODO: Linux
#endif
	}

	static std::string GetFileNameWithoutExtension(std::string path)
	{
#ifdef _WIN32
		wchar_t pFileName[_MAX_PATH];
		_wsplitpath(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), nullptr, nullptr, pFileName, nullptr);
		return StringUtility::ConvertWideToMulti(pFileName, CP_UTF8);
#else
		return ""; // TODO: Linux
#endif
	}

	static std::string GetFileExtension(std::string path)
	{
#ifdef _WIN32
		wchar_t pExt[_MAX_PATH];
		_wsplitpath(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), nullptr, nullptr, nullptr, pExt);
		return StringUtility::ConvertWideToMulti(pExt, CP_UTF8);
#else
		return ""; // TODO: Linux
#endif
	}

	static std::string GetModuleName()
	{
		// Having the burden to incorporate module file name for POSIX platforms (which is not unified) + Windows one for a logging file, better to keep it hardcoded
		return "Danmakufu";
	}

	static std::string GetModuleDirectory()
	{
		char* pStr = SDL_GetBasePath();
		std::string ret = pStr;
		SDL_free(pStr);
		return ret;
	}

	static std::string GetDirectoryWithoutModuleDirectory(std::string path)
	{	//パスから実行ファイルディレクトリを除いたディレクトリを返す
		std::string res = GetFileDirectory(path);
		std::string dirModule = GetModuleDirectory();
		if (res.find(dirModule) != std::string::npos) {
			res = res.substr(dirModule.size());
		}
		return res;
	}
	static std::string GetPathWithoutModuleDirectory(std::string path)
	{	//パスから実行ファイルディレクトリを取り除く
		std::string dirModule = GetModuleDirectory();
		dirModule = ReplaceYenToSlash(dirModule);
		path = Canonicalize(path);
		path = ReplaceYenToSlash(path);

		std::string res = path;
		if (res.find(dirModule) != std::string::npos) {
			res = res.substr(dirModule.size());
		}
		return res;
	}
	static std::string GetRelativeDirectory(std::string from, std::string to)
	{
#if _WIN32
		wchar_t path[_MAX_PATH];
		std::wstring fromW = StringUtility::ConvertMultiToWide(from, CP_UTF8), toW = StringUtility::ConvertMultiToWide(to, CP_UTF8);
		BOOL b = PathRelativePathToW(path, fromW.c_str(), FILE_ATTRIBUTE_DIRECTORY, toW.c_str(), FILE_ATTRIBUTE_DIRECTORY);

		std::string res = "";
		if (b) {
			res = GetFileDirectory(StringUtility::ConvertWideToMulti(path, CP_UTF8));
		}
		return res;
#else
		throw std::runtime_error("LINUX: Not supported"); // TODO: Linux
#endif
	}
	static std::string ReplaceYenToSlash(std::string path)
	{
		std::string res = StringUtility::ReplaceAll(path, "\\", "/");
		return res;
	}
	static std::string Canonicalize(std::string srcPath)
	{
#if _WIN32
		wchar_t destPath[_MAX_PATH];
		std::wstring srcPathW = StringUtility::ConvertMultiToWide(srcPath, CP_UTF8);
		PathCanonicalizeW(destPath, srcPathW.c_str());
		return StringUtility::ConvertWideToMulti(destPath, CP_UTF8);
#else
		return srcPath; // TODO: Linux
#endif
	}
	static std::string GetUnique(std::string srcPath)
	{
		std::string res = StringUtility::ReplaceAll(srcPath, "/", "\\");
		res = Canonicalize(res);
		res = ReplaceYenToSlash(res);
		return res;
	}
};

//================================================================
//BitAccess
class BitAccess {
public:
	template <typename T>
	static bool GetBit(T value, int bit)
	{
		T mask = (T)1 << bit;
		return (value & mask) != 0;
	}
	template <typename T>
	static T& SetBit(T& value, int bit, bool b)
	{
		T mask = (T)1 << bit;
		T write = (T)b << bit;
		value &= ~mask;
		value |= write;
		return value;
	}
	template <typename T>
	static unsigned char GetByte(T value, int bit)
	{
		return (unsigned char)(value >> bit);
	}
	template <typename T>
	static T& SetByte(T& value, int bit, unsigned char c)
	{
		T mask = (T)0xff << bit;
		T write = (T)c << bit;
		value &= ~mask;
		value |= write;
		return value;
	}
};

//================================================================
//IStringInfo
class IStringInfo {
public:
	virtual ~IStringInfo() {}
	virtual std::string GetInfoAsString()
	{
		ptrdiff_t address = (ptrdiff_t)this;
		char* name = (char*)typeid(*this).name();
		return StringUtility::Format("%s[%08x]", name, address);
	}
};

//================================================================
//InnerClass
//C++には内部クラスがないので、外部クラスアクセス用
template <class T>
class InnerClass {
public:
	InnerClass(T* outer = NULL) { outer_ = outer; }

protected:
	T* _GetOuter() { return outer_; }
	void _SetOuter(T* outer) { outer_ = outer; }

private:
	T* outer_;
};

//================================================================
//Singleton
template <class T>
class Singleton {
public:
	virtual ~Singleton(){};
	static T* CreateInstance()
	{
		if (_This() == NULL)
			_This() = new T();
		return _This();
	}
	static T* GetInstance()
	{
		if (_This() == NULL) {
			throw gstd::wexception(L"Singleton::GetInstance 未初期化");
		}
		return _This();
	}
	static void DeleteInstance()
	{
		if (_This() != NULL)
			delete _This();
		_This() = NULL;
	}

protected:
	Singleton(){};
	inline static T*& _This()
	{
		static T* s = NULL;
		return s;
	}
};

//================================================================
//Scanner
class Scanner;
class Token {
	friend Scanner;

public:
	enum class Type {
		Unknown,
		EndOfFile,
		Newline,
		ID,
		Int,
		Real,
		String,

		OpenP,
		CloseP,
		OpenB,
		CloseB,
		OpenC,
		CloseC,
		Sharp,
		Pipe,
		Ampersand,

		Comma,
		Period,
		Equal,
		Asterisk,
		Slash,
		Colon,
		Semicolon,
		Tilde,
		Exclamation,
		Plus,
		Minus,
		Less,
		Greater,
	};

public:
	Token()
	{
		type_ = Type::Unknown;
		posStart_ = 0;
		posEnd_ = 0;
	}
	Token(Type type, std::string& element, int start, int end)
	{
		type_ = type;
		element_ = element;
		posStart_ = start;
		posEnd_ = end;
	}
	virtual ~Token(){};

	Type GetType() { return type_; }
	std::string& GetElement() { return element_; }

	int GetStartPointer() { return posStart_; }
	int GetEndPointer() { return posEnd_; }

	int GetInteger();
	double GetReal();
	bool GetBoolean();
	std::string GetString();
	std::string GetIdentifier();

protected:
	Type type_;
	std::string element_;
	int posStart_;
	int posEnd_;
};

class Scanner {
public:
	enum {

	};

public:
	Scanner(char* str, int size);
	Scanner(std::string str);
	Scanner(std::vector<char> buf);
	virtual ~Scanner();

	void SetPermitSignNumber(bool bEnable) { bPermitSignNumber_ = bEnable; }

	Token& GetToken(); //現在のトークンを取得
	Token& Next();
	bool HasNext();
	void CheckType(Token& tok, Token::Type type);
	void CheckIdentifer(Token& tok, std::string id);
	int GetCurrentLine();

	int GetCurrentPointer();
	void SetCurrentPointer(int pos);
	std::string GetString(int start, int end);

	bool CompareMemory(int start, int end, const char* data);

private:
	void Init(std::vector<char>& buf);

protected:
	void _Initialize(std::vector<char> buffer);

	std::vector<char> buffer_;
	int pointer_; //今の位置
	Token token_; //現在のトークン
	bool bPermitSignNumber_;
	std::list<Token> listDebugToken_;

	char _CurrentChar();
	char _NextChar(); //ポインタを進めて次の文字を調べる

	virtual void _SkipComment(); //コメントをとばす
	virtual void _SkipSpace(); //空白をとばす
	virtual void _RaiseError(std::string str); //例外を投げます
};

//================================================================
//TextParser
class TextParser {
public:
	enum class Type {
		Real,
		Boolean,
		String,
	};

	class Result {
		friend TextParser;

	protected:
		Type type_;
		int pos_;
		std::string valueString_;
		union {
			double valueReal_;
			bool valueBoolean_;
		};

	public:
		Result() : type_(Type::Real), pos_(0), valueString_(""), valueReal_(0.0) {}

		virtual ~Result(){};
		Type GetType() { return type_; }
		double GetReal()
		{
			double res = valueReal_;
			if (IsBoolean())
				res = valueBoolean_ ? 1.0 : 0.0;
			if (IsString())
				res = StringUtility::ToDouble(valueString_);
			return res;
		}
		void SetReal(double value)
		{
			type_ = Type::Real;
			valueReal_ = value;
		}
		bool GetBoolean()
		{
			bool res = valueBoolean_;
			if (IsReal())
				res = (valueReal_ != 0.0 ? true : false);
			if (IsString())
				res = (valueString_ == "true" ? true : false);
			return res;
		}
		void SetBoolean(bool value)
		{
			type_ = Type::Boolean;
			valueBoolean_ = value;
		}
		std::string GetString()
		{
			std::string res = valueString_;
			if (IsReal())
				res = gstd::StringUtility::Format("%f", valueReal_);
			if (IsBoolean())
				res = (valueBoolean_ ? "true" : "false");
			return res;
		}
		void SetString(std::string value)
		{
			type_ = Type::String;
			valueString_ = value;
		}
		bool IsReal() { return type_ == Type::Real; }
		bool IsBoolean() { return type_ == Type::Boolean; }
		bool IsString() { return type_ == Type::String; }
	};

public:
	TextParser();
	TextParser(std::string source);
	virtual ~TextParser();

	void SetSource(std::string source);
	Result GetResult();
	double GetReal();

protected:
	std::unique_ptr<Scanner> scan_;

	void _RaiseError(std::string message);
	Result _ParseComparison(int pos);
	Result _ParseSum(int pos);
	Result _ParseProduct(int pos);
	Result _ParseTerm(int pos);
	virtual Result _ParseIdentifer(int pos);
};

//================================================================
//Font
#ifdef _WIN32
class Font {
public:
	Font();
	virtual ~Font();
	void CreateFont(const wchar_t* type, int size, bool bBold = false, bool bItalic = false, bool bLine = false);
	void CreateFontIndirect(LOGFONT& fontInfo);
	void Clear();
	HFONT GetHandle() { return hFont_; }
	LOGFONT GetInfo() { return info_; }

public:
	const static wchar_t* GOTHIC;
	const static wchar_t* MINCHOH;

protected:
	HFONT hFont_;
	LOGFONT info_;
};
#endif

//================================================================
//ObjectPool
#if 0
template <class T, bool SYNC>
class ObjectPool {
public:
	ObjectPool() {}
	virtual ~ObjectPool() {}
	virtual gstd::shared_ptr<T, SYNC> GetPoolObject(int type)
	{
		gstd::shared_ptr<T, SYNC> res = NULL;
		if (listCachePool_[type].size() > 0) {
			res = listCachePool_[type].back();
			listCachePool_[type].pop_back();
		} else {
			res = _CreatePoolObject(type);
		}
		listUsedPool_[type].push_back(res);
		return res;
	}

	int GetUsedPoolObjectCount()
	{
		int res = 0;
		for (int i = 0; i < listUsedPool_.size(); i++) {
			res += listUsedPool_[i].size();
		}
		return res;
	}

	int GetCachePoolObjectCount()
	{
		int res = 0;
		for (int i = 0; i < listCachePool_.size(); i++) {
			res += listCachePool_[i].size();
		}
		return res;
	}

protected:
	std::vector<std::list<gstd::shared_ptr<T, SYNC>>> listUsedPool_;
	std::vector<std::vector<gstd::shared_ptr<T, SYNC>>> listCachePool_;

	virtual void _CreatePool(int countType)
	{
		listUsedPool_.resize(countType);
		listCachePool_.resize(countType);
	}
	virtual gstd::shared_ptr<T, SYNC> _CreatePoolObject(int type) = 0;
	virtual void _ResetPoolObject(gstd::shared_ptr<T, SYNC>& obj) {}
	virtual void _ArrangePool()
	{
		int countType = listUsedPool_.size();
		for (int iType = 0; iType < countType; iType++) {
			std::list<gstd::shared_ptr<T, SYNC>>* listUsed = &listUsedPool_[iType];
			std::vector<gstd::shared_ptr<T, SYNC>>* listCache = &listCachePool_[iType];

#ifdef __GNUC__
			typename std::list<gstd::ref_count_ptr<T, SYNC>>::iterator itr = listUsed->begin();
#else
			std::list<gstd::ref_count_ptr<T, SYNC>>::iterator itr = listUsed->begin();
#endif
			for (; itr != listUsed->end();) {
				gstd::shared_ptr<T, SYNC> obj = (*itr);
				if (obj.GetReferenceCount() == 2) {
					itr = listUsed->erase(itr);
					_ResetPoolObject(obj);
					listCache->push_back(obj);
				} else {
					itr++;
				}
			}
		}
	}
};
#endif

} // namespace gstd

#endif
