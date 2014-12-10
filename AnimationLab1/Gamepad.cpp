#include "Gamepad.h"
#include "glm\glm.hpp"

int Gamepad::GetPort()
{
	return cId + 1;
}
 
XINPUT_GAMEPAD *Gamepad::GetState()
{
	return &state.Gamepad;
}
 
bool Gamepad::CheckConnection()
{
	int controllerId = -1;
 
	for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
 
		if (XInputGetState(i, &state) == ERROR_SUCCESS)
			controllerId = i;
	}
 
	cId = controllerId;
 
	return controllerId != -1;
}
 
// Returns false if the controller has been disconnected
bool Gamepad::Refresh()
{
	if (cId == -1)
		CheckConnection();
 
	if (cId != -1)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		
		if (XInputGetState(cId, &state) != ERROR_SUCCESS)
		{
			cId = -1;
			return false;
		}
 
		float normLX = glm::max((float)-1, (float) state.Gamepad.sThumbLX / 32767);
		float normLY = glm::max((float)-1, (float) state.Gamepad.sThumbLY / 32767);
 
		leftStickX = (abs(normLX) < deadzoneX ? 0 : (abs(normLX) - deadzoneX) * (normLX / abs(normLX)));
		leftStickY = (abs(normLY) < deadzoneY ? 0 : (abs(normLY) - deadzoneY) * (normLY / abs(normLY)));
 
		if (deadzoneX > 0) leftStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) leftStickY *= 1 / (1 - deadzoneY);
 
		float normRX = glm::max((float)-1, (float) state.Gamepad.sThumbRX / 32767);
		float normRY = glm::max((float)-1, (float) state.Gamepad.sThumbRY / 32767);
 
		rightStickX = (abs(normRX) < deadzoneX ? 0 : (abs(normRX) - deadzoneX) * (normRX / abs(normRX)));
		rightStickY = (abs(normRY) < deadzoneY ? 0 : (abs(normRY) - deadzoneY) * (normRY / abs(normRY)));
 
		if (deadzoneX > 0) rightStickX *= 1 / (1 - deadzoneX);
		if (deadzoneY > 0) rightStickY *= 1 / (1 - deadzoneY);
 
		leftTrigger = (float) state.Gamepad.bLeftTrigger / 255;
		rightTrigger = (float) state.Gamepad.bRightTrigger / 255;
 
		return true;
	}

	return false;
}
 
bool Gamepad::IsPressed(WORD button)
{
	return (state.Gamepad.wButtons & button) != 0;
}


/*Gamepad::Gamepad(int playerNumber)
{
	// Set the Controller Number
	_controllerNum = playerNumber - 1;
}

XINPUT_STATE Gamepad::GetState()
{
	// Zeroise the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	XInputGetState(_controllerNum, &_controllerState);


	//float normLX = glm::max(float(-1), (float) _controllerState.Gamepad.sThumbLX / 32767);
	//float normLY = glm::max(float(-1), (float) _controllerState.Gamepad.sThumbLY / 32767);

	return _controllerState;
}

bool Gamepad::IsConnected()
{
	// Zeroise the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(_controllerNum, &_controllerState);

	if(Result == ERROR_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}*/