#ifndef _BONE_H                // Prevent multiple definitions if this 
#define _BONE_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <glm\glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

#include <list>
#include <vector>

#include "Helper.h"
#include "Common.h"

struct DOFLimits
{
	bool xAxis;
	float xMin, xMax;
	
	bool yAxis;
	float yMin, yMax;
	
	bool zAxis;
	float zMin, zMax;

	DOFLimits() : xAxis(false), yAxis(false), zAxis(false) {}

	void SetXLimits(float min, float max)
	{
		xAxis = true;
		xMin = min;
		xMax = max;
	}

	void SetYLimits(float min, float max)
	{
		yAxis = true;
		yMin = min;
		yMax = max;
	}

	void SetZLimits(float min, float max)
	{
		zAxis = true;
		zMin = min;
		zMax = max;
	}
};

class Bone
{
	private:

	public:

		char name[1024];
		GLuint id; 

		Bone* parent; 
		std::vector<Bone*> children;

		glm::mat4 offset;
		glm::mat4 inv_offset;

		glm::mat4 transform;
		glm::mat4 finalTransform;
		glm::mat4 globalTransform;

		DOFLimits dofLimits;

		bool applyKeyframeFlag;

		glm::vec3 GetMeshSpacePosition() 
		{
			glm::mat4 matAbs = finalTransform * inv_offset; 
			
			return glm::vec3
			(
				matAbs[3][0], 
				matAbs[3][1],
				matAbs[3][2]
			);
		}

		glm::vec3 GetEulerAngles()
		{
			glm::vec3 translation;
			glm::mat4 rotation; 
			glm::vec3 scaling;
	
			decomposeTRS(transform, translation, rotation, scaling); 

			return glm::eulerAngles(glm::toQuat(rotation));
		}
};

#endif