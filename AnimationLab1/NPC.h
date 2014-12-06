#pragma once

#include "Model.h"
#include "Common.h"

enum State { idle = 0, wave };

class NPC //TODO inherist from Character
{
	private:

		int state;
		Skeleton* skeleton;

	public:

		Model *model;

		NPC(vector<Model*> &objectList, Model* model);
		~NPC(){};

		void Update(double deltaTime);
		void SetState(State newState);

		void LoadAnimation(const char* fileName) { model->GetSkeleton()->LoadAnimation(fileName); }

};