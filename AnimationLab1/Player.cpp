#include "Player.h"
#include "Keys.h"
#include <iomanip>

Player::Player(vector<Model*> &objectList, Camera* camera, Gamepad* gamepad, Model* model)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	this->camera = camera;
	camera->SetTarget(model->worldProperties.translation);

	this->gamepad = gamepad;

	//xzSpeed = 0;

	speedScalar = .005f;

	LoadAnimation("Animations/sora_idle_accad_female_look.dae");
	LoadAnimation("Animations/sora_brisk_walk.dae"); 
	LoadAnimation("Animations/sora_punch_cmu_02_05.dae");

	skeleton = model->GetSkeleton();

	state = State::idle;
	skeleton->AddToAnimationQueue(0);
}

void Player::ProcessKeyboardContinuous(bool* keyStates, double deltaTime)
{
	if(camera->mode == CameraMode::tp)
	{
		float vertical = 0.0f;
		float horizontal = 0.0f;

		if(Player::IsAnyDirectionKeyDown(keyStates))
		{
			if(keyStates[KEY::KEY_w] || keyStates[KEY::KEY_W])
				vertical++;	
			if(keyStates[KEY::KEY_a] || keyStates[KEY::KEY_A])
				horizontal--;	
			if(keyStates[KEY::KEY_s] || keyStates[KEY::KEY_S])
				vertical--;	
			if(keyStates[KEY::KEY_d] || keyStates[KEY::KEY_D])
				horizontal++;	

			Move(deltaTime, -horizontal, vertical);
			
		}
		else if(state < State::attack) //i.e. not a oneshot
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
		
	}
}

void Player::Update(double deltaTime)
{
	if(skeleton->animationController.isIdle)
		SetState(State::idle);

	if(camera->mode == CameraMode::tp)
	{
		camera->SetTarget(model->worldProperties.translation);

		/*gamepad->Refresh();
		if(gamepad->leftStickX != 0 || gamepad->leftStickY != 0)
			Move(deltaTime, -gamepad->leftStickX, gamepad->leftStickY);*/
	}

	//lookAngle += deltaTime * .01;
	//model->worldProperties.orientation = glm::toMat4(glm::inverse(camera->viewProperties.rotation)) * glm::rotate(glm::mat4(1), -lookAngle, glm::vec3(0,1,0));
}

void Player::SetState(State newState)
{
	if(state != newState)
	{
		if(newState == State::run)
			skeleton->AddToAnimationQueue(State::run, true, 0.05, TransitionType::Smooth);
		else if(newState == State::idle)
			skeleton->AddToAnimationQueue(State::idle, true, 0.3, TransitionType::Smooth);
		else if(newState == State::attack)
			skeleton->AddToAnimationQueue(State::attack, false, 0.2, TransitionType::Smooth);

		state = newState;
	}
}

void Player::Move(double deltaTime, float horizontal, float vertical)
{
	//horizonal -1 <-> 1  X
	//vertical -1 <-> 1  Y

	SetState(State::run);

	glm::vec3 forwardXZ = camera->viewProperties.forward;
	forwardXZ.y = 0;
	forwardXZ = glm::normalize(forwardXZ);

	glm::vec3 moveDir = glm::inverse(camera->viewProperties.XZrotation) * glm::vec3(horizontal, 0, vertical);
	moveDir = glm::normalize(moveDir);

	glm::vec3 offset = moveDir * float(deltaTime) * speedScalar;

	model->worldProperties.translation += offset;

	model->worldProperties.orientation = glm::toMat4(glm::inverse(camera->viewProperties.XZrotation));
	model->worldProperties.orientation *= glm::toMat4(glm::quat(forwardXZ, moveDir));
}

void Player::PrintOuts(int winw, int winh)
{
	//PRINT PLAYER

	std::stringstream ss;
	ss << "player.pos: (" << std::fixed << std::setprecision(PRECISION) << model->worldProperties.translation.x << ", " << model->worldProperties.translation.y 
		<< ", " << model->worldProperties.translation.z << ")";
	drawText(20,winh-120, ss.str().c_str());

	glm::vec3 euler = glm::eulerAngles(glm::toQuat(model->worldProperties.orientation));
	ss.str(std::string()); // clear
	ss << "player.rot: (" << std::fixed << std::setprecision(PRECISION) << euler.x << ", " << euler.y << ", " << euler.z << ")";
	drawText(20, winh-140, ss.str().c_str());

	ss.str(std::string()); // clear
	glm::vec3 forward = model->GetForward();
	ss << "player.forward: (" << std::fixed << std::setprecision(PRECISION) << forward.x << ", " << forward.y << ", " << forward.z << ")";
	drawText(20, winh-160, ss.str().c_str());
	
	ss.str(std::string()); // clear
	ss << "Current state: ";
	if(state == State::idle)
		ss << "Idle";
	else if(state == State::run)
		ss << "Run";
	else if(state == State::attack)
		ss << "One-shot";
	drawText(20, winh-200, ss.str().c_str());
}
