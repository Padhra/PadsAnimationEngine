#pragma once

#include "Model.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Player 
{
	private:
		
		float xzSpeed;
		float speedScalar;

		float rotation;

		glm::vec3 direction;
		glm::vec3 oldForward;

		Camera* camera;

	public:

		Model *model;

		Player(vector<Model*> &objectList, Camera* camera, Model* model);
		~Player(){};

		void Move(double deltaTime);
		void LoadAnimation(const char* fileName) { model->GetSkeleton()->LoadAnimation(fileName); }

		void ProcessKeyboardContinuous(bool* keyStates, double deltaTime);
		void ProcessKeyboardOnce(unsigned char key, int x, int y);
};