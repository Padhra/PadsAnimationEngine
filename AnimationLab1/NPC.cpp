#include "NPC.h"
#include "Keys.h"

NPC::NPC(vector<Model*> &objectList, Model* model, Player* player)
{
	this->model = model;
	objectList.push_back(model);

	model->serialise = false;

	LoadAnimation("Animations/don_walk.dae"); //idle i.e. patrolling
	LoadAnimation("Animations/don_wave.dae"); 
	LoadAnimation("Animations/don_angry_talk_cmu_79_74.dae"); 
	LoadAnimation("Animations/don_happy_talk_cmu_happy.dae");

	skeleton = model->GetSkeleton();

	state = NPCns::State::idle;
	skeleton->AddToAnimationQueue(NPCns::State::idle);

	this->player = player;

	this->threshold = 3.5f;

	haveAcknowledged = false;
	playerInRadius = false;

	dialogue = "";
	questComplete = false;
}

void NPC::ProcessKeyboardOnce(unsigned char key, int x, int y)
{
	if(key == KEY::KEY_f || key == KEY::KEY_F)
	{
		if(playerInRadius)
		{
			if(!questComplete)
				SetState(NPCns::State::talk);
			else
				SetState(NPCns::State::celebrate);
		}
	}
}

void NPC::Update(double deltaTime)
{
	if(patrolling)
	{
		patrol.Update(deltaTime);

		model->worldProperties.translation = patrol.GetPositionXZ();

		glm::vec3 v0 = glm::normalize(model->GetForward());
		v0.y = 0;
		glm::vec3 v1 = glm::normalize(patrol.GetApproximateForward());
		v1.y = 0;

		glm::quat q = glm::quat(v0,v1);

		model->worldProperties.orientation *= glm::toMat4(q); //glm::lookAt(donald->model->worldProperties.translation,
			//donald->model->worldProperties.translation + glm::normalize(donaldSpline.GetApproximateForward()), glm::vec3(0,1,0));
	}

	if(skeleton->animationController.isIdle)
	{
		SetState(NPCns::State::idle);
	}

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
		{
			skeleton->AddToAnimationQueue(NPCns::State::wave, false, 1, TransitionType::Smooth);
			dialogue = "Donald: Hey Sora, come over here!";
			patrolling = false;
		}
		else if(newState == NPCns::State::idle)
		{
			dialogue = "";
			skeleton->AddToAnimationQueue(NPCns::State::idle, true, 0.1f, TransitionType::Smooth);
			patrolling = true;
		}
		else if(newState == NPCns::State::talk)
		{
			skeleton->AddToAnimationQueue(NPCns::State::talk, false, 1, TransitionType::Smooth);
			dialogue = "Donald: Sora, this is serious! *quack* These Cactuars are running rampant around Destiny island! Please help!";
			patrolling = false;
		}
		else if(newState == NPCns::State::celebrate)
		{
			skeleton->AddToAnimationQueue(NPCns::State::celebrate, false, 1, TransitionType::Smooth);
			dialogue = "Donald: Whoa! You did it! *quack*";
			patrolling = false;
		}

		state = newState;
	}
}
