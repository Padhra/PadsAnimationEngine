#ifndef _OBJ3D_H                // Prevent multiple definitions if this 
#define _OBJ3D_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Common.h"

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

#include "Magick++.h"

using namespace std;

enum VB_TYPES 
{
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	//BONE_VB,
	WEIGHT_VB,
	INDEX_VB,
	NUM_VBs            
};

struct WorldProperties
{
	glm::vec3 translation;
	glm::mat4 orientation;
	glm::vec3 scale;

	WorldProperties()
	{
		translation = glm::vec3(0);
		orientation = glm::mat4(1);
		scale = glm::vec3(0);
	}
};

struct MeshEntry {
    
	MeshEntry()
    {
        NumIndices    = 0;
        BaseVertex    = 0;
        BaseIndex     = 0;
		TextureIndex = 0xFFFFFFFF;
    }
        
    unsigned int NumIndices;
    unsigned int BaseVertex;
    unsigned int BaseIndex;
	unsigned int TextureIndex; 
};

class Model
{
	private:

		std::string fileName;

		GLuint vao;
		vector<int> indices;

		vector<GLuint> textures;
		
		GLuint shaderProgramID;

		int vertexCount;

		Skeleton* skeleton;
		bool hasSkeleton;

		bool wireframe;

	public:
		
		bool serialise;
		bool drawMe;
		bool die;

		glm::mat4 globalInverseTransform;

		Model(glm::vec3 position, glm::mat4 orientation, glm::vec3 scale, const char* file_name, GLuint shaderProgramID, bool serialise = true, bool wireframe = false);
		~Model();

		WorldProperties worldProperties;
		vector<MeshEntry> meshEntries;

		bool Load(const char* file_name);
		
		void Render(GLuint shader);

		void Update(double deltaTime)
		{
			if(die)
			{
				worldProperties.scale.x /= (1.02f);
				worldProperties.scale.y /= (1.02f);
				worldProperties.scale.z /= (1.02f);

				worldProperties.orientation *= glm::rotate(glm::mat4(1), float(deltaTime/3), glm::vec3(0,0,1));

				if(worldProperties.scale.x <= 0.00005)
				{
					die = false;
					drawMe = false;
				}
			}
		}

		GLuint LoadTexture(const char* fileName);
		void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

		void LoadAnimation(const char* file_name) { if (hasSkeleton) skeleton->LoadAnimation(file_name); else std::cout << "\nCan't load an animation, there's no skeleton!\n"; }

		//Getters
		GLuint GetVAO() { return vao; }
		GLuint GetShaderProgramID() { return shaderProgramID; }
		int GetVertexCount() { return vertexCount; }
		Skeleton* GetSkeleton() { return skeleton; }
		bool HasSkeleton() { return hasSkeleton; }
		vector<int> GetIndices() { return indices; }

		std::string GetFileName() { return fileName; }
		
		glm::mat4 GetModelMatrix() 
		{ 
			return 
				glm::translate(glm::mat4(1.0f), worldProperties.translation) 
				* worldProperties.orientation
				* glm::scale(glm::mat4(1.0f), worldProperties.scale)
				* globalInverseTransform;
		}		

		glm::vec3 GetEulerAngles()
		{
			return glm::eulerAngles(glm::toQuat(worldProperties.orientation));
		}

		glm::vec3 GetForward()
		{
			return glm::toQuat(worldProperties.orientation) * glm::vec3(0,0,1);
		}

		//Setters
		void SetShaderProgramID(GLuint p_shaderProgramID) { shaderProgramID = p_shaderProgramID; }

};

#endif