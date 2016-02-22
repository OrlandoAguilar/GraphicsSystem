
#include <assert.h>

#include "SkinnedMesh.h"
#include "OpenGLGraphicsSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Hash.h"
#include "Debug.h"
#include <glm/gtc/quaternion.hpp>
#include "Quaternion.h"
#include "AssimpConvetion.h"
#include "Directory.h"
#include "Interpolation.h"

#define POSITION_LOCATION    0
#define NORMAL_LOCATION      1
#define TEX_COORD_LOCATION   2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

using namespace std;
using namespace GE::Utility;

namespace GE {

	/*Calls assimp functionality to load model and saves all the information in the structures of the object*/
	bool SkinnedMesh::LoadMesh(const string& Filename)
	{

		const aiScene* m_pScene;
		Assimp::Importer m_Importer;

		// Release the previously loaded mesh (if it exists)
		Clear();

		// Create the VAO
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		// Create the buffers for the vertices attributes
		glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

		bool Ret = false;
		m_Importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);		//a maximum of 4 bones for vertex
		m_pScene = m_Importer.ReadFile(Filename.c_str(),
			aiProcess_OptimizeMeshes |
			aiProcess_OptimizeGraph |
			aiProcess_GenUVCoords |
			aiProcess_TransformUVCoords |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_LimitBoneWeights
			);//aiProcessPreset_TargetRealtime_Quality

		if (m_pScene) {
			m_GlobalInverseTransform = Convert(m_pScene->mRootNode->mTransformation);		//gets global transformation for the model
			m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
			Ret = InitFromScene(m_pScene, Filename);			//initialize information related to vertices, normals, textures, etc.
			FillBonesHirarchy(m_pScene);				//Fill the bones with the hirarchy information (father, children)
			ExtractAnimationInformation(m_pScene);		//Extracts the information of the animation (name, duration, ticks per second)
			ExtractNodesInformation(m_pScene);			//Extracts the information of the nodes (bones)
		}
		else {
			printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
		}

		// Make sure the VAO is not changed from the outside
		glBindVertexArray(0);
		return Ret;
	}
	/*Extracts the information of the Animations (name, duration, ticks per second)*/
	void SkinnedMesh::ExtractAnimationInformation(const aiScene* m_pScene) {
		const int numAnim = m_pScene->mNumAnimations;
		printf("animations = %d\n", numAnim);
		for (int z=0; z < numAnim; ++z) {
			auto anim=m_pScene->mAnimations[z];

			Animation sanim;
			sanim.duration = (float)anim->mDuration;
			sanim.ticksPerSecond = (float)(anim->mTicksPerSecond? anim->mTicksPerSecond : 60.0f);	//if not specified, 60 fps
			sanim.name = std::string(anim->mName.data);
			mAnimations.push_back(sanim);
		}
	}

	/*Extract the information of the nodes. 
	Those are like bones, 
	but could be more than bones (lights, the root, etc), besides, 
	the nodes are with respect to the whole mode, but
	the bones can belong to different meshes*/
	void SkinnedMesh::ExtractNodesInformation(const aiScene* m_pScene) {
		ExtractNodesInformation(m_pScene,m_pScene->mRootNode, -1);
	}

	void SkinnedMesh::ExtractNodesInformation(const aiScene* m_pScene,const aiNode* ainode,int father) {
		mNodes.emplace_back();
		int index = mNodes.size() - 1;		//add animation information to the nodes for avoiding checking everytime for interpolation. and change to vqs
		Node& node = mNodes.back();
		node.mName = ainode->mName.data;
		node.mHash = Hash::GetHash(node.mName);
		auto bonIt = m_BoneMapping.find(node.mName);
		if (bonIt != m_BoneMapping.end()) {
			node.mBone = bonIt->second;
		}
		node.father = father;
		node.mTransformation = VQS(Convert(ainode->mTransformation));
		if (father>=0)	
			mNodes[father].bChildren.push_back(index);

		const int numAnim = m_pScene->mNumAnimations;
		for (int z = 0; z < numAnim; ++z) {
			auto anim = m_pScene->mAnimations[z];

			const aiNodeAnim* nodeAnim = FindNodeAnim(anim, node.mName);
			if (nodeAnim) {
				node.mAnimations.push_back(GetVQSFromAnimation(nodeAnim,node));
			}
			else {
				node.mAnimations.emplace_back();//if not, just put placeholder to keep the order of animation
			}
		}

		for (unsigned z = 0; z < ainode->mNumChildren; ++z) {		//goes through all the children recursivelly
			ExtractNodesInformation(m_pScene,ainode->mChildren[z], index);
		}
	}


	/*Gets the next closest time in the keyframes of the model*/
	float SkinnedMesh::GetNextKeyframeTime(const aiNodeAnim* nodeAnim,int scaleIndex,int positionIndex,int rotationIndex, int states,float time) {
		float nextTime = 0.0f;	//smallerTime is first keyframe time in this iteration, either on scale, position or rotation, in this order.
		float scaleTime = states & 1 ? std::numeric_limits<float>::infinity() : (float)nodeAnim->mScalingKeys[scaleIndex].mTime;			//if the scale is already without keyframes, put next scale keyframe to infiity, so that it wont be reached
		float positionTime = states & 2 ? std::numeric_limits<float>::infinity() : (float)nodeAnim->mPositionKeys[positionIndex].mTime;
		float rotationTime = states & 4 ? std::numeric_limits<float>::infinity() : (float)nodeAnim->mRotationKeys[rotationIndex].mTime;

		if (scaleTime>time) nextTime = scaleTime;
		if (positionTime>time && positionTime<nextTime) nextTime = positionTime;
		if (rotationTime>time && rotationTime<nextTime) nextTime = rotationTime;
		return nextTime;
	}

	/*Recovers the keyframes from the model,
	if a keyframe only modifies one component at certain time, this function creates an interpolation of the other 
	componentos to complete the vqs*/
	std::vector<Keyframe> SkinnedMesh::GetVQSFromAnimation(const aiNodeAnim* nodeAnim, Node& node) {
		std::vector<Keyframe> result;
		if (nodeAnim->mNumScalingKeys == 0)	//there is not animation information
			return result;

		unsigned scaleIndex = 0, positionIndex = 0, rotationIndex = 0;	//recovers all the information of the keyframes for this animation. Keyframes are split in
		float time=-1.0f;	//initialize it with an invalid value
		int states = 0;		//bitset array for state of keyframes. I the state is equal to 7, (111), the rotation, scale and position dont have more keyframes
		bool modified = false;
		float firstTime = GetNextKeyframeTime(nodeAnim, 0, 0, 0, 0, 0);		//gets the time of the first keyframe on the model
		if (firstTime > 0) {
			modified = true;
			result.emplace_back(0.0f,node.mTransformation); //if there is not a keyframe for the beginning of the animation, activate a flag to wrap with the last keyframe provided
		}

		do  {
			float nextTime = GetNextKeyframeTime(nodeAnim, scaleIndex, positionIndex, rotationIndex, states, time);	//smallerTime is first keyframe time in this iteration, either on scale, position or rotation, in this order.
			VQS vqs;
			vqs.v = CalcInterpolatedPosition(nextTime, nodeAnim,positionIndex);
			vqs.q = CalcInterpolatedRotation(nextTime, nodeAnim,rotationIndex);
			vqs.s = CalcInterpolatedScaling(nextTime, nodeAnim,scaleIndex);//vqs only supports 1 channel for scale, therefore I will take the maximum. That works for my models
			result.emplace_back(nextTime,vqs);	//creates the vqs for this keyframe

			time = nextTime;//assures advance to next keyframe

			if ( scaleIndex < nodeAnim->mNumScalingKeys - 1) ++scaleIndex; else states |= 1;	//advance next scaleIndex
			if ( positionIndex < nodeAnim->mNumPositionKeys - 1) ++positionIndex; else states |= 2;
			if ( rotationIndex < nodeAnim->mNumRotationKeys - 1) ++rotationIndex; else states |= 4;

		} while (states != 7);
		if (modified)
			result[0].vqs = result.back().vqs;	// if the first key is not provided, wrap last key to the first one.

		/*Calculate factors for incremental interpolation*/
		for (unsigned z = 0; z < result.size()-1; ++z) {
			Keyframe & key = result[z];
			key.ivqs=VQS::ExtFactIncInterpol(key.vqs, result[z+1].vqs,2);	//n=2 (spacing)
		}

		return result;
	}


	unsigned SkinnedMesh::FindKeyframe(const keyframestype& animations,float AnimationTime)
	{
		for (unsigned i = 0; i < animations.size(); ++i) {
			if (AnimationTime < animations[i + 1].time) {
				return i;
			}
		}
		throw;
		return 0;
	}

	/*Initialize all the model information that will be sent to the shaders, (normals, vertices, weights, UV, textures, etc*/
	bool SkinnedMesh::InitFromScene(const aiScene* pScene, const string& Filename)
	{
		m_Entries.resize(pScene->mNumMeshes);
		m_Textures.resize(pScene->mNumMaterials);

		vector<glm::vec3> Positions;
		vector<glm::vec3> Normals;
		vector<glm::vec2> TexCoords;
		vector<VertexBoneData> Bones;
		vector<unsigned> Indices;

		unsigned NumVertices = 0;
		unsigned NumIndices = 0;

		// Count the number of vertices and indices
		for (unsigned i = 0; i < m_Entries.size(); i++) {
			m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
			m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
			m_Entries[i].BaseVertex = NumVertices;
			m_Entries[i].BaseIndex = NumIndices;

			NumVertices += pScene->mMeshes[i]->mNumVertices;
			NumIndices += m_Entries[i].NumIndices;
		}

		// Reserve space in the vectors for the vertex attributes and indices
		Positions.reserve(NumVertices);
		Normals.reserve(NumVertices);
		TexCoords.reserve(NumVertices);
		Bones.resize(NumVertices);
		Indices.reserve(NumIndices);

		// Initialize the meshes in the scene one by one
		for (unsigned i = 0; i < m_Entries.size(); i++) {
			const aiMesh* paiMesh = pScene->mMeshes[i];
			InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
		}

		InitMaterials(pScene, Filename);

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(TEX_COORD_LOCATION);
		glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(BONE_ID_LOCATION);
		glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);

		glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
		glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

		return true;
	}

	/*The model can have several meshes, this function populate the vertices with this information*/
	void SkinnedMesh::InitMesh(unsigned MeshIndex,
		const aiMesh* paiMesh,
		vector<glm::vec3>& Positions,
		vector<glm::vec3>& Normals,
		vector<glm::vec2>& TexCoords,
		vector<VertexBoneData>& Bones,
		vector<unsigned>& Indices)
	{
		const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		// Populate the vertex attribute vectors
		for (unsigned i = 0; i < paiMesh->mNumVertices; i++) {
			const aiVector3D* pPos = &(paiMesh->mVertices[i]);
			const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
			const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

			Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
			Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
			TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
		}

		LoadBones(MeshIndex, paiMesh, Bones);

		// Populate the index buffer
		for (unsigned i = 0; i < paiMesh->mNumFaces; i++) {
			const aiFace& Face = paiMesh->mFaces[i];
			assert(Face.mNumIndices == 3);
			Indices.push_back(Face.mIndices[0]);
			Indices.push_back(Face.mIndices[1]);
			Indices.push_back(Face.mIndices[2]);
		}
	}


	/*For every mesh, fills the bone information and populates the bone array with the boneInfor for every bone*/
	void SkinnedMesh::LoadBones(unsigned MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones)
	{
		for (unsigned i = 0; i < pMesh->mNumBones; i++) {
			unsigned BoneIndex = 0;
			string BoneName(pMesh->mBones[i]->mName.data);

			/*If the bone does not exists in the array of boneInfo, create a new entry with its information*/
			if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
				BoneIndex = m_NumBones;
				++m_NumBones;
				BoneInfo bi;
				m_BoneInfo.push_back(bi);
				BoneInfo & bone = m_BoneInfo[BoneIndex];
				bone.BoneOffset = Convert(pMesh->mBones[i]->mOffsetMatrix);
				bone.name = BoneName;
				bone.hash = Hash::GetId(BoneName);
				m_BoneMapping[BoneName] = BoneIndex;
				bone.index = BoneIndex;
			}
			else {
				BoneIndex = m_BoneMapping[BoneName];
			}

			for (unsigned j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
				unsigned VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
				float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
				Bones[VertexID].AddBoneData(BoneIndex, Weight);			//for every vertex, get the weight of the bones that affect it
			}
		}
	}

	/*Loads the textures for the model*/
	void SkinnedMesh::InitMaterials(const aiScene* pScene, const string& filename)
	{
		string dir = Utility::Directory::GetPath(filename);
		if (dir.size() == 0) dir = ".";
		// initialize the materials
		for (unsigned i = 0; i < pScene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = pScene->mMaterials[i];

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString Path;

				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					string name= Utility::Directory::GetNameFile(Path.data);
					string path = dir + "/" + name;

					m_Textures[i] = GE::Graphics::OpenGLGraphicsSystem::GetInstance().GetTexture(GE::Graphics::OpenGLGraphicsSystem::GetInstance().CreateTextureOGL(path));
					printf("%d - loaded texture '%s  %d %d'\n", i, path.c_str(), i, m_Textures[i]);
				}
			}
		}
	}

	/*
	Interpolation functions for creation of VQS
	*/

	glm::vec3 SkinnedMesh::CalcInterpolatedPosition(float AnimationTime, const aiNodeAnim* pNodeAnim, int positionIndex)
	{
		// if the animation only has one key or the time mathes the time of the keyframe, return the keyframe with interpolation
		if (pNodeAnim->mNumPositionKeys == 1 || pNodeAnim->mPositionKeys[positionIndex].mTime==AnimationTime) 	//dont interpolate if the keyframe has the element in the time
			return Convert(pNodeAnim->mPositionKeys[positionIndex].mValue);

		unsigned nextpositionIndex = (positionIndex + 1);
		assert(nextpositionIndex < pNodeAnim->mNumPositionKeys);
		float dt = (float)(pNodeAnim->mPositionKeys[nextpositionIndex].mTime - pNodeAnim->mPositionKeys[positionIndex].mTime);
		float t = fabs(AnimationTime - (float)pNodeAnim->mPositionKeys[positionIndex].mTime) / dt;
		assert(t >= 0.0f && t <= 1.0f);
		const glm::vec3 p0 = Convert(pNodeAnim->mPositionKeys[positionIndex].mValue);
		const glm::vec3 p1 = Convert(pNodeAnim->mPositionKeys[nextpositionIndex].mValue);
		return Lerp(p0,p1,t);
	}

	Quaternion SkinnedMesh::CalcInterpolatedRotation(float AnimationTime, const aiNodeAnim* pNodeAnim, int RotationIndex)
	{
		// if the animation only has one key or the time mathes the time of the keyframe, return the keyframe with interpolation
		if (pNodeAnim->mNumRotationKeys == 1 || pNodeAnim->mRotationKeys[RotationIndex].mTime == AnimationTime) 	//dont interpolate if the keyframe has the element in the time
			return Convert(pNodeAnim->mRotationKeys[RotationIndex].mValue);

		unsigned NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		float dt = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		float t = fabs(AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / dt;
		assert(t >= 0.0f && t <= 1.0f);
		const Quaternion p0 = Convert(pNodeAnim->mRotationKeys[RotationIndex].mValue);
		const Quaternion p1 = Convert(pNodeAnim->mRotationKeys[NextRotationIndex].mValue);
		return Quaternion::SLerp(p0, p1, t).Normalize();
	}


	float SkinnedMesh::CalcInterpolatedScaling(float AnimationTime, const aiNodeAnim* pNodeAnim,int ScalingIndex)
	{
		// if the animation only has one key or the time mathes the time of the keyframe, return the keyframe with interpolation
		if (pNodeAnim->mNumScalingKeys == 1 || pNodeAnim->mScalingKeys[ScalingIndex].mTime == AnimationTime) { 	//dont interpolate if the keyframe has the element in the time
			auto p0=Convert(pNodeAnim->mScalingKeys[ScalingIndex].mValue);
			const float s0 = max(p0.x, max(p0.y, p0.z));
			return s0;
		}

		unsigned NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float dt = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		float t = fabs(AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / dt;
		assert(t >= 0.0f && t <= 1.0f);
		const glm::vec3 p0 = Convert(pNodeAnim->mScalingKeys[ScalingIndex].mValue);
		const glm::vec3 p1 = Convert(pNodeAnim->mScalingKeys[NextScalingIndex].mValue);

		const float s0 = max(p0.x, max(p0.y, p0.z));		//only the maximum of the scales, that works for my models
		const float s1 = max(p1.x, max(p1.y, p1.z));

		return Elerp(s0, s1, t);
	}

	/*Interpolation of VQS*/

	VQS SkinnedMesh::CalcInterpolatedVQS(float AnimationTime, const keyframestype& keyframe)
	{
		// we need at least two values to interpolate...
		if (keyframe.size()==1) {
			return keyframe[0].vqs;
		}
		unsigned index = FindKeyframe(keyframe,AnimationTime);		//###I can get that from the index function, optimize later
		unsigned nextIndex = (index + 1);
		assert(nextIndex < keyframe.size());
		float dt = (float)(keyframe[nextIndex].time - keyframe[index].time);
		float t = fabs(AnimationTime - (float)keyframe[index].time) / dt;
		assert(t >= 0.0f && t <= 1.0f);
		const VQS& start = keyframe[index].vqs;
		const VQS& end = keyframe[nextIndex].vqs;
		switch (interpolationType){
		case 0: //complete
			return VQS::Interpolate(start, end, t);
		case 1: //incremental
			return VQS::InterpolateIncremental(start, end, t, keyframe[index].ivqs,2);
		default:
			return VQS::InterpolateIncrementalChevyshev(start, end, t);
		}
	}

	/*Fill the bones with the hirarchy information (father, children)*/
	void SkinnedMesh::FillBonesHirarchy(const aiScene* m_pScene) {
		aiNode* root = m_pScene->mRootNode;
		BoneInfo*cbi = GetBoneInfo(root);
		bool isDirectlyConected = false;
		if (cbi) {
			cbi->father = nullptr;
			cbi->isDirectlyConected = false;
			isDirectlyConected = true;
		}
		FillBonesHirarchy(m_pScene, root, cbi, isDirectlyConected);
	}

	/*Recursive call*/
	void SkinnedMesh::FillBonesHirarchy(const aiScene* m_pScene,aiNode*node, BoneInfo*father, bool isDirectlyConected) {
		const unsigned size = node->mNumChildren;
		for (unsigned i = 0; i < size; ++i) {
			bool chiIsDirectlyConected = false;
			auto children = node->mChildren[i];
			BoneInfo*cbi = GetBoneInfo(children);
			if (cbi) {
				cbi->father = father;
				cbi->isDirectlyConected = isDirectlyConected;
				chiIsDirectlyConected = true;
				if (father)
					father->children.push_back(cbi);
			}
			FillBonesHirarchy(m_pScene,children, cbi, chiIsDirectlyConected);
		}
	}

	/*Given a node of assimp, returns the bone that belongs to that node*/
	BoneInfo* SkinnedMesh::GetBoneInfo(aiNode* node) {
		std::string cname = node->mName.data;
		auto bone = m_BoneMapping.find(cname);
		if (bone != m_BoneMapping.end()) {
			int index = bone->second;
			return &m_BoneInfo[index];
		}
		return nullptr;
	}

	/*As part of the process of update of every frame, goes through the hirarchy of the bones, updating the matrices for the animation in the current time*/
	void SkinnedMesh::ReadNodeHeirarchy(float AnimationTime, const Node& node, const VQS& ParentTransform, int anim)
	{
		const Animation& pAnimation = mAnimations[anim];
		
		VQS transformation=node.mTransformation;

		const keyframestype & keyframes = node.mAnimations[anim];
		if (keyframes.size()) {		//if there are keyframes for animation, animate it
			transformation = CalcInterpolatedVQS(AnimationTime, keyframes);
		}

		VQS GlobalTransformation = ParentTransform * transformation;

		auto bone = m_BoneMapping.find(node.mName);
		if (bone != m_BoneMapping.end()) {
			unsigned BoneIndex = bone->second;
			auto& bi = m_BoneInfo[BoneIndex];
			bi.FinalTransformation = m_GlobalInverseTransform * GlobalTransformation.ToMat4() * bi.BoneOffset;
		}

		for (unsigned i = 0; i < node.bChildren.size();++i) {
			ReadNodeHeirarchy(AnimationTime, mNodes[node.bChildren[i]], GlobalTransformation, anim);
		}
	}

	/*Called every frame to update the matrices of the animation*/
	void SkinnedMesh::BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms, unsigned animation)
	{
		assert(animation >= 0 && animation < mAnimations.size());
		glm::mat4 Identity;
		float TicksPerSecond = mAnimations[animation].ticksPerSecond;
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, mAnimations[animation].duration);
		ReadNodeHeirarchy(AnimationTime, mNodes[0], VQS::Identity, animation);
		Transforms.resize(m_NumBones);
		for (unsigned i = 0; i < m_NumBones; i++) {
			BoneInfo & bi = m_BoneInfo[i];
			Transforms[i] = bi.FinalTransformation;
			bi.iBoneOffset = glm::inverse(bi.BoneOffset);
		}
	}

	/*Called every frame to update the matrices of the animation, Some old models have the whole animation together, therefore I need to give a range of keyframes for the animation*/
	void SkinnedMesh::BoneTransformRange(float TimeInSeconds, vector<glm::mat4>& Transforms, float firstTick,float lastTick, unsigned animation)
	{
		assert(animation >= 0 && animation < mAnimations.size());
		glm::mat4 Identity;
		float TicksPerSecond = mAnimations[animation].ticksPerSecond;
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, lastTick-firstTick)+firstTick;
		ReadNodeHeirarchy(AnimationTime, mNodes[0], VQS::Identity, animation);
		Transforms.resize(m_NumBones);
		for (unsigned i = 0; i < m_NumBones; i++) {
			BoneInfo & bi = m_BoneInfo[i];
			Transforms[i] = bi.FinalTransformation;
			bi.iBoneOffset = glm::inverse(bi.BoneOffset);
		}
	}

	/*In the current animation, find the list of keyframes that belong to NodeName node*/
	const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
	{
		for (unsigned i = 0; i < pAnimation->mNumChannels; i++) {
			const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

			if (string(pNodeAnim->mNodeName.data) == NodeName) {
				return pNodeAnim;
			}
		}
		return nullptr;
	}

	unsigned SkinnedMesh::NumBones() const
	{
		return m_NumBones;
	}

	/*Retrieves a constant vector to the animation information (name, time, ticks)*/
	const std::vector<Animation> SkinnedMesh::GetAnimations() const
	{
		return mAnimations;
	}

	void SkinnedMesh::Render(GE::Graphics::Shader* shader)
	{
		glBindVertexArray(m_VAO);

		for (unsigned i = 0; i < m_Entries.size(); i++) {
			const unsigned MaterialIndex = m_Entries[i].MaterialIndex;

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_Textures[MaterialIndex]);
			GLuint texture = glGetUniformLocation(shader->shader, "texture0");
			glUniform1i(texture, 1);

			glDrawElementsBaseVertex(GL_TRIANGLES,
				m_Entries[i].NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned) * m_Entries[i].BaseIndex),
				m_Entries[i].BaseVertex);
		}

		glBindVertexArray(0);
	}

	void SkinnedMesh::RenderBones(glm::mat4 M, GE::Graphics::Shader * shader, int va, int sizeva)
	{
		const int size = m_BoneInfo.size();
		std::vector<glm::vec3> points(size);			//coordinates of the bones

		for (int z = 0; z < size; ++z) {
			const BoneInfo& bi = m_BoneInfo[z];
			auto mm = bi.FinalTransformation * bi.iBoneOffset;
			points[z] = (glm::vec3(mm[3][0], mm[3][1], mm[3][2]));		//from the bone matrix transformation, recovers the bone position at every time of the animation
		}

		for (int z = 0; z < size; ++z) {
			const BoneInfo& boneInfo = m_BoneInfo[z];
			const glm::vec3 bonePosition = points[z];
			for (unsigned c = 0; c < boneInfo.children.size(); ++c) {		//draws the bones from the father bone to the children
				const int childrenIndex = boneInfo.children[c]->index;
				const glm::vec3 childrenPos = points[childrenIndex];
				const glm::vec3 vect = childrenPos - bonePosition;
				const float longitude = glm::length(vect);	//distance between joints of bones

				GE::Quaternion q = GE::Quaternion::QuaternionRotation(glm::vec3(0, 1, 0), vect / longitude).Normalize(); //quaternion to rotate the model bone to the orientation of the animated bone

				glm::mat4 Ms = M*glm::translate(glm::mat4(), glm::vec3(bonePosition)) *
					q.ToMat4()*
					glm::scale(glm::mat4(), glm::vec3(1, longitude, 1));		//Ms is the matrix transformation for the bone, including the position, the orientation in this moment t and the scale to reach the joint of the son

				if (boneInfo.isDirectlyConected) shader->SetUniform("red", 0.5f); else shader->SetUniform("red", 1.0f);	//original bones are drawn in gray. Bones created by assimp are drawn in red.

				shader->SetUniform4mv("M", &Ms[0][0]);
				glBindVertexArray(va);
				glDrawElements(
					GL_TRIANGLES,      // mode
					sizeva,    // count
					GL_UNSIGNED_INT,   // type
					(void*)0           // element array buffer offset
					);
			}
		}
	}

	SkinnedMesh::SkinnedMesh():
		interpolationType(0), m_VAO(0), m_NumBones(0)
	{
		memset(m_Buffers, 0, sizeof(m_Buffers));
	}

	SkinnedMesh::~SkinnedMesh()
	{
		Clear();
	}

	void SkinnedMesh::Clear()
	{
		if (m_Buffers[0] != 0) {
			glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
		}

		if (m_VAO != 0) {
			glDeleteVertexArrays(1, &m_VAO);
			m_VAO = 0;
		}
	}


}