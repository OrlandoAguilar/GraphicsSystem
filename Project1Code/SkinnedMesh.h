#pragma once

#include <map>
#include <vector>
#include <assert.h>
#include <GL/glew.h>
#include <Importer.hpp>      // C++ importer interface
#include <scene.h>       // Output data structure
#include <postprocess.h> // Post processing flags
#include <GLM\glm.hpp>
#include "Shader.h"
#include "VQS.h"
#include "Keyframe.h"
#include "SkinnedMeshStructures.h"		//contains the definition of all the structures used by this class

namespace GE{
	class SkinnedMesh
	{
	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool LoadMesh(const std::string& Filename);
		void Render(GE::Graphics::Shader* shader);
		void RenderBones(glm::mat4 M, GE::Graphics::Shader * shader, int va, int sizeva);
		void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms, unsigned animation);

		void BoneTransformRange(float TimeInSeconds, std::vector<glm::mat4>& Transforms, float firstTick, float lastTick, unsigned animation);

		unsigned NumBones() const;
		const std::vector<Animation> GetAnimations() const;

		int interpolationType;
	private:

		GLuint m_VAO;
		GLuint m_Buffers[NUM_VBs];

		std::vector<MeshEntry> m_Entries;
		std::vector<unsigned> m_Textures;

		std::map<std::string, unsigned> m_BoneMapping; // maps a bone name to its index
		unsigned m_NumBones;
		std::vector<BoneInfo> m_BoneInfo;
		glm::mat4 m_GlobalInverseTransform;

		std::vector<Animation> mAnimations;
		std::vector<Node> mNodes;

		void ExtractAnimationInformation(const aiScene* m_pScene);
		void ExtractNodesInformation(const aiScene* m_pScene);
		void FillBonesHirarchy(const aiScene* m_pScene);
		void FillBonesHirarchy(const aiScene* m_pScene, aiNode * node, BoneInfo * father, bool isDirectlyConected);
		BoneInfo * GetBoneInfo(aiNode * node);

		float CalcInterpolatedScaling(float AnimationTime, const aiNodeAnim * pNodeAnim, int index);
		glm::vec3 CalcInterpolatedPosition(float AnimationTime, const aiNodeAnim * pNodeAnim, int index);
		Quaternion CalcInterpolatedRotation(float AnimationTime, const aiNodeAnim * pNodeAnim, int index);
		const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

		bool InitFromScene(const aiScene* pScene, const std::string& Filename);
		void InitMesh(unsigned MeshIndex,
			const aiMesh* paiMesh,
			std::vector<glm::vec3>& Positions,
			std::vector<glm::vec3>& Normals,
			std::vector<glm::vec2>& TexCoords,
			std::vector<VertexBoneData>& Bones,
			std::vector<unsigned int>& Indices);
		void LoadBones(unsigned MeshIndex, const aiMesh* paiMesh, std::vector<VertexBoneData>& Bones);
		void InitMaterials(const aiScene* pScene, const std::string& Filename);
		void Clear();

		void ExtractNodesInformation(const aiScene* m_pScene,const aiNode * ainode, int father);
		float GetNextKeyframeTime(const aiNodeAnim * nodeAnim, int scaleIndex, int positionIndex, int rotationIndex, int states, float time);
		keyframestype GetVQSFromAnimation(const aiNodeAnim * nodeAnim,Node&);
		unsigned FindKeyframe(const keyframestype & animations, float AnimationTime);
		void ReadNodeHeirarchy(float AnimationTime, const Node& pNode, const VQS& ParentTransform, int anim);
		VQS CalcInterpolatedVQS(float AnimationTime, const keyframestype& keyframe);
	};


}