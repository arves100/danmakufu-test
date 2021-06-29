#ifndef __DIRECTX_DXTEXT__
#define __DIRECTX_DXTEXT__

#include "DxConstant.hpp"
#include "RenderObject.hpp"
#include "Texture.hpp"

namespace directx {

class DxChar;
class DxCharCache;
class DxCharCacheKey;
class DxTextRenderer;
class DxText;

/**********************************************************
//DxFont
**********************************************************/
class DxFont {
	friend DxCharCache;
	friend DxCharCacheKey;

public:
	enum {
		BORDER_NONE,
		BORDER_FULL,
		BORDER_SHADOW,
	};

public:
	DxFont();
	virtual ~DxFont();
	void SetLogFont(LOGFONTW& font) { info_ = font; }
	LOGFONTW& GetLogFont() { return info_; }
	void SetTopColor(D3DCOLOR color) { colorTop_ = color; }
	D3DCOLOR GetTopColor() { return colorTop_; }
	void SetBottomColor(D3DCOLOR color) { colorBottom_ = color; }
	D3DCOLOR GetBottomColor() { return colorBottom_; }
	void SetBorderType(int type) { typeBorder_ = type; }
	int GetBorderType() { return typeBorder_; }
	void SetBorderWidth(int width) { widthBorder_ = width; }
	int GetBorderWidth() { return widthBorder_; }
	void SetBorderColor(D3DCOLOR color) { colorBorder_ = color; }
	D3DCOLOR GetBorderColor() { return colorBorder_; }

protected:
	LOGFONTW info_; //フォント種別
	D3DCOLOR colorTop_;
	D3DCOLOR colorBottom_;
	int typeBorder_; //縁取り
	int widthBorder_;
	D3DCOLOR colorBorder_; //縁取り色
};

/**********************************************************
//DxChar
//文字1文字のテクスチャ
**********************************************************/
class DxChar {
public:
	DxChar();
	virtual ~DxChar();
	bool Create(int code, gstd::Font& winFont, DxFont& dxFont);
	std::shared_ptr<Texture> GetTexture() { return texture_; }
	int GetWidth() { return width_; }
	int GetHeight() { return height_; }
	LOGFONT GetInfo() { return font_.GetLogFont(); }

private:
	std::shared_ptr<Texture> texture_;
	int code_;
	DxFont font_;

	int width_;
	int height_;
};

/**********************************************************
//DxCharCache
//文字キャッシュ
**********************************************************/
class DxCharCacheKey {
	friend DxCharCache;
	friend DxTextRenderer;

public:
	bool operator==(const DxCharCacheKey& key) const
	{
		bool res = true;
		res &= (code_ == key.code_);
		res &= (font_.colorTop_ == key.font_.colorTop_);
		res &= (font_.colorBottom_ == key.font_.colorBottom_);
		res &= (font_.typeBorder_ == key.font_.typeBorder_);
		res &= (font_.widthBorder_ == key.font_.widthBorder_);
		res &= (font_.colorBorder_ == key.font_.colorBorder_);
		if (!res)
			return res;
		res &= (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) == 0);
		return res;
	}
	bool operator<(const DxCharCacheKey& key) const
	{
		if (code_ != key.code_)
			return code_ < key.code_;
		if (font_.colorTop_ != key.font_.colorTop_)
			return font_.colorTop_ < key.font_.colorTop_;
		if (font_.colorBottom_ != key.font_.colorBottom_)
			return font_.colorBottom_ < key.font_.colorBottom_;
		//				if(font_.typeBorder_ != key.font_.typeBorder_)return (font_.typeBorder_ != key.font_.typeBorder_ );
		if (font_.widthBorder_ != key.font_.widthBorder_)
			return font_.widthBorder_ < key.font_.widthBorder_;
		if (font_.colorBorder_ != key.font_.colorBorder_)
			return font_.colorBorder_ < key.font_.colorBorder_;
		return (memcmp(&key.font_.info_, &font_.info_, sizeof(LOGFONT)) < 0);
	}

private:
	int code_;
	DxFont font_;
};
class DxCharCache {
	friend DxTextRenderer;

public:
	DxCharCache();
	~DxCharCache();
	void Clear();
	int GetCacheCount() { return mapCache_.size(); }

	std::shared_ptr<DxChar> GetChar(DxCharCacheKey& key);
	void AddChar(DxCharCacheKey& key, std::shared_ptr<DxChar> value);

private:
	int sizeMax_;
	int countPri_;
	std::map<DxCharCacheKey, std::shared_ptr<DxChar>> mapCache_;
	std::map<int, DxCharCacheKey> mapPriKey_;
	std::map<DxCharCacheKey, int> mapKeyPri_;

	void _arrange();
};

/**********************************************************
//DxTextScanner
**********************************************************/
class DxTextScanner;
class DxTextToken {
	friend DxTextScanner;

public:
	enum Type {
		TK_UNKNOWN,
		TK_EOF,
		TK_NEWLINE,
		TK_ID,
		TK_INT,
		TK_REAL,
		TK_STRING,

		TK_OPENP,
		TK_CLOSEP,
		TK_OPENB,
		TK_CLOSEB,
		TK_OPENC,
		TK_CLOSEC,
		TK_SHARP,

		TK_COMMA,
		TK_EQUAL,
		TK_ASTERISK,
		TK_SLASH,
		TK_COLON,
		TK_SEMICOLON,
		TK_TILDE,
		TK_EXCLAMATION,
		TK_PLUS,
		TK_MINUS,
		TK_LESS,
		TK_GREATER,

		TK_TEXT,
	};

public:
	DxTextToken() { type_ = TK_UNKNOWN; }
	DxTextToken(Type type, std::string element)
	{
		type_ = type;
		element_ = element;
	}

	virtual ~DxTextToken() {}
	Type GetType() { return type_; }
	std::string& GetElement() { return element_; }

	int GetInteger();
	double GetReal();
	bool GetBoolean();
	std::string GetString();
	std::string& GetIdentifier();

protected:
	Type type_;
	std::string element_;
};

class DxTextScanner {
public:
	const static int TOKEN_TAG_START;
	const static int TOKEN_TAG_END;
	const static std::string TAG_START;
	const static std::string TAG_END;
	const static std::string TAG_NEW_LINE;
	const static std::string TAG_RUBY;
	const static std::string TAG_FONT;

public:
	DxTextScanner(char* str, int charCount);
	DxTextScanner(std::string str);
	DxTextScanner(std::vector<char>& buf);
	virtual ~DxTextScanner();

	DxTextToken& GetToken(); //現在のトークンを取得
	DxTextToken& Next();
	bool HasNext();
	void CheckType(DxTextToken& tok, int type);
	void CheckIdentifer(DxTextToken& tok, std::string id);
	int GetCurrentLine();
	int SearchCurrentLine();

	std::vector<char>::iterator GetCurrentPointer();
	void SetCurrentPointer(std::vector<char>::iterator pos);
	void SetPointerBegin() { pointer_ = buffer_.begin(); }
	int GetCurrentPosition();
	void SetTagScanEnable(bool bEnable) { bTagScan_ = bEnable; }

private:
	void Init(std::vector<wchar_t>&);
protected:
	int line_;
	std::vector<char> buffer_;
	std::vector<char>::iterator pointer_; //今の位置
	DxTextToken token_; //現在のトークン
	boolean bTagScan_;

	wchar_t _NextChar(); //ポインタを進めて次の文字を調べる
	virtual void _SkipComment(); //コメントをとばす
	virtual void _SkipSpace(); //空白をとばす
	virtual void _RaiseError(std::string str); //例外を投げます
	bool _IsTextStartSign();
	bool _IsTextScan();
};

/**********************************************************
//DxTextRenderer
//テキスト描画エンジン
**********************************************************/
class DxTextLine;
class DxTextInfo;
class DxTextRenderer;
class DxTextTag;
class DxTextTag {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_RUBY,
		TYPE_FONT
	};

public:
	DxTextTag()
	{
		indexTag_ = 0;
		typeTag_ = TYPE_UNKNOWN;
	}
	virtual ~DxTextTag(){};
	int GetTagType() { return typeTag_; }
	int GetTagIndex() { return indexTag_; }
	void SetTagIndex(int index) { indexTag_ = index; }

protected:
	int typeTag_;
	int indexTag_;
};
class DxTextTag_Ruby : public DxTextTag {
public:
	DxTextTag_Ruby()
	{
		typeTag_ = TYPE_RUBY;
		leftMargin_ = 0;
	}
	int GetLeftMargin() { return leftMargin_; }
	void SetLeftMargin(int left) { leftMargin_ = left; }

	std::string& GetText() { return text_; }
	void SetText(std::string text) { text_ = text; }
	std::string& GetRuby() { return ruby_; }
	void SetRuby(std::string ruby) { ruby_ = ruby; }

	std::shared_ptr<DxText> GetRenderText() { return dxText_; }
	void SetRenderText(std::shared_ptr<DxText> text) { dxText_ = text; }

private:
	int leftMargin_;
	std::string text_;
	std::string ruby_;
	std::shared_ptr<DxText> dxText_;
};
class DxTextTag_Font : public DxTextTag {
private:
	DxFont font_;

public:
	DxTextTag_Font() { typeTag_ = TYPE_FONT; }
	void SetFont(DxFont font) { font_ = font; }
	DxFont& GetFont() { return font_; }

};
class DxTextLine {
	friend DxTextRenderer;

public:
	DxTextLine()
	{
		width_ = 0;
		height_ = 0;
		sidePitch_ = 0;
	}
	virtual ~DxTextLine(){};
	int GetWidth() { return width_; }
	int GetHeight() { return height_; }
	int GetSidePitch() { return sidePitch_; }
	void SetSidePitch(int pitch) { sidePitch_ = pitch; }
	std::vector<int>& GetTextCodes() { return code_; }
	int GetTextCodeCount() { return code_.size(); }
	int GetTagCount() { return tag_.size(); }
	std::shared_ptr<DxTextTag> GetTag(int index) { return tag_[index]; }

protected:
	int width_;
	int height_;
	int sidePitch_;
	std::vector<int> code_;
	std::vector<std::shared_ptr<DxTextTag>> tag_;
};
class DxTextInfo {
	friend DxTextRenderer;

public:
	DxTextInfo()
	{
		totalWidth_ = 0;
		totalHeight_ = 0;
		lineValidStart_ = 1;
		lineValidEnd_ = 0;
		bAutoIndent_ = false;
	}
	virtual ~DxTextInfo(){};
	int GetTotalWidth() { return totalWidth_; }
	int GetTotalHeight() { return totalHeight_; }
	int GetValidStartLine() { return lineValidStart_; }
	int GetValidEndLine() { return lineValidEnd_; }
	void SetValidStartLine(int line) { lineValidStart_ = line; }
	void SetValidEndLine(int line) { lineValidEnd_ = line; }
	bool IsAutoIndent() { return bAutoIndent_; }
	void SetAutoIndent(bool bEnable) { bAutoIndent_ = bEnable; }

	int GetLineCount() { return textLine_.size(); }
	void AddTextLine(std::shared_ptr<DxTextLine> text) { textLine_.push_back(text), lineValidEnd_++; }
	std::shared_ptr<DxTextLine> GetTextLine(int pos) { return textLine_[pos]; }

protected:
	int totalWidth_;
	int totalHeight_;
	int lineValidStart_;
	int lineValidEnd_;
	bool bAutoIndent_;
	std::vector<std::shared_ptr<DxTextLine>> textLine_;
};

class DxTextRenderObject {
private:
	struct ObjectData {
		POINT bias;
		std::shared_ptr<Sprite2D> sprite;
	};

public:
	DxTextRenderObject();
	virtual ~DxTextRenderObject() {}

	void Render();
	void AddRenderObject(std::shared_ptr<Sprite2D> obj);
	void AddRenderObject(std::shared_ptr<DxTextRenderObject> obj, POINT bias);

	POINT GetPosition() { return position_; }
	void SetPosition(POINT pos)
	{
		position_.x = pos.x;
		position_.y = pos.y;
	}
	void SetPosition(int x, int y)
	{
		position_.x = x;
		position_.y = y;
	}
	void SetVertexColor(D3DCOLOR color) { color_ = color; }

	void SetAngle(D3DXVECTOR3& angle) { angle_ = angle; }
	void SetScale(D3DXVECTOR3& scale) { scale_ = scale; }
	void SetTransCenter(D3DXVECTOR2& center) { center_ = center; }
	void SetAutoCenter(bool bAuto) { bAutoCenter_ = bAuto; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }

	std::shared_ptr<Shader> GetShader() { return shader_; }
	void SetShader(std::shared_ptr<Shader> shader) { shader_ = shader; }

protected:
	POINT position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
	D3DCOLOR color_;
	std::list<ObjectData> listData_;
	D3DXVECTOR2 center_; //座標変換の中心
	bool bAutoCenter_;
	bool bPermitCamera_;
	std::shared_ptr<Shader> shader_;
};

class DxTextRenderer {
private:
	static DxTextRenderer* thisBase_;

public:
	enum {
		ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT,
		ALIGNMENT_TOP,
		ALIGNMENT_BOTTOM,
		ALIGNMENT_CENTER,
	};

public:
	DxTextRenderer();
	virtual ~DxTextRenderer();
	static DxTextRenderer* GetBase() { return thisBase_; }
	bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	void ClearCache() { cache_.Clear(); }
	void SetFont(LOGFONT& logFont) { winFont_.CreateFontIndirect(logFont); }
	void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }
	std::shared_ptr<DxTextInfo> GetTextInfo(std::shared_ptr<DxText> dxText);

	std::shared_ptr<DxTextRenderObject> CreateRenderObject(std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo);

	void Render(std::shared_ptr<DxText> dxText);
	void Render(std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo);

	int GetCacheCount() { return cache_.GetCacheCount(); }

	bool AddFontFromFile(std::string path);

protected:
	DxCharCache cache_;
	gstd::Font winFont_;
	D3DCOLOR colorVertex_;
	gstd::CriticalSection lock_;

	SIZE _GetTextSize(HDC hDC, const wchar_t* pText, int size = 1);
	SIZE _GetTextSize(HDC hDC, const char* pText, int size = 1);
	std::shared_ptr<DxTextLine> _GetTextInfoSub(std::string text, std::shared_ptr<DxText> dxText, std::shared_ptr<DxTextInfo> textInfo, std::shared_ptr<DxTextLine> textLine, HDC& hDC, int& totalWidth, int& totalHeight);
	void _CreateRenderObject(std::shared_ptr<DxTextRenderObject> objRender, POINT pos, DxFont& dxFont, std::shared_ptr<DxTextLine> textLine);
	std::string _ReplaceRenderText(std::string& text);
};

/**********************************************************
//DxText
//テキスト描画
**********************************************************/
class DxText {
	friend DxTextRenderer;

public:
	enum {
		ALIGNMENT_LEFT = DxTextRenderer::ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT = DxTextRenderer::ALIGNMENT_RIGHT,
		ALIGNMENT_TOP = DxTextRenderer::ALIGNMENT_TOP,
		ALIGNMENT_BOTTOM = DxTextRenderer::ALIGNMENT_BOTTOM,
		ALIGNMENT_CENTER = DxTextRenderer::ALIGNMENT_CENTER,
	};
public:
	DxText();
	virtual ~DxText();
	void Copy(const DxText& src);
	virtual void Render();
	void Render(std::shared_ptr<DxTextInfo> textInfo);

	std::shared_ptr<DxTextInfo> GetTextInfo();
	std::shared_ptr<DxTextRenderObject> CreateRenderObject();
	std::shared_ptr<DxTextRenderObject> CreateRenderObject(std::shared_ptr<DxTextInfo> textInfo);

	DxFont& GetFont() { return dxFont_; }
	void SetFont(DxFont font) { dxFont_ = font; }
	void SetFont(LOGFONT logFont) { dxFont_.SetLogFont(logFont); }

	void SetFontType(const wchar_t* type);
	int GetFontSize() { return dxFont_.GetLogFont().lfHeight; }
	void SetFontSize(int size)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfHeight = size;
		SetFont(info);
	}
	void SetFontBold(bool bBold)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfWeight = (bBold == false) * FW_NORMAL + (bBold == TRUE) * FW_BOLD;
		SetFont(info);
	}
	void SetFontItalic(bool bItalic)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfItalic = bItalic;
		SetFont(info);
	}
	void SetFontUnderLine(bool bLine)
	{
		LOGFONT info = dxFont_.GetLogFont();
		info.lfUnderline = bLine;
		SetFont(info);
	}

	void SetFontColorTop(D3DCOLOR color) { dxFont_.SetTopColor(color); }
	void SetFontColorBottom(D3DCOLOR color) { dxFont_.SetBottomColor(color); }
	void SetFontBorderWidth(int width) { dxFont_.SetBorderWidth(width); }
	void SetFontBorderType(int type) { dxFont_.SetBorderType(type); }
	void SetFontBorderColor(D3DCOLOR color) { dxFont_.SetBorderColor(color); }

	POINT GetPosition() { return pos_; }
	void SetPosition(int x, int y)
	{
		pos_.x = x;
		pos_.y = y;
	}
	void SetPosition(POINT pos) { pos_ = pos; }
	int GetMaxWidth() { return widthMax_; }
	void SetMaxWidth(int width) { widthMax_ = width; }
	int GetMaxHeight() { return heightMax_; }
	void SetMaxHeight(int height) { heightMax_ = height; }
	int GetSidePitch() { return sidePitch_; }
	void SetSidePitch(int pitch) { sidePitch_ = pitch; }
	int GetLinePitch() { return linePitch_; }
	void SetLinePitch(int pitch) { linePitch_ = pitch; }
	RECT GetMargin() { return margin_; }
	void SetMargin(RECT margin) { margin_ = margin; }
	int GetHorizontalAlignment() { return alignmentHorizontal_; }
	void SetHorizontalAlignment(int value) { alignmentHorizontal_ = value; }
	int GetVerticalAlignment() { return alignmentVertical_; }
	void SetVerticalAlignment(int value) { alignmentVertical_ = value; }

	D3DCOLOR GetVertexColor() { return colorVertex_; }
	void SetVertexColor(D3DCOLOR color) { colorVertex_ = color; }
	bool IsPermitCamera() { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
	bool IsSyntacticAnalysis() { return bSyntacticAnalysis_; }
	void SetSyntacticAnalysis(bool bEnable) { bSyntacticAnalysis_ = bEnable; }

	std::string& GetText() { return text_; }
	void SetText(std::string text) { text_ = text; }

	std::shared_ptr<Shader> GetShader() { return shader_; }
	void SetShader(std::shared_ptr<Shader> shader) { shader_ = shader; }

protected:
	DxFont dxFont_;
	POINT pos_;
	int widthMax_;
	int heightMax_;
	int sidePitch_;
	int linePitch_;
	RECT margin_;
	int alignmentHorizontal_;
	int alignmentVertical_;
	D3DCOLOR colorVertex_;
	bool bPermitCamera_;
	bool bSyntacticAnalysis_;

	std::shared_ptr<Shader> shader_;
	std::string text_;
};

/**********************************************************
//DxTextStepper
**********************************************************/
class DxTextStepper {
public:
	DxTextStepper();
	virtual ~DxTextStepper();
	void Clear();

	std::wstring GetText() { return text_; }
	void Next();
	void NextSkip();
	bool HasNext();
	void SetSource(std::wstring text);

protected:
	int posNext_;
	std::wstring text_;
	std::wstring source_;
	int framePerSec_;
	int countNextPerFrame_;
	double countFrame_;
	void _Next();
};

} // namespace directx

#endif
