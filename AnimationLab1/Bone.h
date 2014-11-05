#ifndef _BONE_H                // Prevent multiple definitions if this 
#define _BONE_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <glm\glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>
#include <vector>

#include "Helper.h"

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

struct ScaleKeyFrame
{
	glm::vec3 scale;
	double time;
};

struct Weight
{
	int vertexID;
	int weighting;
};

class Bone
{
	private:

	public:

		char name[50];
		GLfloat id; 

		aiBone* aibone;

		Bone* parent; 
		std::vector<Bone*> children;

		//std::vector<Weight> weights;

		glm::mat4 offset;
		glm::mat4 inv_offset;

		glm::mat4 localTransform;
		glm::mat4 globalTransform;

		std::vector<PosKeyFrame*> posKeyframes;
		std::vector<RotKeyFrame*> rotKeyframes;
		std::vector<ScaleKeyFrame*> scaleKeyframes;

		glm::vec3 GetMeshSpacePosition() 
		{
			//glm::vec3 worldPosition;
			//decomposeTRS(globalTransform, worldPosition, glm::mat4(1), glm::vec3(1));

			//return worldPosition;
			
			///*return glm::vec3
			//(
			//	globalTransform[3][0], 
			//	globalTransform[3][1],
			//	globalTransform[3][2]
			//)*/

			glm::mat4 matAbs = globalTransform * inv_offset; 
			
			return glm::vec3
			(
				matAbs[3][0], 
				matAbs[3][1],
				matAbs[3][2]
			);
		}
};

#endif