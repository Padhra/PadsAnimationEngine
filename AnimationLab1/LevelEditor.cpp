#include "LevelEditor.h"

const glm::vec3 LevelEditor::rotationAxes[3] = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };

LevelEditor::LevelEditor(vector<Model*> *objectList)
{
	option = 1;
	axis = 0;

	selectedObject = 0;
	
	translationSpeed = 0.01f; 
	rotationSpeed = 0.01f;
	scaleSpeed = 0.000001f;

	this->objectList = objectList;

	fileSelect = 0;
}

void LevelEditor::ProcessKeyboardContinuous(bool* keyStates, bool* directionKeys, double deltaTime)
{
	if(option == 2 || option == 3)
	{
		if (directionKeys[DKEY::Left])
			objectList->at(selectedObject % objectList->size())->worldProperties.translation += glm::vec3(1,0,0) * translationSpeed * float(deltaTime);
		else if (directionKeys[DKEY::Right])
			objectList->at(selectedObject % objectList->size())->worldProperties.translation -= glm::vec3(1,0,0) * translationSpeed * float(deltaTime);

		if(option == 2)
		{
			if (directionKeys[DKEY::Up])
				objectList->at(selectedObject % objectList->size())->worldProperties.translation += glm::vec3(0,1,0) * translationSpeed * float(deltaTime);
			else if (directionKeys[DKEY::Down])
				objectList->at(selectedObject % objectList->size())->worldProperties.translation -= glm::vec3(0,1,0) * translationSpeed * float(deltaTime);
		}
		else if(option == 3)
		{
			if (directionKeys[DKEY::Up])
				objectList->at(selectedObject % objectList->size())->worldProperties.translation += glm::vec3(0,0,1) * translationSpeed * float(deltaTime);
			else if (directionKeys[DKEY::Down])
				objectList->at(selectedObject % objectList->size())->worldProperties.translation -= glm::vec3(0,0,1) * translationSpeed * float(deltaTime);
		}
	}

	if(option == 4)
	{
		if (directionKeys[DKEY::Left])
			objectList->at(selectedObject % objectList->size())->worldProperties.orientation *= glm::toMat4(glm::angleAxis(-rotationSpeed * float(deltaTime), rotationAxes[axis]));
		else if (directionKeys[DKEY::Right])
			objectList->at(selectedObject % objectList->size())->worldProperties.orientation *= glm::toMat4(glm::angleAxis(rotationSpeed * float(deltaTime), rotationAxes[axis]));
	}

	if(option == 5)
	{
		if (directionKeys[DKEY::Left])
			objectList->at(selectedObject % objectList->size())->worldProperties.scale -= scaleSpeed * float(deltaTime);
		else if (directionKeys[DKEY::Right])
			objectList->at(selectedObject % objectList->size())->worldProperties.scale += scaleSpeed * float(deltaTime);
	}
}

void LevelEditor::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	if(key == KEY::KEY_x || key == KEY::KEY_X)
		axis = 0;
	if(key == KEY::KEY_y || key == KEY::KEY_Y)
		axis = 1;
	if(key == KEY::KEY_z || key == KEY::KEY_Z)
		axis = 2;

	if(key == KEY::KEY_1)
		option = 1;
	if(key == KEY::KEY_2)
		option = 2;
	if(key == KEY::KEY_3)
		option = 3;
	if(key == KEY::KEY_4)
		option = 4;
	if(key == KEY::KEY_5)
		option = 5;
	if(key == KEY::KEY_6)
		option = 6;

	if(key == KEY::KEY_7)
		Save(*objectList, fileSelect);

	if(option == 1)
	{
		if(key == GLUT_KEY_LEFT)
			selectedObject--;
		else if(key == GLUT_KEY_RIGHT)
			selectedObject++;
	}
	else if(option == 6)
	{
		if(key == GLUT_KEY_LEFT)
			fileSelect--;
		else if(key == GLUT_KEY_RIGHT)
			fileSelect++;
	}
}

void LevelEditor::PrintOuts(int winw, int winh)
{
	int numUIEntries = 10;

	std::stringstream ss;
	ss.str(std::string()); // clear
	ss << "LEVEL EDITOR";
	drawText(winw-(strlen(ss.str().c_str())*10), numUIEntries*20, ss.str().c_str());

	if(objectList->size() > 0)
	{
		ss.str(std::string()); // clear
		ss << "|1| objectList: [" << selectedObject % objectList->size() << "] " << objectList->at(selectedObject % objectList->size())->GetFileName();
		drawText(winw-(strlen(ss.str().c_str())*10), (numUIEntries-1)*20, ss.str().c_str());
	
		ss.str(std::string()); // clear
		ss << "pos: (x: " << objectList->operator[](selectedObject % objectList->size())->worldProperties.translation.x 
			<< ", y: " << objectList->operator[](selectedObject % objectList->size())->worldProperties.translation.y
			<< ", z: " << objectList->operator[](selectedObject % objectList->size())->worldProperties.translation.z;
		drawText(winw-(strlen(ss.str().c_str())*10), (numUIEntries-2)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|2| XY translation";
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-3)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|3| XZ translation";
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-4)*20, ss.str().c_str());

		ss.str(std::string()); // clear
	
		glm::vec3 euler = objectList->operator[](selectedObject % objectList->size())->GetEulerAngles();
		ss << "|4| rot: (x: " << euler.x 
			<< ", y: " << euler.y
			<< ", z: " << euler.z;
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-5)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|5| Scale = " << objectList->operator[](selectedObject % objectList->size())->worldProperties.scale.x;
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-6)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|6| Level file select - " << fileSelect;
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-7)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|7| Save file";
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-8)*20, ss.str().c_str());

		ss.str(std::string()); // clear
		ss << "|8| Load file";
		drawText(winw-(strlen(ss.str().c_str())*10),(numUIEntries-9)*20, ss.str().c_str());
	}
}