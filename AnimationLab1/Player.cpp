#include "Player.h"
#include "Keys.h"

Player::Player(vector<Model*> &objectList, Camera* camera, Model* model)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	this->camera = camera;
	camera->SetTarget(model->worldProperties.translation);

	//xzSpeed = 0;
	//lookAngle / rotation

	speedScalar = .005f;

	oldForward = camera->viewProperties.forward;
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

void Player::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	//Animation one shots

	if(key == KEY::KEY_k)
		model->GetSkeleton()->SetAnimationImmediate(0);

	if(key == KEY::KEY_j)
		model->GetSkeleton()->SetAnimationGradual(1, 50);
}

void Player::Update(double deltaTime)
{

}

void Player::Move(double deltaTime)
{
	//TODO - make better
	//theMovementDirection = Quaternion.Euler(0, lookAngle, 0) * new Vector3(horizontalInput, 0, verticalInput);
    //theMovementDirection = theMovementDirection.normalized;

	//theMovementOffset = theMovementDirection * movementSpeed;
    //transform.LookAt(transform.position + theMovementOffset);

	glm::vec3 forwardXZ = camera->viewProperties.forward;
	forwardXZ.y = 0;
	forwardXZ = glm::normalize(forwardXZ);

	model->worldProperties.translation += forwardXZ * float(deltaTime) * speedScalar;
	camera->SetTarget(model->worldProperties.translation);

	double cosAngle = glm::dot(forwardXZ, oldForward);
	float turnAngle = glm::degrees(glm::acos(cosAngle));

	if(cosAngle < 0.9999f) 
	{
		glm::vec3 rotationAxis = glm::normalize(glm::cross(oldForward, forwardXZ));

		if(rotationAxis.y < 0)
			turnAngle = -turnAngle;

		model->worldProperties.orientation *= glm::rotate(glm::mat4(1), turnAngle, glm::vec3(0,0,1));
	}

	oldForward = forwardXZ;
}

