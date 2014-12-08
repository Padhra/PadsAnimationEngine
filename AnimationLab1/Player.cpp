#include "Player.h"
#include "Keys.h"
#include <iomanip>

Player::Player(vector<Model*> &objectList, Camera* camera, Model* model)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	this->camera = camera;
	camera->SetTarget(model->worldProperties.translation);

	//xzSpeed = 0;
	lookAngle = 0;

	speedScalar = .005f;

	glm::vec3 forwardXZ = camera->viewProperties.forward;
	forwardXZ.y = 0;
	oldForward = glm::normalize(forwardXZ);

	LoadAnimation("Animations/fight.dae");
	LoadAnimation("Models/sora.dae"); 
	LoadAnimation("Animations/twirl.dae"); 

	skeleton = model->GetSkeleton();

	state = State::idle;
	skeleton->AddToAnimationQueue(0);
}

void Player::ProcessKeyboardContinuous(bool* keyStates, double deltaTime)
{
	if(camera->mode == CameraMode::tp)
	{
		if(keyStates[KEY::KEY_w] || keyStates[KEY::KEY_W])
		{
			Move(deltaTime);
			SetState(State::run);
		}
		else if(state < State::twirl) //i.e. not a oneshot
		{
			SetState(State::idle);
		}
	}	
}

void Player::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	//Animation one shots
	if(key == KEY::KEY_r)
	{
		if(state != State::run)
		{
			SetState(State::twirl);
		}
	}
}

void Player::Update(double deltaTime)
{
	if(skeleton->animationController.isIdle)
	{
		SetState(State::idle);
	}
}

void Player::SetState(State newState)
{
	if(state != newState)
	{
		if(newState == State::run)
			skeleton->AddToAnimationQueue(State::run, true, 0.05, TransitionType::Frozen);
		else if(newState == State::idle)
			skeleton->AddToAnimationQueue(State::idle, true, 1, TransitionType::Frozen);
		else if(newState == State::twirl)
			skeleton->AddToAnimationQueue(State::twirl, false, 1, TransitionType::Frozen);

		state = newState;
	}
}

void Player::Move(double deltaTime)
{
	glm::vec3 forwardXZ = camera->viewProperties.forward;
	forwardXZ.y = 0;
	forwardXZ = glm::normalize(forwardXZ);
	model->worldProperties.translation += forwardXZ * float(deltaTime) * speedScalar;
	
	camera->SetTarget(model->worldProperties.translation);

	model->worldProperties.orientation = glm::toMat4(glm::inverse(camera->viewProperties.rotation));
}

void Player::PrintOuts(int winw, int winh)
{
	//PRINT PLAYER

	std::stringstream ss;
	ss << "player.pos: (" << std::fixed << std::setprecision(PRECISION) << model->worldProperties.translation.x << ", " << model->worldProperties.translation.y 
		<< ", " << model->worldProperties.translation.z << ")";
	drawText(20,winh-100, ss.str().c_str());

	glm::vec3 euler = glm::eulerAngles(glm::toQuat(model->worldProperties.orientation));
	ss.str(std::string()); // clear
	ss << "player.rot: (" << std::fixed << std::setprecision(PRECISION) << euler.x << ", " << euler.y << ", " << euler.z << ")";
	drawText(20, winh-120, ss.str().c_str());

	ss.str(std::string()); // clear
	glm::vec3 forward = model->GetForward();
	ss << "player.forward: (" << std::fixed << std::setprecision(PRECISION) << forward.x << ", " << forward.y << ", " << forward.z << ")";
	drawText(20, winh-140, ss.str().c_str());
	
	ss.str(std::string()); // clear
	ss << "Current state: ";
	if(state == State::idle)
		ss << "Idle";
	else if(state == State::run)
		ss << "Run";
	else if(state == State::twirl)
		ss << "One-shot";
	drawText(20, winh-160, ss.str().c_str());
}
