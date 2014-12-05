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

struct PosKeyFrame
{
	glm::vec3 position;
	double time;
};

struct RotKeyFrame
{
	glm::quat rotation;
	double time;
};

struct BoneAnimation
{
	int boneID;
	std::vector<PosKeyFrame*> posKeyframes;
	std::vector<RotKeyFrame*> rotKeyframes;
};

struct Animation {

	int animationID;

	double localClock;
	double duration;

	std::vector<BoneAnimation*> boneAnimations;

	float weight;
	bool frozen;
	
	Animation(int p_id, double p_duration)
	{
		animationID = p_id;
		duration = p_duration;

		localClock = 0.0;
		weight = 0.0; //How much is it contributing to the pose?
		frozen = true; //Is the clock running?
	}

	void Start()
	{
		localClock = 0.0;
		weight = 1.0; 
		frozen = false; 
	}

	void Stop()
	{
		localClock = 0.0;
		weight = 0.0;
		frozen = true; 
	}
};

enum TransitionType { Smooth = 0, Frozen };

struct WeightBlender
{
	bool active;
	float blendTimer;
	float blendDuration;

	Animation* prev;
	Animation* next;

	TransitionType transitionType;

	WeightBlender()
	{
		active = false;
		blendTimer = 0.0;
		blendDuration = 0.0;

		transitionType = TransitionType::Smooth;
	}

	void Start(Animation* p_prev, Animation* p_next, float p_blendDuration, TransitionType p_transitionType)
	{
		active = true;
		transitionType = p_transitionType;
		
		prev = p_prev;

		if(transitionType == TransitionType::Frozen)
			prev->frozen = true; 
		
		next = p_next;

		next->frozen = false;
		next->localClock = 0.0;
		next->weight = 0;

		blendDuration = p_blendDuration;
	}

	void Blend(double deltaTime)
	{
		blendTimer += deltaTime/1000;
		float t = blendTimer / blendDuration;

		next->weight = lerp(0.0f, 1.0f, t);
		prev->weight = 1 - next->weight;

		if(t >= 1)
		{
			next->weight = 1;

			prev->Stop();

			active = false;
			blendTimer = 0.0;
			blendDuration = 0.0;
		}
	}
};

class Skeleton
{
	private:
		
		Model* model;
		WeightBlender blender;

		std::map<int, Bone*> bones;
		std::map<std::string, int> boneNameToID;
		std::vector<std::string> bonesAdded;

		std::vector<Animation*> animations;
		int currentAnimationIndex;

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

		//Setters
		//void SetAnimDuration(double pAnimationDuration) { animationDuration = pAnimationDuration; }

		void SetAnimationImmediate(int index) 
		{ 
			if(index < animations.size()) 
			{
				if(currentAnimationIndex != -1)
					animations[currentAnimationIndex]->Stop();

				animations[index]->Start();
				
				currentAnimationIndex = index;
			}
		}

		void SetAnimationGradual(int index, float blendDuration) //blendMode frozen by default
		{ 
			if(index < animations.size()) 
			{
				if(currentAnimationIndex == -1)
					return; //Need an animation to blend from!

				blender.Start(animations[currentAnimationIndex], animations[index], blendDuration, TransitionType::Frozen);
				
				currentAnimationIndex = index;
			}
		}

		void PrintOuts(int winw, int winh);
};

#endif