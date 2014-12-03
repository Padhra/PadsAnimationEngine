#pragma once

#include "Model.h"
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm\gtx\quaternion.hpp>

class LevelSaver // This class serialise the level / loads the level and creates the objects
{
	public:

	void Save(vector<Model*> objects)
	{
		std::stringstream ss;
		ss << "Levels/level" /*<< selectedFile */ << ".txt";
		ofstream outfile (ss.str());
		
		if (outfile.is_open())
		{
			for(int i = 0; i < objects.size(); i++)
			{
				outfile << objects[i]->GetFileName() << "\n";

				outfile << ShaderManager::Instance->GetShaderProgramName(objects[i]->GetShaderProgramID());

				WorldProperties wp = objects[i]->worldProperties;

				outfile << wp.translation.x << "\n";
				outfile << wp.translation.y << "\n";
				outfile << wp.translation.z << "\n";
				
				glm::quat q = glm::toQuat(wp.orientation);

				outfile << q[0] << "\n";
				outfile << q[1] << "\n";
				outfile << q[2] << "\n";
				outfile << q[3] << "\n";

				outfile << wp.scale.x << "\n";

				if(i != objects.size()-1)
					outfile << "\n";
			}

			outfile.close();
		}
	}

	vector<Model*> Load()
	{
		ifstream infile;

		std::stringstream ss;
		ss << "Levels/level" /*<< selectedFile */ << ".txt";
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