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

#include <iostream>

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
	bool loop;

	AnimationCommand()
	{
	}

	AnimationCommand(Animation* animation, bool loop, float blendDuration, TransitionType transitionType)
	{
		this->animation = animation;
		this->loop = loop;
		this->blendDuration = blendDuration;
		this->transitionType = transitionType;
	}
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

		current = 0;
		next = 0;

		transitionType = TransitionType::Smooth;
	}

	void Reset()
	{
		isBlending = false;
		blendTimer = 0.0;
		blendDuration = 0.0;
	}

	void Update(double deltaTime)
	{
		if(isBlending) //If blending do that
		{
			blendTimer += deltaTime/1000;
			float t = blendTimer / blendDuration;

			next->weight = lerp(0.0f, 1.0f, t);
			current->weight = 1 - next->weight;

			if(t >= 1)
			{
				next->weight = 1; //could be a little higher than 1
				current->Stop();
				
				current = next;

				Reset();
			}
		}
		else //otherwise pop an animation off the command queue, if there is something in it
		{
			if(commandQueue.size() > 0)
			{
				AnimationCommand command = commandQueue.front();
				commandQueue.pop();

				if(command.animation != current)
				{
					transitionType = command.transitionType;

					if(transitionType == TransitionType::Immediate)
					{
						//if(current != 0)
							//current->Stop();
						//if(next != 0)
							//next->Stop();
						//commandQueue.empty();

						current = command.animation;
						current->Start(1, command.loop);
					}
					else
					{
						if(current != 0)
						{
							if(transitionType == TransitionType::Frozen)
								current->frozen = true; 
		
							next = command.animation;
							next->Start(0, command.loop);

							blendDuration = command.blendDuration;
							isBlending = true;
						}
					}
				}
			}
		}
	}
};

class Skeleton
{
	private:
		
		Model* model;
		
		std::map<int, Bone*> bones;
		std::map<std::string, int> boneNameToID;
		std::vector<std::string> bonesAdded;

		std::vector<Animation*> animations;

	public:
		Bone* root;
		AnimationController animationController;

		bool hasKeyframes;
		std::map<std::string, std::vector<Bone*>> ikChains;

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

		void AddToAnimationQueue(int index, bool loop = true, float blendDuration = 0, TransitionType transitionType = TransitionType::Immediate)
		{ 
			if(index < animations.size()) 
			{
				AnimationCommand command = AnimationCommand(animations[index], loop, blendDuration, transitionType);
				animationController.commandQueue.push(command);
			}
		}

		void PrintOuts(int winw, int winh);

		//Getters
		std::map<int, Bone*> GetBones() { return bones; }
		
		Bone* GetBone(int id) { return bones[id]; }
		Bone* GetBone(std::string name) { return bones[boneNameToID[name]]; }
		Bone* operator [](int i) { return bones[i]; }

		Bone* GetRootBone() { return root; }

};

#endif