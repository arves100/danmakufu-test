#ifndef __DIRECTX_SHADER__
#define __DIRECTX_SHADER__

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"
#include "Texture.hpp"

namespace directx {

// http://msdn.microsoft.com/ja-jp/library/bb944006(v=vs.85).aspx
// http://msdn.microsoft.com/ja-jp/library/bb509647(v=vs.85).aspx

class ShaderManager;
class Shader;
class ShaderData;
/**********************************************************
//ShaderData
**********************************************************/
class ShaderData {
	friend Shader;
	friend ShaderManager;

public:
	ShaderData();
	virtual ~ShaderData();
	std::wstring GetName() { return name_; }

private:
	ShaderManager* manager_;
	ID3DXEffect* effect_;
	std::wstring name_;
	volatile bool bLoad_;
	volatile bool bText_;
};

/**********************************************************
//ShaderManager
**********************************************************/
class ShaderManager : public DirectGraphicsListener {
	friend Shader;
	friend ShaderData;

public:
	ShaderManager();
	virtual ~ShaderManager();
	static ShaderManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }
	void Clear();

	virtual void ReleaseDirectGraphics() { ReleaseDxResource(); }
	virtual void RestoreDirectGraphics() { RestoreDxResource(); }
	void ReleaseDxResource();
	void RestoreDxResource();

	virtual bool IsDataExists(std::string name);
	std::shared_ptr<ShaderData> GetShaderData(std::string name);
	std::shared_ptr<Shader> CreateFromFile(std::string path); //読み込みます。ShaderDataは保持しますが、Shaderは保持しません。
	std::shared_ptr<Shader> CreateFromText(std::string source); //読み込みます。ShaderDataは保持しますが、Shaderは保持しません。
	std::shared_ptr<Shader> CreateFromFileInLoadThread(std::string path);
	virtual void CallFromLoadThread(std::unique_ptr<gstd::FileManager::LoadThreadEvent>& event);

	void AddShader(std::string name, std::shared_ptr<Shader> shader);
	void DeleteShader(std::string name);
	std::shared_ptr<Shader> GetShader(std::string name);
	std::shared_ptr<Shader> GetDefaultSkinnedMeshShader();

	void CheckExecutingShaderZero();
	std::string GetLastError();

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<Shader>> mapShader_;
	std::map<std::string, std::shared_ptr<ShaderData>> mapShaderData_;

	std::list<Shader*> listExecuteShader_;
	std::string lastError_;

	void _ReleaseShaderData(std::string name);
	bool _CreateFromFile(std::string path);
	bool _CreateFromText(std::string& source);
	void _BeginShader(Shader* shader, int pass);
	void _EndShader(Shader* shader);
	static std::string _GetTextSourceID(std::string& source);

private:
	static ShaderManager* thisBase_;
};

/**********************************************************
//ShaderParameter
**********************************************************/
class ShaderParameter {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_MATRIX,
		TYPE_MATRIX_ARRAY,
		TYPE_VECTOR,
		TYPE_FLOAT,
		TYPE_FLOAT_ARRAY,
		TYPE_TEXTURE,
	};

public:
	ShaderParameter();
	virtual ~ShaderParameter();

	int GetType() { return type_; }
	void SetMatrix(D3DXMATRIX& matrix);
	D3DXMATRIX GetMatrix();
	void SetMatrixArray(std::vector<D3DXMATRIX>& matrix);
	std::vector<D3DXMATRIX> GetMatrixArray();
	void SetVector(D3DXVECTOR4& vector);
	D3DXVECTOR4 GetVector();
	void SetFloat(float value);
	float GetFloat();
	void SetFloatArray(std::vector<float>& values);
	std::vector<float> GetFloatArray();
	void SetTexture(std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture();

private:
	int type_;
	std::shared_ptr<gstd::ByteBuffer> value_;
	std::shared_ptr<Texture> texture_;
};

/**********************************************************
//Shader
**********************************************************/
class Shader {
	friend ShaderManager;

public:
	Shader();
	Shader(Shader* shader);
	virtual ~Shader();
	void Release();

	int Begin(int pass = 0);
	void End();

	ID3DXEffect* GetEffect();
	void ReleaseDxResource();
	void RestoreDxResource();

	bool CreateFromFile(std::string path);
	bool CreateFromText(std::string& source);
	bool IsLoad() { return data_ != NULL && data_->bLoad_; }

	bool SetTechnique(std::string name);
	bool SetMatrix(std::string name, D3DXMATRIX& matrix);
	bool SetMatrixArray(std::string name, std::vector<D3DXMATRIX>& matrix);
	bool SetVector(std::string name, D3DXVECTOR4& vector);
	bool SetFloat(std::string name, float value);
	bool SetFloatArray(std::string name, std::vector<float>& values);
	bool SetTexture(std::string name, std::shared_ptr<Texture> texture);

protected:
	std::unique_ptr<ShaderData> data_;

	// bool bLoadShader_;
	// IDirect3DVertexShader9* pVertexShader_;
	// IDirect3DPixelShader9* pPixelShader_;

	std::string technique_;
	std::map<std::string, std::shared_ptr<ShaderParameter>> mapParam_;

	std::shared_ptr<ShaderParameter> _GetParameter(std::string name, bool bCreate);

	int _Begin();
	void _End();
	void _BeginPass(int pass = 0);
	void _EndPass();
	bool _SetupParameter();
};

} // namespace directx

#endif
