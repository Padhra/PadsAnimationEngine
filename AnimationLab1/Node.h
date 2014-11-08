#pragma once 

#include <glm\glm.hpp>
#include <vector>
#include "Helper.h"
#include "Model.h"
#include "ShaderManager.h"

#define BOX "Models/cubeTri.obj"

class Node
{
	private:

	public:

		static ShaderManager* shaderManager;
		static vector<Model*>* objectList;

		Node(glm::vec3 position) 
		{
			box = new Model(position, glm::mat4(1), glm::vec3(.03), BOX, shaderManager->GetShaderProgramID("diffuse"));

			objectList->push_back(box);
		};

		glm::vec3 GetPosition()
		{
			return box->worldProperties.translation;
		}

		Model* box;

};
