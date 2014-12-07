#include "NPC.h"
#include "Keys.h"

NPC::NPC(vector<Model*> &objectList, Model* model, Player* player)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	LoadAnimation("Animations/fight.dae");
	LoadAnimation("Animations/twirl.dae"); 
	LoadAnimation("Models/sora.dae"); 

	skeleton = model->GetSkeleton();

	state = NPCns::State::idle;
	skeleton->AddToAnimationQueue(NPCns::State::idle);

	this->player = player;

	this->threshold = 3;

	haveAcknowledged = false;
	playerInRadius = false;
}

void NPC::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	if(key == KEY::KEY_v)
	{
		if(playerInRadius)
		{
			SetState(NPCns::State::talk);
		}
	}
}

void NPC::Update(double deltaTime)
{
	if(skeleton->animationController.isIdle)
		SetState(NPCns::State::idle);

	if(glm::distance(model->worldProperties.translation, player->model->worldProperties.translation) < threshold)
	{
		playerInRadius = true;

		if(!haveAcknowledged)
		{
			SetState(NPCns::State::wave);
			haveAcknowledged = true;
		}
	}
	else
	{
		playerInRadius = false;
		haveAcknowledged = false;

		SetState(NPCns::State::idle);
	}
}

void NPC::SetState(NPCns::State newState)
{
	if(state != newState)
	{
		if(newState == NPCns::State::wave)
			skeleton->AddToAnimationQueue(NPCns::State::wave, false, 1, TransitionType::Frozen);
		else if(newState == NPCns::State::idle)
			skeleton->AddToAnimationQueue(NPCns::State::idle, true, 1, TransitionType::Frozen);
		else if(newState == NPCns::State::talk)
			skeleton->AddToAnimationQueue(NPCns::State::talk, true, 1, TransitionType::Frozen);

		state = newState;
	}
}
