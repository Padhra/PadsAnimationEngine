#pragma once

#include "Model.h"
#include "Camera.h"

#include "Common.h"



class Player 
{
	private:
		
		//float xzSpeed;
		//float lookAngle;

		glm::vec3 oldForward;
		float speedScalar;

		Camera* camera;

	public:

		Model *model;

		Player(vector<Model*> &objectList, Camera* camera, Model* model);
		~Player(){};

		void Update(double deltaTime);

		void Move(double deltaTime);
		void LoadAnimation(const char* fileName) { model->GetSkeleton()->LoadAnimation(fileName); }

		void ProcessKeyboardContinuous(bool* keyStates, double deltaTime);
		void ProcessKeyboardOnce(unsigned char key, int x, int y);
};