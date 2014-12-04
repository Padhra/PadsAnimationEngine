#include "Player.h"
#include "Keys.h"

Player::Player(vector<Model*> &objectList, Camera* camera, Model* model)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	this->camera = camera;
	camera->SetTarget(model->worldProperties.translation);

	direction = glm::vec3(0,0,1);
	xzSpeed = 0;

	speedScalar = .005f;

	rotation = 0;

	oldForward = camera->viewProperties.forward;

	//Move(0.333f);
}

void Player::ProcessKeyboardContinuous(bool* keyStates, double deltaTime)
{
	if(!camera->flycam)
	{
		if(keyStates[KEY::KEY_w] || keyStates[KEY::KEY_W])
		{
			Move(deltaTime);
		}
	}	
}

void Player::Move(double deltaTime)
{
	glm::vec3 forwardXZ = camera->viewProperties.forward;
	forwardXZ.y = 0;
	forwardXZ = glm::normalize(forwardXZ);

	model->worldProperties.translation += forwardXZ * float(deltaTime) * speedScalar;
	camera->SetTarget(model->worldProperties.translation);

	double cosAngle = glm::dot(forwardXZ, oldForward);
	float turnAngle = glm::degrees(glm::acos(cosAngle));

	if(cosAngle < 0.9999f) // IF THE DOT PRODUCT RETURNS 1.0, I DON'T NEED TO ROTATE AS IT IS 0 DEGREES
	{
		glm::vec3 rotationAxis = glm::normalize(glm::cross(oldForward, forwardXZ));

		if(rotationAxis.y < 0)
			turnAngle = -turnAngle;

		//rotation += turnAngle;

		//glm::quat correctBlender = glm::quat();
		//correctBlender *= glm::angleAxis(-90.0f, glm::vec3(1,0,0));

		//rotation++;
		model->worldProperties.orientation *= glm::rotate(glm::mat4(1), turnAngle, glm::vec3(0,0,1));
		//model->worldProperties.orientation = glm::lookAt(model->worldProperties.translation, model->worldProperties.translation + forwardXZ, glm::vec3(0,1,0)) * glm::toMat4(correctBlender);
	}

	oldForward = forwardXZ;
}

void Player::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	//Animation switcher
}