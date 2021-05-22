#include "ElfreinaMesh.hpp"
#include "DirectGraphics.hpp"
#include "DxUtility.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ElfreinaMesh
**********************************************************/
//ElfreinaMeshData
ElfreinaMeshData::ElfreinaMeshData()
{
}
ElfreinaMeshData::~ElfreinaMeshData()
{
}
bool ElfreinaMeshData::CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader)
{
	bool res = false;
	path_ = reader->GetOriginalPath();
	int size = reader->GetFileSize();
	std::vector<char> textUtf8;
	textUtf8.resize(size + 1);
	reader->Read(&textUtf8[0], size);
	textUtf8[size] = '\0';

	//文字コード変換

	gstd::Scanner scanner(textUtf8);
	try {
		while (scanner.HasNext()) {
			gstd::Token& tok = scanner.Next();
			if (tok.GetElement() == "MeshContainer") {
				scanner.CheckType(scanner.Next(), Token::Type::OpenC);
				_ReadMeshContainer(scanner);
			} else if (tok.GetElement() == "HierarchyList") {
				scanner.CheckType(scanner.Next(), Token::Type::OpenC);
				_ReadHierarchyList(scanner);
			} else if (tok.GetElement() == "AnimationList") {
				scanner.CheckType(scanner.Next(), Token::Type::OpenC);
				_ReadAnimationList(scanner);
			}
		}

		//頂点バッファ作成
		for (int iMesh = 0; iMesh < mesh_.size(); iMesh++) {
			mesh_[iMesh]->InitializeVertexBuffer();
		}

		res = true;
	} catch (std::exception& e) {
		Logger::WriteTop(StringUtility::Format(u8"ElfreinaMeshData読み込み失敗 %s %d", e.what(), scanner.GetCurrentLine()));
		res = false;
	}

#if 0
		Logger::WriteTop(StringUtility::Format(u8"ElfreinaMeshData読み込み成功[%s]", reader->GetOriginalPath().c_str()));
		for (int iMesh = 0; iMesh < mesh_.size(); iMesh++) {
			std::string name = mesh_[iMesh]->name_;
			std::string log = " Mesh ";
			log += StringUtility::Format("name[%s] ", name.c_str());
			log += StringUtility::Format("vert[%d] ", mesh_[iMesh]->GetVertexCount());
			Logger::WriteTop(u8og);

			if (iMesh == 6) {
				int countVert = mesh_[iMesh]->GetVertexCount();
				int iVert = 0;
				for (iVert = 0; iVert < countVert; iVert++) {
					VERTEX_B4NX* vert = mesh_[iMesh]->GetVertex(iVert);
					//VERTEX_B2NX* vert = mesh_[iMesh]->GetVertex(iVert);
					log = "  vert ";
					log += StringUtility::Format("pos[%f,%f,%f] ", vert->position.x, vert->position.y, vert->position.z);
					log += StringUtility::Format("blend1[%d,%f] ", BitAccess::GetByte(vert->blendIndex, 0), vert->blendRate);
					log += StringUtility::Format("blend2[%d] ", BitAccess::GetByte(vert->blendIndex, 4));
					Logger::WriteTop(u8og);
				}

				int iFace = 0;
				std::vector<short>& indexVert = mesh_[iMesh]->vertexIndices_;
				int countFace = mesh_[iMesh]->vertexIndices_.size() / 3;
				for (iFace = 0; iFace < countFace; iFace++) {
					log = "  vert ";
					log += StringUtility::Format("index[%d,%d,%d] ",
						indexVert[iFace * 3], indexVert[iFace * 3 + 1], indexVert[iFace * 3 + 2]);
					Logger::WriteTop(u8og);
				}
			}
		}
		for (int iBone = 0; iBone < bone_.size(); iBone++) {
			std::string log = " Bone ";
			std::string name = bone_[iBone]->name_;
			log += StringUtility::Format("name[%s] ", name.c_str());
			Logger::WriteTop(u8og);
		}

		std::map<std::string, std::shared_ptr<AnimationData>>::iterator itr;
		for (itr = anime_.begin(); itr != anime_.end(); itr++) {
			std::string name = itr->first;
			std::string log = " Anime ";
			log += StringUtility::Format("name[%s] ", name.c_str());
			Logger::WriteTop(u8og);
		}
#endif

	return res;
}
void ElfreinaMeshData::_ReadMeshContainer(gstd::Scanner& scanner)
{
	int countReadMesh = 0;
	while (scanner.HasNext()) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;

		if (tok.GetElement() == "Name") {
		} else if (tok.GetElement() == "BoneCount") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			int countBone = tok.GetInteger();
			bone_.resize(countBone);
			for (int iBone = 0; iBone < countBone; iBone++) {
				bone_[iBone] = std::make_shared<Bone>();
			}
		} else if (tok.GetElement() == "MeshCount") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			int countObj = tok.GetInteger();
			mesh_.resize(countObj);
			for (int iObj = 0; iObj < countObj; iObj++) {
				mesh_[iObj] = std::make_shared<Mesh>();
			}
		} else if (tok.GetElement() == "VertexFormat") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			while (true) {
				if (scanner.Next().GetType() == Token::Type::CloseC)
					break;
			}
		} else if (tok.GetElement() == "BoneNames") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			int iBone = 0;
			while (true) {
				tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;
				if (tok.GetType() != Token::Type::String)
					continue;
				bone_[iBone]->name_ = tok.GetString();
				iBone++;
			}
		} else if (tok.GetElement() == "OffsetMatrices") {
			int iCount = 0;
			int iBone = -1;
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			while (true) {
				tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;

				if (tok.GetType() == Token::Type::Newline) {
					if (iCount != 0 && iBone != -1 && iCount != 16)
						throw std::exception(u8"ElfreinaMeshData:OffsetMatricesの数が不正です");
					iCount = 0;
					iBone++;
				}

				if (tok.GetType() != Token::Type::Int && tok.GetType() != Token::Type::Real)
					continue;

				bone_[iBone]->matOffset_.m[iCount / 4][iCount % 4] = tok.GetReal();
				iCount++;
			}
		} else if (tok.GetElement() == "Materials") {
			int posMat = 0;
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			while (true) {
				tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;
				if (tok.GetElement() == "MaterialCount") {
					scanner.CheckType(scanner.Next(), Token::Type::Equal);
					tok = scanner.Next();
					int countMaterial = tok.GetInteger();
					material_.resize(countMaterial);
					for (int iMat = 0; iMat < countMaterial; iMat++) {
						material_[iMat] = std::make_shared<Material>();
					}
				} else if (tok.GetElement() == "Material") {
					scanner.CheckType(scanner.Next(), Token::Type::OpenC);
					_ReadMaterials(scanner, *material_[posMat]);
					posMat++;
				}
			}
		} else if (tok.GetElement() == "Mesh") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			_ReadMesh(scanner, *mesh_[countReadMesh]);
			countReadMesh++;
		}
	}
}
void ElfreinaMeshData::_ReadMaterials(gstd::Scanner& scanner, Material& material)
{
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;

		if (tok.GetElement() == "Name") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			material.name_ = tok.GetString();
		} else if (tok.GetElement() == "Diffuse" || tok.GetElement() == "Ambient" || tok.GetElement() == "Emissive" || tok.GetElement() == "Specular") {
			D3DMATERIAL9& mat = material.mat_;
			D3DCOLORVALUE* color;
			if (tok.GetElement() == "Diffuse")
				color = &mat.Diffuse;
			else if (tok.GetElement() == "Ambient")
				color = &mat.Ambient;
			else if (tok.GetElement() == "Emissive")
				color = &mat.Emissive;
			else if (tok.GetElement() == "Specular")
				color = &mat.Specular;

			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			color->a = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::Type::Colon);
			color->r = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::Type::Colon);
			color->g = scanner.Next().GetReal();
			scanner.CheckType(scanner.Next(), Token::Type::Colon);
			color->b = scanner.Next().GetReal();
		} else if (tok.GetElement() == "SpecularSharpness") {
		} else if (tok.GetElement() == "TextureFilename") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();

			std::string wPathTexture = tok.GetString();
			std::string path = PathProperty::GetFileDirectory(path_) + wPathTexture;
			material.texture_ = std::make_shared<Texture>();
			material.texture_->CreateFromFile(path);
		}
	}
}
void ElfreinaMeshData::_ReadMesh(gstd::Scanner& scanner, Mesh& mesh)
{
	std::vector<std::vector<Mesh::BoneWeightData>> vectWeight;
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;

		if (tok.GetElement() == "Name") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			mesh.name_ = tok.GetString();
		} else if (tok.GetElement() == "VertexCount") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			int countVertex = tok.GetInteger();
			mesh.SetVertexCount(countVertex);
			vectWeight.resize(countVertex);
		} else if (tok.GetElement() == "FaceCount") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			tok = scanner.Next();
			int countFace = tok.GetInteger();
			mesh.vertexIndices_.resize(countFace * 3);
		} else if (tok.GetElement() == "Positions" || tok.GetElement() == "Normals") {
			bool bPosition = tok.GetElement() == "Positions";
			bool bNormal = tok.GetElement() == "Normals";
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);

			int countVertex = mesh.GetVertexCount();
			for (int iVert = 0; iVert < countVertex; iVert++) {
				float x = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				float y = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				float z = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Newline);
				if (bPosition)
					mesh.SetVertexPosition(iVert, x, y, z);
				else if (bNormal)
					mesh.SetVertexNormal(iVert, x, y, z);
			}
			scanner.CheckType(scanner.Next(), Token::Type::CloseC);
		} else if (tok.GetElement() == "BlendList") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			while (true) {
				gstd::Token& tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;
				if (tok.GetElement() == "BlendPart") {
					scanner.CheckType(scanner.Next(), Token::Type::OpenC);
					scanner.CheckType(scanner.Next(), Token::Type::Newline);

					scanner.CheckIdentifer(scanner.Next(), "BoneName");
					scanner.CheckType(scanner.Next(), Token::Type::Equal);
					std::string name = scanner.Next().GetString();

					scanner.CheckType(scanner.Next(), Token::Type::Newline);
					scanner.CheckIdentifer(scanner.Next(), "TransformIndex");
					scanner.CheckType(scanner.Next(), Token::Type::Equal);
					int indexBone = scanner.Next().GetInteger();

					scanner.CheckType(scanner.Next(), Token::Type::Newline);
					scanner.CheckIdentifer(scanner.Next(), "VertexBlend");
					scanner.CheckType(scanner.Next(), Token::Type::OpenC);
					auto bone = bone_[indexBone];
					bone->name_ = name;

					while (true) {
						tok = scanner.Next();
						if (tok.GetType() == Token::Type::CloseC)
							break;
						if (tok.GetType() != Token::Type::Int && tok.GetType() != Token::Type::Real)
							continue;

						int index = tok.GetInteger();
						scanner.CheckType(scanner.Next(), Token::Type::Comma);
						float weight = scanner.Next().GetReal();

						Mesh::BoneWeightData data;
						data.index = indexBone;
						data.weight = weight;
						vectWeight[index].push_back(data);
					}
					scanner.CheckType(scanner.Next(), Token::Type::Newline);
					scanner.CheckType(scanner.Next(), Token::Type::CloseC);
				}
			}

		} else if (tok.GetElement() == "TextureUV") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);

			int countVertex = mesh.GetVertexCount();
			for (int iVert = 0; iVert < countVertex; iVert++) {
				float u = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				float v = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Newline);
				mesh.SetVertexUV(iVert, u, v);
			}
			scanner.CheckType(scanner.Next(), Token::Type::CloseC);
		} else if (tok.GetElement() == "VertexIndices") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);

			int countFace = mesh.vertexIndices_.size() / 3;
			for (int iFace = 0; iFace < countFace; iFace++) {
				int face = scanner.Next().GetInteger();
				if (face != 3)
					throw std::exception(u8"3頂点で形成されていない面があります");
				scanner.CheckType(scanner.Next(), Token::Type::Comma);
				mesh.vertexIndices_[iFace * 3] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				mesh.vertexIndices_[iFace * 3 + 1] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				mesh.vertexIndices_[iFace * 3 + 2] = scanner.Next().GetInteger();
				scanner.CheckType(scanner.Next(), Token::Type::Newline);
			}
			scanner.CheckType(scanner.Next(), Token::Type::CloseC);
		} else if (tok.GetElement() == "Attributes") {
			int attr = -1;
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			int countFace = mesh.vertexIndices_.size() / 3;
			for (int iFace = 0; iFace < countFace; iFace++) {
				int index = scanner.Next().GetInteger();
				if (attr == -1)
					attr = index;
				if (attr != index)
					throw std::exception(u8"1オブジェクトに複数マテリアルは非対応");
				scanner.CheckType(scanner.Next(), Token::Type::Newline);
			}
			mesh.material_ = material_[attr];
			scanner.CheckType(scanner.Next(), Token::Type::CloseC);
		}
	}

	//ボーンウェイトデータ整理
	int boneCount = bone_.size();
	std::vector<double> totalWeight;
	totalWeight.resize(boneCount);
	ZeroMemory(&totalWeight[0], sizeof(double) * bone_.size());
	for (int iVert = 0; iVert < vectWeight.size(); iVert++) {
		std::vector<Mesh::BoneWeightData>& datas = vectWeight[iVert];
#if 0
			//BLEND2
			Mesh::BoneWeightData* data1 = NULL;
			Mesh::BoneWeightData* data2 = NULL;
			for (int iDatas = 0; iDatas < datas.size(); iDatas++) {
				Mesh::BoneWeightData* data = &datas[iDatas];
				totalWeight[data->index] += data->weight;
				if (data1 == NULL)
					data1 = data;
				else if (data2 == NULL) {
					data2 = data;
					if (data1->weight < data2->weight) {
						Mesh::BoneWeightData* temp = data1;
						data1 = data2;
						data2 = temp;
					}
				} else {
					if (data1->weight < data->weight) {
						data2 = data1;
						data1 = data;
					} else if (data2->weight < data->weight) {
						data2 = data;
					}
				}
			}

			//頂点データに設定
			if (data1 != NULL && data2 != NULL) {
				float sub = 1.0f - data1->weight - data2->weight;
				data1->weight += sub / 2.0f;
				data2->weight += sub / 2.0f;
				if (data1->weight > 1.0f)
					data1->weight = 1.0f;
				if (data2->weight > 1.0f)
					data2->weight = 1.0f;
			}

			if (data1 != NULL)
				mesh.SetVertexBlend(iVert, 0, data1->index, data1->weight);
			if (data2 != NULL)
				mesh.SetVertexBlend(iVert, 1, data2->index, data2->weight);
#endif
			{
			int dataCount = datas.size();
			for (int iDatas = 0; iDatas < dataCount; iDatas++) {
				Mesh::BoneWeightData* data = &datas[iDatas];
				if (data == NULL)
					continue;
				totalWeight[data->index] += data->weight;
				mesh.SetVertexBlend(iVert, iDatas, data->index, data->weight);
			}
		}
	}

	//重心
	double maxWeight = 0;
	mesh.CalculateWeightCenter();
	for (int iBone = 0; iBone < totalWeight.size(); iBone++) {
		if (totalWeight[iBone] < maxWeight)
			continue;
		mesh.indexWeightForCalucZValue_ = iBone;
		maxWeight = totalWeight[iBone];
	}
}

void ElfreinaMeshData::_ReadHierarchyList(gstd::Scanner& scanner)
{
	for (int iBone = 0; iBone < bone_.size(); iBone++) {
		std::string name = bone_[iBone]->name_;
		mapBoneNameIndex_[name] = iBone;
	}

	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "Node") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			_ReadNode(scanner, Bone::NO_PARENT);
		}
	}
}
int ElfreinaMeshData::_ReadNode(gstd::Scanner& scanner, int parent)
{
	int indexBone = -1;
	std::shared_ptr<Bone> bone;
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "NodeName") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			std::string name = scanner.Next().GetString();
			indexBone = mapBoneNameIndex_[name];
			bone = bone_[indexBone];
			bone->indexParent_ = parent;
		} else if (tok.GetElement() == "InitPostureMatrix") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			for (int iCount = 0; iCount < 16; iCount++) {
				bone->matInitPosture_.m[iCount / 4][iCount % 4] = scanner.Next().GetReal();
				if (iCount <= 14)
					scanner.CheckType(scanner.Next(), Token::Type::Colon);
			}
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
		}
		if (tok.GetElement() == "Node") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			int indexChild = _ReadNode(scanner, indexBone);
			bone->indexChild_.push_back(indexChild);
		}
	}
	return indexBone;
}

void ElfreinaMeshData::_ReadAnimationList(gstd::Scanner& scanner)
{
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "AnimationCount") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			scanner.Next().GetInteger();
		} else if (tok.GetElement() == "AnimationData") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			std::shared_ptr<AnimationData> anime = _ReadAnimationData(scanner);
			if (anime != NULL) {
				anime_[anime->name_] = anime;
			}
		}
	}
}
std::shared_ptr<ElfreinaMeshData::AnimationData> ElfreinaMeshData::_ReadAnimationData(gstd::Scanner& scanner)
{
	std::shared_ptr<AnimationData> anime = std::make_shared<AnimationData>();
	anime->animeBone_.resize(bone_.size());
	auto pAnime = anime;
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "AnimationName") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			pAnime->name_ = scanner.Next().GetString();
		} else if (tok.GetElement() == "AnimationTime") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			pAnime->timeTotal_ = scanner.Next().GetInteger();
		} else if (tok.GetElement() == "FrameParSecond") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			pAnime->framePerSecond_ = scanner.Next().GetInteger();
		} else if (tok.GetElement() == "TransitionTime") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
		} else if (tok.GetElement() == "Priority") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
		} else if (tok.GetElement() == "Loop") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			std::string id = scanner.Next().GetElement();
			pAnime->bLoop_ = id == "True";
		} else if (tok.GetElement() == "BoneAnimation") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			_ReadBoneAnimation(scanner, *pAnime);
		} else if (tok.GetElement() == "UVAnimation") {
			// TODO
			int count = 1;
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			while (true) {
				tok = scanner.Next();
				if (tok.GetType() == Token::Type::OpenC)
					count++;
				else if (tok.GetType() == Token::Type::CloseC)
					count--;
				if (count == 0)
					break;
			}
		}
	}
	return anime;
}
void ElfreinaMeshData::_ReadBoneAnimation(gstd::Scanner& scanner, AnimationData& anime)
{
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "TimeKeys") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			while (scanner.Next().GetType() != Token::Type::CloseC) {
			}
		} else if (tok.GetElement() == "AnimationPart") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			_ReadBoneAnimationPart(scanner, anime);
		}
	}
}
void ElfreinaMeshData::_ReadBoneAnimationPart(gstd::Scanner& scanner, AnimationData& anime)
{
	int indexBone = -1;
	auto part = std::make_shared<BoneAnimationPart>();
	while (true) {
		gstd::Token& tok = scanner.Next();
		if (tok.GetType() == Token::Type::CloseC)
			break;
		if (tok.GetElement() == "NodeName") {
			scanner.CheckType(scanner.Next(), Token::Type::Equal);
			std::string name = scanner.Next().GetString();
			indexBone = mapBoneNameIndex_[name];
		} else if (tok.GetElement() == "TimeKeys") {
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			while (true) {
				gstd::Token& tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;
				if (tok.GetType() != Token::Type::Int && tok.GetType() != Token::Type::Real)
					continue;
				part->keyTime_.push_back(tok.GetReal());
			}
		} else if (tok.GetElement() == "TransKeys" || tok.GetElement() == "ScaleKeys" || tok.GetElement() == "RotateKeys") {
			bool bTrans = tok.GetElement() == "TransKeys";
			bool bScale = tok.GetElement() == "ScaleKeys";
			bool bRotate = tok.GetElement() == "RotateKeys";
			scanner.CheckType(scanner.Next(), Token::Type::OpenC);
			scanner.CheckType(scanner.Next(), Token::Type::Newline);
			while (true) {
				gstd::Token& tok = scanner.Next();
				if (tok.GetType() == Token::Type::CloseC)
					break;
				if (tok.GetType() != Token::Type::Int && tok.GetType() != Token::Type::Real)
					continue;
				float x = tok.GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				float y = scanner.Next().GetReal();
				scanner.CheckType(scanner.Next(), Token::Type::Colon);
				float z = scanner.Next().GetReal();

				if (bTrans || bScale) {
					D3DXVECTOR3 vect(x, y, z);
					if (bTrans)
						part->keyTrans_.push_back(vect);
					else if (bScale)
						part->keyScale_.push_back(vect);
				} else if (bRotate) {
					scanner.CheckType(scanner.Next(), Token::Type::Colon);
					float w = scanner.Next().GetReal();

					D3DXQUATERNION vect(x, y, z, w);
					part->keyRotate_.push_back(vect);
				}

				scanner.CheckType(scanner.Next(), Token::Type::Newline);
			}
		}
	}
	if (indexBone != -1)
		anime.animeBone_[indexBone] = part;
}

//ElfreinaMeshData::Mesh
ElfreinaMeshData::Mesh::Mesh()
{
	SetPrimitiveType(D3DPT_TRIANGLELIST);
}
ElfreinaMeshData::Mesh::~Mesh()
{
}
void ElfreinaMeshData::Mesh::Render()
{
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	auto material = material_;
	SetTexture(material->texture_);
	materialBNX_ = material->mat_;
	device->SetMaterial(&materialBNX_);
	RenderObjectB4NX::Render();
	//RenderObjectB2NX::Render();
	/*
	{
		RenderObjectLX obj;
		int count = GetVertexCount();
		obj.SetVertexCount(count);
		for (int iVert = 0; iVert < count; iVert++) {
			VERTEX_B2NX* b2 = GetVertex(iVert);
			obj.SetVertexPosition(iVert, b2->position.x, b2->position.y, b2->position.z);
			obj.SetVertexUV(iVert, b2->texcoord.x, b2->texcoord.y);
			obj.SetVertexAlpha(iVert, 255);
			obj.SetVertexColorARGB(iVert, 255, 255, 255, 255);
		}
		obj.SetVertexIndicies(vertexIndices_);
		obj.SetTexture(mat->texture_);
		obj.Render();
	}
	*/
}
std::shared_ptr<RenderBlock> ElfreinaMeshData::Mesh::CreateRenderBlock()
{
	std::shared_ptr<RenderObjectElfreinaBlock> res = std::make_shared<RenderObjectElfreinaBlock>();
	res->SetPosition(position_);
	res->SetAngle(angle_);
	res->SetScale(scale_);
	res->SetMatrix(matrix_);
	return res;
}

//ElfreinaMeshData::AnimationData
std::shared_ptr<Matrices> ElfreinaMeshData::AnimationData::CreateBoneAnimationMatrix(double time, std::shared_ptr<ElfreinaMeshData> mesh)
{
	std::vector<std::shared_ptr<Bone>>& bones = mesh->GetBones();

	std::shared_ptr<Matrices> matrix = std::make_shared<Matrices>();
	matrix->SetSize(bones.size());

	//最上位の親を探す
	for (int iBone = 0; iBone < bones.size(); iBone++) {
		if (bones[iBone]->GetParentIndex() != ElfreinaMeshData::Bone::NO_PARENT)
			continue;

		D3DXMATRIX& matInit = bones[iBone]->GetInitPostureMatrix();
		D3DXMATRIX& matOffset = bones[iBone]->GetOffsetMatrix();
		D3DXMATRIX mat = _CalculateMatrix(time, iBone) * matInit;

		auto v = matOffset * mat;
		
		//自身の行列を設定
		matrix->SetMatrix(iBone, v);

		//子の行列を計算
		std::vector<int>& indexChild = bones[iBone]->GetChildIndex();
		for (int iChild = 0; iChild < indexChild.size(); iChild++) {
			int index = indexChild[iChild];
			_CreateBoneAnimationMatrix(time, mesh, matrix, index, mat);
		}
	}

	return matrix;
}
void ElfreinaMeshData::AnimationData::_CreateBoneAnimationMatrix(int time, std::shared_ptr<ElfreinaMeshData> mesh, std::shared_ptr<Matrices> matrix, int indexOwn, D3DXMATRIX& matrixParentAnime)
{
	std::vector<std::shared_ptr<Bone>>& bones = mesh->GetBones();

	D3DXMATRIX& matInit = bones[indexOwn]->GetInitPostureMatrix();
	D3DXMATRIX& matOffset = bones[indexOwn]->GetOffsetMatrix();
	D3DXMATRIX mat = _CalculateMatrix(time, indexOwn) * matInit * matrixParentAnime;
	auto v = matOffset * mat;
	
	//自身の行列を設定
	matrix->SetMatrix(indexOwn, v);

	//子の行列を計算
	std::vector<int>& indexChild = bones[indexOwn]->GetChildIndex();
	for (int iChild = 0; iChild < indexChild.size(); iChild++) {
		int index = indexChild[iChild];
		_CreateBoneAnimationMatrix(time, mesh, matrix, index, mat);
	}
}
D3DXMATRIX ElfreinaMeshData::AnimationData::_CalculateMatrix(double time, int index)
{
	D3DXMATRIX res;
	auto part = animeBone_[index];
	if (part == NULL) {
		D3DXMatrixIdentity(&res);
		return res;
	}

	std::vector<float>& keyTime = part->GetTimeKey();

	//アニメーションインデックスを検索
	int keyNext = -1;
	int keyPrevious = -1;
	for (int iTime = 0; iTime < keyTime.size(); iTime++) {
		int tTime = timeTotal_ * keyTime[iTime];
		if (tTime < time)
			continue;
		keyPrevious = iTime - 1;
		keyNext = iTime;
		break;
	}
	if (keyNext == -1) {
		keyNext = keyTime.size() - 1;
		keyPrevious = keyNext - 1;
	}
	if (keyPrevious < 0)
		keyPrevious = 0;
	if (keyNext < 0)
		keyNext = 0;

	float timeDiffKey = (float)timeTotal_ * keyTime[keyNext] - (float)timeTotal_ * keyTime[keyPrevious];
	float timeDiffAnime = (float)(time - timeTotal_ * keyTime[keyPrevious]);
	float rateToNext = timeDiffKey != 0.0f ? timeDiffAnime / timeDiffKey : 0;

	//各行列を作成
	std::vector<D3DXVECTOR3>& keyTrans = part->GetTransKey();
	D3DXMATRIX matTrans;
	D3DXMatrixIdentity(&matTrans);
	D3DXVECTOR3 pos;
	pos.x = keyTrans[keyPrevious].x + (keyTrans[keyNext].x - keyTrans[keyPrevious].x) * rateToNext;
	pos.y = keyTrans[keyPrevious].y + (keyTrans[keyNext].y - keyTrans[keyPrevious].y) * rateToNext;
	pos.z = keyTrans[keyPrevious].z + (keyTrans[keyNext].z - keyTrans[keyPrevious].z) * rateToNext;
	D3DXMatrixTranslation(&matTrans, pos.x, pos.y, pos.z);

	std::vector<D3DXVECTOR3>& keyScale = part->GetScaleKey();
	D3DXMATRIX matScale;
	D3DXMatrixIdentity(&matScale);
	D3DXVECTOR3 scale;
	scale.x = keyScale[keyPrevious].x + (keyScale[keyNext].x - keyScale[keyPrevious].x) * rateToNext;
	scale.y = keyScale[keyPrevious].y + (keyScale[keyNext].y - keyScale[keyPrevious].y) * rateToNext;
	scale.z = keyScale[keyPrevious].z + (keyScale[keyNext].z - keyScale[keyPrevious].z) * rateToNext;
	D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);

	std::vector<D3DXQUATERNION>& keyRotate = part->GetRotateKey();
	D3DXMATRIX matRotate;
	D3DXMatrixIdentity(&matRotate);
	D3DXQUATERNION quat;
	D3DXQuaternionSlerp(&quat, &keyRotate[keyPrevious], &keyRotate[keyNext], rateToNext);
	D3DXMatrixRotationQuaternion(&matRotate, &quat);

	res = matScale * matRotate * matTrans;
	return res;
}

//RenderObjectElfreinaBlock
RenderObjectElfreinaBlock::~RenderObjectElfreinaBlock()
{
}
bool RenderObjectElfreinaBlock::IsTranslucent()
{
	auto obj = std::dynamic_pointer_cast<ElfreinaMeshData::Mesh>(obj_);
	D3DMATERIAL9& mat = obj->material_->mat_;
	bool bTrans = true;
	bTrans &= (mat.Diffuse.a != 1.0f);
	return RenderObjectB2NXBlock::IsTranslucent() || bTrans;
}
void RenderObjectElfreinaBlock::CalculateZValue()
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	IDirect3DDevice9* pDevice = graph->GetDevice();
	auto obj = std::dynamic_pointer_cast<ElfreinaMeshData::Mesh>(obj_);

	D3DXMATRIX matTrans = obj->_CreateWorldTransformMaxtrix();
	D3DXMATRIX matView;
	D3DXMATRIX matPers;
	pDevice->GetTransform(D3DTS_VIEW, &matView);
	pDevice->GetTransform(D3DTS_PROJECTION, &matPers);
	D3DXMATRIX matAnime = matrix_->GetMatrix(obj->indexWeightForCalucZValue_);

	D3DXMATRIX mat = matAnime * matTrans * matView * matPers;
	D3DXVECTOR3 posCenter = obj->GetWeightCenter();
	D3DXVec3TransformCoord(&posCenter, &posCenter, &mat);
	posSortKey_ = posCenter.z;
}

//ElfreinaMesh
bool ElfreinaMesh::CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader)
{
	bool res = false;
	{
		Lock lock(DxMeshManager::GetBase()->GetLock());
		if (data_ != NULL)
			Release();

		std::string name = reader->GetOriginalPath();

		data_ = _GetFromManager(name);
		if (data_ == NULL) {
			if (!reader->Open())
				throw std::exception(u8"ファイルが開けません");
			data_ = std::make_shared<ElfreinaMeshData>();
			data_->SetName(name);
			res = data_->CreateFromFileReader(reader);
			if (res) {
				Logger::WriteTop(StringUtility::Format(u8"メッシュを読み込みました[%s]", name.c_str()));
				_AddManager(name, data_);

			} else {
				data_ = NULL;
			}
		} else {
			res = true;
		}
	}
	return res;
}
bool ElfreinaMesh::CreateFromFileInLoadThread(std::string path)
{
	return DxMesh::CreateFromFileInLoadThread(path, MESH_ELFREINA);
}
std::string ElfreinaMesh::GetPath()
{
	if (data_ == NULL)
		return "";
	return ((ElfreinaMeshData*)data_.get())->path_;
}
void ElfreinaMesh::Render()
{
	if (data_ == NULL)
		return;

	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);
	for (int iMesh = 0; iMesh < data->mesh_.size(); iMesh++) {
		auto mesh = data->mesh_[iMesh];
		mesh->SetPosition(position_);
		mesh->SetAngle(angle_);
		mesh->SetScale(scale_);
		mesh->SetColor(color_);
		mesh->SetCoordinate2D(bCoordinate2D_);
		mesh->Render();
	}
	//mesh_[6]->Render();
}
void ElfreinaMesh::Render(std::string nameAnime, int time)
{
	if (data_ == NULL)
		return;

	std::shared_ptr<Matrices> matrix = CreateAnimationMatrix(nameAnime, time);
	if (matrix == NULL)
		return;

	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);
	while (!data->bLoad_) {
		::Sleep(1);
	}

	for (int iMesh = 0; iMesh < data->mesh_.size(); iMesh++) {
		auto mesh = data->mesh_[iMesh];
		mesh->SetMatrix(matrix);
	}
	Render();
}
std::shared_ptr<RenderBlocks> ElfreinaMesh::CreateRenderBlocks()
{
	if (data_ == NULL)
		return NULL;
	std::shared_ptr<RenderBlocks> res = std::make_shared<RenderBlocks>();
	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);;
	for (int iMesh = 0; iMesh < data->mesh_.size(); iMesh++) {
		std::shared_ptr<ElfreinaMeshData::Mesh> mesh = data->mesh_[iMesh];
		mesh->SetPosition(position_);
		mesh->SetAngle(angle_);
		mesh->SetScale(scale_);
		mesh->SetColor(color_);

		std::shared_ptr<RenderBlock> block = mesh->CreateRenderBlock();
		block->SetRenderObject(mesh);
		res->Add(block);
	}
	return res;
}
std::shared_ptr<RenderBlocks> ElfreinaMesh::CreateRenderBlocks(std::string nameAnime, double time)
{
	if (data_ == NULL)
		return NULL;

	std::shared_ptr<Matrices> matrix = CreateAnimationMatrix(nameAnime, time);
	if (matrix == NULL)
		return NULL;
	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);
	for (int iMesh = 0; iMesh < data->mesh_.size(); iMesh++) {
		auto mesh = data->mesh_[iMesh];
		mesh->SetMatrix(matrix);
	}

	return CreateRenderBlocks();
}
double ElfreinaMesh::_CalcFrameToTime(double time, std::shared_ptr<ElfreinaMeshData::AnimationData> anime)
{
	bool bLoop = anime->bLoop_;
	int framePerSec = anime->framePerSecond_;
	time = (int)((double)1000 / (double)framePerSec * (double)time);

	int timeTotal = anime->timeTotal_;
	if (bLoop) {
		int tTime = (int)time / timeTotal;
		time -= (int)timeTotal * tTime;
	} else {
		if (time > timeTotal)
			time = timeTotal;
	}
	return time;
}
std::shared_ptr<Matrices> ElfreinaMesh::CreateAnimationMatrix(std::string nameAnime, double time)
{
	if (data_ == NULL)
		return NULL;

	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);
	bool bExist = data->anime_.find(nameAnime) != data->anime_.end();
	if (!bExist)
		return NULL;
	std::shared_ptr<ElfreinaMeshData::AnimationData> anime = data->anime_[nameAnime];

	//ループ有無で時間を計算する
	time = _CalcFrameToTime(time, anime);

	std::shared_ptr<Matrices> matrix = anime->CreateBoneAnimationMatrix(time, data);
	return matrix;
}

D3DXMATRIX ElfreinaMesh::GetAnimationMatrix(std::string nameAnime, double time, std::string nameBone)
{
	D3DXMATRIX res;
	auto data = std::dynamic_pointer_cast<ElfreinaMeshData>(data_);
	if (data->mapBoneNameIndex_.find(nameBone) != data->mapBoneNameIndex_.end()) {
		int indexBone = data->mapBoneNameIndex_[nameBone];
		bool bExist = data->anime_.find(nameAnime) != data->anime_.end();
		if (bExist) {
			std::shared_ptr<ElfreinaMeshData::AnimationData> anime = data->anime_[nameAnime];

			//ループ有無で時間を計算する
			time = _CalcFrameToTime(time, anime);
			std::shared_ptr<Matrices> matrix = anime->CreateBoneAnimationMatrix(time, data);
			D3DXMATRIX matBone = matrix->GetMatrix(indexBone);

			D3DXMATRIX matInv = data->bone_[indexBone]->matOffset_;
			D3DXMatrixInverse(&matInv, NULL, &matInv);
			// D3DXVECTOR3 pos;
			// D3DXVec3TransformCoord(&pos, &D3DXVECTOR3(0, 0, 0), &matInv);
			// D3DXMatrixTranslation(&matInv, pos.x, pos.y, pos.z);
			res = matInv * matBone;

			D3DXMATRIX matScale;
			D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
			res = res * matScale;

			D3DXMATRIX matRot;
			D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angle_.z));
			res = res * matRot;

			D3DXMATRIX matTrans;
			D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
			res = res * matTrans;
		}
	}
	return res;
}
