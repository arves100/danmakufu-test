#ifndef __DIRECTX_EVENTSCRIPT__
#define __DIRECTX_EVENTSCRIPT__

#include "DxScript.hpp"
#include "DxText.hpp"
#include "DxWindow.hpp"
#include "RenderObject.hpp"
#include "TransitionEffect.hpp"

namespace directx {

class EventScriptSource;
class EventScriptCompiler;
class EventScriptCode;
class EventScriptBlock;
class EventScriptBlock_Main;

class EventValueParser;
class EventEngine;
class EventImage;

class DxScriptForEvent;

/**********************************************************
//EventScriptSource
//コンパイルされたイベントスクリプトコード
**********************************************************/
class EventScriptSource {
	friend EventScriptCompiler;

public:
	EventScriptSource();
	virtual ~EventScriptSource();

	int GetCodeCount() { return code_.size(); }
	void AddCode(std::shared_ptr<EventScriptCode> code);
	std::shared_ptr<EventScriptCode> GetCode(int index) { return code_[index]; }
	std::shared_ptr<EventScriptBlock_Main> GetEventBlock(std::string name);

protected:
	std::vector<std::shared_ptr<EventScriptCode>> code_;
	std::map<std::string, std::shared_ptr<EventScriptBlock_Main>> event_;
};

/**********************************************************
//EventScriptScanner
**********************************************************/
class EventScriptScanner;
class EventScriptToken {
	friend EventScriptScanner;

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
	EventScriptToken() { type_ = TK_UNKNOWN; }
	EventScriptToken(Type type, std::string element)
	{
		type_ = type;
		element_ = element;
	}

	virtual ~EventScriptToken() {}
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

class EventScriptScanner {
public:
	const static int TOKEN_TAG_START;
	const static int TOKEN_TAG_END;
	const static std::string TAG_START;
	const static std::string TAG_END;
	const static std::string TAG_NEW_LINE;
	const static std::string TAG_RUBY;
	const static std::string TAG_FONT;

public:
	EventScriptScanner(char* str, int size);
	EventScriptScanner(std::string str);
	EventScriptScanner(std::vector<char>& buf);
	virtual ~EventScriptScanner();

	EventScriptToken& GetToken(); //現在のトークンを取得
	EventScriptToken& Next();
	bool HasNext();
	void CheckType(EventScriptToken& tok, int type);
	void CheckIdentifer(EventScriptToken& tok, std::string id);
	int GetCurrentLine();
	int SearchCurrentLine();

	std::vector<char>::iterator GetCurrentPointer();
	void SetCurrentPointer(std::vector<char>::iterator pos);
	void SetPointerBegin() { pointer_ = buffer_.begin(); }
	int GetCurrentPosition();
	void SetTagScanEnable(bool bEnable) { bTagScan_ = bEnable; }

private:
	void Init(std::vector<char>& buf);

protected:
	int line_;
	std::vector<char> buffer_;
	std::vector<char>::iterator pointer_; //今の位置
	EventScriptToken token_; //現在のトークン
	boolean bTagScan_;

	char _NextChar(); //ポインタを進めて次の文字を調べる
	virtual void _SkipComment(); //コメントをとばす
	virtual void _SkipSpace(); //空白をとばす
	virtual void _RaiseError(std::wstring str); //例外を投げます
	bool _IsTextStartSign();
	bool _IsTextScan();
};

/**********************************************************
//EventScriptCompiler
**********************************************************/
class EventScriptCompiler {
public:
	EventScriptCompiler();
	virtual ~EventScriptCompiler();

	void SetPath(std::wstring path) { path_ = path; }
	std::shared_ptr<EventScriptSource> Compile();

private:
	std::wstring path_;
	std::shared_ptr<EventScriptScanner> scan_;
	std::shared_ptr<EventScriptSource> source_;
	std::list<std::vector<std::shared_ptr<EventScriptBlock>>> block_;

	void _ParseBlock(std::shared_ptr<EventScriptCode> blockStartCode);
	std::shared_ptr<EventScriptCode> _ParseTag(std::shared_ptr<EventScriptCode> blockStartCode);
	std::map<std::string, EventScriptToken> _GetAllTagElement();
	std::string _GetNextTagElement();
	std::string _GetNextTagElement(int type);
};

/**********************************************************
//EventScriptBlock
**********************************************************/
class EventScriptBlock {
	friend EventScriptSource;

public:
	enum {
		POS_NULL = -1,
	};
	const static std::string BLOCK_GLOBAL;

public:
	EventScriptBlock()
	{
		bInner_ = false;
		posStart_ = POS_NULL;
		posEnd_ = POS_NULL;
		posReturn_ = POS_NULL;
	}
	virtual ~EventScriptBlock() {}
	int GetStartPosition() { return posStart_; }
	void SetStartPosition(int pos) { posStart_ = pos; }
	int GetEndPosition() { return posEnd_; }
	void SetEndPosition(int pos) { posEnd_ = pos; }
	int GetReturnPosition() { return posReturn_; }
	void SetReturnPosition(int pos) { posReturn_ = pos; }
	bool IsInner() { return bInner_; }
	virtual bool IsGlobal() { return false; }

protected:
	bool bInner_;
	int posStart_;
	int posEnd_;
	int posReturn_;
};

class EventScriptBlock_Main : public EventScriptBlock {
	friend EventScriptSource;

public:
	EventScriptBlock_Main() { bInner_ = false; }
	virtual ~EventScriptBlock_Main() {}
	std::string GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }
	bool IsGlobal() { return name_ == BLOCK_GLOBAL; }

private:
	std::string name_;
};

/**********************************************************
//EventScriptCode
**********************************************************/
class EventScriptCode {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_TEXT,
		TYPE_NEXT_LINE,
		TYPE_WAIT_CLICK,
		TYPE_WAIT_NEXT_PAGE,
		TYPE_WAIT_TIME,
		TYPE_CLEAR_MESSAGE,
		TYPE_NAME,
		TYPE_TRANSITION,
		TYPE_VISIBLE_TEXT,

		TYPE_VAR,
		TYPE_EVAL,
		TYPE_SYSVAL,
		TYPE_OUTPUT,

		TYPE_IMAGE,
		TYPE_SOUND,

		TYPE_IF,
		TYPE_ELSEIF,
		TYPE_ENDIF,
		TYPE_LOOP,
		TYPE_JUMP,

		TYPE_SCRIPT,
		TYPE_END,
		TYPE_BATTLE,
	};
	const static std::string TAG_WAIT_CLICK;
	const static std::string TAG_WAIT_NEXT_PAGE;
	const static std::string TAG_WAIT_TIME;
	const static std::string TAG_CLEAR_MESSAGE;
	const static std::string TAG_NAME;
	const static std::string TAG_TRANSITION;
	const static std::string TAG_VISIBLE_TEXT;
	const static std::string TAG_VAR;
	const static std::string TAG_EVAL;
	const static std::string TAG_SYSVAL;
	const static std::string TAG_OUTPUT;
	const static std::string TAG_IMAGE;
	const static std::string TAG_BGM;
	const static std::string TAG_SE;
	const static std::string TAG_IF;
	const static std::string TAG_ELSEIF;
	const static std::string TAG_ENDIF;
	const static std::string TAG_GOSUB;
	const static std::string TAG_GOTO;
	const static std::string TAG_SCRIPT;
	const static std::string TAG_SCRIPT_END;
	const static std::string TAG_END;
	const static std::string TAG_BATTLE;

	const static std::string STRING_INVALID;

public:
	EventScriptCode();
	virtual ~EventScriptCode();

	int GetType() { return type_; }
	int GetLine() { return line_; }
	void SetLine(int line) { line_ = line; }
	virtual std::string GetCodeText() { return ""; }

protected:
	int type_;
	int line_;
};

class EventScriptCode_Text : public EventScriptCode {
public:
	EventScriptCode_Text();
	std::string GetText() { return text_; }
	std::string GetCodeText() { return GetText(); }
	void SetText(std::string text) { text_ = text; }

protected:
	std::string text_;
};
class EventScriptCode_NextLine : public EventScriptCode {
public:
	EventScriptCode_NextLine();
	std::string GetCodeText();
};
class EventScriptCode_WaitClick : public EventScriptCode {
public:
	EventScriptCode_WaitClick();
};
class EventScriptCode_WaitNextPage : public EventScriptCode {
public:
	EventScriptCode_WaitNextPage();
};
class EventScriptCode_WaitTime : public EventScriptCode {
public:
	EventScriptCode_WaitTime();
	std::string GetTime() { return time_; }
	void SetTime(std::string time) { time_ = time; }
	bool IsSkipEnable() { return bSkipEnable_; }
	void SetSkipEnable(bool bEnable) { bSkipEnable_ = bEnable; }

private:
	std::string time_;
	bool bSkipEnable_;
};
class EventScriptCode_ClearMessage : public EventScriptCode {
public:
	EventScriptCode_ClearMessage();
};
class EventScriptCode_Name : public EventScriptCode {
public:
	EventScriptCode_Name();
	std::string& GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }

private:
	std::string name_;
};
class EventScriptCode_Transition : public EventScriptCode {
public:
	enum {
		TRANS_NONE,
		TRANS_FADE,
		TRANS_SCRIPT,
	};

public:
	EventScriptCode_Transition();
	int GetTransType() { return typeTrans_; }
	void SetTransType(int type) { typeTrans_ = type; }
	std::string GetFrame() { return frame_; }
	void SetFrame(std::string frame) { frame_ = frame; }
	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }
	std::string GetMethod() { return method_; }
	void SetMethod(std::string method) { method_ = method; }

protected:
	int typeTrans_;
	std::string frame_;
	std::string path_;
	std::string method_;
};
class EventScriptCode_VisibleText : public EventScriptCode {
public:
	EventScriptCode_VisibleText();
	bool IsVisible() { return bVisible_; }
	void SetVisible(bool bVisible) { bVisible_ = bVisible; }

private:
	bool bVisible_;
};
class EventScriptCode_Var : public EventScriptCode {
public:
	EventScriptCode_Var();
	std::string& GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(std::string value) { value_ = value; }

private:
	std::string name_;
	std::string value_;
};
class EventScriptCode_Eval : public EventScriptCode {
public:
	EventScriptCode_Eval();
	std::string& GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(std::string value) { value_ = value; }

private:
	std::string name_;
	std::string value_;
};
class EventScriptCode_SysVal : public EventScriptCode {
public:
	EventScriptCode_SysVal();
	std::string& GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }
	std::string& GetValue() { return value_; }
	void SetValue(std::string value) { value_ = value; }
	bool IsGlobal() { return bGlobal_; }
	void SetGlobal(bool b) { bGlobal_ = b; }

private:
	bool bGlobal_;
	std::string name_;
	std::string value_;
};
class EventScriptCode_Output : public EventScriptCode {
public:
	EventScriptCode_Output();
	std::string& GetValue() { return value_; }
	void SetValue(std::string value) { value_ = value; }

private:
	std::string value_;
};
class EventScriptCode_Image : public EventScriptCode {
public:
	enum {
		VALUE_INVALID = INT_MIN,
	};

public:
	EventScriptCode_Image();
	std::string GetObjectIdentifier() { return idObject_; }
	void SetObjectIdentifier(std::string id) { idObject_ = id; }
	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }
	int GetLayer() { return layer_; }
	void SetLayer(int layer) { layer_ = layer; }
	std::string GetPriority() { return pri_; }
	void SetPriority(std::string pri) { pri_ = pri; }
	std::string GetVisible() { return bVisible_; }
	void SetVisible(std::string bVisible) { bVisible_ = bVisible_; }
	std::string GetLeftDestPoint() { return posDestLeft_; }
	void SetLeftDestPoint(std::string left) { posDestLeft_ = left; }
	std::string GetTopDestPoint() { return posDestTop_; }
	void SetTopDestPoint(std::string top) { posDestTop_ = top; }
	std::string GetTransition() { return bTransition_; }
	void SetTransition(std::string bTrans) { bTransition_ = bTrans; }
	std::string GetWaitEnd() { return bWaitEnd_; }
	void SetWaitEnd(std::string bWaitEnd) { bWaitEnd_ = bWaitEnd; }

protected:
	std::string idObject_;
	std::string path_;
	int layer_;
	std::string pri_;
	std::string bVisible_;
	std::string posDestLeft_;
	std::string posDestTop_;
	std::string bTransition_;
	std::string bWaitEnd_;
};
class EventScriptCode_Sound : public EventScriptCode {
public:
	EventScriptCode_Sound();
	int GetSoundType() { return typeSound_; }
	void SetSoundType(int type) { typeSound_ = type; }
	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }

private:
	int typeSound_;
	std::string path_;
};
class EventScriptCode_If : public EventScriptCode, public EventScriptBlock {
public:
	EventScriptCode_If();
	std::string GetParameter() { return param_; }
	void SetParameter(std::string param) { param_ = param; }
	int GetNextElsePosition() { return posNextElse_; }
	void SetNextElsePosition(int pos) { posNextElse_ = pos; }

protected:
	std::string param_;
	int posNextElse_;
};

class EventScriptCode_Jump : public EventScriptCode {
public:
	EventScriptCode_Jump();

	bool IsGoSub() { return bGoSub_; }
	void SetGosub(bool b) { bGoSub_ = b; }

	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }
	std::string GetName() { return name_; }
	void SetName(std::string name) { name_ = name; }

protected:
	bool bGoSub_;
	std::string path_;
	std::string name_;
};

class EventScriptCode_Script : public EventScriptCode {
public:
	EventScriptCode_Script();
	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }
	std::string GetMethod() { return method_; }
	void SetMethod(std::string method) { method_ = method; }
	std::string GetWaitEnd() { return bWaitEnd_; }
	void SetWaitEnd(std::string bWait) { bWaitEnd_ = bWait; }
	std::string GetTargetId() { return targetId_; }
	void SetTargetId(std::string target) { targetId_ = target; }
	std::string GetCode() { return code_; }
	void SetCode(std::string code) { code_ = code; }
	std::string GetId() { return id_; }
	void SetId(std::string id) { id_ = id; }
	std::vector<std::string> GetArgumentList() { return arg_; }
	void SetArgument(int index, std::string arg)
	{
		if (index >= arg_.size())
			arg_.resize(index + 1);
		arg_[index] = arg;
	}

	bool IsEndScript() { return bEndScript_; }
	void SetEndScript(bool bEnd) { bEndScript_ = bEnd; }

private:
	std::string path_;
	std::string method_;
	std::string bWaitEnd_;
	std::string targetId_;
	std::string code_;
	std::string id_;
	std::vector<std::string> arg_;
	bool bEndScript_;
};

class EventScriptCode_End : public EventScriptCode {
public:
	EventScriptCode_End();
	std::string GetArgument() { return arg_; }
	void SetArgument(std::string arg) { arg_ = arg; }

protected:
	std::string arg_;
};

class EventScriptCode_Battle : public EventScriptCode {
public:
	EventScriptCode_Battle();
	std::string GetPath() { return path_; }
	void SetPath(std::string path) { path_ = path; }

protected:
	std::string path_;
};

/**********************************************************
//EventScriptCodeExecuter
**********************************************************/
class EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter(EventEngine* engine);
	virtual ~EventScriptCodeExecuter();
	virtual void Execute() = 0;
	bool IsEnd() { return bEnd_; }

protected:
	EventEngine* engine_;
	bool bEnd_;

	int _GetElementInteger(std::string value);
	double _GetElementReal(std::string value);
	bool _GetElementBoolean(std::string value);
	std::string _GetElementString(std::string value);
	bool _IsValieElement(std::string value);
};

class EventScriptCodeExecuter_WaitClick : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitClick(EventEngine* engine)
		: EventScriptCodeExecuter(engine)
	{
	}
	virtual void Execute();
};

class EventScriptCodeExecuter_WaitNextPage : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitNextPage(EventEngine* engine)
		: EventScriptCodeExecuter(engine)
	{
	}
	virtual void Execute();
};

class EventScriptCodeExecuter_WaitTime : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_WaitTime(EventEngine* engine, EventScriptCode_WaitTime* code);
	virtual void Execute();

private:
	int count_;
	int time_;
	bool bSkipEnable_;
};

class EventScriptCodeExecuter_Transition : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Transition(EventEngine* engine, EventScriptCode_Transition* code);
	virtual void Execute();

private:
	class DxScriptRenderObject_Transition : public DxScriptObjectBase {
	public:
		DxScriptRenderObject_Transition(TransitionEffect* effect)
		{
			effect_ = effect;
			priRender_ = 1.0;
		}
		virtual void Render() { effect_->Render(); }
		virtual void SetRenderState() {}

	private:
		TransitionEffect* effect_;
	};
	EventScriptCode_Transition* code_;
	std::shared_ptr<DxScriptForEvent> script_;
	std::shared_ptr<TransitionEffect> effect_;
	int frame_;
};

class EventScriptCodeExecuter_Image : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Image(EventEngine* engine, EventScriptCode_Image* code);
	~EventScriptCodeExecuter_Image();
	virtual void Execute();

private:
	EventScriptCode_Image* code_;
	std::string pathImage_;
	std::shared_ptr<DxScriptObjectManager> objManager_;
	std::shared_ptr<DxScriptSpriteObject2D>::unsync nowSprite_;
	std::shared_ptr<DxScriptSpriteObject2D>::unsync oldSprite_;
	int frame_;
	bool bTrans_;
	double priority_;

	void _Initialize();
};

class EventScriptCodeExecuter_Script : public EventScriptCodeExecuter {
public:
	EventScriptCodeExecuter_Script(EventEngine* engine, DxScriptForEvent* script);
	virtual void Execute();

private:
	DxScriptForEvent* script_;
};

/**********************************************************
//EventWindowManager
**********************************************************/
class EventMouseCaptureLayer;
class EventTextWindow;
class EventNameWindow;
class EventLogWindow;
class EventWindowManager : public DxWindowManager {
public:
	EventWindowManager(EventEngine* engine);
	EventEngine* GetEngine() { return engine_; }
	virtual bool Initialize();
	virtual void Work();
	virtual void Render();

	std::shared_ptr<EventMouseCaptureLayer> GetMouseCaptureLayer() { return layerCapture_; }
	std::shared_ptr<EventTextWindow> GetTextWindow() { return wndText_; }
	std::shared_ptr<EventLogWindow> GetLogWindow() { return wndLog_; }
	std::shared_ptr<EventNameWindow> GetNameWindow() { return wndName_; }

	std::shared_ptr<DxButton> GetSaveButton() { return btnSave_; }
	std::shared_ptr<DxButton> GetLoadButton() { return btnLoad_; }

	bool IsTextVisible() { return bVisibleText_; }
	void SetTextVisible(bool bVisible) { bVisibleText_ = bVisible; }

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

protected:
	EventEngine* engine_;
	std::shared_ptr<EventMouseCaptureLayer> layerCapture_;
	std::shared_ptr<EventTextWindow> wndText_;
	std::shared_ptr<EventLogWindow> wndLog_;
	std::shared_ptr<EventNameWindow> wndName_;

	std::shared_ptr<DxButton> btnSave_;
	std::shared_ptr<DxButton> btnLoad_;

	bool bVisibleText_;
};

class EventWindow : public DxWindow {
public:
	EventWindow() { bApplyVisibleText_ = true; }
	virtual bool IsWindowVisible();

protected:
	bool bApplyVisibleText_;
	EventWindowManager* _GetManager() { return (EventWindowManager*)manager_; }
};
class EventButton : public DxButton {
public:
	EventButton() { bApplyVisibleText_ = true; }
	virtual bool IsWindowVisible();

protected:
	bool bApplyVisibleText_;
	EventWindowManager* _GetManager() { return (EventWindowManager*)manager_; }
};
class EventMouseCaptureLayer : public EventWindow {
public:
	virtual void AddedManager();
	virtual void DispatchedEvent(std::shared_ptr<DxWindowEvent> event);
	void ClearEvent();
	std::shared_ptr<DxWindowEvent> GetEvent() { return event_; }

protected:
	std::shared_ptr<DxWindowEvent> event_;
};
class EventTextWindow : public EventWindow {
public:
	EventTextWindow();
	virtual void AddedManager();
	virtual void Render();
	bool IsWait();

protected:
	RECT rcMargin_;
	std::shared_ptr<DxText> dxText_;
};
class EventNameWindow : public EventWindow {
public:
	EventNameWindow();
	void Work();
	void Render();
	virtual void RenderText();
	std::wstring GetText() { return text_->GetText(); }
	void SetText(std::wstring text) { text_->SetText(text); }

private:
	std::shared_ptr<DxText> text_;
};
class EventLogWindow : public EventWindow {
public:
	EventLogWindow();
	virtual void AddedManager();
	virtual void Work();
	virtual void Render();

	std::shared_ptr<DxText> GetRenderer() { return dxText_; }
	void ResetPosition();

private:
	int posMin_; //最低表示位置
	int pos_;
	std::shared_ptr<DxText> dxText_;
};

/**********************************************************
//EventEngine
**********************************************************/
class EventScriptObjectManager : public DxScriptObjectManager, public gstd::Recordable {
public:
	virtual int AddObject(std::shared_ptr<DxScriptObjectBase>::unsync obj);

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

private:
	enum {
		INDEX_FREE_START = 256,
	};
};
class EventText : public DxTextStepper {
public:
	bool IsVoiceText();
};
class EventLogText {
public:
	EventLogText(EventEngine* engine);
	virtual ~EventLogText();
	void Add(std::string text, std::string name);

	int GetInfoCount() { return listInfo_.size(); }
	std::shared_ptr<DxTextInfo> GetTextInfo(int pos) { return listInfo_[pos]; }

private:
	EventEngine* engine_;
	int max_;
	std::vector<std::shared_ptr<DxTextInfo>> listInfo_;
};
class EventValue : public gstd::Recordable {
	friend EventValueParser;

public:
	enum {
		TYPE_UNKNOWN,
		TYPE_REAL,
		TYPE_BOOLEAN,
		TYPE_STRING,
	};

public:
	EventValue() { type_ = TYPE_UNKNOWN; }
	virtual ~EventValue(){};
	int GetType() { return type_; }
	double GetReal()
	{
		double res = valueReal_;
		if (IsBoolean())
			res = valueBoolean_ ? 1.0 : 0.0;
		if (IsString())
			res = atof(valueString_.c_str());
		return res;
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
	std::string GetString()
	{
		std::string res = valueString_;
		if (IsReal())
			res = gstd::StringUtility::Format("%f", valueReal_);
		if (IsBoolean())
			res = (valueBoolean_ ? "true" : "false");
		return res;
	}
	bool IsReal() { return type_ == TYPE_REAL; }
	bool IsBoolean() { return type_ == TYPE_BOOLEAN; }
	bool IsString() { return type_ == TYPE_STRING; }

	void SetReal(double v)
	{
		type_ = TYPE_REAL;
		valueReal_ = v;
	}
	void SetBoolean(bool v)
	{
		type_ = TYPE_BOOLEAN;
		valueBoolean_ = v;
	}
	void SetString(std::string str)
	{
		type_ = TYPE_STRING;
		valueString_ = str;
	}

	gstd::TextParser::Result ConvertToTextParserResult();
	void Copy(gstd::TextParser::Result& val);
	void Copy(EventValue& val);

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

protected:
	int type_;
	std::string valueString_;
	union {
		double valueReal_;
		bool valueBoolean_;
	};
};
class EventFrame {
public:
	EventFrame();
	virtual ~EventFrame();
	void SetActiveSource(std::shared_ptr<EventScriptSource> source) { sourceActive_ = source; }
	std::shared_ptr<EventScriptBlock> GetBlock() { return block_; }
	void SetBlock(std::shared_ptr<EventScriptBlock> block);
	std::shared_ptr<EventScriptSource> GetActiveSource() { return sourceActive_; }

	int GetCurrentPosition() { return posCode_; }
	void SetCurrentPosition(int pos) { posCode_ = pos; }
	std::shared_ptr<EventScriptCode> NextCode();
	std::shared_ptr<EventScriptCode> GetCurrentCode();
	bool HasNextCode();
	void SetEnd() { bEnd_ = true; }
	bool IsEnd() { return bEnd_; }
	std::shared_ptr<EventValue> GetValue(std::string key);
	void AddValue(std::string key, std::shared_ptr<EventValue> val);
	void SetValue(std::string key, std::shared_ptr<EventValue> val);

	int GetReturnPosition() { return posReturn_; }
	void SetReturnPosition(int pos) { posReturn_ = pos; }
	bool IsInnerBlock();

	bool IsAutoGlobal() { return bAutoGlobal_; }
	void SetAutoGlobal(bool bAuto) { bAutoGlobal_ = bAuto; }

	//保存
	void ReadRecord(gstd::RecordBuffer& record, EventEngine* engine);
	void WriteRecord(gstd::RecordBuffer& record, EventEngine* engine);

protected:
	bool bEnd_;
	int posCode_;
	int posReturn_;
	bool bAutoGlobal_;
	std::shared_ptr<EventScriptSource> sourceActive_;
	std::shared_ptr<EventScriptBlock> block_;
	std::map<std::string, std::shared_ptr<EventValue>> mapValue_;
};

class EventValueParser : public gstd::TextParser {
public:
	EventValueParser(EventEngine* engine);
	std::shared_ptr<EventValue> GetEventValue(std::string text);
protected:
	EventEngine* engine_;
	virtual Result _ParseIdentifer(std::vector<char>::iterator pos);
	std::vector<std::string> _GetFuctionArgument();

};

class EventImage : public gstd::Recordable {
public:
	enum {
		LAYER_FOREGROUND,
		LAYER_BACKGROUND,
		MAX_OBJECT = 256 * 4,
		INDEX_OLD_START = 100,
		ID_TRANSITION = 255,
	};

public:
	EventImage();
	virtual ~EventImage();
	void Render(int layer);
	int GetForegroundLayerIndex();
	int GetBackgroundLayerIndex();
	void SwapForeBackLayerIndex();
	std::shared_ptr<DxScriptObjectManager> GetObjectManager(int layer) { return objManager_[layer]; }

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

protected:
	std::vector<std::shared_ptr<EventScriptObjectManager>> objManager_;
	int indexForeground_;
};

class EventKeyState {
public:
	EventKeyState(EventEngine* engine);
	virtual ~EventKeyState();
	void Work();

	void SetNextEnable(bool bEnable) { bNextEnable_ = bEnable; }
	bool IsNext();
	bool IsSkip();

protected:
	EventEngine* engine_;
	bool bNextEnable_;
};

class EventSound : public gstd::Recordable {
public:
	enum {
		TYPE_BGM,
		TYPE_SE,
	};

public:
	EventSound();
	virtual ~EventSound();

	void Play(int type, std::string path);
	void Delete(int type);

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

private:
	std::shared_ptr<SoundPlayer> playerBgm_;
	std::shared_ptr<SoundPlayer> playerSe_;
};

class EventEngine : public gstd::TaskBase, public gstd::Recordable {
public:
	enum {
		STATE_RUN,
		STATE_LOG,
		STATE_HIDE_TEXT,
		STATE_STOP,
	};

public:
	EventEngine();
	virtual ~EventEngine();
	virtual bool Initialize();

	virtual void Work();
	virtual void Render();
	void SetSource(std::wstring path);

	std::shared_ptr<EventScriptSource> GetSource(std::wstring path);
	std::wstring GetSourcePath(std::shared_ptr<EventScriptSource> source);
	virtual bool IsEnd();

	std::shared_ptr<EventText> GetEventText() { return textEvent_; }
	std::shared_ptr<EventLogText> GetEventLogText() { return logEvent_; }
	std::shared_ptr<EventWindowManager> GetWindowManager() { return windowManager_; }

	std::shared_ptr<EventValue> GetEventValue(std::string key);
	std::shared_ptr<EventImage> GetEventImage() { return image_; }
	std::shared_ptr<EventKeyState> GetEventKeyState() { return keyState_; }

	std::shared_ptr<EventScriptCodeExecuter> GetActiveCodeExecuter() { return activeCodeExecuter_; }
	std::shared_ptr<EventFrame> GetGlobalFrame() { return frameGlobal_; }

	std::shared_ptr<gstd::ScriptEngineCache> GetScriptEngineCache() { return cacheScriptEngine_; }

	bool IsCriticalFrame() { return bCriticalFrame_; }

	void SetState(int state);
	int GetState() { return state_; }
	virtual void CheckStateChenge();

	bool IsSaveEnable();
	bool Load(std::wstring path);
	bool Load(gstd::RecordBuffer& record);
	bool Save(std::wstring path);
	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

protected:
	enum {
		RUN_RETURN_NONE,
		RUN_RETURN_DO,
	};

	int state_;
	std::shared_ptr<EventWindowManager> windowManager_;
	std::shared_ptr<EventText> textEvent_;
	std::shared_ptr<EventLogText> logEvent_;
	std::map<std::wstring, std::shared_ptr<EventScriptSource>> mapSource_;
	std::vector<std::shared_ptr<EventFrame>> frame_;
	std::shared_ptr<EventImage> image_;
	std::shared_ptr<EventKeyState> keyState_;
	std::shared_ptr<EventSound> sound_;
	bool bCriticalFrame_;
	std::shared_ptr<EventFrame> frameGlobal_; //グローバル変数用

	std::shared_ptr<EventScriptCodeExecuter> activeCodeExecuter_;
	std::list<std::shared_ptr<EventScriptCodeExecuter>> parallelCodeExecuter_;
	std::list<std::shared_ptr<DxScriptForEvent>> listScript_;
	std::shared_ptr<gstd::ScriptEngineCache> cacheScriptEngine_;

	void _RaiseError(std::wstring msg);
	virtual void _RunCode();
	virtual int _RunCode(std::shared_ptr<EventFrame> frameActive, std::shared_ptr<EventScriptCode> code) { return RUN_RETURN_NONE; }
	virtual void _RunScript();
	virtual void _WorkWindow();
	std::shared_ptr<EventScriptSource> _GetSource(std::wstring path);
};

/**********************************************************
//DxScriptForEvent
**********************************************************/
class DxScriptForEvent : public DxScript, public gstd::Recordable {
public:
	DxScriptForEvent(EventEngine* engine);
	~DxScriptForEvent();
	void Clear();
	virtual bool SetSourceFromFile(std::wstring path);
	virtual void SetSource(std::string source);
	virtual int AddObject(std::shared_ptr<DxScriptObjectBase>::unsync obj);
	virtual void DeleteObject(int id);

	std::string GetMethod() { return method_; }
	void SetMethod(std::string method) { method_ = method; }

	void EndScript() { bScriptEnd_ = true; }
	bool IsScriptEnd() { return bScriptEnd_; }

	void SetTargetId(int target) { targetId_ = target; }
	std::string GetScriptId() { return scriptId_; }
	void SetScriptId(std::string id) { scriptId_ = id; }
	std::string GetCode() { return code_; }

	void AddArgumentValue(std::shared_ptr<EventValue> arg);

	void Read(gstd::RecordBuffer& record);
	void Write(gstd::RecordBuffer& record);

	//関数：スクリプト操作
	static gstd::value Func_EndScript(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetTarget(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_GetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv);
	static gstd::value Func_SetEventValue(gstd::script_machine* machine, int argc, gstd::value const* argv);

	//関数：キー入力
	static gstd::value Func_IsSkip(gstd::script_machine* machine, int argc, gstd::value const* argv);

private:
	EventEngine* engine_;
	bool bScriptEnd_;
	std::string method_;
	int targetId_;
	std::string scriptId_;
	std::set<int> listObj_;
	std::string code_;

	virtual std::vector<char> _Include(std::vector<char>& source);
};

} // namespace directx

#endif
