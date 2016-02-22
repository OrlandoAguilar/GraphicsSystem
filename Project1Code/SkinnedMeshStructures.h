#pragma once
#include <string>
#include <GLM\glm.hpp>
#include <vector>
#include "Keyframe.h"
/*
In this structures, the Animated mesh class stores the information of the model
*/
namespace GE{

#define INVALID_MATERIAL 0xFFFFFFFF
#define NUM_BONES_PER_VEREX 4
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

	typedef int hashtype;
	typedef std::vector<Keyframe> keyframestype;

	/*For the animations, stores the duration, the number of ticks per second and the name*/
	struct Animation {
		float duration;
		float ticksPerSecond;
		std::string name;
	};

	/*Stores the information of every bone*/
	struct BoneInfo
	{
		glm::mat4 BoneOffset;		//matrix to transform from local to bone space
		glm::mat4 iBoneOffset;
		glm::mat4 FinalTransformation;	//matrix animation for current time
		BoneInfo* father;		//father
		std::vector<BoneInfo*> children;		//children
		std::string name;		//name of bone
		hashtype hash;			//hash name of bone for rapid comparison
		int index;				//index on array
		bool isDirectlyConected;//is directly conected to other bone of the hirarhcy?
		BoneInfo();
	};

	/*Stores the bones that affect a vertex and their weights*/
	struct VertexBoneData
	{
		unsigned IDs[NUM_BONES_PER_VEREX];
		float Weights[NUM_BONES_PER_VEREX];

		VertexBoneData();
		void Reset();
		void AddBoneData(unsigned BoneID, float Weight);
	};

	enum VB_TYPES {
		INDEX_BUFFER,
		POS_VB,
		NORMAL_VB,
		TEXCOORD_VB,
		BONE_VB,
		NUM_VBs
	};

	/*Stores information about every mesh (number of indices, shift for the indices, material index, etc)*/
	struct MeshEntry {
		MeshEntry();

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

	
	/*Stores information about every node (hierarchy, name, index of bone, vqs default tranformation and array of animations)*/
	struct Node {
		std::vector<int> bChildren;		//indexes of childrens (no pointers because reallocation invalidates everything)
		int mBone;		//index for the bone in the array of BoneInfo
		std::string mName;
		hashtype mHash;
		int father;
		VQS mTransformation;
		std::vector<keyframestype > mAnimations;
	};

}