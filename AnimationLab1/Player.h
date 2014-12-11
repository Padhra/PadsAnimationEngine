#pragma once

#include "Model.h"
#include "Camera.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

#include "Gamepad.h"

#include "Common.h"

#include "glm\glm.hpp"


enum State { idle = 0, run, attack };

class Player 
{
	private:
		
		//float xzSpeed;
		//float lookAngle;

		float speedScalar;

		int state;

		Camera* camera;
		Gamepad* gamepad;

		Skeleton* skeleton;

		vector<Model*> *objectList;

	public:

		Model *model;

		Player(vector<Model*> &objectList, Camera* camera, Gamepad* gamepad, Model* model);
		~Player(){};

		void Update(double deltaTime);

		void SetState(State newState);
		int GetState() { return state; }

		void Move(double deltaTime, float horizontal, float vertical);
		void LoadAnimation(const char* fileName) { model->GetSkeleton()->LoadAnimation(fileName); }

		void ProcessKeyboardContinuous(bool* keyStates, double deltaTime);
		void ProcessKeyboardOnce(unsigned char key, int x, int y);
		void ProcessMouseButton(int button, int state, int x, int y)
		{
			switch(button) {
				case GLUT_LEFT_BUTTON:
					if(state != State::run)
					{
						SetState(State::attack);
					}
					break;
			}
		}

		void PrintOuts(int winw, int winh);

		static bool IsAnyDirectionKeyDown(bool* keyStates)
		{
			return (keyStates[KEY::KEY_w] || keyStates[KEY::KEY_W]
			|| keyStates[KEY::KEY_s] || keyStates[KEY::KEY_S]
			|| keyStates[KEY::KEY_a] || keyStates[KEY::KEY_A]
			|| keyStates[KEY::KEY_d] || keyStates[KEY::KEY_D]
			);
		}
};