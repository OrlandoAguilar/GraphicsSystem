#include "SkinnedMeshStructures.h"

namespace GE{

	void VertexBoneData::AddBoneData(unsigned BoneID, float Weight)
	{
		for (unsigned i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
			if (Weights[i] == 0.0) {
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}
		throw "more bones than available"; //this should never happen because I am asking assimp to only allow 4 bones
	}

	BoneInfo::BoneInfo() : BoneOffset(0.0f), FinalTransformation(0.0f), isDirectlyConected(false)//, father(nullptr)
	{}

	VertexBoneData::VertexBoneData()
	{
		Reset();
	};

	void VertexBoneData::Reset()
	{
		ZERO_MEM(IDs);
		ZERO_MEM(Weights);
	}

	MeshEntry::MeshEntry()
	{
		NumIndices = 0;
		BaseVertex = 0;
		BaseIndex = 0;
		MaterialIndex = INVALID_MATERIAL;
	}

}