#ifndef _OBJ3D_H                // Prevent multiple definitions if this 
#define _OBJ3D_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <vector>

#include "Bone.h"
#include "helper.h"
#include "Skeleton.h"

using namespace std;

enum VB_TYPES 
{
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	BONE_VB,
	WEIGHT_VB,
	INDEX_VB,
	NUM_VBs            
};

struct WorldProperties
{
	glm::vec3 translation;
	glm::vec3 rotationAxis;
	float rotationDegrees;
	glm::vec3 scale;

	WorldProperties()
	{
		translation = glm::vec3(0);
		rotationAxis = glm::vec3(0);
		rotationDegrees = 0;
		scale = glm::vec3(0);
	}
};

struct VertexBoneData
{ 
    float boneID[4];
    float weightAmount[4];

	void AddBoneData(int BoneID, float Weight) 
	{
		for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(boneID); i++) 
		{
			if (weightAmount[i] == 0.0) //if it hasn't been assigned yet
			{
				boneID[i]     = BoneID;
				weightAmount[i] = Weight;
				return;
			}        
		}
    
		// should never get here - more bones than we have space for
		assert(0);
	}
};

class Mesh
{
	private:

		const aiScene* scene;
		const aiMesh* mesh;
		Skeleton* skeleton;

		int vertexCount;
		GLuint shaderProgramID;

		GLuint vao;
		GLuint buffers[NUM_VBs];

		//Every vertex in this Mesh has a boneID(s) - which bone(s) it is influenced by
		GLfloat* vertexBoneIDs; 
		
	public:
		vector<int> Indices;

		WorldProperties worldProperties;
		
		Mesh(glm::vec3 position, glm::vec3 rotationAxis, float rotationDegrees, glm::vec3 scale, const char* file_name);
		~Mesh();

		bool Load(const char* file_name);

		//Getters
		GLuint GetVAO() { return vao; }
		GLuint GetShaderProgramID() { return shaderProgramID; }
		int GetVertexCount() { return vertexCount; }
		Skeleton* GetSkeleton() { return skeleton; }
		
		glm::mat4 GetModelMatrix() { 
			return glm::translate(glm::mat4(1.0f), worldProperties.translation) 
				* glm::rotate(glm::mat4(1.0f), worldProperties.rotationDegrees, worldProperties.rotationAxis) 
				* glm::scale(glm::mat4(1.0f), worldProperties.scale);
		}		

		//Setters
		void SetShaderProgramID(GLuint p_shaderProgramID) { shaderProgramID = p_shaderProgramID; }
};

#endif