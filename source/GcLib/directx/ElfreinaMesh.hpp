#ifndef __DIRECTX_ElfreinaMeshData__
#define __DIRECTX_ElfreinaMeshData__

#include "RenderObject.hpp"

namespace directx {

class RenderObjectElfreinaBlock;
/**********************************************************
//ElfreinaMesh
**********************************************************/
class ElfreinaMesh;
class ElfreinaMeshData : public DxMeshData {
	friend ElfreinaMesh;

public:
	class Bone;
	class Material;
	class Mesh;
	class AnimationData;
	class BoneAnimationPart;

public:
	ElfreinaMeshData();
	~ElfreinaMeshData();
	bool CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader);
	std::vector<std::shared_ptr<Bone>>& GetBones() { return bone_; }

protected:
	std::string path_;
	std::vector<std::shared_ptr<Mesh>> mesh_;
	std::vector<std::shared_ptr<Bone>> bone_;
	std::vector<std::shared_ptr<Material>> material_;
	std::map<std::string, std::shared_ptr<AnimationData>> anime_;

	std::map<std::string, int> mapBoneNameIndex_;

	void _ReadMeshContainer(gstd::Scanner& scanner);
	void _ReadMaterials(gstd::Scanner& scanner, Material& material);
	void _ReadMesh(gstd::Scanner& scanner, Mesh& mesh);

	void _ReadHierarchyList(gstd::Scanner& scanner);
	int _ReadNode(gstd::Scanner& scanner, int parent);

	void _ReadAnimationList(gstd::Scanner& scanner);
	std::shared_ptr<AnimationData> _ReadAnimationData(gstd::Scanner& scanner);
	void _ReadBoneAnimation(gstd::Scanner& scanner, AnimationData& anime);
	void _ReadBoneAnimationPart(gstd::Scanner& scanner, AnimationData& anime);
};

class ElfreinaMeshData::Bone {
	friend ElfreinaMesh;
	friend ElfreinaMeshData;

public:
	enum {
		NO_PARENT = -1,
	};
public:
	Bone(){};
	virtual ~Bone(){};
	int GetParentIndex() { return indexParent_; }
	std::vector<int>& GetChildIndex() { return indexChild_; }
	D3DXMATRIX& GetOffsetMatrix() { return matOffset_; }
	D3DXMATRIX& GetInitPostureMatrix() { return matInitPosture_; }

protected:
	std::string name_;
	D3DXMATRIX matOffset_;
	D3DXMATRIX matInitPosture_;

	int indexParent_;
	std::vector<int> indexChild_;
};
class ElfreinaMeshData::Material {
	friend ElfreinaMeshData;
	friend ElfreinaMeshData::Mesh;
	friend RenderObjectElfreinaBlock;

public:
	Material(){};
	virtual ~Material(){};

protected:
	std::string name_;
	D3DMATERIAL9 mat_;
	std::shared_ptr<Texture> texture_;
};
class ElfreinaMeshData::Mesh : public RenderObjectB4NX {
	friend ElfreinaMeshData;
	friend RenderObjectElfreinaBlock;

public:
	Mesh();
	virtual ~Mesh();
	virtual void Render();
	std::shared_ptr<RenderBlock> CreateRenderBlock();

protected:
	std::string name_;
	std::shared_ptr<Material> material_;
	int indexWeightForCalucZValue_;

private:
	//頂点ボーンデータ読み込みに使う一時データ
	struct BoneWeightData {
		int index;
		float weight;
	};
};
class ElfreinaMeshData::AnimationData {
	friend ElfreinaMesh;
	friend ElfreinaMeshData;

public:
	AnimationData(){};
	virtual ~AnimationData(){};
	std::shared_ptr<Matrices> CreateBoneAnimationMatrix(double time, std::shared_ptr<ElfreinaMeshData> mesh);

protected:
	std::string name_;
	int timeTotal_;
	int framePerSecond_;
	bool bLoop_;
	std::vector<std::shared_ptr<ElfreinaMeshData::BoneAnimationPart>> animeBone_;

	void _CreateBoneAnimationMatrix(int time, std::shared_ptr<ElfreinaMeshData> mesh, std::shared_ptr<Matrices> matrix, int indexOwn, D3DXMATRIX& matrixParentAnime);
	D3DXMATRIX _CalculateMatrix(double time, int index);
};
class ElfreinaMeshData::BoneAnimationPart {
	friend ElfreinaMeshData;

public:
	BoneAnimationPart(){};
	virtual ~BoneAnimationPart(){};
	std::vector<float>& GetTimeKey() { return keyTime_; }
	std::vector<D3DXVECTOR3>& GetTransKey() { return keyTrans_; }
	std::vector<D3DXQUATERNION>& GetRotateKey() { return keyRotate_; }
	std::vector<D3DXVECTOR3>& GetScaleKey() { return keyScale_; }

protected:
	std::vector<float> keyTime_;
	std::vector<D3DXVECTOR3> keyTrans_;
	std::vector<D3DXQUATERNION> keyRotate_;
	std::vector<D3DXVECTOR3> keyScale_;
};

class RenderObjectElfreinaBlock : public RenderObjectB2NXBlock {
public:
	~RenderObjectElfreinaBlock();
	virtual bool IsTranslucent();
	virtual void CalculateZValue();
};

class ElfreinaMesh : public DxMesh {
public:
	ElfreinaMesh() {}
	virtual ~ElfreinaMesh() {}
	virtual bool CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader);
	virtual bool CreateFromFileInLoadThread(std::string path);
	virtual std::string GetPath();
	virtual void Render();
	virtual void Render(std::string nameAnime, int time);

	std::shared_ptr<RenderBlocks> CreateRenderBlocks();
	std::shared_ptr<RenderBlocks> CreateRenderBlocks(std::string nameAnime, double time);

	std::shared_ptr<Matrices> CreateAnimationMatrix(std::string nameAnime, double time);
	virtual D3DXMATRIX GetAnimationMatrix(std::string nameAnime, double time, std::string nameBone);

protected:
	double _CalcFrameToTime(double time, std::shared_ptr<ElfreinaMeshData::AnimationData> anime);
};

} // namespace directx

#endif
