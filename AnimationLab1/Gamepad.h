#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

#include "glm\glm.hpp"

class Gamepad
{
	private:
	int cId;
	XINPUT_STATE state;
 
	float deadzoneX;
	float deadzoneY;
 
	public:
	Gamepad() : deadzoneX(0.05f), deadzoneY(0.02f) {}
	Gamepad(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY) {}
 
	float leftStickX;
	float leftStickY;
	float rightStickX;
	float rightStickY;
	float leftTrigger;
	float rightTrigger;
 
	int  GetPort();
	XINPUT_GAMEPAD *GetState();
	bool CheckConnection();
	bool Refresh();
	bool IsPressed(WORD);
};
 
/*#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

class Gamepad
{
	private:
		XINPUT_STATE _controllerState;
		int _controllerNum;

	public:
		Gamepad(int playerNumber);
		
		XINPUT_STATE GetState();
		bool IsConnected();
};*/