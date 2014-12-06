#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include <vector>

#include "Helper.h"
#include "Common.h"

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

struct BoneAnimationData
{
	int boneID;
	std::vector<PosKeyFrame*> posKeyframes;
	std::vector<RotKeyFrame*> rotKeyframes;
};

struct Animation {

	int animationID;
	std::string name;

	double localClock;
	double duration;

	std::vector<BoneAnimationData*> animationData;

	float weight;
	bool frozen;
	
	Animation(std::string name, int id, double duration)
	{
		this->name = name;
		animationID = id;
		this->duration = duration;

		localClock = 0.0;
		weight = 0.0; //How much is it contributing to the pose?
		frozen = true; //Is the clock running?
	}

	void Start(float weight)
	{
		localClock = 0.0;
		this->weight = weight; 
		frozen = false; 
	}

	void Stop()
	{
		localClock = 0.0;
		weight = 0.0;
		frozen = true; 
	}
};