#pragma once

#include "Model.h"
#include "Camera.h"

#include "Common.h"


enum State { idle = 0, run, twirl };

class Player 
{
	private:
		
		//float xzSpeed;
		//float lookAngle;
		glm::vec3 oldForward;
		float speedScalar;

		int state;

		Camera* camera;

		Skeleton* skeleton;

		float lookAngle;

	public:

		Model *model;

		Player(vector<Model*> &objectList, Camera* camera, Model* model);
		~Player(){};

		void Update(double deltaTime);

		void SetState(State newState);

		void Move(double deltaTime);
		void LoadAnimation(const char* fileName) { model->GetSkeleton()->LoadAnimation(fileName); }

		void ProcessKeyboardContinuous(bool* keyStates, double deltaTime);
		void ProcessKeyboardOnce(unsigned char key, int x, int y);

		void PrintOuts(int winw, int winh);
};