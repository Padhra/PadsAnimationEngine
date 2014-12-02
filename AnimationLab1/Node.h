#pragma once 

#include <glm\glm.hpp>
#include <vector>
#include "Helper.h"
#include "Model.h"
#include "ShaderManager.h"

#define BOX "Models/cubeTri.obj" //TODO - move definitions to a common.h

class Node
{
	private:

	public:

		//For creating the box 
		static vector<Model*>* objectList;
		
		Model* box;

		Node(glm::vec3 position) 
		{
			box = new Model(position, glm::mat4(1), glm::vec3(.03f), BOX, ShaderManager::Instance->GetShaderProgramID("diffuse"));
			objectList->push_back(box);
		};

		~Node()
		{
			//delete box; //Clean this up in model ?
		}

		glm::vec3 GetPosition()
		{
			return box->worldProperties.translation;
		}
};
