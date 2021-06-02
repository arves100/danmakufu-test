#include "Shader.hpp"
#include "HLSL.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ShaderManager
**********************************************************/
const std::string NAME_DEFAULT_SKINNED_MESH = "__NAME_DEFAULT_SKINNED_MESH__";
ShaderManager* ShaderManager::thisBase_ = nullptr;

ShaderManager::ShaderManager() = default;

ShaderManager::~ShaderManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);

	Clear();
}

bool ShaderManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;

	bool res = true;
	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);

	auto shaderSkinedMesh = std::make_shared<Shader>();
	std::string sourceSkinedMesh = HLSL_DEFAULT_SKINED_MESH;
	shaderSkinedMesh->CreateFromText(sourceSkinedMesh);
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderSkinedMesh);

	return res;
}

void ShaderManager::Clear()
{
	Lock lock(lock_);
	mapShader_.clear();
	mapShaderData_.clear();
}

void ShaderManager::_ReleaseShaderData(std::string name)
{
	Lock lock(lock_);
	if (IsDataExists(name)) {
		mapShaderData_.erase(name);
		Logger::WriteTop(StringUtility::Format(L"ShaderManager：Shaderを解放しました(Shader Released)[%s]", name.c_str()));
	}
}

bool ShaderManager::_CreateFromFile(std::string path)
{
	lastError_ = L"";
	if (IsDataExists(path)) {
		return true;
	}

	auto wpath = PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8));
	auto path = StringUtility::ConvertWideToMulti(wpath);
	
	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(wpath);
	if (reader == NULL || !reader->Open()) {
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s", path.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
		return false;
	}

	int size = reader->GetFileSize();
	ByteBuffer buf;
	buf.SetSize(size);
	reader->Read(buf.GetPointer(), size);

	std::string source;
	source.resize(size);
	memcpy(&source[0], buf.GetPointer(), size);

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ID3DXBuffer* pErr = NULL;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr);

	bool res = true;
	if (FAILED(hr)) {
		res = false;
		std::wstring err = L"";
		if (pErr != NULL) {
			char* cText = (char*)pErr->GetBufferPointer();
			err = StringUtility::ConvertMultiToWide(cText);
		}
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s\r\n[%s]", path.c_str(), err.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
	} else {
		std::wstring log = StringUtility::Format(L"Shader読み込み(Shader Load Success)：\r\n%s", path.c_str());
		Logger::WriteTop(log);

		mapShaderData_[path] = data;
		data->manager_ = this;
		data->name_ = path;
	}
	return res;
}

bool ShaderManager::_CreateFromText(std::string& source)
{
	lastError_ = L"";
	std::string id = _GetTextSourceID(source);
	if (IsDataExists(id)) {
		return true;
	}

	bool res = true;
	DirectGraphics* graphics = DirectGraphics::GetBase();

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());
	ID3DXBuffer* pErr = NULL;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr);

	std::string tStr = StringUtility::Slice(source, 128);
	if (FAILED(hr)) {
		res = false;
		char* err = "";
		if (pErr != NULL)
			err = (char*)pErr->GetBufferPointer();
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Load Shader Failed)：\r\n%s\r\n[%s]", tStr.c_str(), err);
		Logger::WriteTop(log);
		lastError_ = log;
	} else {
		std::wstring log = L"Shader読み込み(Load Shader Success)：";
		log += StringUtility::FormatToWide("%s", tStr.c_str());
		Logger::WriteTop(log);

		mapShaderData_[id] = data;
		data->manager_ = this;
		data->name_ = id;
		data->bText_ = true;
	}
	return res;
}

std::string ShaderManager::_GetTextSourceID(std::string& source)
{
	return StringUtility::Slice(source, 64);
}

#if 0
void ShaderManager::ReleaseDxResource()
{
	for (auto& shader : mapShader_)
	{
		shader.second->ReleaseDxResource();
	}
}

void ShaderManager::RestoreDxResource()
{
	for (auto& shader : mapShader_)
	{
		shader.second->RestoreDxResource();
	}
}
#endif

bool ShaderManager::IsDataExists(std::string name)
{
	Lock lock(lock_);
	return mapShaderData_.find(name) != mapShaderData_.end();
}

bool ShaderManager::GetShaderData(std::string name, std::shared_ptr<ShaderData>& shader)
{
	Lock lock(lock_);
	if (mapShaderData_.find(name) != mapShaderData_.end())
	{
		shader = mapShaderData_[name];
		return true;
	}
	
	return false;
}

bool ShaderManager::CreateFromFile(std::string path, std::shared_ptr<Shader>& shader)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8);

	Lock lock(lock_);
	if (mapShader_.find(path) != mapShader_.end()) {
		shader = mapShader_[path];
		return true;
	}

	if (_CreateFromFile(path)) {
		shader = std::make_shared<Shader>();
		shader->data_ = mapShaderData_[path];
		return true;
	}

	return false;
}

bool ShaderManager::CreateFromText(std::string source, std::shared_ptr<Shader>& shader)
{
	Lock lock(lock_);
	const std::string id = _GetTextSourceID(source);
	if (mapShader_.find(id) != mapShader_.end())
	{
		shader = mapShader_[id];
	}

	if (_CreateFromText(source))
	{
		shader = std::make_shared<Shader>();
		shader->data_ = mapShaderData_[id];
		return true;
	}

	return false;
}


void ShaderManager::AddShader(std::string name, std::shared_ptr<Shader>& shader)
{
	Lock lock(lock_);
	mapShader_[name] = shader;
}
void ShaderManager::DeleteShader(std::string name)
{
	Lock lock(lock_);
	mapShader_.erase(name);
}

bool ShaderManager::GetShader(std::string name, std::shared_ptr<Shader>& shader)
{
	Lock lock(lock_);
	if (mapShader_.find(name) != mapShader_.end())
	{
		shader = mapShader_[name];
		return true;
	}

	return false;
}

std::shared_ptr<Shader>& ShaderManager::GetDefaultSkinnedMeshShader()
{
	return mapShader_[NAME_DEFAULT_SKINNED_MESH];
}

std::wstring ShaderManager::GetLastError()
{
	std::wstring res;
	{
		Lock lock(lock_);
		res = lastError_;
	}
	return res;
}

#if 0
/**********************************************************
//ShaderParameter
**********************************************************/
ShaderParameter::ShaderParameter()
{
	type_ = TYPE_UNKNOWN;
	value_ = new ByteBuffer();
}
ShaderParameter::~ShaderParameter()
{
}
void ShaderParameter::SetMatrix(D3DXMATRIX matrix)
{
	type_ = TYPE_MATRIX;
	int size = sizeof(D3DXMATRIX);
	value_->Seek(0);
	value_->Write(&matrix, size);
}
D3DXMATRIX ShaderParameter::GetMatrix()
{
	D3DXMATRIX res = (D3DXMATRIX&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetMatrixArray(std::vector<D3DXMATRIX>& listMatrix)
{
	type_ = TYPE_MATRIX_ARRAY;
	value_->Seek(0);
	for (int iMatrix = 0; iMatrix < listMatrix.size(); iMatrix++) {
		int size = sizeof(D3DMATRIX);
		D3DXMATRIX& matrix = listMatrix[iMatrix];
		value_->Write(&matrix.m, size);
	}
}
std::vector<D3DXMATRIX> ShaderParameter::GetMatrixArray()
{
	std::vector<D3DXMATRIX> res;
	int count = value_->GetSize() / sizeof(D3DMATRIX);
	res.resize(count);

	value_->Seek(0);
	for (int iMatrix = 0; iMatrix < res.size(); iMatrix++) {
		value_->Read(&res[iMatrix].m, sizeof(D3DMATRIX));
	}

	return res;
}
void ShaderParameter::SetVector(D3DXVECTOR4 vector)
{
	type_ = TYPE_VECTOR;
	int size = sizeof(D3DXVECTOR4);
	value_->Seek(0);
	value_->Write(&vector, size);
}
D3DXVECTOR4 ShaderParameter::GetVector()
{
	D3DXVECTOR4 res = (D3DXVECTOR4&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetFloat(float value)
{
	type_ = TYPE_FLOAT;
	int size = sizeof(float);
	value_->Seek(0);
	value_->Write(&value, size);
}
float ShaderParameter::GetFloat()
{
	float res = (float&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetFloatArray(std::vector<float>& values)
{
	type_ = TYPE_FLOAT_ARRAY;
	int size = sizeof(float) * values.size();
	value_->Seek(0);
	value_->Write(&values[0], size);
}
std::vector<float> ShaderParameter::GetFloatArray()
{
	std::vector<float> res;
	int count = value_->GetSize() / sizeof(float);
	res.resize(count);

	value_->Seek(0);
	value_->Read(&res[0], value_->GetSize());
	return res;
}
void ShaderParameter::SetTexture(gstd::ref_count_ptr<Texture> texture)
{
	type_ = TYPE_TEXTURE;
	texture_ = texture;
}
gstd::ref_count_ptr<Texture> ShaderParameter::GetTexture()
{
	return texture_;
}
#endif

/**********************************************************
//Shader
**********************************************************/
Shader::Shader() = default;

Shader::Shader(Shader* shader)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	data_ = shader->data_;
}

Shader::~Shader()
{
	Release();
}

void Shader::Release()
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr) {
		ShaderManager* manager = ShaderManager::GetBase();
		if (manager != nullptr && manager->IsDataExists(data_->Name))
		{
			auto countRef = data_.use_count();
			//自身とTextureManager内の数だけになったら削除
			if (countRef == 2)
			{
				manager->_ReleaseShaderData(data_->Name);
			}
		}
		
		data_.reset();
	}
}

void Shader::Submit()
{
	bgfx::submit(0, data_->Program);
}

bool Shader::CreateFromFile(std::string path)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8);

	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();
	
	ShaderManager* manager = ShaderManager::GetBase();

	std::shared_ptr<Shader> shader;
	if (manager->CreateFromFile(path, shader))
	{
		data_ = shader->data_;
	}

	return data_ != nullptr;
}

bool Shader::CreateFromText(std::string& source)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();

	ShaderManager* manager = ShaderManager::GetBase();
	std::shared_ptr<Shader> shader;
	
	if (manager->CreateFromText(source, shader))
	{
		data_ = shader->data_;
	}

	return data_ != nullptr;
}

#if 0
bool Shader::_SetupParameter()
{
	ID3DXEffect* effect = GetEffect();
	if (effect == NULL)
		return false;
	HRESULT hr = effect->SetTechnique(technique_.c_str());
	if (FAILED(hr))
		return false;

	std::map<std::string, gstd::ref_count_ptr<ShaderParameter>>::iterator itrParam;
	for (itrParam = mapParam_.begin(); itrParam != mapParam_.end(); itrParam++) {
		std::string name = itrParam->first;
		gstd::ref_count_ptr<ShaderParameter> param = itrParam->second;
		int type = param->GetType();
		switch (type) {
		case ShaderParameter::TYPE_MATRIX: {
			D3DXMATRIX matrix = param->GetMatrix();
			hr = effect->SetMatrix(name.c_str(), &matrix);
			break;
		}
		case ShaderParameter::TYPE_MATRIX_ARRAY: {
			std::vector<D3DXMATRIX> matrixArray = param->GetMatrixArray();
			hr = effect->SetMatrixArray(name.c_str(), &matrixArray[0], matrixArray.size());
			break;
		}
		case ShaderParameter::TYPE_VECTOR: {
			D3DXVECTOR4 vect = param->GetVector();
			hr = effect->SetVector(name.c_str(), &vect);
			break;
		}
		case ShaderParameter::TYPE_FLOAT: {
			float value = param->GetFloat();
			hr = effect->SetFloat(name.c_str(), value);
			break;
		}
		case ShaderParameter::TYPE_FLOAT_ARRAY: {
			std::vector<float> value = param->GetFloatArray();
			hr = effect->SetFloatArray(name.c_str(), &value[0], value.size());
			break;
		}
		case ShaderParameter::TYPE_TEXTURE: {
			gstd::ref_count_ptr<Texture> texture = param->GetTexture();
			IDirect3DTexture9* pTex = texture->GetD3DTexture();
			hr = effect->SetTexture(name.c_str(), pTex);
			break;
		}
		}
		// if (FAILED(hr))
		// 	return false;
	}
	return true;
}

ShaderParameter& Shader::_GetParameter(std::string name, bool bCreate)
{
	bool bFind = mapParam_.find(name) != mapParam_.end();
	if (!bFind && !bCreate)
		return NULL;

	gstd::ref_count_ptr<ShaderParameter> res = NULL;
	if (!bFind) {
		res = new ShaderParameter();
		mapParam_[name] = res;
	} else {
		res = mapParam_[name];
	}

	return res;
}
#endif

#if 0
bool Shader::SetMatrix(std::string name, glm::mat4 matrix)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == NULL)
	// 	return false;
	// effect->SetMatrix(name.c_str(), &matrix);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrix(matrix);

	return true;
}

bool Shader::SetMatrixArray(std::string name, std::vector<D3DXMATRIX>& matrix)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == NULL)
	// 	return false;
	// effect->SetMatrixArray(name.c_str(), &matrix[0], matrix.size());

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrixArray(matrix);

	return true;
}
bool Shader::SetVector(std::string name, D3DXVECTOR4 vector)
{
	// ID3DXEffect* effect = GetEffect();
	// if (effect == NULL)
	// 	return false;
	// effect->SetVector(name.c_str(), &vector);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetVector(vector);
	return true;
}
bool Shader::SetFloat(std::string name, float value)
{
	//ID3DXEffect* effect = GetEffect();
	//if (effect == NULL)
	// 	return false;
	//effect->SetFloat(name.c_str(), value);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloat(value);
	return true;
}
bool Shader::SetFloatArray(std::string name, std::vector<float>& values)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloatArray(values);
	return true;
}
bool Shader::SetTexture(std::string name, gstd::ref_count_ptr<Texture> texture)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetTexture(texture);
	return true;
}
#endif
