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
#include <queue>

#include "helper.h"
#include "Bone.h"
#include "Common.h"

#include "Animation.h"

class Model;

struct Pose
{
	glm::vec3 translation;
	glm::quat orientation;
	float weight;
};

enum TransitionType { Smooth = 0, Frozen, Immediate };

struct AnimationCommand
{
	Animation* animation;
	float blendDuration;
	TransitionType transitionType;
};

struct AnimationController
{
	bool isBlending;
	
	float blendTimer;
	float blendDuration;

	Animation* current;
	Animation* next;

	TransitionType transitionType;

	std::queue<AnimationCommand> commandQueue; //FIFO

	AnimationController()
	{
		isBlending = false;
		blendTimer = 0.0;
		blendDuration = 0.0;

		transitionType = TransitionType::Smooth;
	}

	void StartBlend(Animation* p_prev, Animation* p_next, float p_blendDuration, TransitionType p_transitionType)
	{
		isBlending = true;
		transitionType = p_transitionType;
		
		current = p_prev;

		if(transitionType == TransitionType::Frozen)
			current->frozen = true; 
		
		next = p_next;

		next->frozen = false;
		next->localClock = 0.0;
		next->weight = 0;

		blendDuration = p_blendDuration;
	}

	void Update(double deltaTime)
	{
		if(isBlending)
		{
			blendTimer += deltaTime/1000;
			float t = blendTimer / blendDuration;

			next->weight = lerp(0.0f, 1.0f, t);
			current->weight = 1 - next->weight;

			if(t >= 1)
			{
				next->weight = 1;

				current->Stop();

				isBlending = false;
				blendTimer = 0.0;
				blendDuration = 0.0;
			}
		}
		else
		{

		}
	}
};

class Skeleton
{
	private:
		
		Model* model;
		AnimationController animationController;

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
		std::map<int, std::vector<Pose>> SampleKeyframes();

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

		void PrintOuts(int winw, int winh);

		void SetAnimation(int index, float blendDuration = 0, TransitionType transitionType = TransitionType::Immediate) //blendMode smooth
		{ 
			if(index < animations.size()) 
			{
				if(transitionType == TransitionType::Immediate)
				{
					if(currentAnimationIndex != -1)
					animations[currentAnimationIndex]->Stop();

					animations[index]->Start();
				
					currentAnimationIndex = index;
				}
				else
				{
					if(currentAnimationIndex == -1)
						return; //Need an animation to blend from!

					animationController.StartBlend(animations[currentAnimationIndex], animations[index], blendDuration, transitionType);
				
					currentAnimationIndex = index;
				}
			}
		}
};

#endif