#include "NPC.h"
#include "Keys.h"

NPC::NPC(vector<Model*> &objectList, Model* model)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	LoadAnimation("Animations/fight.dae");
	LoadAnimation("Models/sora.dae"); 
	LoadAnimation("Animations/twirl.dae"); 

	skeleton = model->GetSkeleton();

	state = State::idle;
	skeleton->AddToAnimationQueue(0);
}

void NPC::Update(double deltaTime)
{
	//if player in radius
	//{
	//	playerInRadius = true;
	//
	//	if haveAcknoledged == false
	//		SetState(wave);
	//		haveAckniledged = true
	//
	//
	//if player leave radius
	//
	//
	//
	//
		//if 
}

void NPC::SetState(State newState)
{
	if(state != newState)
	{
		//if set state wave
		//	play one shot

		state = newState;
	}
}
