#pragma once

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
};