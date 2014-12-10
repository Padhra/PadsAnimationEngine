#pragma once

#include "Model.h"
#include "Helper.h"
#include "Common.h"
#include "ShaderManager.h"

#include "Spline.h"
#include "Camera.h"

#include <fstream>

#include "Keys.h"

class SplineEditor
{
	public:

	Model* tester;

	Spline spline;
	int selectedNode;
	
	int option;

	bool altDirectional;
	float translationSpeed;

	float fileSelect;

	Camera *camera;

	SplineEditor(Camera* camera)
	{
		option = 1;
		altDirectional = false;

		translationSpeed = 0.01f; 
		fileSelect = 0;

		tester = new Model(glm::vec3(0), glm::mat4(1), glm::vec3(.06f), BOX, ShaderManager::Instance->GetShaderProgramID("black"), false, true);
		Node::objectList->push_back(tester);
		tester->drawMe = false;

		this->camera = camera;
	}

	~SplineEditor()
	{
		delete tester;
	};

	void ProcessKeyboardContinuous(bool* keyStates, bool* directionKeys, double deltaTime)
	{
		if(option == 2)
		{
			if(spline.nodes.size() > 0)
			{
				Node* node = spline.nodes[selectedNode % spline.nodes.size()];

				if(directionKeys[DKEY::Left])
					node->SetPosition(node->GetPosition() += glm::vec3(-1,0,0) * translationSpeed * float(deltaTime));
				if(directionKeys[DKEY::Right])
					node->SetPosition(node->GetPosition() += glm::vec3(1,0,0) * translationSpeed * float(deltaTime));

				if(altDirectional)
				{
					if(directionKeys[DKEY::Up])
						node->SetPosition(node->GetPosition() += glm::vec3(0,1,0) * translationSpeed * float(deltaTime));
					if(directionKeys[DKEY::Down])
						node->SetPosition(node->GetPosition() += glm::vec3(0,-1,0) * translationSpeed * float(deltaTime));
				}
				else
				{
					if(directionKeys[DKEY::Up])
						node->SetPosition(node->GetPosition() += glm::vec3(0,0,-1) * translationSpeed * float(deltaTime));
					if(directionKeys[DKEY::Down])
						node->SetPosition(node->GetPosition() += glm::vec3(0,0,1) * translationSpeed * float(deltaTime));
				}
			}	
		}
	}

	void ProcessKeyboardOnce(unsigned char key, int x, int y)
	{
		if(key == KEY::KEY_SPACE)
			altDirectional = !altDirectional;

		if(key == KEY::KEY_1)
			option = 1; //Node select
		else if(key == KEY::KEY_2)
			option = 2; //Node move
		else if(key == KEY::KEY_5)
			option = 5; //select file

		if(key == KEY::KEY_3) // Add Node
		{
			/*if(spline.nodes.size() == 0)
			{
				spline.AddNode(new Node(glm::vec3(0,0,0)));
			}
			else
			{
				spline.nodes[selectedNode % spline.nodes.size()]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("black"));
				spline.AddNode(new Node(spline.nodes[selectedNode % spline.nodes.size()]->GetPosition() + glm::vec3(0,0.5,0)));
			}*/
			spline.AddNode(new Node(camera->viewProperties.position));

			//selectedNode = spline.nodes.size()-1;
			//spline.nodes[selectedNode % spline.nodes.size()]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("red"));
		}
		else if(key == KEY::KEY_4) //Delete Node
		{
			if(spline.nodes.size() > 0)
			{
				Node* node = spline.nodes[selectedNode % spline.nodes.size()];
				
				//if visible
				Node::objectList->erase(std::remove(Node::objectList->begin(), Node::objectList->end(), node->marker), Node::objectList->end());
				
				spline.DeleteNode(node);
			}
		}
		else if(key == KEY::KEY_6)
		{
			spline.Save(fileSelect);
		}
		else if(key == KEY::KEY_7)
		{
			spline.Load(fileSelect);
		}

		if(option == 1)
		{
			if(key == GLUT_KEY_LEFT)
			{
				selectedNode--;

				if(spline.nodes.size() > 0)
				{
					for(int i = 0; i < spline.nodes.size(); i++)
						spline.nodes[i]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("black"));
					
					spline.nodes[selectedNode % spline.nodes.size()]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("red"));
				}
			}
			else if(key == GLUT_KEY_RIGHT)
			{
				selectedNode++;

				if(spline.nodes.size() > 0)
				{
					for(int i = 0; i < spline.nodes.size(); i++)
						spline.nodes[i]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("black"));
					
					spline.nodes[selectedNode % spline.nodes.size()]->marker->SetShaderProgramID(ShaderManager::Instance->GetShaderProgramID("red"));
				}
			}
		}
		else if(option == 5)
		{
			if(key == GLUT_KEY_LEFT)
			{
				fileSelect--;
				if(fileSelect < 0)
					fileSelect = 0;
			}
			else if(key == GLUT_KEY_RIGHT)
			{
				fileSelect++;
			}
		}
	}

	void PrintOuts(int winw, int winh)
	{
		int numUIEntries = 9+1;

		std::stringstream ss;
		ss.str(std::string()); // clear
		ss << "Spline Editor";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH), numUIEntries*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|1| Selected Node - ";
		if(spline.nodes.size() > 0)
			ss << selectedNode % spline.nodes.size();
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH), (numUIEntries-1)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|2| Translation";
		if(spline.nodes.size() > 0)
		{
			Node* node = spline.nodes[selectedNode % spline.nodes.size()];

			glm::vec3 pos;
			pos = node->GetPosition();
			ss << "(x: " << pos.x << ", y: " << pos.y << ", z: " << pos.z << ")";
		}
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH), (numUIEntries-2)*20, ss.str().c_str());
	
		ss.str(std::string()); // clear
		ss << "|3| Add Node";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH), (numUIEntries-3)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|4| Delete Node";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH), (numUIEntries-4)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|5| fileSelect - " << fileSelect;
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH),(numUIEntries-5)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|6| Save file ";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH),(numUIEntries-6)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|7| Load file ";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH),(numUIEntries-7)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		glm::vec3 testPos = tester->worldProperties.translation;
		ss << "Tester: (x: " << testPos.x << ", y: " << testPos.y << ", z: " << testPos.z << ")";
		drawText(winw-(strlen(ss.str().c_str())*LETTER_WIDTH),(numUIEntries-8)*20, ss.str().c_str());

	}
};