#include "GstdUtility.hpp"
#include "Logger.hpp"
using namespace gstd;

//================================================================
//DebugUtility
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
void DebugUtility::DumpMemoryLeaksOnExit()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}
#endif

//================================================================
//Encoding
#ifdef _WIN32
//================================================================
//StringUtility
std::string StringUtility::ConvertWideToMulti(std::wstring const& wstr, int codeMulti)
{
	if (wstr == L"")
		return "";

	//マルチバイト変換後のサイズを調べます
	//WideCharToMultiByteの第6引数に0を渡すと変換後のサイズが返ります
	int sizeMulti = ::WideCharToMultiByte(codeMulti, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (sizeMulti == 0)
		return ""; //失敗(とりあえず空文字とします)

	//最後の\0が含まれるため
	sizeMulti = sizeMulti - 1;

	//マルチバイトに変換します
	std::string str;
	str.resize(sizeMulti);
	::WideCharToMultiByte(codeMulti, 0, wstr.c_str(), -1, &str[0],
		sizeMulti, NULL, NULL);
	return str;
}

std::wstring StringUtility::ConvertMultiToWide(std::string const& str, int codeMulti)
{
	if (str == "")
		return L"";

	//UNICODE変換後のサイズを調べます
	//MultiByteToWideCharの第6引数に0を渡すと変換後のサイズが返ります
	int sizeWide = ::MultiByteToWideChar(codeMulti, 0, str.c_str(), -1, NULL, 0);
	if (sizeWide == 0)
		return L""; //失敗(とりあえず空文字とします)

	//最後の\0が含まれるため
	sizeWide = sizeWide - 1;

	//UNICODEに変換します
	std::wstring wstr;
	wstr.resize(sizeWide);
	::MultiByteToWideChar(codeMulti, 0, str.c_str(), -1, &wstr[0], sizeWide);
	return wstr;
}
#endif

//----------------------------------------------------------------
std::vector<std::string> StringUtility::Split(std::string str, std::string delim)
{
	std::vector<std::string> res;
	Split(str, delim, res);
	return res;
}
void StringUtility::Split(std::string str, std::string delim, std::vector<std::string>& res)
{
	char* wsource = (char*)str.c_str();
	char* pStr = nullptr;
	char* cDelim = const_cast<char*>(delim.c_str());
	while ((pStr = strtok(pStr == nullptr ? wsource : nullptr, cDelim)) != nullptr) {
		//切り出した文字列を追加
		std::string s = std::string(pStr);
		// s = s.substr(0, s.size() - 1);//最後の\0を削除
		res.push_back(s);
	}
}
std::string StringUtility::Format(const char* str, ...)
{
	std::string res;
	char buf[256];
	va_list vl;
	va_start(vl, str);
	if (_vsnprintf(buf, sizeof(buf) / 2, str, vl) < 0) { //バッファを超えていた場合、動的に確保する
		int size = sizeof(buf);
		while (true) {
			size *= 2;
			char* nBuf = new char[size];
			if (_vsnprintf(nBuf, size / 2, str, vl) >= 0) {
				res = nBuf;
				delete[] nBuf;
				break;
			}
			delete[] nBuf;
		}
	} else {
		res = buf;
	}
	va_end(vl);
	return res;
}
size_t StringUtility::CountCharacter(std::vector<char>& str, char c)
{
	if (str.size() == 0)
		return 0;

	size_t count = 0;
	char* pbuf = &str[0];
	char* ebuf = &str[str.size() - 1];
	while (pbuf <= ebuf) {
		if (*pbuf == c)
			count++;
		
		pbuf++;
	}
	return count;
}
size_t StringUtility::CountCharacter(std::string& str, char c)
{
	size_t count = 0;
	char* pbuf = &str[0];
	char* ebuf = &str[str.size() - 1];
	while (pbuf <= ebuf) {
		if (*pbuf == c)
			count++;
	}
	return count;
}
int StringUtility::ToInteger(std::string const& s)
{
	return std::stoi(s);
}
double StringUtility::ToDouble(std::string const& s)
{
	char* stopscan;
	return strtod(s.c_str(), &stopscan);
	// return _wtof(s.c_str());
}
std::string StringUtility::Replace(std::string& source, std::string pattern, std::string placement)
{
	return ReplaceAll(source, pattern, placement, 1);
}
std::string StringUtility::ReplaceAll(std::string& source, std::string pattern, std::string placement, size_t replaceCount, size_t start, size_t end)
{
	std::string result;
	if (end == 0)
		end = source.size();
	size_t pos_before = 0;
	size_t pos = start;
	size_t len = pattern.size();

	size_t count = 0;
	while ((pos = source.find(pattern, pos)) != std::string::npos) {
		result.append(source, pos_before, pos - pos_before);
		result.append(placement);
		pos += len;
		pos_before = pos;

		count++;
		if (count >= replaceCount)
			break;
	}
	result.append(source, pos_before, source.size() - pos_before);
	return result;
}
std::string StringUtility::Slice(std::string const& s, size_t length)
{
	length = _MIN(s.size() - 1, length);
	return s.substr(0, length);
}
std::string StringUtility::Trim(const std::string& str)
{
	if (str.size() == 0)
		return str;

	int left = 0;
	for (; left < str.size(); left++) {
		char wch = str[left];
		if (wch != 0x20 && wch != 0x09)
			break;
	}

	int right = str.size() - 1;
	for (; right >= 0; right--) {
		char wch = str[right];
		if (wch != 0x20 && wch != 0x09 && wch != 0x0 && wch != '\r' && wch != '\n') {
			right++;
			break;
		}
	}

	std::string res = str;
	if (left <= right) {
		res = str.substr(left, right - left);
	}
	return res;
}

//================================================================
//ErrorUtility
#ifdef _WIN32
std::string ErrorUtility::GetLastErrorMessage(DWORD error)
{
	LPVOID lpMsgBuf;
	::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	std::string res = StringUtility::ConvertWideToMulti(reinterpret_cast<const wchar_t*>(lpMsgBuf), CP_UTF8);
	::LocalFree(lpMsgBuf);
	return res;
}
#else
std::string ErrorUtility::GetLastErrorMessage(errno_t error)
{
	return strerror(error);
}
#endif

std::string ErrorUtility::GetLastErrorMessage()
{
#ifdef _WIN32
	return GetLastErrorMessage(GetLastError());
#else
	return GetLastErrorMessage(errno);
#endif
}
std::string ErrorUtility::GetErrorMessage(int type)
{
	std::string res = u8"unknown error";
	if (type == ERROR_FILE_NOTFOUND)
		res = u8"cannot file open";
	else if (type == ERROR_PARSE)
		res = u8"parse failed";
	else if (type == ERROR_END_OF_FILE)
		res = u8"end of file error";
	else if (type == ERROR_OUTOFRANGE_INDEX)
		res = u8"invalid index";
	return res;
}
std::string ErrorUtility::GetFileNotFoundErrorMessage(std::string path)
{
	std::string res = GetErrorMessage(ERROR_FILE_NOTFOUND);
	res += StringUtility::Format(" path[%s]", path.c_str());
	return res;
}
std::string ErrorUtility::GetParseErrorMessage(int line, std::string what)
{
	return GetParseErrorMessage("", line, what);
}
std::string ErrorUtility::GetParseErrorMessage(std::string path, int line, std::string what)
{
	std::string res = GetErrorMessage(ERROR_PARSE);
	res += StringUtility::Format(" path[%s] line[%d] msg[%s]", path.c_str(), line, what.c_str());
	return res;
}

//================================================================
//Math
void Math::InitializeFPU()
{
#ifdef _MSC_VER
	__asm { finit };
#else
	__asm__("finit");
#endif
}

//================================================================
//ByteOrder
void ByteOrder::Reverse(void* buf, size_t size)
{
	unsigned char* pStart = (unsigned char*)buf;
	unsigned char* pEnd = (unsigned char*)buf + size - 1;

	for (; pStart < pEnd;) {
		unsigned char temp = *pStart;
		*pStart = *pEnd;
		*pEnd = temp;

		pStart++;
		pEnd--;
	}
}

//================================================================
//Scanner
Scanner::Scanner(char* str, int size)
{
	std::vector<char> buf;
	buf.resize(size);
	memcpy(&buf[0], str, size);
	buf.push_back('\0');
	_Initialize(buf);
}
Scanner::Scanner(std::string str)
{
	std::vector<char> buf;
	buf.resize(str.size() + 1);
	memcpy(&buf[0], str.c_str(), str.size() + 1);
	_Initialize(buf);
}

Scanner::Scanner(std::vector<char> buf)
{
	_Initialize(buf);
}

void Scanner::_Initialize(std::vector<char> buf)
{
	bPermitSignNumber_ = true;
	buffer_ = buf;
	pointer_ = 0;

	buffer_.push_back(0);
}

Scanner::~Scanner()
{
}
char Scanner::_CurrentChar()
{
	char res = '\0';
	if (pointer_ < buffer_.size()) {
		char ch = buffer_[pointer_];
		res = ch;
	}
	return res;
}

char Scanner::_NextChar()
{
	if (HasNext() == false) {
		Logger::WriteTop(u8"終端異常発生->");

		int size = buffer_.size();
		std::string source = GetString(0, size);
		std::string target = StringUtility::Format(u8"字句解析対象 -> \r\n%s...", source.c_str());
		Logger::WriteTop(target);

		int index = 1;
		std::list<Token>::iterator itr;
		for (itr = listDebugToken_.begin(); itr != listDebugToken_.end(); itr++) {
			Token token = *itr;
			std::string log = StringUtility::Format("  %2d token -> type=%2d, element=%s, start=%d, end=%d",
				index, token.GetType(), token.GetElement().c_str(), token.GetStartPointer(), token.GetEndPointer());
			Logger::WriteTop(log);
			index++;
		}

		_RaiseError(u8"_NextChar:すでに文字列終端です");
	}
	pointer_++;

	return _CurrentChar();
}
void Scanner::_SkipComment()
{
	while (true) {
		int posStart = pointer_;
		_SkipSpace();

		wchar_t ch = _CurrentChar();

		if (ch == '/') { //コメントアウト処理
			int tPos = pointer_;
			ch = _NextChar();
			if (ch == '/') { // "//"
				while (ch != '\r' && ch != '\n' && HasNext())
					ch = _NextChar();
			} else if (ch == '*') { // "/*"-"*/"
				while (true) {
					ch = _NextChar();
					if (ch == '*') {
						ch = _NextChar();
						if (ch == '/')
							break;
					}
				}
				ch = _NextChar();
			} else {
				pointer_ = tPos;
				ch = '/';
			}
		}

		//スキップも空白飛ばしも無い場合、終了
		if (posStart == pointer_)
			break;
	}
}
void Scanner::_SkipSpace()
{
	char ch = _CurrentChar();
	while (true) {
		if (ch != ' ' && ch != '\t')
			break;
		ch = _NextChar();
	}
}
void Scanner::_RaiseError(std::string str)
{
	throw std::runtime_error(str);
}

Token& Scanner::GetToken()
{
	return token_;
}
Token& Scanner::Next()
{
	if (!HasNext()) {
		_RaiseError(u8"Next:すでに終端です");
	}

	_SkipComment(); //コメントをとばします

	char ch = _CurrentChar();

	Token::Type type = Token::Type::Unknown;
	int posStart = pointer_; //先頭を保存

	switch (ch) {
	case '\0':
		type = Token::Type::EndOfFile;
		break; //終端
	case ',':
		_NextChar();
		type = Token::Type::Comma;
		break;
	case '.':
		_NextChar();
		type = Token::Type::Period;
		break;
	case '=':
		_NextChar();
		type = Token::Type::Equal;
		break;
	case '(':
		_NextChar();
		type = Token::Type::OpenP;
		break;
	case ')':
		_NextChar();
		type = Token::Type::CloseP;
		break;
	case '[':
		_NextChar();
		type = Token::Type::OpenB;
		break;
	case ']':
		_NextChar();
		type = Token::Type::CloseB;
		break;
	case '{':
		_NextChar();
		type = Token::Type::OpenC;
		break;
	case '}':
		_NextChar();
		type = Token::Type::CloseC;
		break;
	case '*':
		_NextChar();
		type = Token::Type::Asterisk;
		break;
	case '/':
		_NextChar();
		type = Token::Type::Slash;
		break;
	case ':':
		_NextChar();
		type = Token::Type::Colon;
		break;
	case ';':
		_NextChar();
		type = Token::Type::Semicolon;
		break;
	case '~':
		_NextChar();
		type = Token::Type::Tilde;
		break;
	case '!':
		_NextChar();
		type = Token::Type::Exclamation;
		break;
	case '#':
		_NextChar();
		type = Token::Type::Sharp;
		break;
	case '|':
		_NextChar();
		type = Token::Type::Pipe;
		break;
	case '&':
		_NextChar();
		type = Token::Type::Ampersand;
		break;
	case '<':
		_NextChar();
		type = Token::Type::Less;
		break;
	case '>':
		_NextChar();
		type = Token::Type::Greater;
		break;

	case '"': {
		ch = _NextChar(); //1つ進めて
		//while( ch != '"' )ch = _NextChar();//次のダブルクオーテーションまで進める
		char pre = ch;
		while (true) {
			if (ch == '"' && pre != '\\')
				break;
			pre = ch;
			ch = _NextChar(); //次のダブルクオーテーションまで進める
		}
		if (ch == L'"')
			_NextChar(); //ダブルクオーテーションだったら1つ進める
		else {
			std::string error = GetString(posStart, pointer_);
			std::string log = StringUtility::Format(u8"Next:すでに文字列終端です(String字句解析) -> %s", error.c_str());
			_RaiseError(log);
		}
		type = Token::Type::String;
		break;
	}

	case '\r':
	case '\n': //改行
		//改行がいつまでも続くようなのも1つの改行として扱う
		while (ch == '\r' || ch == '\n')
			ch = _NextChar();
		type = Token::Type::Newline;
		break;

	case '+':
	case '-': {
		if (ch == '+') {
			ch = _NextChar();
			type = Token::Type::Plus;

		} else if (ch == '-') {
			ch = _NextChar();
			type = Token::Type::Minus;
		}

		if (!bPermitSignNumber_ || !isdigit(ch))
			break; //次が数字でないなら抜ける
	}

	default: {
		if (isdigit(ch)) {
			//整数か実数
			while (isdigit(ch))
				ch = _NextChar(); //数字だけの間ポインタを進める
			type = Token::Type::Int;
			if (ch == '.') {
				//実数か整数かを調べる。小数点があったら実数
				ch = _NextChar();
				while (isdigit(ch))
					ch = _NextChar(); //数字だけの間ポインタを進める
				type = Token::Type::Real;
			}

			if (ch == 'E' || ch == 'e') {
				//1E-5みたいなケース
				ch = _NextChar();
				while (isdigit(ch) || ch == '-')
					ch = _NextChar(); //数字だけの間ポインタを進める
				type = Token::Type::Real;
			}

		} else if (isalpha(ch) || ch == '_') {
			//たぶん識別子
			while (isalpha(ch) || isdigit(ch) || ch == '_')
				ch = _NextChar(); //たぶん識別子な間ポインタを進める
			type = Token::Type::ID;
		} else {
			_NextChar();
			type = Token::Type::Unknown;
		}

		break;
	}
	}

	if (type == Token::Type::String) {
		char* pPosStart = &buffer_[posStart];
		char* pPosEnd = &buffer_[pointer_];
		std::string str = std::string(pPosStart, pPosEnd);
		str = StringUtility::ReplaceAll(str, "\\\"", "\"");

		token_ = Token(type, str, posStart, pointer_);
	} else {
		char* pPosStart = &buffer_[posStart];
		char* pPosEnd = &buffer_[pointer_];
		std::string str = std::string(pPosStart, pPosEnd);
		token_ = Token(type, str, posStart, pointer_);
	}

	listDebugToken_.push_back(token_);

	return token_;
}
bool Scanner::HasNext()
{
	// bool res = true;
	// res &= pointer_ < buffer_.size();
	// res &= _CurrentChar() != L'\0';
	// res &= token_.GetType() != Token::TK_EOF;
	return pointer_ < buffer_.size() && _CurrentChar() != '\0' && token_.GetType() != Token::Type::EndOfFile;
}
void Scanner::CheckType(Token& tok, Token::Type type)
{
	if (tok.type_ != type) {
		_RaiseError(StringUtility::Format(u8"CheckType error[%s]:", tok.element_.c_str()));
	}
}
void Scanner::CheckIdentifer(Token& tok, std::string id)
{
	if (tok.type_ != Token::Type::ID || tok.GetIdentifier() != id) {
		_RaiseError(StringUtility::Format(u8"CheckID error[%s]:", tok.element_.c_str()));
	}
}
int Scanner::GetCurrentLine()
{
	if (buffer_.size() == 0)
		return 0;

	int line = 1;
	char* pbuf = &buffer_[0];
	char* ebuf = &buffer_[pointer_];
	while (true) {
		if (pbuf >= ebuf)
			break;
		if (*pbuf == '\n')
			line++;
		pbuf++;
	}
	return line;
}
int Scanner::GetCurrentPointer()
{
	return pointer_;
}
void Scanner::SetCurrentPointer(int pos)
{
	pointer_ = pos;
}

std::string Scanner::GetString(int start, int end)
{
	char* pPosStart = &buffer_[start];
	char* pPosEnd = &buffer_[end];
	std::string str = std::string(pPosStart, pPosEnd);

	return str;
}
bool Scanner::CompareMemory(int start, int end, const char* data)
{
	if (end >= buffer_.size())
		return false;

	int bufSize = end - start;
	bool res = memcmp(&buffer_[start], data, bufSize) == 0;
	return res;
}

//Token
std::string Token::GetIdentifier()
{
	if (type_ != Type::ID) {
		throw std::runtime_error(u8"Token::GetIdentifier:データのタイプが違います");
	}
	return element_;
}
std::string Token::GetString()
{
	if (type_ != Type::String) {
		throw std::runtime_error(u8"Token::GetString:データのタイプが違います");
	}
	return element_.substr(1, element_.size() - 2);
}
int Token::GetInteger()
{
	if (type_ != Type::Int) {
		throw std::runtime_error(u8"Token::GetInterger:データのタイプが違います");
	}
	return StringUtility::ToInteger(element_);
}
double Token::GetReal()
{
	if (type_ != Type::Real && type_ != Type::Int) {
		throw std::runtime_error(u8"Token::GetReal:データのタイプが違います");
	}
	return StringUtility::ToDouble(element_);
}
bool Token::GetBoolean()
{
	bool res = false;
	if (type_ == Type::Real || type_ == Type::Int) {
		res = GetReal() == 1;
	} else {
		res = element_ == "true";
	}
	return res;
}

//================================================================
//TextParser
TextParser::TextParser()
{
	
}
TextParser::TextParser(std::string source)
{
	SetSource(source);
}
TextParser::~TextParser()
{
}
void TextParser::_RaiseError(std::string message)
{
	throw std::runtime_error(message);
}
TextParser::Result TextParser::_ParseComparison(int pos)
{
	Result res = _ParseSum(pos);
	while (scan_->HasNext()) {
		scan_->SetCurrentPointer(res.pos_);

		Token& tok = scan_->Next();
		Token::Type type = tok.GetType();
		if (type == Token::Type::Exclamation || type == Token::Type::Equal) {
			//「==」「!=」
			bool bNot = type == Token::Type::Exclamation;
			tok = scan_->Next();
			type = tok.GetType();
			if (type != Token::Type::Equal)
				break;

			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if (res.type_ == Type::Boolean && tRes.type_ == Type::Boolean) {
				res.valueBoolean_ = bNot ? res.valueBoolean_ != tRes.valueBoolean_ : res.valueBoolean_ == tRes.valueBoolean_;
			} else if (res.type_ == Type::Real && tRes.type_ == Type::Real) {
				res.valueBoolean_ = bNot ? res.valueReal_ != tRes.valueReal_ : res.valueReal_ == tRes.valueReal_;
			} else {
				_RaiseError(u8"比較できない型");
			}
			res.type_ = Type::Boolean;
		} else if (type == Token::Type::Pipe) {
			tok = scan_->Next();
			type = tok.GetType();
			if (type != Token::Type::Pipe)
				break;
			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if (res.type_ == Type::Boolean && tRes.type_ == Type::Boolean) {
				res.valueBoolean_ = res.valueBoolean_ || tRes.valueBoolean_;
			} else {
				_RaiseError(u8"真偽値以外での||");
			}
		} else if (type == Token::Type::Ampersand) {
			tok = scan_->Next();
			type = tok.GetType();
			if (type != Token::Type::Ampersand)
				break;
			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if (res.type_ == Type::Boolean && tRes.type_ == Type::Boolean) {
				res.valueBoolean_ = res.valueBoolean_ && tRes.valueBoolean_;
			} else {
				_RaiseError(u8"真偽値以外での&&");
			}
		} else
			break;
	}
	return res;
}

TextParser::Result TextParser::_ParseSum(int pos)
{
	Result res = _ParseProduct(pos);
	while (scan_->HasNext()) {
		scan_->SetCurrentPointer(res.pos_);

		Token& tok = scan_->Next();
		Token::Type type = tok.GetType();
		if (type != Token::Type::Plus && type != Token::Type::Minus)
			break;

		Result tRes = _ParseProduct(scan_->GetCurrentPointer());
		if (res.IsString() || tRes.IsString()) {
			res.type_ = Type::String;
			res.valueString_ = res.GetString() + tRes.GetString();
		} else {
			if (tRes.type_ == Type::Boolean)
				_RaiseError(u8"真偽値の加算減算");
			res.pos_ = tRes.pos_;
			if (type == Token::Type::Plus) {
				res.valueReal_ += tRes.valueReal_;
			} else if (type == Token::Type::Minus) {
				res.valueReal_ -= tRes.valueReal_;
			}
		}
	}
	return res;
}
TextParser::Result TextParser::_ParseProduct(int pos)
{
	Result res = _ParseTerm(pos);
	while (scan_->HasNext()) {
		scan_->SetCurrentPointer(res.pos_);
		Token& tok = scan_->Next();
		Token::Type type = tok.GetType();
		if (type != Token::Type::Asterisk && type != Token::Type::Slash)
			break;

		Result tRes = _ParseTerm(scan_->GetCurrentPointer());
		if (tRes.type_ == Type::Boolean)
			_RaiseError(u8"真偽値の乗算除算");

		res.type_ = tRes.type_;
		res.pos_ = tRes.pos_;
		if (type == Token::Type::Asterisk) {
			res.valueReal_ *= tRes.valueReal_;
		} else if (type == Token::Type::Slash) {
			res.valueReal_ /= tRes.valueReal_;
		}
	}
	return res;
}

TextParser::Result TextParser::_ParseTerm(int pos)
{
	scan_->SetCurrentPointer(pos);
	Result res;
	Token& tok = scan_->Next();

	bool bMinus = false;
	bool bNot = false;
	Token::Type type = tok.GetType();
	if (type == Token::Type::Plus || type == Token::Type::Minus || type == Token::Type::Exclamation) {
		if (type == Token::Type::Minus)
			bMinus = true;
		if (type == Token::Type::Exclamation)
			bNot = true;
		tok = scan_->Next();
	}

	if (tok.GetType() == Token::Type::OpenP) {
		res = _ParseComparison(scan_->GetCurrentPointer());
		scan_->SetCurrentPointer(res.pos_);
		tok = scan_->Next();
		if (tok.GetType() != Token::Type::CloseP)
			_RaiseError(u8")がありません");
	} else {
		Token::Type type = tok.GetType();
		if (type == Token::Type::Int || type == Token::Type::Real) {
			res.valueReal_ = tok.GetReal();
			res.type_ = Type::Real;
		} else if (type == Token::Type::ID) {
			Result tRes = _ParseIdentifer(scan_->GetCurrentPointer());
			res = tRes;
		} else if (type == Token::Type::String) {
			res.valueString_ = tok.GetString();
			res.type_ = Type::String;
		} else
			_RaiseError(StringUtility::Format(u8"不正なトークン:%s", tok.GetElement().c_str()));
	}

	if (bMinus) {
		if (res.type_ != Type::Real)
			_RaiseError(u8"実数以外での負記号");
		res.valueReal_ = -res.valueReal_;
	}
	if (bNot) {
		if (res.type_ != Type::Boolean)
			_RaiseError(u8"真偽値以外での否定");
		res.valueBoolean_ = !res.valueBoolean_;
	}
	res.pos_ = scan_->GetCurrentPointer();
	return res;
}
TextParser::Result TextParser::_ParseIdentifer(int pos)
{
	Result res;
	res.pos_ = scan_->GetCurrentPointer();

	Token& tok = scan_->GetToken();
	std::string id = tok.GetElement();
	if (id == "true") {
		res.type_ = Type::Boolean;
		res.valueBoolean_ = true;
	} else if (id == "false") {
		res.type_ = Type::Boolean;
		res.valueBoolean_ = false;
	} else {
		_RaiseError(StringUtility::Format(u8"不正な識別子:%s", id.c_str()));
	}
	return res;
}

void TextParser::SetSource(std::string source)
{
	std::vector<char> buf;
	buf.resize(source.size() + 1);
	memcpy(&buf[0], source.c_str(), source.size() + 1);
	scan_ = std::make_unique<Scanner>(buf);
	scan_->SetPermitSignNumber(false);
}
TextParser::Result TextParser::GetResult()
{
	if (scan_ == NULL)
		_RaiseError(u8"テキストが設定されていません");
	Result res = _ParseComparison(scan_->GetCurrentPointer());
	if (scan_->HasNext())
		_RaiseError(StringUtility::Format(u8"不正なトークン:%s", scan_->GetToken().GetElement().c_str()));
	return res;
}
double TextParser::GetReal()
{
	if (scan_ == NULL)
		_RaiseError(u8"テキストが設定されていません");
	Result res = _ParseSum(scan_->GetCurrentPointer());
	if (scan_->HasNext())
		_RaiseError(StringUtility::Format(u8"不正なトークン:%s", scan_->GetToken().GetElement().c_str()));
	return res.GetReal();
}

#ifdef _WIN32
//================================================================
//Font
// const wchar_t* Font::GOTHIC  = L"標準ゴシック";
// const wchar_t* Font::MINCHOH = L"標準明朝";
const wchar_t* Font::GOTHIC = L"MS Gothic";
const wchar_t* Font::MINCHOH = L"MS Mincho";

Font::Font()
{
	hFont_ = NULL;
	ZeroMemory(&info_, sizeof(LOGFONT));
}
Font::~Font()
{
	this->Clear();
}
void Font::Clear()
{
	if (hFont_ != NULL) {
		::DeleteObject(hFont_);
		hFont_ = NULL;
		ZeroMemory(&info_, sizeof(LOGFONT));
	}
}
void Font::CreateFont(const wchar_t* type, int size, bool bBold, bool bItalic, bool bLine)
{
	LOGFONTW fontInfo;

	lstrcpy(fontInfo.lfFaceName, type);
	fontInfo.lfWeight = (bBold == false) * FW_NORMAL + (bBold == TRUE) * FW_BOLD;
	fontInfo.lfEscapement = 0;
	fontInfo.lfWidth = 0;
	fontInfo.lfHeight = size;
	fontInfo.lfItalic = bItalic;
	fontInfo.lfUnderline = bLine;
	fontInfo.lfCharSet = ANSI_CHARSET;
	for (int i = 0; i < (int)wcslen(type); i++) {
		if (!(IsCharAlphaNumericW(type[i]) || type[i] == L' ' || type[i] == L'-')) {
			fontInfo.lfCharSet = SHIFTJIS_CHARSET;
			break;
		}
	}

	this->CreateFontIndirect(fontInfo);
}
void Font::CreateFontIndirect(LOGFONT& fontInfo)
{
	if (hFont_ != NULL)
		this->Clear();
	hFont_ = ::CreateFontIndirectW(&fontInfo);
	info_ = fontInfo;
}
#endif
