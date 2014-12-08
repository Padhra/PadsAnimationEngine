#pragma once

#include "Model.h"
#include "Helper.h"
#include "Common.h"
#include "ShaderManager.h"

#include <fstream>

#include "Keys.h"

class LevelEditor // This class serialise the level / loads the level and creates the objects
{
	public:

	vector<Model*> *objectList;
	
	int option;
	int axis;

	static const glm::vec3 rotationAxes[3];

	int selectedObject;

	float translationSpeed;
	float rotationSpeed;
	float scaleSpeed;

	float fileSelect;

	LevelEditor(vector<Model*> *objectList);
	~LevelEditor(){};

	void ProcessKeyboardContinuous(bool* keyStates, bool* directionKeys, double deltaTime);
	void ProcessKeyboardOnce(unsigned char key, int x, int y);

	void PrintOuts(int winw, int winh);

	static void Save(vector<Model*> objects, int file)
	{
		std::stringstream ss;
		ss << "Levels/level" << file << ".txt";
		ofstream outfile (ss.str());

		bool firstEntry = true;
		
		if (outfile.is_open())
		{
			for(int i = 0; i < objects.size(); i++)
			{
				if(objects[i]->serialise == true)
				{
					if(!firstEntry)
						outfile << "\n";
					
					firstEntry = false;

					outfile << objects[i]->GetFileName() << "\n";

					outfile << ShaderManager::Instance->GetShaderProgramName(objects[i]->GetShaderProgramID()) << "\n";

					WorldProperties wp = objects[i]->worldProperties;

					outfile << wp.translation.x << "\n";
					outfile << wp.translation.y << "\n";
					outfile << wp.translation.z << "\n";
				
					glm::quat q = glm::toQuat(wp.orientation);

					outfile << q[0] << "\n";
					outfile << q[1] << "\n";
					outfile << q[2] << "\n";
					outfile << q[3] << "\n";

					outfile << wp.scale.x;
				}
			}

			outfile.close();
		}
	}

	static vector<Model*> Load(int file)
	{
		ifstream infile;

		std::stringstream ss;
		ss << "Levels/level" << file << ".txt";
		infile.open (ss.str(), ifstream::in);

		std::vector<Model*> objects;

		while (infile.good()) 
		{   
			string fileName;
			getline(infile, fileName);

			string shaderName;
			getline(infile, shaderName);

			glm::vec3 translation = glm::vec3();
			for(int i = 0; i < 3; i++)
			{
				string s;
				getline(infile, s);
				translation[i] = std::stof(s);
			}

			glm::quat orientation = glm::quat();
			for(int i = 0; i < 4; i++)
			{
				string s;
				getline(infile, s);
				orientation[i] = std::stof(s);
			}

			string scaleStr;
			getline(infile, scaleStr);

			float scale = std::stof(scaleStr);

			objects.push_back(new Model(translation, glm::toMat4(orientation), glm::vec3(scale), fileName.c_str(), ShaderManager::Instance->GetShaderProgramID(shaderName)));
		}
          
		infile.close();

		return objects;
	}
};