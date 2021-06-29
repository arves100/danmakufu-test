#include "ScriptClient.hpp"
#include "File.hpp"
#include "Logger.hpp"
#include <ctime>

using namespace gstd;

/**********************************************************
//ScriptEngineData
**********************************************************/
ScriptEngineData::ScriptEngineData()
{
	mapLine_ = std::shared_ptr<ScriptFileLineMap>();
}
ScriptEngineData::~ScriptEngineData()
{
}
void ScriptEngineData::SetSource(std::vector<char>& source)
{
	source_ = source;
}

/**********************************************************
//ScriptEngineCache
**********************************************************/
ScriptEngineCache::ScriptEngineCache()
{
}
ScriptEngineCache::~ScriptEngineCache()
{
}
void ScriptEngineCache::Clear()
{
	cache_.clear();
}
void ScriptEngineCache::AddCache(std::string name, std::shared_ptr<ScriptEngineData> data)
{
	cache_[name] = data;
}
std::shared_ptr<ScriptEngineData> ScriptEngineCache::GetCache(std::string name)
{
	if (!IsExists(name))
		return std::shared_ptr<ScriptEngineData>();
	return cache_[name];
}
bool ScriptEngineCache::IsExists(std::string name)
{
	return cache_.find(name) != cache_.end();
}

/**********************************************************
//ScriptClientBase
**********************************************************/
script_type_manager ScriptClientBase::typeManagerDefault_;
function const commonFunction[] = {
	//共通関数：スクリプト引数結果
	{ "GetScriptArgument", ScriptClientBase::Func_GetScriptArgument, 1 },
	{ "GetScriptArgumentCount", ScriptClientBase::Func_GetScriptArgumentCount, 0 },
	{ "SetScriptResult", ScriptClientBase::Func_SetScriptResult, 1 },

	//共通関数：数学系
	{ "min", ScriptClientBase::Func_Min, 2 },
	{ "max", ScriptClientBase::Func_Max, 2 },
	{ "log", ScriptClientBase::Func_Log, 1 },
	{ "log10", ScriptClientBase::Func_Log10, 1 },
	{ "cos", ScriptClientBase::Func_Cos, 1 },
	{ "sin", ScriptClientBase::Func_Sin, 1 },
	{ "tan", ScriptClientBase::Func_Tan, 1 },
	{ "acos", ScriptClientBase::Func_Acos, 1 },
	{ "asin", ScriptClientBase::Func_Asin, 1 },
	{ "atan", ScriptClientBase::Func_Atan, 1 },
	{ "atan2", ScriptClientBase::Func_Atan2, 2 },
	{ "rand", ScriptClientBase::Func_Rand, 2 },

	//共通関数：文字列操作
	{ "ToString", ScriptClientBase::Func_ToString, 1 },
	{ "IntToString", ScriptClientBase::Func_IntToString, 1 },
	{ "itoa", ScriptClientBase::Func_ItoA, 1 },
	{ "rtoa", ScriptClientBase::Func_RtoA, 1 },
	{ "rtos", ScriptClientBase::Func_RtoS, 2 },
	{ "vtos", ScriptClientBase::Func_VtoS, 2 },
	{ "atoi", ScriptClientBase::Func_AtoI, 1 },
	{ "ator", ScriptClientBase::Func_AtoR, 1 },
	{ "TrimString", ScriptClientBase::Func_TrimString, 1 },
	{ "SplitString", ScriptClientBase::Func_SplitString, 2 },

	//共通関数：パス関連
	{ "GetModuleDirectory", ScriptClientBase::Func_GetModuleDirectory, 0 },
	{ "GetMainScriptDirectory", ScriptClientBase::Func_GetMainScriptDirectory, 0 },
	{ "GetCurrentScriptDirectory", ScriptClientBase::Func_GetCurrentScriptDirectory, 0 },
	{ "GetFileDirectory", ScriptClientBase::Func_GetFileDirectory, 1 },
	{ "GetFilePathList", ScriptClientBase::Func_GetFilePathList, 1 },
	{ "GetDirectoryList", ScriptClientBase::Func_GetDirectoryList, 1 },

	//共通関数：時刻関連
	{ "GetCurrentDateTimeS", ScriptClientBase::Func_GetCurrentDateTimeS, 0 },

	//共通関数：デバッグ関連
	{ "WriteLog", ScriptClientBase::Func_WriteLog, 1 },
	{ "RaiseError", ScriptClientBase::Func_RaiseError, 1 },

	//共通関数：共通データ
	{ "SetDefaultCommonDataArea", ScriptClientBase::Func_SetDefaultCommonDataArea, 1 },
	{ "SetCommonData", ScriptClientBase::Func_SetCommonData, 2 },
	{ "GetCommonData", ScriptClientBase::Func_GetCommonData, 2 },
	{ "ClearCommonData", ScriptClientBase::Func_ClearCommonData, 0 },
	{ "DeleteCommonData", ScriptClientBase::Func_DeleteCommonData, 1 },
	{ "SetAreaCommonData", ScriptClientBase::Func_SetAreaCommonData, 3 },
	{ "GetAreaCommonData", ScriptClientBase::Func_GetAreaCommonData, 3 },
	{ "ClearAreaCommonData", ScriptClientBase::Func_ClearAreaCommonData, 1 },
	{ "DeleteAreaCommonData", ScriptClientBase::Func_DeleteAreaCommonData, 2 },
	{ "CreateCommonDataArea", ScriptClientBase::Func_CreateCommonDataArea, 1 },
	{ "CopyCommonDataArea", ScriptClientBase::Func_CopyCommonDataArea, 2 },
	{ "IsCommonDataAreaExists", ScriptClientBase::Func_IsCommonDataAreaExists, 1 },
	{ "GetCommonDataAreaKeyList", ScriptClientBase::Func_GetCommonDataAreaKeyList, 0 },
	{ "GetCommonDataValueKeyList", ScriptClientBase::Func_GetCommonDataValueKeyList, 1 },

	//定数
	{ "NULL", constant<0>::func, 0 },
};

ScriptClientBase::ScriptClientBase()
{
	bError_ = false;
	engine_ = std::make_unique<gstd::ScriptEngineData>();
	machine_ = NULL;
	mainThreadID_ = 0;
	idScript_ = ID_SCRIPT_FREE;
	valueRes_ = value();

	commonDataManager_ = std::make_unique<ScriptCommonDataManager>();
	mt_ = std::make_unique<MersenneTwister>();
	mt_->Initialize(SDL_GetTicks());
	_AddFunction(commonFunction, sizeof(commonFunction) / sizeof(function));
}
ScriptClientBase::~ScriptClientBase()
{
	if (machine_ != NULL)
		delete machine_;
	machine_ = NULL;
}

void ScriptClientBase::_AddFunction(char const* name, callback f, unsigned arguments)
{
	function tFunc;
	tFunc.name = name;
	tFunc.func = f;
	tFunc.arguments = arguments;
	func_.push_back(tFunc);
}
void ScriptClientBase::_AddFunction(const function* f, int count)
{
	int funcPos = func_.size();
	func_.resize(funcPos + count);
	memcpy(&func_[funcPos], f, sizeof(function) * count);
}

void ScriptClientBase::_RaiseError(int line, std::string message)
{
	bError_ = true;
	std::string errorPos = _GetErrorLineSource(line);

	std::shared_ptr<ScriptFileLineMap>& mapLine = engine_->GetScriptFileLineMap();
	ScriptFileLineMap::Entry entry = mapLine->GetEntry(line);
	int lineOriginal = entry.lineEndOriginal_ - (entry.lineEnd_ - line);

	std::string fileName = PathProperty::GetFileName(entry.path_);

	std::string str = StringUtility::Format(u8"%s\r\n%s \r\n%s line(行)=%d\r\n\r\n↓\r\n%s\r\n～～～",
		message.c_str(),
		entry.path_.c_str(),
		fileName.c_str(),
		lineOriginal,
		errorPos.c_str());
	throw ScriptException(str);
}
void ScriptClientBase::_RaiseErrorFromEngine()
{
	int line = engine_->GetEngine()->get_error_line();
	_RaiseError(line, engine_->GetEngine()->get_error_message());
}
void ScriptClientBase::_RaiseErrorFromMachine()
{
	int line = machine_->get_error_line();
	_RaiseError(line, machine_->get_error_message());
}
std::string ScriptClientBase::_GetErrorLineSource(int line)
{
	if (line == 0)
		return "";
	int encoding = engine_->GetEncoding();
	std::vector<char>& source = engine_->GetSource();
	char* pbuf = (char*)&source[0];
	char* sbuf = pbuf;
	char* ebuf = sbuf + source.size();

	int tLine = 1;
	int rLine = line;
	while (pbuf < ebuf) {
		if (tLine == rLine)
			break;

		if (*pbuf == '\n')
			tLine++;
		pbuf++;
	}

	const int countMax = 256;
	int count = 0;
	sbuf = pbuf;
	while (pbuf < ebuf && count < countMax) {
		pbuf++;
		count++;
	}

	int size = _MAX(count - 1, 0);
	std::wstring res;
	if (encoding == Encoding::UTF16LE) {
		wchar_t* wbufS = (wchar_t*)sbuf;
		wchar_t* wbufE = wbufS + size;
		res = std::wstring(wbufS, wbufE);
	} else {
		std::string sStr = std::string(sbuf, sbuf + size);
		res = StringUtility::ConvertMultiToWide(sStr);
	}

	return res;
}
std::vector<char> ScriptClientBase::_Include(std::vector<char>& source)
{
	//TODO とりあえず実装
	std::string pathSource = engine_->GetPath();
	std::vector<char> res = source;
	FileManager* fileManager = FileManager::GetBase();
	std::set<std::string> setReadPath;

	std::shared_ptr<ScriptFileLineMap> mapLine = std::make_shared<ScriptFileLineMap>();
	engine_->SetScriptFileLineMap(mapLine);

	mapLine->AddEntry(pathSource,
		1,
		StringUtility::CountCharacter(source, '\n') + 1);

	bool bEnd = false;
	while (true) {
		if (bEnd)
			break;
		Scanner scanner(res);
		int resSize = res.size();

		bEnd = true;
		while (scanner.HasNext()) {
			Token& tok = scanner.Next();
			if (tok.GetType() == Token::Type::EndOfFile) { //Eofの識別子が来たらファイルの調査終了
				break;
			} else if (tok.GetType() == Token::Type::Sharp) {
				int posInclude = scanner.GetCurrentPointer() - 1;
	
				tok = scanner.Next();
				if (tok.GetElement() != "include")
					continue;

				bEnd = false;
				int posCurrent = scanner.GetCurrentPointer();
				std::string wPath = scanner.Next().GetString();
				bool bNeedNewLine = false;
				if (scanner.HasNext()) {
					int posBeforeNewLine = scanner.GetCurrentPointer();
					if (scanner.Next().GetType() != Token::Type::Newline) {
						int line = scanner.GetCurrentLine();
						source = res;
						engine_->SetSource(source);

						std::string error;
						error += "New line is not found after #include.\r\n";
						error += u8"(#include後に改行がありません)";
						_RaiseError(line, error);
					}
					scanner.SetCurrentPointer(posBeforeNewLine);
				} else {
					// bNeedNewLine = true;
				}
				int posAfterInclude = scanner.GetCurrentPointer();
				scanner.SetCurrentPointer(posCurrent);

				// "../"から始まっていたら、前に"./"をつける。
				if (wPath.find("../") == 0 || wPath.find("..\\") == 0) {
					wPath = "./" + wPath;
				}

				if (wPath.find(".\\") != std::string::npos || wPath.find("./") != std::string::npos) { //".\"展開
					int line = scanner.GetCurrentLine();
					std::string linePath = mapLine->GetPath(line);
					std::string tDir = PathProperty::GetFileDirectory(linePath);
					// std::string tDir = PathProperty::GetFileDirectory(pathSource);
					wPath = tDir.substr(PathProperty::GetModuleDirectory().size()) + wPath.substr(2);
				}
				wPath = PathProperty::GetModuleDirectory() + wPath;
				wPath = PathProperty::GetUnique(wPath);

				bool bReadPath = setReadPath.find(wPath) != setReadPath.end();
				if (bReadPath) { //すでに読み込まれていた
					std::vector<char> newSource;
					int size1 = posInclude;
					int size2 = res.size() - posAfterInclude;
					newSource.resize(size1 + size2);
					memcpy(&newSource[0], &res[0], size1);
					memcpy(&newSource[size1], &res[posAfterInclude], size2);

					res = newSource;
					break;
				}

				std::vector<char> placement;
				auto reader = fileManager->GetFileReader(wPath);
				if (reader == NULL || !reader->Open()) {
					int line = scanner.GetCurrentLine();
					source = res;
					engine_->SetSource(source);

					std::string error;
					error += StringUtility::Format("#Include file is not found[%s].\r\n", wPath.c_str());
					error += StringUtility::Format(u8"(#includeで置換するファイル[%s]が見つかりません)", wPath.c_str());
					_RaiseError(line, error);
				}

				//ファイルを読み込み最後に改行を付加
				int targetBomSize = 0;
				reader->SetFilePointerBegin();

				{
					//読み込み対象がShiftJis
					int newLineSize = bNeedNewLine ? 2 : 0;
					placement.resize(reader->GetFileSize() + newLineSize);
					reader->Read(&placement[0], reader->GetFileSize());
					if (bNeedNewLine)
						memcpy(&placement[reader->GetFileSize() - 1], "\r\n", newLineSize);
				}
				mapLine->AddEntry(wPath,
					scanner.GetCurrentLine(),
					StringUtility::CountCharacter(placement, '\n') + 1);

				{ //置換
					std::vector<char> newSource;
					int size1 = posInclude;
					int size2 = res.size() - posAfterInclude;
					int sizeP = placement.size();
					newSource.resize(size1 + sizeP + size2);
					memcpy(&newSource[0], &res[0], size1);
					memcpy(&newSource[size1], &placement[0], sizeP);
					memcpy(&newSource[size1 + sizeP], &res[posAfterInclude], size2);

					res = newSource;
				}
				setReadPath.insert(wPath);

				if (false) {
					static int countTest = 0;
					static std::string tPath = "";
					if (tPath != pathSource) {
						countTest = 0;
						tPath = pathSource;
					}
					std::string pathTest = PathProperty::GetModuleDirectory() + StringUtility::Format("temp\\script_%s%03d.txt", PathProperty::GetFileName(pathSource).c_str(), countTest);
					File file(pathTest);
					file.CreateDirectory();
					file.Create();
					file.Write(&res[0], res.size());

					std::string strNewLine = "\r\n";
					file.Write(&strNewLine[0], strNewLine.size());
					file.Write(&strNewLine[0], strNewLine.size());

					std::list<ScriptFileLineMap::Entry> listEntry = mapLine->GetEntryList();
					std::list<ScriptFileLineMap::Entry>::iterator itr = listEntry.begin();
					for (; itr != listEntry.end(); itr++) {
						ScriptFileLineMap::Entry entry = (*itr);
						std::string strPath = entry.path_ + "\r\n";
						std::string strLineStart = StringUtility::Format("  lineStart   :%4d\r\n", entry.lineStart_);
						std::string strLineEnd = StringUtility::Format("  lineEnd     :%4d\r\n", entry.lineEnd_);
						std::string strLineStartOrg = StringUtility::Format("  lineStartOrg:%4d\r\n", entry.lineStartOriginal_);
						std::string strLineEndOrg = StringUtility::Format("  lineEndOrg  :%4d\r\n", entry.lineEndOriginal_);

						file.Write(&strPath[0], strPath.size());
						file.Write(&strLineStart[0], strLineStart.size());
						file.Write(&strLineEnd[0], strLineEnd.size());
						file.Write(&strLineStartOrg[0], strLineStartOrg.size());
						file.Write(&strLineEndOrg[0], strLineEndOrg.size());
						file.Write(&strNewLine[0], strNewLine.size());
						
					}

					countTest++;
				}

				break;
			}
		}
	}

	res.push_back(0);

	return res;
}
bool ScriptClientBase::_CreateEngine()
{
	if (machine_ != NULL)
		delete machine_;
	machine_ = NULL;

	std::vector<char>& source = engine_->GetSource();

	auto engine = std::make_unique<script_engine>(&typeManagerDefault_, source, func_.size(), &func_[0]);
	engine_->SetEngine(engine);
	return true;
}
bool ScriptClientBase::SetSourceFromFile(std::string path)
{
	path = PathProperty::GetUnique(path);
	if (cache_ != NULL && cache_->IsExists(path)) {
		engine_ = cache_->GetCache(path);
		return true;
	}

	engine_->SetPath(path);
	auto reader = FileManager::GetBase()->GetFileReader(path);
	if (reader == NULL)
		throw std::runtime_error(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());
	if (!reader->Open())
		throw std::runtime_error(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());

	int size = reader->GetFileSize();
	std::vector<char> source;
	source.resize(size);
	reader->Read(&source[0], size);
	this->SetSource(source);
	return true;
}
void ScriptClientBase::SetSource(std::string source)
{
	std::vector<char> vect;
	vect.resize(source.size());
	memcpy(&vect[0], &source[0], source.size());
	this->SetSource(vect);
}
void ScriptClientBase::SetSource(std::vector<char>& source)
{
	engine_->SetSource(source);
	std::shared_ptr<ScriptFileLineMap> mapLine = engine_->GetScriptFileLineMap();
	mapLine->AddEntry(engine_->GetPath(), 1, StringUtility::CountCharacter(source, '\n') + 1);
}
void ScriptClientBase::Compile()
{
	if (engine_->GetEngine() == NULL) {
		std::vector<char> source = _Include(engine_->GetSource());
		engine_->SetSource(source);

		_CreateEngine();
		if (engine_->GetEngine()->get_error()) {
			bError_ = true;
			_RaiseErrorFromEngine();
		}
		if (cache_ != NULL && engine_->GetPath().size() != 0) {
			cache_->AddCache(engine_->GetPath(), engine_);
		}
	}

	if (machine_ != NULL)
		delete machine_;
	machine_ = new script_machine(engine_->GetEngine());
	if (machine_->get_error()) {
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	machine_->data = this;
}

bool ScriptClientBase::Run()
{
	if (bError_)
		return false;
	machine_->run();
	if (machine_->get_error()) {
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	return true;
}

bool ScriptClientBase::Run(std::string target)
{
	if (bError_)
		return false;
	if (!machine_->has_event(target)) {
		std::string error;
		error += StringUtility::Format("@ not found[%s]", target.c_str());
		error += StringUtility::Format(u8"(イベントが存在しません[%s])", target.c_str());
		_RaiseError(0, error);
	}

	Run();
	machine_->call(target);
	if (machine_->get_error()) {
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	return true;
}
bool ScriptClientBase::IsEventExists(std::string name)
{
	if (bError_)
		return false;
	return machine_->has_event(name);
}
int ScriptClientBase::GetThreadCount()
{
	if (machine_ == NULL)
		return 0;
	int res = machine_->get_thread_count();
	return res;
}
void ScriptClientBase::SetArgumentValue(value v, int index)
{
	if (listValueArg_.size() <= index) {
		listValueArg_.resize(index + 1);
	}
	listValueArg_[index] = v;
}
value ScriptClientBase::CreateRealValue(long double r)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	value res(typeManager->get_real_type(), (long double)r);
	return res;
}
value ScriptClientBase::CreateBooleanValue(bool b)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	value res(typeManager->get_boolean_type(), b);
	return res;
}
value ScriptClientBase::CreateStringValue(std::string s)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	std::string wstr = s;
	value res(typeManager->get_string_type(), wstr);
	return res;
}
value ScriptClientBase::CreateRealArrayValue(std::vector<long double>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	value res(typeManager->get_string_type(), std::string());
	for (int iData = 0; iData < list.size(); iData++) {
		value data = CreateRealValue(list[iData]);
		res.append(typeManager->get_array_type(typeManager->get_real_type()), data);
	}

	return res;
}
value ScriptClientBase::CreateStringArrayValue(std::vector<std::string>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	value res(typeManager->get_string_type(), std::string());
	for (int iData = 0; iData < list.size(); iData++) {
		value data = CreateStringValue(list[iData]);
		res.append(typeManager->get_array_type(typeManager->get_string_type()), data);
	}

	return res;
}
value ScriptClientBase::CreateValueArrayValue(std::vector<value>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	value res(typeManager->get_string_type(), std::string());
	for (int iData = 0; iData < list.size(); iData++) {
		value data = list[iData];
		res.append(typeManager->get_array_type(typeManager->get_real_type()), data);
	}
	return res;
}
bool ScriptClientBase::IsRealValue(value& v)
{
	if (bError_)
		return false;
	if (!v.has_data())
		return false;

	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	return v.get_type() == typeManager->get_real_type();
}
bool ScriptClientBase::IsBooleanValue(value& v)
{
	if (bError_)
		return false;
	if (!v.has_data())
		return false;

	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	return v.get_type() == typeManager->get_boolean_type();
}
bool ScriptClientBase::IsStringValue(value& v)
{
	if (bError_)
		return false;
	if (!v.has_data())
		return false;

	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	return v.get_type() == typeManager->get_string_type();
}
bool ScriptClientBase::IsRealArrayValue(value& v)
{
	if (bError_)
		return false;
	if (!v.has_data())
		return false;

	script_type_manager* typeManager = GetEngine()->GetEngine()->get_type_manager();
	return v.get_type() == typeManager->get_array_type(typeManager->get_real_type());
}
void ScriptClientBase::CheckRunInMainThread()
{
	if (mainThreadID_ == 0)
		return;
	if (mainThreadID_ != SDL_ThreadID()) {
		std::string error;
		error += u8"This function can call in main thread only.\r\n";
		error += u8"(メインスレッド以外では実行できません。)";
		machine_->raise_error(error);
	}
}
std::string ScriptClientBase::_ExtendPath(std::string path)
{
	int line = machine_->get_current_line();
	std::string pathScript = GetEngine()->GetScriptFileLineMap()->GetPath(line);

	path = StringUtility::ReplaceAll(path, "\\", "/");
	path = StringUtility::ReplaceAll(path, "./", pathScript);

	return path;
}

//共通関数：スクリプト引数結果
value ScriptClientBase::Func_GetScriptArgument(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	int index = (int)argv[0].as_real();
	if (index < 0 || index >= script->listValueArg_.size()) {
		std::string error;
		error += "Invalid script argument index.\r\n";
		error += u8"(スクリプト引数のインデックスが不正です。)";
		throw std::runtime_error(error);
	}
	return script->listValueArg_[index];
}
value ScriptClientBase::Func_GetScriptArgumentCount(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	int res = script->listValueArg_.size();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value ScriptClientBase::Func_SetScriptResult(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	script->valueRes_ = argv[0];
	return value();
}

//組み込み関数：数学系
value ScriptClientBase::Func_Min(script_machine* machine, int argc, value const* argv)
{
	long double v1 = argv[0].as_real();
	long double v2 = argv[1].as_real();
	long double res = v1 <= v2 ? v1 : v2;
	return value(machine->get_engine()->get_real_type(), res);
}
value ScriptClientBase::Func_Max(script_machine* machine, int argc, value const* argv)
{
	long double v1 = argv[0].as_real();
	long double v2 = argv[1].as_real();
	long double res = v1 >= v2 ? v1 : v2;
	return value(machine->get_engine()->get_real_type(), res);
}
value ScriptClientBase::Func_Log(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)log(argv[0].as_real()));
}
value ScriptClientBase::Func_Log10(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)log10(argv[0].as_real()));
}
value ScriptClientBase::Func_Cos(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)cos(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Sin(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)sin(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Tan(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)tan(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Acos(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)Math::RadianToDegree(acos(argv[0].as_real())));
}
value ScriptClientBase::Func_Asin(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)Math::RadianToDegree(asin(argv[0].as_real())));
}
value ScriptClientBase::Func_Atan(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)Math::RadianToDegree(atan(argv[0].as_real())));
}
value ScriptClientBase::Func_Atan2(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_real_type(), (long double)Math::RadianToDegree(atan2(argv[0].as_real(), argv[1].as_real())));
}
value ScriptClientBase::Func_Rand(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	script->CheckRunInMainThread();

	double min = argv[0].as_real();
	double max = argv[1].as_real();
	long double res = script->mt_->GetReal(min, max);
	return value(machine->get_engine()->get_real_type(), res);
}

//組み込み関数：文字列操作
value ScriptClientBase::Func_ToString(script_machine* machine, int argc, value const* argv)
{
	return value(machine->get_engine()->get_string_type(), argv[0].as_string());
}
value ScriptClientBase::Func_IntToString(script_machine* machine, int argc, value const* argv)
{
	std::string res = StringUtility::Format("%d", (int)argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_ItoA(script_machine* machine, int argc, value const* argv)
{
	std::string res = StringUtility::Format("%d", (int)argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_RtoA(script_machine* machine, int argc, value const* argv)
{
	std::string res = StringUtility::Format("%f", argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_RtoS(script_machine* machine, int argc, value const* argv)
{
	std::string res = "";
	std::string fmtV = argv[0].as_string();
	double num = argv[1].as_real();

	try {
		bool bF = false;
		int countIS = 0;
		int countI0 = 0;
		int countF = 0;

		for (int iCh = 0; iCh < fmtV.size(); iCh++) {
			char ch = fmtV[iCh];

			if (ch == '#')
				countIS++;
			else if (ch == '.' && bF)
				throw false;
			else if (ch == '.')
				bF = true;
			else if (ch == '0') {
				if (bF)
					countF++;
				else
					countI0++;
			}
		}

		std::string fmt = "";
		if (countI0 > 0 && countF >= 0) {
			fmt += "%0";
			fmt += StringUtility::Format("%d", countI0);
			fmt += ".";
			fmt += StringUtility::Format("%d", countF);
			fmt += "f";
		} else if (countIS > 0 && countF >= 0) {
			fmt += "%";
			fmt += StringUtility::Format("%d", countIS);
			fmt += ".";
			fmt += StringUtility::Format("%d", countF);
			fmt += "f";
		}

		if (fmt.size() > 0) {
			res = StringUtility::Format((char*)fmt.c_str(), num);
		}
	} catch (...) {
		res = "error format";
	}

	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_VtoS(script_machine* machine, int argc, value const* argv)
{
	std::string res = "";
	std::string fmtV = argv[0].as_string();

	try {
		int countIS = 0;
		int countI0 = 0;
		int countF = 0;

		int advance = 0; //0:-, 1:0, 2:num, 3:[d,s,f], 4:., 5:num
		for (int iCh = 0; iCh < fmtV.size(); iCh++) {
			char ch = fmtV[iCh];
			if (advance == 0 && ch == '-')
				advance = 1;
			else if ((advance == 0 || advance == 1 || advance == 2) && (ch >= '0' && ch <= '9'))
				advance = 2;
			else if (advance == 2 && (ch == 'd' || ch == 's' || ch == 'f'))
				advance = 4;
			else if (advance == 2 && ('.'))
				advance = 5;
			else if (advance == 4 && ('.'))
				advance = 5;
			else if (advance == 5 && (ch >= '0' && ch <= '9'))
				advance = 5;
			else if (advance == 5 && (ch == 'd' || ch == 's' || ch == 'f'))
				advance = 6;
			else
				throw false;
		}

		fmtV = std::string("%") + fmtV;
		char* fmt = (char*)fmtV.c_str();
		if (strstr(fmt, "d") != NULL)
			res = StringUtility::Format(fmt, (int)argv[1].as_real());
		else if (strstr(fmt, "f") != NULL)
			res = StringUtility::Format(fmt, argv[1].as_real());
		else if (strstr(fmt, "s") != NULL)
			res = StringUtility::Format(fmt, argv[1].as_string().c_str());
	} catch (...) {
		res = "error format";
	}

	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_AtoI(script_machine* machine, int argc, value const* argv)
{
	std::string str = argv[0].as_string();
	int num = StringUtility::ToInteger(str);
	return value(machine->get_engine()->get_real_type(), (long double)num);
}
value ScriptClientBase::Func_AtoR(script_machine* machine, int argc, value const* argv)
{
	std::string str = argv[0].as_string();
	double num = StringUtility::ToDouble(str);
	return value(machine->get_engine()->get_real_type(), (long double)num);
}
value ScriptClientBase::Func_TrimString(script_machine* machine, int argc, value const* argv)
{
	std::string str = argv[0].as_string();
	std::string res = StringUtility::Trim(str);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_SplitString(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string str = argv[0].as_string();
	std::string delim = argv[1].as_string();
	std::vector<std::string> list = StringUtility::Split(str, delim);

	gstd::value res = script->CreateStringArrayValue(list);
	return res;
}

//共通関数：パス関連
value ScriptClientBase::Func_GetModuleDirectory(script_machine* machine, int argc, value const* argv)
{
	std::string res = PathProperty::GetModuleDirectory();
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetMainScriptDirectory(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string path = script->GetEngine()->GetPath();
	std::string res = PathProperty::GetFileDirectory(path);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetCurrentScriptDirectory(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	int line = machine->get_current_line();
	std::string path = script->GetEngine()->GetScriptFileLineMap()->GetPath(line);
	std::string res = PathProperty::GetFileDirectory(path);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetFileDirectory(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string path = argv[0].as_string();
	// path = StringUtility::ReplaceAll(path, "/", "\\");
	std::string res = PathProperty::GetFileDirectory(path);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetFilePathList(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string path = argv[0].as_string();
	std::string dir = PathProperty::GetFileDirectory(path);
	std::vector<std::string> listDir = File::GetFilePathList(dir);
	gstd::value res = script->CreateStringArrayValue(listDir);
	return res;
}
value ScriptClientBase::Func_GetDirectoryList(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string path = argv[0].as_string();
	std::string dir = PathProperty::GetFileDirectory(path);
	std::vector<std::string> listDir = File::GetDirectoryPathList(dir);
	gstd::value res = script->CreateStringArrayValue(listDir);
	return res;
}

//共通関数：時刻関連
value ScriptClientBase::Func_GetCurrentDateTimeS(script_machine* machine, int argc, value const* argv)
{
	time_t ltime;
	time(&ltime);
	auto date = localtime(&ltime);

	std::string strDateTime = StringUtility::Format(
		"%04d%02d%02d%02d%02d%02d",
		date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
		date->tm_hour, date->tm_min, date->tm_sec);
	std::string res = strDateTime;
	return value(machine->get_engine()->get_string_type(), res);
}

//共通関数：デバッグ関連
value ScriptClientBase::Func_WriteLog(script_machine* machine, int argc, value const* argv)
{
	std::string msg = argv[0].as_string();
	Logger::WriteTop(msg);
	return value();
}
value ScriptClientBase::Func_RaiseError(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::string msg = argv[0].as_string();
	script->RaiseError(msg);

	return value();
}

//共通関数：共通データ
value ScriptClientBase::Func_SetDefaultCommonDataArea(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string name = argv[0].as_string();
	commonDataManager->SetDefaultAreaName(name);
	return value();
}
value ScriptClientBase::Func_SetCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = argv[0].as_string();
	value val = argv[1];
	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->SetValue(key, val);
	return value();
}
value ScriptClientBase::Func_GetCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = argv[0].as_string();
	value dv = argv[1];
	if (!commonDataManager->IsExists(area))
		return dv;
	auto data = commonDataManager->GetData(area);
	if (!data->IsExists(key))
		return dv;
	return data->GetValue(key);
}
value ScriptClientBase::Func_ClearCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->Clear();
	return value();
}
value ScriptClientBase::Func_DeleteCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = argv[0].as_string();
	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->DeleteValue(key);
	return value();
}
value ScriptClientBase::Func_SetAreaCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	std::string key = argv[1].as_string();
	value val = argv[2];

	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->SetValue(key, val);
	return value();
}
value ScriptClientBase::Func_GetAreaCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	std::string key = argv[1].as_string();
	value dv = argv[2];
	if (!commonDataManager->IsExists(area))
		return dv;
	auto data = commonDataManager->GetData(area);
	if (!data->IsExists(key))
		return dv;
	return data->GetValue(key);
}
value ScriptClientBase::Func_ClearAreaCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->Clear();
	return value();
}
value ScriptClientBase::Func_DeleteAreaCommonData(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	std::string key = argv[1].as_string();
	if (!commonDataManager->IsExists(area))
		return value();
	auto data = commonDataManager->GetData(area);
	data->DeleteValue(key);
	return value();
}
value ScriptClientBase::Func_CreateCommonDataArea(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	commonDataManager->CreateArea(area);
	return value();
}
value ScriptClientBase::Func_CopyCommonDataArea(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string areaDest = argv[0].as_string();
	std::string areaSrc = argv[1].as_string();
	if (commonDataManager->IsExists(areaSrc))
		commonDataManager->CopyArea(areaDest, areaSrc);
	return value();
}
value ScriptClientBase::Func_IsCommonDataAreaExists(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::string area = argv[0].as_string();
	bool res = commonDataManager->IsExists(area);
	return value(machine->get_engine()->get_boolean_type(), res);
}
value ScriptClientBase::Func_GetCommonDataAreaKeyList(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::vector<std::string> listKey = commonDataManager->GetKeyList();
	gstd::value res = script->CreateStringArrayValue(listKey);
	return res;
}
value ScriptClientBase::Func_GetCommonDataValueKeyList(script_machine* machine, int argc, value const* argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	auto& commonDataManager = script->GetCommonDataManager();

	std::vector<std::string> listKey;
	std::string area = argv[0].as_string();
	if (commonDataManager->IsExists(area)) {
		auto data = commonDataManager->GetData(area);
		listKey = data->GetKeyList();
	}
	gstd::value res = script->CreateStringArrayValue(listKey);
	return res;
}

/**********************************************************
//ScriptFileLineMap
**********************************************************/
ScriptFileLineMap::ScriptFileLineMap()
{
}
ScriptFileLineMap::~ScriptFileLineMap()
{
}
void ScriptFileLineMap::AddEntry(std::string path, int lineAdd, int lineCount)
{
	Entry entryNew;
	entryNew.path_ = path;
	entryNew.lineStartOriginal_ = 1;
	entryNew.lineEndOriginal_ = lineCount;
	entryNew.lineStart_ = lineAdd;
	entryNew.lineEnd_ = entryNew.lineStart_ + lineCount - 1;
	if (listEntry_.size() == 0) {
		listEntry_.push_back(entryNew);
		return;
	}

	std::list<Entry>::iterator itrInsert;
	for (itrInsert = listEntry_.begin(); itrInsert != listEntry_.end(); itrInsert++) {
		auto& entryDivide = *itrInsert;
		if (lineAdd >= entryDivide.lineStart_ && lineAdd <= entryDivide.lineEnd_)
		{
			// once we enter here we should break, so we have no reason to check if itrInsert is valid

			if (entryDivide.lineStart_ == lineAdd) {
				entryDivide.lineStartOriginal_++;
				listEntry_.insert(itrInsert, entryNew);
			}
			else if (entryDivide.lineEnd_ == lineAdd) {
				entryDivide.lineEnd_--;
				entryDivide.lineEndOriginal_--;

				listEntry_.insert(itrInsert, entryNew);
				itrInsert++;
			}
			else {
				Entry entryNew2 = entryDivide;
				entryDivide.lineEnd_ = lineAdd - 1;
				entryDivide.lineEndOriginal_ = lineAdd - entryDivide.lineStart_;

				entryNew2.lineStartOriginal_ = entryDivide.lineEndOriginal_ + 2;
				entryNew2.lineStart_ = entryNew.lineEnd_ + 1;
				entryNew2.lineEnd_ += lineCount - 1;

				if (itrInsert != listEntry_.end())
					itrInsert++;
				listEntry_.insert(itrInsert, entryNew);
				listEntry_.insert(itrInsert, entryNew2);
			}

			for (; itrInsert != listEntry_.end(); itrInsert++) {
				Entry& entry = *itrInsert;
				entry.lineStart_ += lineCount - 1;
				entry.lineEnd_ += lineCount - 1;
			}

			break;
		}
	}
}

ScriptFileLineMap::Entry ScriptFileLineMap::GetEntry(int line)
{
	Entry res;
	std::list<Entry>::iterator itrInsert;
	for (itrInsert = listEntry_.begin(); itrInsert != listEntry_.end(); itrInsert++) {
		res = *itrInsert;
		if (line >= res.lineStart_ && line <= res.lineEnd_)
			break;
	}
	return res;
}
std::string ScriptFileLineMap::GetPath(int line)
{
	Entry entry = GetEntry(line);
	return entry.path_;
}

/**********************************************************
//ScriptCommonDataManager
**********************************************************/
ScriptCommonDataManager::ScriptCommonDataManager()
{
	nameAreaDefailt_ = "";
	CreateArea("");
}
ScriptCommonDataManager::~ScriptCommonDataManager()
{
}
void ScriptCommonDataManager::Clear()
{
	mapData_.clear();
}
bool ScriptCommonDataManager::IsExists(std::string name)
{
	return mapData_.find(name) != mapData_.end();
}
void ScriptCommonDataManager::CreateArea(std::string name)
{
	if (IsExists(name))
		return;
	mapData_[name] = std::make_unique<ScriptCommonData>();
}
void ScriptCommonDataManager::CopyArea(std::string nameDest, std::string nameSrc)
{
	auto& dataSrc = mapData_[nameSrc];
	auto dataDest = std::make_shared<ScriptCommonData>();
	dataDest->Copy(dataSrc);
	mapData_[nameDest] = dataDest;
}
std::shared_ptr<ScriptCommonData> ScriptCommonDataManager::GetData(std::string name)
{
	if (!IsExists(name))
		return std::shared_ptr<ScriptCommonData>();
	return mapData_[name];
}
void ScriptCommonDataManager::SetData(std::string name, std::shared_ptr<ScriptCommonData> commonData)
{
	mapData_[name] = commonData;
}
std::vector<std::string> ScriptCommonDataManager::GetKeyList()
{
	std::vector<std::string> res;
	auto itrValue = mapData_.begin(), end = mapData_.end();
	for (; itrValue != end; itrValue++) {
		std::string key = itrValue->first;
		res.push_back(key);
	}
	return res;
}
/**********************************************************
//ScriptCommonData
**********************************************************/
ScriptCommonData::ScriptCommonData()
{
}
ScriptCommonData::~ScriptCommonData()
{
}
void ScriptCommonData::Clear()
{
	mapValue_.clear();
}
bool ScriptCommonData::IsExists(std::string name)
{
	return mapValue_.find(name) != mapValue_.end();
}
gstd::value ScriptCommonData::GetValue(std::string name)
{
	if (!IsExists(name))
		return value();
	return mapValue_[name];
}
void ScriptCommonData::SetValue(std::string name, gstd::value v)
{
	mapValue_[name] = v;
}
void ScriptCommonData::DeleteValue(std::string name)
{
	mapValue_.erase(name);
}
void ScriptCommonData::Copy(std::shared_ptr<ScriptCommonData> dataSrc)
{
	mapValue_.clear();
	std::vector<std::string> listSrcKey = dataSrc->GetKeyList();
	for (int iKey = 0; iKey < listSrcKey.size(); iKey++) {
		std::string key = listSrcKey[iKey];
		gstd::value vSrc = dataSrc->GetValue(key);
		gstd::value vDest = vSrc;
		vDest.unique();
		mapValue_[key] = vDest;
	}
}
std::vector<std::string> ScriptCommonData::GetKeyList()
{
	std::vector<std::string> res;
	std::map<std::string, gstd::value>::iterator itrValue;
	for (itrValue = mapValue_.begin(); itrValue != mapValue_.end(); itrValue++) {
		std::string key = itrValue->first;
		res.push_back(key);
	}
	return res;
}
void ScriptCommonData::ReadRecord(gstd::RecordBuffer& record)
{
	mapValue_.clear();

	std::vector<std::string> listKey = record.GetKeyList();
	for (int iKey = 0; iKey < listKey.size(); iKey++) {
		std::string key = listKey[iKey];
		std::string keyValSize = StringUtility::Format("%s_size", key.c_str());
		if (!record.IsExists(keyValSize)) //サイズ自身がキー登録されている
			continue;
		int valSize = record.GetRecordAsInteger(keyValSize);

		gstd::ByteBuffer buffer;
		buffer.SetSize(valSize);
		record.GetRecord(key, buffer.GetPointer(), valSize);
		gstd::value comVal = _ReadRecord(buffer);
		mapValue_[key] = comVal;
	}
}
gstd::value ScriptCommonData::_ReadRecord(gstd::ByteBuffer& buffer)
{
	script_type_manager* scriptTypeManager = ScriptClientBase::GetDefaultScriptTypeManager();
	gstd::value res;
	type_data::type_kind kind = (type_data::type_kind)buffer.ReadInteger();

	if (kind == type_data::tk_real) {
		long double data = buffer.ReadDouble();
		res = gstd::value(scriptTypeManager->get_real_type(), data);
	} else if (kind == type_data::tk_char) {
		char data;
		buffer.Read(&data, sizeof(char));
		res = gstd::value(scriptTypeManager->get_char_type(), data);
	} else if (kind == type_data::tk_boolean) {
		bool data = buffer.ReadBoolean();
		res = gstd::value(scriptTypeManager->get_boolean_type(), data);
	} else if (kind == type_data::tk_array) {
		int arrayLength = buffer.ReadInteger();
		value v;
		for (int iArray = 0; iArray < arrayLength; iArray++) {
			value arrayValue = _ReadRecord(buffer);
			v.append(scriptTypeManager->get_array_type(arrayValue.get_type()),
				arrayValue);
		}
		res = v;
	}

	return res;
}
void ScriptCommonData::WriteRecord(gstd::RecordBuffer& record)
{
	std::map<std::string, gstd::value>::iterator itrValue;
	for (itrValue = mapValue_.begin(); itrValue != mapValue_.end(); itrValue++) {
		std::string key = itrValue->first;
		gstd::value comVal = itrValue->second;

		gstd::ByteBuffer buffer;
		_WriteRecord(buffer, comVal);
		std::string keyValSize = StringUtility::Format("%s_size", key.c_str());
		int valSize = buffer.GetSize();
		record.SetRecordAsInteger(keyValSize, valSize);
		record.SetRecord(key, buffer.GetPointer(), valSize);
	}
}
void ScriptCommonData::_WriteRecord(gstd::ByteBuffer& buffer, gstd::value& comValue)
{
	type_data::type_kind kind = comValue.get_type()->get_kind();
	buffer.WriteInteger(kind);
	if (kind == type_data::tk_real) {
		buffer.WriteDouble(comValue.as_real());
	} else if (kind == type_data::tk_char) {
		wchar_t wch = comValue.as_char();
		buffer.Write(&wch, sizeof(wchar_t));
	} else if (kind == type_data::tk_boolean) {
		buffer.WriteBoolean(comValue.as_boolean());
	} else if (kind == type_data::tk_array) {
		int arrayLength = comValue.length_as_array();
		buffer.WriteInteger(arrayLength);

		for (int iArray = 0; iArray < arrayLength; iArray++) {
			value& arrayValue = comValue.index_as_array(iArray);
			_WriteRecord(buffer, arrayValue);
		}
	}
}
