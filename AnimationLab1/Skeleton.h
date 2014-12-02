#ifndef _SKELETON_H                // Prevent multiple definitions if this 
#define _SKELETON_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <vector>
#include <map>

#include "helper.h"
#include "Bone.h"
#include "Common.h"

class Model;

class Skeleton
{
	private:
		
		Model* model;

		std::map<int, Bone*> bones;
		std::map<std::string, int> boneNameToID;

		int animationIndex;
		int numAnimations;

		double animationDuration;
		double animationTimer;

		std::vector<std::string> bonesAdded;

	public:
		Bone* root;

		bool hasKeyframes;
		std::map<std::string, std::vector<Bone*>> ikChains;
		GLuint line_vao;

		static bool ConstraintsEnabled;
		static float AnimationSpeedScalar;

		Skeleton(Model* myModel);
		virtual ~Skeleton();

		bool ImportAssimpBoneHierarchy(const aiScene* scene, aiNode* aiBone, Bone* parent, bool print = true);
		void Animate(double deltaTime);

		//void Control(bool *keyStates);
		
		bool ComputeIK(std::string chainName, glm::vec3 D, int steps);
		void DefineIKChain(std::string name, std::vector<Bone*> chain);
		void ImposeDOFRestrictions(Bone* bone);

		void UpdateGlobalTransforms(Bone* bone, glm::mat4 parentTransform);

		void PrintHeirarchy(Bone* root);
		void PrintAiHeirarchy(aiNode* root);

		bool LoadAnimation(const char* file_name);

		//Getters
		std::map<int, Bone*> GetBones() { return bones; }
		
		Bone* GetBone(int id) { return bones[id]; }
		Bone* GetBone(std::string name) { return bones[boneNameToID[name]]; }
		Bone* operator [](int i) { return bones[i]; }

		Bone* GetRootBone() { return root; }
		
		double GetAnimationTimer() { return animationTimer; }

		//Setters
		void SetAnimDuration(double pAnimationDuration) { animationDuration = pAnimationDuration; }

		void SetAnimation(int index) { if(animationIndex < numAnimations) animationIndex = index; }
};

#endif