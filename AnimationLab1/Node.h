#pragma once 

#include <glm\glm.hpp>
#include <vector>

#include "Helper.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Common.h"

class Node
{
	private:
		//glm::vec3 position;
		
	public:

		Model* marker;

		//For creating the box 
		static vector<Model*>* objectList;

		Node(glm::vec3 position, bool debug = true) 
		{
			//this->position = position;
			marker = new Model(position, glm::mat4(1), glm::vec3(.03f), BOX, ShaderManager::Instance->GetShaderProgramID("black"), false, true);
			
			if(debug)
				objectList->push_back(marker);
		};

		~Node()
		{
			delete marker;
		}

		glm::vec3 GetPosition()
		{
			//return position;
			return marker->worldProperties.translation;
		}

		void SetPosition(glm::vec3 pos)
		{
			//position = pos;

			//if display
			marker->worldProperties.translation = pos;
		}
};
