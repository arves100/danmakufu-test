#include "DxText.hpp"
#include "DirectGraphics.hpp"
#include "DxUtility.hpp"
#include "RenderObject.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//DxFont
**********************************************************/
DxFont::DxFont()
{
	ZeroMemory(&info_, sizeof(LOGFONT));
	colorTop_ = D3DCOLOR_ARGB(255, 128, 128, 255);
	colorBottom_ = D3DCOLOR_ARGB(255, 32, 32, 255);
	typeBorder_ = BORDER_NONE;
	widthBorder_ = 2;
	colorBorder_ = D3DCOLOR_ARGB(128, 255, 255, 255);
}
DxFont::~DxFont()
{
}

/**********************************************************
//DxChar
**********************************************************/
DxChar::DxChar()
{
}
DxChar::~DxChar()
{
}

bool DxChar::Create(int code, Font& winFont, DxFont& dxFont)
{
	code_ = code;
	font_ = dxFont;

	int typeBorder = font_.GetBorderType();
	D3DCOLOR colorTop = font_.GetTopColor();
	D3DCOLOR colorBottom = font_.GetBottomColor();
	D3DCOLOR colorBorder = font_.GetBorderColor();
	int widthBorder = typeBorder != DxFont::BORDER_NONE ? font_.GetBorderWidth() : 0;

	HDC hDC = ::GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC, winFont.GetHandle());

	// フォントビットマップ取得
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);
	GLYPHMETRICS gm;
	CONST MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	int uFormat = GGO_GRAY2_BITMAP; //typeBorder == DxFont::BORDER_FULL ? GGO_BITMAP : GGO_GRAY2_BITMAP;
	if (dxFont.GetLogFont().lfHeight <= 12)
		uFormat = GGO_BITMAP;
	DWORD size = ::GetGlyphOutline(hDC, code, uFormat, &gm, 0, NULL, &mat);
	std::vector<BYTE> ptr;
	ptr.reserve(size);
	ptr.resize(size);
	::GetGlyphOutline(hDC, code, uFormat, &gm, size, ptr.data(), &mat);

	// デバイスコンテキストとフォントハンドルの解放
	::SelectObject(hDC, oldFont);
	::ReleaseDC(NULL, hDC);

	//テクスチャ作成
	int tex_x = gm.gmCellIncX + widthBorder * 2;
	int tex_y = tm.tmHeight + widthBorder * 2;
	int widthTexture = 0;
	int heightTexture = 0;
	int num_x = 1;
	while (TRUE) {
		tex_x /= 2;
		if (tex_x == 0) {
			widthTexture = pow(2., num_x);
			break;
		}
		num_x++;
	}
	int num_y = 1;
	while (TRUE) {
		tex_y /= 2;
		if (tex_y == 0) {
			heightTexture = pow(2., num_y);
			break;
		}
		num_y++;
	}
	IDirect3DTexture9* pTexture = NULL;
	HRESULT hr = DirectGraphics::GetBase()->GetDevice()->CreateTexture(
		widthTexture, heightTexture,
		1, D3DPOOL_DEFAULT, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL);
	if (FAILED(hr))
		return false;

	D3DLOCKED_RECT lock;
	if (FAILED(pTexture->LockRect(0, &lock, NULL, D3DLOCK_DISCARD))) {
		return false;
	}
	int iOfs_x = gm.gmptGlyphOrigin.x;
	int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
	int iBmp_w = uFormat != GGO_BITMAP ? gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4 : gm.gmBlackBoxX;
	int iBmp_h = gm.gmBlackBoxY;
	int level = 5; //GGO_GRAY4_BITMAP;
	FillMemory(lock.pBits, lock.Pitch * tm.tmHeight, 0);

	double gapColor = 1.0 / (double)iBmp_h;
	double gapColorR = (double)(ColorAccess::GetColorR(colorTop) - ColorAccess::GetColorR(colorBottom)) * gapColor;
	double gapColorG = (double)(ColorAccess::GetColorG(colorTop) - ColorAccess::GetColorG(colorBottom)) * gapColor;
	double gapColorB = (double)(ColorAccess::GetColorB(colorTop) - ColorAccess::GetColorB(colorBottom)) * gapColor;

	int xMax = iBmp_w + iOfs_x + widthBorder * 2;
	int yMax = iBmp_h + iOfs_y + widthBorder * 2;
	for (int iy = 0; iy < yMax && size > 0; iy++) {
		int yBmp = iy - iOfs_y - widthBorder;
		int colorR = ColorAccess::GetColorR(colorTop) - gapColorR * yBmp;
		int colorG = ColorAccess::GetColorG(colorTop) - gapColorG * yBmp;
		int colorB = ColorAccess::GetColorB(colorTop) - gapColorB * yBmp;

		for (int ix = 0; ix < xMax; ix++) {
			int xBmp = ix - iOfs_x - widthBorder;

			DWORD alpha = 255;
			if (uFormat != GGO_BITMAP) {
				int posBmp = xBmp + iBmp_w * yBmp;
				alpha = xBmp >= 0 && xBmp < iBmp_w && yBmp >= 0 && yBmp < iBmp_h ? (255 * ptr[posBmp]) / (level - 1) : 0;
			} else {
				if (xBmp >= 0 && xBmp < iBmp_w && yBmp >= 0 && yBmp < iBmp_h) {
					int lineByte = (1 + (iBmp_w / 32)) * 4; // 1行に使用しているBYTE数（4バイト境界あり）
					int posBmp = xBmp / 8 + lineByte * yBmp;
					alpha = BitAccess::GetBit(ptr[posBmp], 7 - xBmp % 8) ? 255 : 0;
				} else
					alpha = 0;
			}

			DWORD color = 0;
			if (typeBorder != DxFont::BORDER_NONE && alpha != 255) {
				if (alpha == 0) {
					int count = 0;
					int antiDist = 0;
					int bx = typeBorder == DxFont::BORDER_FULL ? xBmp + widthBorder + antiDist : xBmp + 1;
					int by = typeBorder == DxFont::BORDER_FULL ? yBmp + widthBorder + antiDist : yBmp + 1;
					int minAlphaEnableDist = 255 * 255;
					for (int ax = xBmp - widthBorder - antiDist; ax <= bx; ax++) {
						for (int ay = yBmp - widthBorder - antiDist; ay <= by; ay++) {
							int dist = abs(ax - xBmp) + abs(ay - yBmp);
							if (dist > widthBorder + antiDist || dist == 0)
								continue;

							DWORD tAlpha = 255;
							if (uFormat != GGO_BITMAP) {
								tAlpha = ax >= 0 && ax < iBmp_w && ay >= 0 && ay < iBmp_h ? (255 * ptr[ax + iBmp_w * ay]) / (level - 1) : 0;
							} else {
								if (ax >= 0 && ax < iBmp_w && ay >= 0 && ay < iBmp_h) {
									int lineByte = (1 + (iBmp_w / 32)) * 4; // 1行に使用しているBYTE数（4バイト境界あり）
									int tPos = ax / 8 + lineByte * ay;
									tAlpha = BitAccess::GetBit(ptr[tPos], 7 - ax % 8) ? 255 : 0;
								} else
									tAlpha = 0;
							}

							if (tAlpha > 0)
								minAlphaEnableDist = min(minAlphaEnableDist, dist);

							int tCount = tAlpha /= dist;
							tCount *= 2;
							if (typeBorder == DxFont::BORDER_SHADOW && (ax >= xBmp || ay >= yBmp))
								tCount /= 2;
							count += tCount;
						}
					}
					color = colorBorder;

					int destAlpha = 0;
					if (minAlphaEnableDist < widthBorder)
						destAlpha = 255;
					else if (minAlphaEnableDist == widthBorder)
						destAlpha = count;
					else
						destAlpha = 0;
					//color = ColorAccess::SetColorA(color, ColorAccess::GetColorA(colorBorder)*count/255);
					color = ColorAccess::SetColorA(color, ColorAccess::GetColorA(colorBorder) * destAlpha / 255);
				} else {
					int oAlpha = alpha + 64;
					if (alpha > 255)
						alpha = 255;

					int count = 0;
					int cDist = 1;
					int bx = typeBorder == DxFont::BORDER_FULL ? xBmp + cDist : xBmp + 1;
					int by = typeBorder == DxFont::BORDER_FULL ? yBmp + cDist : yBmp + 1;
					for (int ax = xBmp - cDist; ax <= bx; ax++) {
						for (int ay = yBmp - cDist; ay <= by; ay++) {
							DWORD tAlpha = 255;
							if (uFormat != GGO_BITMAP) {
								tAlpha = tAlpha = ax >= 0 && ax < iBmp_w && ay >= 0 && ay < iBmp_h ? (255 * ptr[ax + iBmp_w * ay]) / (level - 1) : 0;
							} else {
								if (ax >= 0 && ax < iBmp_w && ay >= 0 && ay < iBmp_h) {
									int lineByte = (1 + (iBmp_w / 32)) * 4; // 1行に使用しているBYTE数（4バイト境界あり）
									int tPos = ax / 8 + lineByte * ay;
									tAlpha = BitAccess::GetBit(ptr[tPos], 7 - ax % 8) ? 255 : 0;
								} else
									tAlpha = 0;
							}

							if (tAlpha > 0)
								count++;
						}
					}
					if (count >= 2)
						oAlpha = alpha;

					int bAlpha = 255 - oAlpha;
					color = ColorAccess::SetColorA(color, 255);
					color = ColorAccess::SetColorR(color, colorR * oAlpha / 255 + ColorAccess::GetColorR(colorBorder) * bAlpha / 255);
					color = ColorAccess::SetColorG(color, colorG * oAlpha / 255 + ColorAccess::GetColorG(colorBorder) * bAlpha / 255);
					color = ColorAccess::SetColorB(color, colorB * oAlpha / 255 + ColorAccess::GetColorB(colorBorder) * bAlpha / 255);
				}
			} else {
				if (typeBorder != DxFont::BORDER_NONE && alpha > 0)
					alpha = 255;
				color = 0x00ffffff | (alpha << 24);
				color = ColorAccess::SetColorR(color, colorR);
				color = ColorAccess::SetColorG(color, colorG);
				color = ColorAccess::SetColorB(color, colorB);
			}
			memcpy((BYTE*)lock.pBits + lock.Pitch * iy + 4 * ix, &color, sizeof(DWORD));
		}
	}
	pTexture->UnlockRect(0);

	texture_ = std::make_shared<Texture>();
	texture_->SetTexture(pTexture);

	width_ = gm.gmCellIncX + widthBorder * 2;
	height_ = tm.tmHeight + widthBorder * 2;

	return true;
}

/**********************************************************
//DxCharCache
**********************************************************/
DxCharCache::DxCharCache()
{
	sizeMax_ = 512;
	countPri_ = 0;
}
DxCharCache::~DxCharCache()
{
	Clear();
}
void DxCharCache::_arrange()
{
	countPri_ = 0;
	std::map<int, DxCharCacheKey> mapPriKeyLast = mapPriKey_;
	mapPriKey_.clear();
	mapKeyPri_.clear();
	std::map<int, DxCharCacheKey>::iterator itr;
	for (itr = mapPriKeyLast.begin(); itr != mapPriKeyLast.end(); itr++) {
		DxCharCacheKey key = itr->second;
		int pri = countPri_;

		mapPriKey_[pri] = key;
		mapKeyPri_[key] = pri;

		countPri_++;
	}
}
void DxCharCache::Clear()
{
	mapPriKey_.clear();
	mapKeyPri_.clear();
	mapCache_.clear();
}
std::shared_ptr<DxChar> DxCharCache::GetChar(DxCharCacheKey& key)
{
	std::shared_ptr<DxChar> res;
	bool bExist = mapCache_.find(key) != mapCache_.end();
	if (bExist) {
		res = mapCache_[key];
		/*
		//キーの優先順位をトップにする
		int tPri = mapKeyPri_[key];
		mapPriKey_.erase(tPri);

		countPri_++;
		int nPri = countPri_;
		mapKeyPri_[key] = nPri;
		mapPriKey_[nPri] = key;

		if(countPri_ >= INT_MAX)
		{
			//再配置
			_arrange();
		}
		*/
	}
	return res;
}

void DxCharCache::AddChar(DxCharCacheKey& key, std::shared_ptr<DxChar> value)
{
	bool bExist = mapCache_.find(key) != mapCache_.end();
	if (bExist)
		return;
	mapCache_[key] = value;

	if (mapCache_.size() >= sizeMax_) {
		mapCache_.clear();
		/*
		//優先度の低いキャッシュを削除
		std::map<int, DxCharCacheKey>::iterator itrMinPri = mapPriKey_.begin();
		int minPri = itrMinPri->first;
		DxCharCacheKey keyMinPri = itrMinPri->second;

		mapCache_.erase(keyMinPri);
		mapKeyPri_.erase(keyMinPri);
		mapPriKey_.erase(minPri);
		*/
	}
}

/**********************************************************
//DxTextScanner
**********************************************************/
const int DxTextScanner::TOKEN_TAG_START = DxTextToken::TK_OPENB;
const int DxTextScanner::TOKEN_TAG_END = DxTextToken::TK_CLOSEB;
const std::string DxTextScanner::TAG_START = "[";
const std::string DxTextScanner::TAG_END = "]";
const std::string DxTextScanner::TAG_NEW_LINE = "r";
const std::string DxTextScanner::TAG_RUBY = "ruby";
const std::string DxTextScanner::TAG_FONT = "font";
const char CHAR_TAG_START = '[';
const char CHAR_TAG_END = ']';
DxTextScanner::DxTextScanner(char* str, int charCount)
{
	std::vector<char> buf;
	buf.resize(charCount);
	if (buf.size() != 0) {
		memcpy(&buf[0], str, charCount * sizeof(char));
	}

	buf.push_back(L'\0');
	this->DxTextScanner::DxTextScanner(buf);
}
DxTextScanner::DxTextScanner(std::string str)
{
	std::vector<char> buf;
	buf.resize(str.size() + 1);
	memcpy(&buf[0], str.c_str(), (str.size() + 1) * sizeof(char));
	this->DxTextScanner::DxTextScanner(buf);
}
DxTextScanner::DxTextScanner(std::vector<char>& buf)
{
	buffer_ = buf;
	pointer_ = buffer_.begin();

	line_ = 1;
	bTagScan_ = false;
}
DxTextScanner::~DxTextScanner()
{
}

wchar_t DxTextScanner::_NextChar()
{
	if (HasNext() == false) {
		std::string source;
		source.resize(buffer_.size());
		memcpy(&source[0], &buffer_[0], source.size() * sizeof(char));
		std::string log = StringUtility::Format(u8"_NextChar(Text):すでに文字列終端です -> %s", source.c_str());
		_RaiseError(log);
	}

	pointer_++;

	if (*pointer_ == '\n')
		line_++;
	return *pointer_;
}
void DxTextScanner::_SkipComment()
{
	while (true) {
		std::vector<char>::iterator posStart = pointer_;
		_SkipSpace();

		char ch = *pointer_;

		if (ch == '/') { //コメントアウト処理
			std::vector<char>::iterator tPos = pointer_;
			ch = _NextChar();
			if (ch == L'/') { // "//"
				while (true) {
					ch = _NextChar();
					if (ch == '\r' || ch == '\n')
						break;
				}
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
void DxTextScanner::_SkipSpace()
{
	char ch = *pointer_;
	while (true) {
		if (ch != ' ' && ch != '\t')
			break;
		ch = _NextChar();
	}
}
void DxTextScanner::_RaiseError(std::string str)
{
	throw std::exception(str.c_str());
}
bool DxTextScanner::_IsTextStartSign()
{
	if (bTagScan_)
		return false;

	bool res = false;
	char ch = *pointer_;

#if 0
	if (ch == '\\') {
		std::vector<char>::iterator pos = pointer_;
		ch = _NextChar(); //次のタグまで進める
		bool bLess = ch == CHAR_TAG_START;
		if (!bLess) {
			res = false;
			SetCurrentPointer(pos);
		} else {
			res = !bLess;
		}
	} else 
#endif
	{
		bool bLess = ch == CHAR_TAG_START;
		res = !bLess;
	}

	return res;
}
bool DxTextScanner::_IsTextScan()
{
	bool res = false;
	char ch = _NextChar();
	if (!HasNext()) {
		return false;
	} else if (ch == '/') {
		ch = *(pointer_ + 1);
		if (ch == '/' || ch == '*')
			res = false;
	} else if (false && ch == '\\') {
		ch = _NextChar(); //次のタグまで進める
		res = true;
	} else {
		bool bGreater = ch == CHAR_TAG_END;
		if (bGreater) {
			_RaiseError(u8"テキスト中にタグ終了文字が存在しました");
		}
		bool bNotLess = ch != CHAR_TAG_START;
		res = bNotLess;
	}
	return res;
}
DxTextToken& DxTextScanner::GetToken()
{
	return token_;
}
DxTextToken& DxTextScanner::Next()
{
	if (!HasNext()) {
		std::string source;
		source.resize(buffer_.size());
		memcpy(&source[0], &buffer_[0], source.size() * sizeof(char));
		std::string log = StringUtility::Format(u8"Next(Text):すでに終端です -> %s", source.c_str());
		_RaiseError(log);
	}

	_SkipComment(); //コメントをとばします

	char ch = *pointer_;
	if (ch == '\0') {
		token_ = DxTextToken(DxTextToken::TK_EOF, "\0");
		return token_;
	}

	DxTextToken::Type type = DxTextToken::TK_UNKNOWN;
	std::vector<char>::iterator posStart = pointer_; //先頭を保存

	if (_IsTextStartSign()) {
		ch = *pointer_;

		posStart = pointer_;
		while (_IsTextScan()) {
		}

		ch = *pointer_;
		if (ch == CHAR_TAG_START) {
		} else if (!HasNext()) {
		}
		// else _RaiseError("Next:すでに文字列終端です");

		type = DxTextToken::TK_TEXT;
		std::string text = std::string(posStart, pointer_);
		text = StringUtility::ReplaceAll(text, "\\", "");
		token_ = DxTextToken(type, text);
	} else {
		switch (ch) {
		case '\0':
			type = DxTextToken::TK_EOF;
			break; //終端
		case ',':
			_NextChar();
			type = DxTextToken::TK_COMMA;
			break;
		case '=':
			_NextChar();
			type = DxTextToken::TK_EQUAL;
			break;
		case '(':
			_NextChar();
			type = DxTextToken::TK_OPENP;
			break;
		case ')':
			_NextChar();
			type = DxTextToken::TK_CLOSEP;
			break;
		case '[':
			_NextChar();
			type = DxTextToken::TK_OPENB;
			break;
		case ']':
			_NextChar();
			type = DxTextToken::TK_CLOSEB;
			break;
		case '{':
			_NextChar();
			type = DxTextToken::TK_OPENC;
			break;
		case '}':
			_NextChar();
			type = DxTextToken::TK_CLOSEC;
			break;
		case '*':
			_NextChar();
			type = DxTextToken::TK_ASTERISK;
			break;
		case '/':
			_NextChar();
			type = DxTextToken::TK_SLASH;
			break;
		case ':':
			_NextChar();
			type = DxTextToken::TK_COLON;
			break;
		case ';':
			_NextChar();
			type = DxTextToken::TK_SEMICOLON;
			break;
		case '~':
			_NextChar();
			type = DxTextToken::TK_TILDE;
			break;
		case '!':
			_NextChar();
			type = DxTextToken::TK_EXCLAMATION;
			break;
		case '#':
			_NextChar();
			type = DxTextToken::TK_SHARP;
			break;
		case '<':
			_NextChar();
			type = DxTextToken::TK_LESS;
			break;
		case '>':
			_NextChar();
			type = DxTextToken::TK_GREATER;
			break;

		case '"': {
			ch = _NextChar(); //1つ進めて
			char pre = ch;
			while (true) {
				if (ch == '"' && pre != '\\')
					break;
				pre = ch;
				ch = _NextChar(); //次のダブルクオーテーションまで進める
			}

			if (ch == '"')
				_NextChar(); //ダブルクオーテーションだったら1つ進める
			else
				_RaiseError(u8"Next(Text):すでに文字列終端です");
			type = DxTextToken::TK_STRING;
			break;
		}

		case '\r':
		case '\n': //改行
			//改行がいつまでも続くようなのも1つの改行として扱う
			while (ch == '\r' || ch == '\n')
				ch = _NextChar();
			type = DxTextToken::TK_NEWLINE;
			break;

		case '+':
		case '-': {
			if (ch == '+') {
				ch = _NextChar();
				type = DxTextToken::TK_PLUS;

			} else if (ch == '-') {
				ch = _NextChar();
				type = DxTextToken::TK_MINUS;
			}

			if (!isdigit(ch))
				break; //次が数字でないなら抜ける
		}

		default: {
			if (isdigit(ch)) {
				//整数か実数
				while (isdigit(ch))
					ch = _NextChar(); //数字だけの間ポインタを進める
				type = DxTextToken::TK_INT;
				if (ch == '.') {
					//実数か整数かを調べる。小数点があったら実数
					ch = _NextChar();
					while (isdigit(ch))
						ch = _NextChar(); //数字だけの間ポインタを進める
					type = DxTextToken::TK_REAL;
				}

				if (ch == 'E' || ch == 'e') {
					//1E-5みたいなケース
					std::vector<CHAR>::iterator pos = pointer_;
					ch = _NextChar();
					while (isdigit(ch) || ch == '-')
						ch = _NextChar(); //数字だけの間ポインタを進める
					type = DxTextToken::TK_REAL;
				}

			} else if (isalpha(ch) || ch == '_') {
				//たぶん識別子
				while (isalpha(ch) || isdigit(ch) || ch == '_')
					ch = _NextChar(); //たぶん識別子な間ポインタを進める
				type = DxTextToken::TK_ID;
			} else {
				_NextChar();
				type = DxTextToken::TK_UNKNOWN;
			}
			break;
		}
		}
		if (type == DxTextScanner::TOKEN_TAG_START)
			bTagScan_ = true;
		else if (type == DxTextScanner::TOKEN_TAG_END)
			bTagScan_ = false;

		if (type == DxTextToken::TK_STRING) {
			//\を除去
			auto str2 = std::string(posStart, pointer_);
			std::string str = StringUtility::ReplaceAll(str2, "\\\"", "\"");
			token_ = DxTextToken(type, str);
		} else {
			token_ = DxTextToken(type, std::string(posStart, pointer_));
		}
	}

	return token_;
}
bool DxTextScanner::HasNext()
{
	return pointer_ != buffer_.end() && *pointer_ != '\0' && token_.GetType() != DxTextToken::TK_EOF;
}
void DxTextScanner::CheckType(DxTextToken& tok, int type)
{
	if (tok.type_ != type) {
		std::string str = StringUtility::Format("CheckType error[%s]:", tok.element_.c_str());
		_RaiseError(str);
	}
}
void DxTextScanner::CheckIdentifer(DxTextToken& tok, std::string id)
{
	if (tok.type_ != DxTextToken::TK_ID || tok.GetIdentifier() != id) {
		std::string str = StringUtility::Format("CheckID error[%s]:", tok.element_.c_str());
		_RaiseError(str);
	}
}
int DxTextScanner::GetCurrentLine()
{
	return line_;
}
int DxTextScanner::SearchCurrentLine()
{
	int line = 1;
	char* pbuf = &(*buffer_.begin());
	char* ebuf = &(*pointer_);
	while (true) {
		if (pbuf >= ebuf)
			break;
		else if (*pbuf == '\n')
			line++;

		pbuf++;
	}
	return line;
}
std::vector<char>::iterator DxTextScanner::GetCurrentPointer()
{
	return pointer_;
}
void DxTextScanner::SetCurrentPointer(std::vector<char>::iterator pos)
{
	pointer_ = pos;
}
int DxTextScanner::GetCurrentPosition()
{
	if (buffer_.size() == 0)
		return 0;
	char* pos = (char*)&*pointer_;
	return pos - &buffer_[0];
}

//DxTextToken
std::string& DxTextToken::GetIdentifier()
{
	if (type_ != TK_ID) {
		throw std::exception(u8"DxTextToken::GetIdentifier:データのタイプが違います");
	}
	return element_;
}
std::string DxTextToken::GetString()
{
	if (type_ != TK_STRING) {
		throw std::exception(u8"DxTextToken::GetString:データのタイプが違います");
	}
	return element_.substr(1, element_.size() - 2);
}
int DxTextToken::GetInteger()
{
	if (type_ != TK_INT) {
		throw std::exception(u8"DxTextToken::GetInterger:データのタイプが違います");
	}
	int res = StringUtility::ToInteger(element_);
	return res;
}
double DxTextToken::GetReal()
{
	if (type_ != TK_REAL && type_ != TK_INT) {
		throw std::exception(u8"DxTextToken::GetReal:データのタイプが違います");
	}

	double res = StringUtility::ToDouble(element_);
	return res;
}
bool DxTextToken::GetBoolean()
{
	bool res = false;
	if (type_ == TK_REAL && type_ == TK_INT) {
		res = GetReal() == 1;
	} else {
		res = element_ == "true";
	}
	return res;
}

/**********************************************************
//DxTextRenderer
//テキスト描画エンジン
**********************************************************/
//Tag

//DxTextTag_Ruby

//DxTextRenderObject
DxTextRenderObject::DxTextRenderObject()
{
	position_.x = 0;
	position_.y = 0;

	angle_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	scale_ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	center_ = D3DXVECTOR2(0.0f, 0.0f);
	bAutoCenter_ = true;
	bPermitCamera_ = true;
}
void DxTextRenderObject::Render()
{
	POINT pos = position_;

	bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
	bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;
	bool bMatrix = bAngle || bScale;
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	if (bScale) {
		D3DXMATRIX matScale;
		D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
		mat = mat * matScale;
	}
	if (bAngle) {
		D3DXMATRIX matRot;
		D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angle_.z));
		mat = mat * matRot;
	}

	D3DXVECTOR2 center = center_;
	if (bMatrix && bAutoCenter_) {
		RECT rect;
		ZeroMemory(&rect, sizeof(RECT));
		std::list<ObjectData>::iterator itr = listData_.begin();
		for (; itr != listData_.end(); itr++) {
			ObjectData obj = *itr;
			std::shared_ptr<Sprite2D> sprite = obj.sprite;
			RECT_D rcDest = sprite->GetDestinationRect();
			rect.left = min(rect.left, rcDest.left);
			rect.top = min(rect.top, rcDest.top);
			rect.right = max(rect.right, rcDest.right);
			rect.bottom = max(rect.bottom, rcDest.bottom);
		}
		center.x = (rect.right + rect.left) / 2;
		center.y = (rect.bottom + rect.top) / 2;
	}

	std::list<ObjectData>::iterator itr = listData_.begin();
	for (; itr != listData_.end(); itr++) {
		ObjectData obj = *itr;
		POINT bias = obj.bias;
		std::shared_ptr<Sprite2D> sprite = obj.sprite;
		sprite->SetColorRGB(color_);
		sprite->SetAlpha(ColorAccess::GetColorA(color_));
		RECT_D rcDestCopy = sprite->GetDestinationRect();

		//座標変換
		if (bMatrix) {
			int countVertex = sprite->GetVertexCount();
			for (int iVert = 0; iVert < countVertex; iVert++) {
				VERTEX_TLX* vert = sprite->GetVertex(iVert);
				vert->position.x -= center.x;
				vert->position.y -= center.y;
				D3DXVec3TransformCoord((D3DXVECTOR3*)&vert->position, (D3DXVECTOR3*)&vert->position, &mat);
				vert->position.x += center.x + pos.x + bias.x;
				vert->position.y += center.y + pos.y + bias.y;
			}
		} else {
			RECT_D tRect = sprite->GetDestinationRect();
			tRect.left += pos.x + bias.x;
			tRect.right += pos.x + bias.x;
			tRect.top += pos.y + bias.y;
			tRect.bottom += pos.y + bias.y;
			sprite->SetDestinationRect(tRect);
		}

		sprite->SetPermitCamera(bPermitCamera_);
		sprite->SetShader(shader_);
		sprite->Render();
		sprite->SetDestinationRect(rcDestCopy);
	}
}
void DxTextRenderObject::AddRenderObject(std::shared_ptr<Sprite2D> obj)
{
	ObjectData data;
	ZeroMemory(&data.bias, sizeof(POINT));
	data.sprite = obj;
	listData_.push_back(data);
}
void DxTextRenderObject::AddRenderObject(std::shared_ptr<DxTextRenderObject> obj, POINT bias)
{
	std::list<ObjectData>::iterator itr = obj->listData_.begin();
	for (; itr != obj->listData_.end(); itr++) {
		(*itr).bias = bias;
		listData_.push_back(*itr);
	}
}

//DxTextRenderer
DxTextRenderer* DxTextRenderer::thisBase_ = NULL;
DxTextRenderer::DxTextRenderer()
{
	colorVertex_ = D3DCOLOR_ARGB(255, 255, 255, 255);
}
DxTextRenderer::~DxTextRenderer()
{
}
bool DxTextRenderer::Initialize()
{
	if (thisBase_ != NULL)
		return false;

	winFont_.CreateFontW(Font::GOTHIC, 20, true);

	thisBase_ = this;
	return true;
}
SIZE DxTextRenderer::_GetTextSize(HDC hDC, const char* pText, int size)
{
	return _GetTextSize(hDC, StringUtility::ConvertMultiToWide(pText, CP_UTF8).c_str(), size);
}
SIZE DxTextRenderer::_GetTextSize(HDC hDC, const wchar_t* pText, int size)
{
	//文字コード
	int charCount = 1;
	int code = 0;

	//文字サイズ計算
	SIZE ssize;
	::GetTextExtentPoint32W(hDC, pText, charCount, &ssize);
	return ssize;
}
std::shared_ptr<DxTextLine> DxTextRenderer::_GetTextInfoSub(std::string text, std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo, std::shared_ptr<DxTextLine> textLine, HDC& hDC, int& totalWidth, int& totalHeight)
{
	DxFont& dxFont = dxText->GetFont();
	int sidePitch = dxText->GetSidePitch();
	int linePitch = dxText->GetLinePitch();
	int widthMax = dxText->GetMaxWidth();
	int heightMax = dxText->GetMaxHeight();
	int widthBorder = dxFont.GetBorderType() != DxFont::BORDER_NONE ? dxFont.GetBorderWidth() : 0;
	textLine->SetSidePitch(sidePitch);

	const std::string strFirstForbid = "」、。";

	char* pText = const_cast<char*>(text.data());
	char* eText = const_cast<char*>(text.data() + text.size());
	while (true) {
		if (*pText == '\0' || pText >= eText)
			break;

		//文字コード
		int charCount = 1;
		int code = *pText;

		//禁則処理
		SIZE sizeNext;
		ZeroMemory(&sizeNext, sizeof(SIZE));
		char* pNextChar = pText + charCount;
		if (pNextChar < eText) {
			//次の文字
			std::string strNext = "";
			strNext.resize(1);
			memcpy(&strNext[0], pNextChar, strNext.size() * sizeof(char));

			bool bFirstForbid = strFirstForbid.find(strNext) != std::string::npos;
			if (bFirstForbid)
				sizeNext = _GetTextSize(hDC, pNextChar);
		}

		//文字サイズ計算
		SIZE size = _GetTextSize(hDC, pText);
		int lw = size.cx + widthBorder + sidePitch;
		int lh = size.cy;
		if (textLine->width_ + lw + sizeNext.cx >= widthMax) {
			//改行
			totalWidth = max(totalWidth, textLine->width_);
			totalHeight += textLine->height_ + linePitch;
			textInfo->AddTextLine(textLine);
			textLine = std::make_shared<DxTextLine>();
			textLine->SetSidePitch(sidePitch);
			continue;
		}
		if (totalHeight + size.cy > heightMax) {
			textLine = NULL;
			break;
		}
		textLine->width_ += lw;
		textLine->height_ = max(textLine->height_, lh);
		textLine->code_.push_back(code);

		pText += charCount;
	}
	return textLine;
}
std::shared_ptr<DxTextInfo> DxTextRenderer::GetTextInfo(std::shared_ptr<DxText> dxText)
{
	SetFont(dxText->dxFont_.GetLogFont());
	auto res = std::make_shared<DxTextInfo>();
	std::string& text = dxText->GetText();
	DxFont& dxFont = dxText->GetFont();
	int linePitch = dxText->GetLinePitch();
	int widthMax = dxText->GetMaxWidth();
	int heightMax = dxText->GetMaxHeight();
	RECT margin = dxText->GetMargin();

	std::shared_ptr<Font> fontTemp;

	HDC hDC = ::GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC, winFont_.GetHandle());

	bool bEnd = false;
	int totalWidth = 0;
	int totalHeight = 0;
	int widthBorder = dxFont.GetBorderType() != DxFont::BORDER_NONE ? dxFont.GetBorderWidth() : 0;
	std::shared_ptr<DxTextLine> textLine = std::make_shared<DxTextLine>();
	textLine->width_ = margin.left;

	if (dxText->IsSyntacticAnalysis()) {
		DxTextScanner scan(text);
		while (!bEnd) {
			if (!scan.HasNext()) {
				//残りを加える
				if (textLine->code_.size() > 0) {
					totalWidth = max(totalWidth, textLine->width_);
					totalHeight += textLine->height_;
					res->AddTextLine(textLine);
				}
				break;
			}

			DxTextToken& tok = scan.Next();
			int typeToken = tok.GetType();
			if (typeToken == DxTextToken::TK_TEXT) {
				std::string text = tok.GetElement();
				text = _ReplaceRenderText(text);
				if (text.size() == 0 || text == "")
					continue;

				textLine = _GetTextInfoSub(text, dxText, res, textLine, hDC, totalWidth, totalHeight);
				if (textLine == NULL)
					bEnd = true;
			} else if (typeToken == DxTextScanner::TOKEN_TAG_START) {
				int indexTag = textLine->code_.size();
				tok = scan.Next();
				std::string element = tok.GetElement();
				if (element == DxTextScanner::TAG_NEW_LINE) {
					//改行
					if (textLine->height_ == 0) {
						//空文字の場合も空白文字で高さを計算する
						textLine = _GetTextInfoSub(" ", dxText, res, textLine, hDC, totalWidth, totalHeight);
					}

					if (textLine != NULL) {
						totalWidth = max(totalWidth, textLine->width_);
						totalHeight += textLine->height_ + linePitch;
						res->AddTextLine(textLine);
					}

					textLine = std::make_shared<DxTextLine>();
				} else if (element == DxTextScanner::TAG_RUBY) {
					std::shared_ptr<DxTextTag_Ruby> tag = std::make_shared <DxTextTag_Ruby>();
					tag->SetTagIndex(indexTag);

					while (true) {
						tok = scan.Next();
						if (tok.GetType() == DxTextScanner::TOKEN_TAG_END)
							break;
						std::string str = tok.GetElement();
						if (str == "rb") {
							scan.CheckType(scan.Next(), DxTextToken::TK_EQUAL);
							std::string text = scan.Next().GetString();
							tag->SetText(text);
						} else if (str == "rt") {
							scan.CheckType(scan.Next(), DxTextToken::TK_EQUAL);
							std::string text = scan.Next().GetString();
							tag->SetRuby(text);
						}
					}

					int linePos = res->GetLineCount();
					int codeCount = textLine->GetTextCodes().size();
					std::string text = tag->GetText();
					std::shared_ptr<DxTextLine> textLineRuby = textLine;
					textLine = _GetTextInfoSub(text, dxText, res, textLine, hDC, totalWidth, totalHeight);

					SIZE sizeText = _GetTextSize(hDC, &text[0], text.size());
					int rubyWidth = dxText->GetFontSize() / 2;
					std::string sRuby = tag->GetRuby();
					int rubyCount = sRuby.length();
					if (rubyCount > 0) {
						int rubyPitch = max(sizeText.cx / rubyCount - rubyWidth, 0);
						int rubyMarginLeft = rubyPitch / 2;
						tag->SetLeftMargin(rubyMarginLeft);

						std::shared_ptr<DxText> dxTextRuby = std::make_shared<DxText>();
						dxTextRuby->SetText(tag->GetRuby());
						dxTextRuby->SetFont(dxFont);
						dxTextRuby->SetPosition(dxText->GetPosition());
						dxTextRuby->SetMaxWidth(dxText->GetMaxWidth());
						dxTextRuby->SetSidePitch(rubyPitch);
						dxTextRuby->SetLinePitch(linePitch + dxText->GetFontSize() - rubyWidth);
						dxTextRuby->SetFontBold(true);
						dxTextRuby->SetFontItalic(false);
						dxTextRuby->SetFontUnderLine(false);
						dxTextRuby->SetFontSize(rubyWidth);
						dxTextRuby->SetFontBorderWidth(dxFont.GetBorderWidth() / 2);
						tag->SetRenderText(dxTextRuby);

						int currentCodeCount = textLineRuby->GetTextCodes().size();
						if (codeCount == currentCodeCount) {
							//タグが完全に次の行に回る場合
							tag->SetTagIndex(0);
							textLine->tag_.push_back(tag);
						} else {
							textLineRuby->tag_.push_back(tag);
						}
					}

				} else if (element == DxTextScanner::TAG_FONT) {
					auto tag = std::make_shared<DxTextTag_Font>();
					DxFont font = dxText->GetFont();
					LOGFONT logFont = font.GetLogFont();
					tag->SetTagIndex(indexTag);

					bool bClear = false;
					while (true) {
						tok = scan.Next();
						if (tok.GetType() == DxTextScanner::TOKEN_TAG_END)
							break;
						std::string str = tok.GetElement();
						if (str == "clear") {
							bClear = true;
						} else if (str == "size") {
							scan.CheckType(scan.Next(), DxTextToken::TK_EQUAL);
							int size = scan.Next().GetInteger();
							logFont.lfHeight = size;
						}
					}

					if (bClear) {
						widthBorder = dxFont.GetBorderType() != DxFont::BORDER_NONE ? dxFont.GetBorderWidth() : 0;
						fontTemp = NULL;
						oldFont = (HFONT)SelectObject(hDC, winFont_.GetHandle());
					} else {
						widthBorder = font.GetBorderType() != DxFont::BORDER_NONE ? font.GetBorderWidth() : 0;
						fontTemp = std::make_shared<Font>();
						fontTemp->CreateFontIndirect(logFont);
						font.SetLogFont(logFont);
						oldFont = (HFONT)SelectObject(hDC, fontTemp->GetHandle());
					}
					tag->SetFont(font);
					textLine->tag_.push_back(tag);
				} else {
					std::string text = DxTextScanner::TAG_START;
					text += tok.GetElement();
					while (true) {
						if (tok.GetType() == DxTextScanner::TOKEN_TAG_END)
							break;
						tok = scan.Next();
						text += tok.GetElement();
					}
					text = _ReplaceRenderText(text);
					if (text.size() == 0 || text == "")
						continue;

					textLine = _GetTextInfoSub(text, dxText, res, textLine, hDC, totalWidth, totalHeight);
					if (textLine == NULL)
						bEnd = true;
				}
			}
		}
	} else {
		std::string& text = dxText->GetText();
		text = _ReplaceRenderText(text);
		if (text.size() > 0) {
			textLine = _GetTextInfoSub(text, dxText, res, textLine, hDC, totalWidth, totalHeight);
			res->AddTextLine(textLine);
		}
	}

	res->totalWidth_ = totalWidth + widthBorder;
	res->totalHeight_ = totalHeight + widthBorder;
	::SelectObject(hDC, oldFont);
	::ReleaseDC(NULL, hDC);
	return res;
}
std::string DxTextRenderer::_ReplaceRenderText(std::string& text)
{
	text = StringUtility::ReplaceAll(text, "\r", "");
	text = StringUtility::ReplaceAll(text, "\n", "");
	text = StringUtility::ReplaceAll(text, "\t", "");
	text = StringUtility::ReplaceAll(text, "&nbsp;", " ");
	text = StringUtility::ReplaceAll(text, "&quot;", "\"");
	text = StringUtility::ReplaceAll(text, "&osb;", "[");
	text = StringUtility::ReplaceAll(text, "&csb;", "]");
	return text;
}

void DxTextRenderer::_CreateRenderObject(std::shared_ptr<DxTextRenderObject> objRender, POINT pos, DxFont& dxFont, std::shared_ptr<DxTextLine> textLine)
{
	SetFont(dxFont.GetLogFont());
	DxCharCacheKey keyFont;
	keyFont.font_ = dxFont;
	int textHeight = textLine->GetHeight();

	int xRender = pos.x;
	int yRender = pos.y;

	int countTag = textLine->GetTagCount();
	int indexTag = 0;
	int countCode = textLine->code_.size();
	for (int iCode = 0; iCode < countCode; iCode++) {
		for (; indexTag < countTag;) {
			std::shared_ptr<DxTextTag> tag = textLine->GetTag(indexTag);
			int tagNo = tag->GetTagIndex();
			if (tagNo != iCode)
				break;
			int type = tag->GetTagType();
			if (type == DxTextTag::TYPE_FONT) {
				auto font = std::dynamic_pointer_cast<DxTextTag_Font>(tag);
				dxFont = font->GetFont();
				keyFont.font_ = dxFont;
				auto longfont = dxFont.GetLogFont();
				winFont_.CreateFontIndirect(longfont);
				indexTag++;
			} else if (type == DxTextTag::TYPE_RUBY) {
				auto ruby = std::dynamic_pointer_cast<DxTextTag_Ruby>(tag);
				std::shared_ptr<DxText> textRuby = ruby->GetRenderText();

				RECT margin;
				ZeroMemory(&margin, sizeof(RECT));
				margin.left = xRender + ruby->GetLeftMargin();
				margin.top = pos.y - textRuby->GetFontSize();
				textRuby->SetMargin(margin);

				POINT bias;
				ZeroMemory(&bias, sizeof(POINT));

				objRender->AddRenderObject(textRuby->CreateRenderObject(), bias);

				auto logfont = dxFont.GetLogFont();
				SetFont(logfont);

				indexTag++;
			} else
				break;
		}

		//文字コード
		int code = textLine->code_[iCode];

		//キャッシュに存在するか確認
		keyFont.code_ = code;
		keyFont.font_ = dxFont;
		std::shared_ptr<DxChar> dxChar = cache_.GetChar(keyFont);
		if (dxChar == NULL) {
			//キャッシュにない場合、作成して追加
			dxChar = std::make_shared<DxChar>();
			dxChar->Create(code, winFont_, dxFont);
			cache_.AddChar(keyFont, dxChar);
		}

		//描画
		std::shared_ptr<Sprite2D> spriteText = std::make_shared<Sprite2D>();
		int yGap = 0;
		yRender = pos.y + yGap;
		std::shared_ptr<Texture> texture = dxChar->GetTexture();
		spriteText->SetTexture(texture);

		//		int objWidth = texture->GetWidth();//dxChar->GetWidth();
		//		int objHeight = texture->GetHeight();//dxChar->GetHeight();
		int objWidth = dxChar->GetWidth();
		int objHeight = dxChar->GetHeight();
		RECT_D rcDest = { (double)xRender, (double)yRender, (double)(objWidth + xRender), (double)(objHeight + yRender) };
		RECT_D rcSrc = { 0., 0., (double)objWidth, (double)objHeight };
		spriteText->SetVertex(rcSrc, rcDest, colorVertex_);
		objRender->AddRenderObject(spriteText);

		//次の文字
		xRender += dxChar->GetWidth() - dxFont.GetBorderWidth() + textLine->GetSidePitch();
	}
}

std::shared_ptr<DxTextRenderObject> DxTextRenderer::CreateRenderObject(std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo)
{
	{
		Lock lock(lock_);
		std::shared_ptr<DxTextRenderObject> objRender = std::make_shared <DxTextRenderObject>();
		objRender->SetPosition(dxText->GetPosition());
		objRender->SetVertexColor(dxText->GetVertexColor());
		objRender->SetPermitCamera(dxText->IsPermitCamera());
		objRender->SetShader(dxText->GetShader());

		DxFont& dxFont = dxText->GetFont();
		int linePitch = dxText->GetLinePitch();
		int widthMax = dxText->GetMaxWidth();
		int heightMax = dxText->GetMaxHeight();
		RECT margin = dxText->GetMargin();
		int alignmentHorizontal = dxText->GetHorizontalAlignment();
		int alignmentVertical = dxText->GetVerticalAlignment();
		POINT pos;
		ZeroMemory(&pos, sizeof(POINT));
		bool bAutoIndent = textInfo->IsAutoIndent();

		switch (alignmentVertical) {
		case ALIGNMENT_TOP:
			break;
		case ALIGNMENT_CENTER: {
			int cy = pos.y + heightMax / 2;
			pos.y = cy - textInfo->totalHeight_ / 2;
			break;
		}
		case ALIGNMENT_BOTTOM: {
			int by = pos.y + heightMax;
			pos.y = by - textInfo->totalHeight_;
			break;
		}
		}
		pos.y += margin.top;

		int heightTotal = 0;
		int countLine = textInfo->textLine_.size();
		int lineStart = textInfo->GetValidStartLine() - 1;
		int lineEnd = textInfo->GetValidEndLine() - 1;
		for (int iLine = lineStart; iLine <= lineEnd; iLine++) {
			std::shared_ptr<DxTextLine> textLine = textInfo->GetTextLine(iLine);
			pos.x = 0; //dxText->GetPosition().x;
			if (iLine == 0)
				pos.x += margin.left;
			switch (alignmentHorizontal) {
			case ALIGNMENT_LEFT:
				if (iLine >= 1 && bAutoIndent)
					pos.x = dxText->GetFontSize();
				break;
			case ALIGNMENT_CENTER: {
				int cx = pos.x + widthMax / 2;
				pos.x = cx - textLine->width_ / 2;
				break;
			}
			case ALIGNMENT_RIGHT: {
				int rx = pos.x + widthMax;
				pos.x = rx - textLine->width_;
				break;
			}
			}

			heightTotal += textLine->height_ + linePitch;
			if (heightTotal > heightMax)
				break;

			_CreateRenderObject(objRender, pos, dxFont, textLine);

			pos.y += textLine->height_ + linePitch;
		}
		return objRender;
	}
}

void DxTextRenderer::Render(std::shared_ptr<DxText> dxText)
{
	{
		Lock lock(lock_);
		std::shared_ptr<DxTextInfo> textInfo = GetTextInfo(dxText);
		Render(dxText, textInfo);
	}
}
void DxTextRenderer::Render(std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo)
{
	{
		Lock lock(lock_);
		std::shared_ptr<DxTextRenderObject> objRender = CreateRenderObject(dxText, textInfo);
		objRender->Render();
	}
}
bool DxTextRenderer::AddFontFromFile(std::string path)
{
	std::shared_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if (reader == NULL)
		throw std::exception(StringUtility::Format(u8"フォントファイルが見つかりません(%s)", path.c_str()).c_str());
	if (!reader->Open())
		throw std::exception(StringUtility::Format(u8"フォントファイルを開けません(%s)", path.c_str()).c_str());

	int size = reader->GetFileSize();
	ByteBuffer buf;
	buf.SetSize(size);
	reader->Read(buf.GetPointer(), size);

	DWORD count = 0;
	HANDLE handle = ::AddFontMemResourceEx(buf.GetPointer(), size, NULL, &count);

	Logger::WriteTop(u8"フォント読み込み：" + path);
	return handle != 0;
}

/**********************************************************
//DxText
//テキスト描画
**********************************************************/
DxText::DxText()
{
	dxFont_.SetTopColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxFont_.SetBottomColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	dxFont_.SetBorderType(DxFont::BORDER_NONE);
	dxFont_.SetBorderWidth(0);
	dxFont_.SetBorderColor(D3DCOLOR_ARGB(255, 255, 255, 255));

	LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(LOGFONT));
	logFont.lfEscapement = 0;
	logFont.lfWidth = 0;
	logFont.lfCharSet = ANSI_CHARSET;
	SetFont(logFont);
	SetFontSize(20);
	SetFontType(Font::GOTHIC);
	SetFontBold(false);
	SetFontItalic(false);
	SetFontUnderLine(false);

	pos_.x = 0;
	pos_.y = 0;
	widthMax_ = INT_MAX;
	heightMax_ = INT_MAX;
	sidePitch_ = 0;
	linePitch_ = 4;
	alignmentHorizontal_ = ALIGNMENT_LEFT;
	alignmentVertical_ = ALIGNMENT_TOP;
	ZeroMemory(&margin_, sizeof(RECT));

	colorVertex_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	bPermitCamera_ = true;
	bSyntacticAnalysis_ = true;
}
DxText::~DxText()
{
}
void DxText::Copy(const DxText& src)
{
	dxFont_ = src.dxFont_;
	pos_ = src.pos_;
	widthMax_ = src.widthMax_;
	heightMax_ = src.heightMax_;
	sidePitch_ = src.sidePitch_;
	linePitch_ = src.linePitch_;
	margin_ = src.margin_;
	alignmentHorizontal_ = src.alignmentHorizontal_;
	alignmentVertical_ = src.alignmentVertical_;
	colorVertex_ = src.colorVertex_;
	text_ = src.text_;
}
void DxText::SetFontType(const wchar_t* type)
{
	LOGFONTW info = dxFont_.GetLogFont();
	lstrcpy(info.lfFaceName, type);
	info.lfCharSet = DEFAULT_CHARSET;
	SetFont(info);
}
void DxText::Render()
{
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	{
		Lock lock(renderer->GetLock());
		renderer->SetVertexColor(colorVertex_);
		renderer->Render(std::shared_ptr<DxText>(this));
	}
}
void DxText::Render(std::shared_ptr<DxTextInfo> textInfo)
{
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	{
		Lock lock(renderer->GetLock());
		renderer->SetVertexColor(colorVertex_);
		renderer->Render(std::shared_ptr<DxText>(this), textInfo);
	}
}
std::shared_ptr<DxTextInfo> DxText::GetTextInfo()
{
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	{
		Lock lock(renderer->GetLock());
		renderer->SetVertexColor(colorVertex_);
		return renderer->GetTextInfo(std::shared_ptr<DxText>(this));
	}
}
std::shared_ptr<DxTextRenderObject> DxText::CreateRenderObject()
{
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	{
		Lock lock(renderer->GetLock());
		renderer->SetVertexColor(colorVertex_);
		std::shared_ptr<DxTextInfo> textInfo = renderer->GetTextInfo(std::shared_ptr<DxText>(this));
		std::shared_ptr<DxTextRenderObject> res = renderer->CreateRenderObject(std::shared_ptr<DxText>(this), textInfo);
		return res;
	}
}
std::shared_ptr<DxTextRenderObject> DxText::CreateRenderObject(std::shared_ptr<DxTextInfo> textInfo)
{
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	{
		Lock lock(renderer->GetLock());
		renderer->SetVertexColor(colorVertex_);
		std::shared_ptr<DxTextRenderObject> res = renderer->CreateRenderObject(std::shared_ptr<DxText>(this), textInfo);
		return res;
	}
}

/**********************************************************
//DxTextStepper
**********************************************************/
DxTextStepper::DxTextStepper()
{
	posNext_ = 0;
	framePerSec_ = 60;
	countNextPerFrame_ = 60;
	countFrame_ = 0;
	Clear();
}
DxTextStepper::~DxTextStepper()
{
}
void DxTextStepper::Clear()
{
	posNext_ = 0;
	text_ = L"";
	source_ = L"";
}
void DxTextStepper::_Next()
{
	if (!HasNext())
		return;

	if (source_[posNext_] == DxTextScanner::TAG_START[0]) {
		text_ += source_[posNext_];
		posNext_++;
		while (true) {
			bool bBreak = (source_[posNext_] == DxTextScanner::TAG_END[0]);
			text_ += source_[posNext_];
			posNext_++;
			if (bBreak)
				break;
		}
	} else {
		text_ += source_[posNext_];
		posNext_++;
	}
}
void DxTextStepper::Next()
{
	double ratioFrame = (double)countNextPerFrame_ / (double)framePerSec_;
	int lastCountFrame = (int)countFrame_;
	countFrame_ += ratioFrame;
	while (true) {
		if (countFrame_ < 1.0)
			break;
		countFrame_ -= 1.0;
		_Next();
	}
}
void DxTextStepper::NextSkip()
{
	while (HasNext())
		_Next();
}
bool DxTextStepper::HasNext()
{
	return posNext_ < source_.size();
}
void DxTextStepper::SetSource(std::wstring text)
{
	posNext_ = 0;
	source_ = text;
	// text_ = "";
}
