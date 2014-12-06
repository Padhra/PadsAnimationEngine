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

	double localClock;
	double duration;

	std::vector<BoneAnimationData*> animationData;

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